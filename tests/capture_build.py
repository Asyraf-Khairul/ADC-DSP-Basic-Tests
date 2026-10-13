"""Capture local source, build settings, binaries and hashes without changing Git."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument("destination")
parser.add_argument("--build", default="build")
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parents[1]
destination = root / args.destination
destination.mkdir(parents=True, exist_ok=False)
def git(*arguments):
    return subprocess.check_output(["git", *arguments], cwd=root)
files = git("ls-files", "-z").decode().split("\0")
files += git("ls-files", "--others", "--exclude-standard", "-z").decode().split("\0")
files = sorted(set(f for f in files if f and not f.startswith(".vscode/")))
source_hashes = {}
with zipfile.ZipFile(destination / "source.zip", "w", zipfile.ZIP_DEFLATED) as archive:
    for name in files:
        path = root / name
        if path.is_file():
            archive.write(path, name)
            source_hashes[name] = hashlib.sha256(path.read_bytes()).hexdigest()
(destination / "working-tree.patch").write_bytes(git("diff", "--binary", "HEAD"))
build = root / args.build
binary_hashes = {}
for name in ("CMakeCache.txt", "build.ninja", "adc_dsp_standalone.uf2", "adc_dsp_standalone.elf"):
    path = build / name
    if path.exists():
        data = path.read_bytes()
        (destination / name).write_bytes(data)
        binary_hashes[name] = hashlib.sha256(data).hexdigest()
cache = (build / "CMakeCache.txt").read_text()
settings = [line for line in cache.splitlines() if line.startswith((
    "CMAKE_BUILD_TYPE:", "BENCH_SELF_TEST:", "BENCH_OPERATION_BENCHMARK:", "CMAKE_CXX_FLAGS_RELEASE:",
    "CMAKE_CXX_COMPILER:", "PICO_BOARD:", "PICO_SDK_PATH:", "PICO_TOOLCHAIN_PATH:"))]
metadata = dict(commit=git("rev-parse", "HEAD").decode().strip(),
    branch=git("branch", "--show-current").decode().strip(),
    status=git("status", "--short").decode(), build_settings=settings,
    source_sha256=source_hashes, artifact_sha256=binary_hashes)
(destination / "manifest.json").write_text(json.dumps(metadata, indent=2))
print(json.dumps(dict(destination=str(destination), commit=metadata["commit"],
    build_settings=settings, artifact_sha256=binary_hashes), indent=2))
