#include <perforator/internal/linguist/jvm/analysis/static/static_analysis.h>

#include <library/cpp/getopt/last_getopt.h>

#include <library/cpp/json/json_value.h>
#include <library/cpp/json/json_writer.h>

#include <contrib/libs/protobuf/src/google/protobuf/text_format.h>

#include <dlfcn.h>

#include <string>

namespace {
struct TDeleter {
    void operator()(void* handle) {
        int err = dlclose(handle);
        if (err != 0) {
            std::string msg = dlerror();
            Cerr << "failed to close libjvm.so: " << msg << Endl;
        }
    }
};

NPerforator::NLinguist::NJvm::TJvmAnalysis DumpDynamic(std::string libjvmPath) {
    using namespace NPerforator::NLinguist::NJvm;
    void* rawHandle = dlopen(libjvmPath.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (rawHandle == nullptr) {
        char* msg = dlerror();
        throw yexception() << "failed to load libjvm.so: " << msg;
    }
    std::unique_ptr<void, TDeleter> handle(rawHandle);

    auto GetSym = [&](const std::string& sym) {
        void* addr = dlsym(handle.get(), sym.c_str());
        if (addr == nullptr) {
            char* msg = dlerror();
            throw yexception() << "failed to load symbol " << sym << ": " << msg;
        }
        return addr;
    };

    TVMStructsAddresses addresses;
    addresses.StructsAddress = GetSym(std::string{TVMStructsAddresses::StructsAddressSym});
    addresses.TypesAddress = GetSym(std::string{TVMStructsAddresses::TypesAddressSym});

    return NPerforator::NLinguist::NJvm::ProcessDynamicLinkedJVM(addresses);
}

}

int main(int argc, char** argv) {
    using namespace std::literals;
    using namespace NPerforator::NLinguist::NJvm;

    NLastGetopt::TOpts opts;
    opts.AddLongOption("mode").Required().Choices({"for-normal", "for-minimal"});
    opts.AddLongOption("jvm-path").Help("Path to libjvm.so");


    NLastGetopt::TOptsParseResult parsed{&opts, argc, argv};

    TJvmAnalysis spec = NPerforator::NLinguist::NJvm::ProcessJVMHeaders();
    if ("for-minimal"s == parsed.Get("mode")) {
        const char* path = parsed.Get("jvm-path");
        if (path == nullptr) {
            throw yexception() << "--jvm-path is required when using for-minimal mode";
        }
        spec.Cheatsheet.MergeFrom(DumpDynamic(path).Cheatsheet);
    } else if ("for-normal"s != parsed.Get("mode")) {
        Y_ABORT();
    }

    TProtoStringType repr;
    google::protobuf::TextFormat::PrintToString(spec.Cheatsheet, &repr);

    Cout << repr << Endl;
}
