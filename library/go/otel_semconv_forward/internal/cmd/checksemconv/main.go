package main

import (
	"flag"
	"log"

	"golang.org/x/tools/go/packages"

	"github.com/yandex/perforator/library/go/otel_semconv_forward/internal/versioncfg"
)

func main() {
	var pkgPath string
	flag.StringVar(&pkgPath, "pkg", "", "semconv package import path")
	flag.Parse()

	if pkgPath == "" {
		resolved, err := versioncfg.ReadSemconvPackage("versions.mk")
		if err != nil {
			log.Fatalf("resolve semconv package: %v", err)
		}
		pkgPath = resolved
	}

	cfg := &packages.Config{
		Mode: packages.NeedName | packages.NeedFiles | packages.NeedImports | packages.NeedDeps,
	}
	pkgs, err := packages.Load(cfg, pkgPath)
	if err != nil {
		log.Fatalf("packages.Load(%q): %v", pkgPath, err)
	}
	if packages.PrintErrors(pkgs) > 0 {
		log.Fatalf("semconv package %q is not available with current module versions", pkgPath)
	}
	if len(pkgs) != 1 || len(pkgs[0].GoFiles) == 0 {
		log.Fatalf("semconv package %q resolved, but no Go files were loaded", pkgPath)
	}

	log.Printf("OK: %s", pkgPath)
}
