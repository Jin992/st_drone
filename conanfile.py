import json
import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class DroneConan(ConanFile):
    name = "drone"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

        self._write_user_presets()

    def _write_user_presets(self):
        """Write CMakeUserPresets.json at source root so VS Code CMake Tools discovers Conan presets."""
        rel_path = os.path.relpath(self.generators_folder, self.source_folder).replace("\\", "/")
        include_entry = f"{rel_path}/CMakePresets.json"

        user_presets_path = os.path.join(self.source_folder, "CMakeUserPresets.json")
        if os.path.exists(user_presets_path):
            with open(user_presets_path) as f:
                data = json.load(f)
        else:
            data = {"version": 6, "include": []}

        if include_entry not in data.get("include", []):
            data.setdefault("include", []).append(include_entry)

        with open(user_presets_path, "w") as f:
            json.dump(data, f, indent=2)
            f.write("\n")
