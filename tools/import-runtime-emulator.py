from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


def digest(path: Path) -> str:
    result = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Import a legally owned LavaXOS runtime into test NAND"
    )
    parser.add_argument("source", type=Path)
    parser.add_argument("--emulator-root", type=Path, required=True)
    parser.add_argument("--nand", type=Path, required=True)
    parser.add_argument("--target", default="/LavaXOS")
    parser.add_argument(
        "--boot-program",
        type=Path,
        help="isolated smoke test: replace System/Shell.sys with this .lav",
    )
    args = parser.parse_args()

    source = args.source.resolve()
    emulator_root = args.emulator_root.resolve()
    nand = args.nand.resolve()
    shell = source / "System" / "Shell.sys"
    if not source.is_dir() or not shell.is_file():
        raise SystemExit("Source must contain System/Shell.sys")
    if shell.read_bytes()[:4] != b"LAV\x12":
        raise SystemExit("System/Shell.sys is not a LavaX bytecode file")
    if not nand.is_file():
        raise SystemExit(f"NAND does not exist: {nand}")

    selected = [
        path for path in source.rglob("*")
        if path.is_file() and path.suffix.lower() != ".nds"
    ]
    expected = {
        path.relative_to(source).as_posix(): {
            "size": path.stat().st_size,
            "sha256": digest(path),
        }
        for path in selected
    }
    boot_program = args.boot_program.resolve() if args.boot_program else None
    if boot_program is not None:
        if not boot_program.is_file() or boot_program.read_bytes()[:4] != b"LAV\x12":
            raise SystemExit("--boot-program must be a LavaX bytecode file")
        expected["System/Shell.sys"] = {
            "size": boot_program.stat().st_size,
            "sha256": digest(boot_program),
        }
    sys.path.insert(0, str(emulator_root))
    from emu.qemu.nand_fs import mutate_nand_files, replace_fat_file

    target = "/" + args.target.strip("/")

    def operation(fs):
        fs.makedirs(target, recreate=True)
        for path in selected:
            relative = path.relative_to(source).as_posix()
            destination = f"{target}/{relative}"
            fs.makedirs(destination.rsplit("/", 1)[0], recreate=True)
            with path.open("rb") as stream:
                replace_fat_file(fs, destination, stream)
        if boot_program is not None:
            with boot_program.open("rb") as stream:
                replace_fat_file(fs, f"{target}/System/Shell.sys", stream)

    def validator(fs):
        for relative, metadata in expected.items():
            destination = f"{target}/{relative}"
            if not fs.isfile(destination):
                raise ValueError(f"Missing imported file: {destination}")
            if fs.getsize(destination) != metadata["size"]:
                raise ValueError(f"Imported size mismatch: {destination}")

    mutate_nand_files(nand, operation, validator=validator)
    print(json.dumps({
        "ok": True,
        "source": str(source),
        "nand": str(nand),
        "target": target,
        "files": len(selected),
        "bytes": sum(item["size"] for item in expected.values()),
        "excluded_nds": len(list(source.rglob("*.nds"))),
        "boot_program": str(boot_program) if boot_program else None,
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
