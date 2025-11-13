package main

import (
	"bytes"
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/btf"
)

func generateForSpec(pkg string, prefix string, spec *ebpf.CollectionSpec) (string, error) {
	outBuffer := bytes.NewBuffer(nil)

	f := NewFormatter(outBuffer, spec.Types)

	f.SetPackage(pkg)
	f.SetPrefix(prefix)

	for _, m := range spec.Maps {
		if m.Name == ".rodata" {
			// For some reason, .rodata section gets parsed as a map.
			// But from our PoV it is not a map, so we skip it.
			continue
		}
		f.AddPublicMap(m)
	}
	for _, p := range spec.Programs {
		f.AddProgram(p)
	}

	// Add exported types by BTF_EXPORT
	for iter := spec.Types.Iterate(); iter.Next(); {
		s, ok := iter.Type.(*btf.Struct)
		if !ok {
			continue
		}
		if !strings.HasPrefix(s.Name, "btf_export") {
			continue
		}

		for _, m := range s.Members {
			f.AddPublicType(m.Type)
		}
	}

	err := f.Print()
	if err != nil {
		return "", err
	}

	return outBuffer.String(), nil
}

type stringList []string

// String implements flag.Value
func (i *stringList) String() string {
	return strings.Join(*i, ",")
}

// Set implements flag.Value
func (i *stringList) Set(value string) error {
	*i = append(*i, value)
	return nil
}

func run() error {
	pkg := flag.String("package", "", "Generated package name")
	var paths stringList
	flag.Var(&paths, "elf", "List of paths to the compiled ebpf object files")
	var inconsistencies stringList
	flag.Var(&inconsistencies, "ignore", "List of inconsistencies to ignore")
	prefix := flag.String("prefix", "", "Prefix to add to each user-defined type")
	output := flag.String("output", "", "Path to the generated file")
	flag.Parse()
	if len(paths) == 0 {
		return fmt.Errorf("--elf is required")
	}
	var res string
	var golden *ebpf.CollectionSpec
	checker := consistencyChecker{
		actualInconsistencies:   make(map[string]struct{}),
		expectedInconsistencies: make(map[string]struct{}),
	}
	for _, x := range inconsistencies {
		checker.expectedInconsistencies[x] = struct{}{}
	}

	for idx, path := range paths {
		spec, err := ebpf.LoadCollectionSpec(path)
		if err != nil {
			return fmt.Errorf("failed to parse file %q: %w", path, err)
		}
		res, err = generateForSpec(*pkg, *prefix, spec)
		if err != nil {
			return fmt.Errorf("failed to process file %q: %w", path, err)
		}
		if idx == 0 {
			golden = spec
		} else {
			err := checker.checkBTFConsistency(golden, spec)
			if err != nil {
				return fmt.Errorf("detected potential incompatibility between input files %q and %q: %v", paths[0], paths[idx], err)
			}
		}
	}
	err := checker.finalize()
	if err != nil {
		return err
	}
	return os.WriteFile(*output, []byte(res), 0644)
}

func main() {
	err := run()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %+v\n", err)
		os.Exit(1)
	}
}
