import json
from dataclasses import dataclass

from devtools.frontend_build_platform.libraries.logging import timeit

from .base_builder import BaseLegacyBuilder
from ..models import BuildError, CommonBuildersOptions
from ..utils import bundle_fs_entries, popen


@dataclass
class PackageBuilderOptions(CommonBuildersOptions):
    pass


class PackageBuilder(BaseLegacyBuilder):
    @timeit
    def _get_pack_files(self) -> list[str]:
        """Run pnpm pack --json and return list of files to include in archive"""
        args = [
            self.options.nodejs_bin,
            self.options.pm_script,
            'pack',
            '--json',
            '--dry-run',
            '--config.ignoreScripts=true',
        ]
        return_code, stdout, stderr = popen(args, env=self._get_envs(), cwd=self.options.bindir)

        if return_code != 0:
            raise BuildError(self.options.command, return_code, stdout, stderr)

        # Parse JSON output
        pack_data = json.loads(stdout)

        # Extract file paths from json['files'][]['path']
        # todo: всем прописать files в package.json
        return [
            file_entry['path']
            for file_entry in pack_data['files']
            if not file_entry['path'].startswith('__tarball__') and file_entry['path'] != 'pnpm-workspace.yaml'
        ]

    @timeit
    def bundle(self):
        """Create output archive from files listed by pnpm pack"""
        file_paths = self._get_pack_files()
        # if self.options.with_after_build and self.options.after_build_outdir:
        #     file_paths.append(self.options.after_build_outdir)
        bundle_fs_entries(file_paths, self.options.bindir, self.options.output_file)

    def _build(self):
        pass
