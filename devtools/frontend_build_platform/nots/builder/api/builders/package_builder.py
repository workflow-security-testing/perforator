from dataclasses import dataclass

from devtools.frontend_build_platform.libraries.logging import timeit

from .base_builder import BaseLegacyBuilder
from ..models import CommonBuildersOptions


def touch(file_path: str) -> None:
    with open(file_path, 'w'):
        pass


@dataclass
class PackageBuilderOptions(CommonBuildersOptions):
    pass


class PackageBuilder(BaseLegacyBuilder):
    @timeit
    def bundle(self):
        if self.options.with_after_build and self.options.after_build_outdir:
            return self.bundle_dirs([self.options.after_build_outdir], self.options.bindir, self.options.output_file)
        else:
            touch(self.options.output_file)

    def _build(self):
        pass
