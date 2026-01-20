#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

EXCLUDE_DIRS = {"__pycache__", "build", "icons", "images"}
ALLOWED_EXTS = {".cpp", ".h", ".cu", ".cuh", ".txt", ".md", ".sh"}
MAX_FILE_SIZE = 50000  # Skip files > 50K bytes

def should_include_dir(d):
    return not (d.startswith('.') or d in EXCLUDE_DIRS)

def should_include_file(fname):
    _, ext = os.path.splitext(fname.lower())
    return ext in ALLOWED_EXTS

def pack_dir(root_dir: str, out_path: str):
    root_dir = os.path.abspath(root_dir)
    with open(out_path, "w", encoding="utf-8", newline="\n") as out:
        for dirpath, dirnames, filenames in os.walk(root_dir):
            # Filter directories
            dirnames[:] = [d for d in dirnames if should_include_dir(d)]

            for fname in filenames:
                if not should_include_file(fname):
                    continue

                full_path = os.path.join(dirpath, fname)
                rel_path = os.path.relpath(full_path, root_dir)

                # Check size before reading
                if os.path.getsize(full_path) > MAX_FILE_SIZE:
                    print(f"⚠️ Skipping large file: {rel_path} ({os.path.getsize(full_path)} bytes)")
                    continue

                try:
                    with open(full_path, "r", encoding="utf-8") as f:
                        content = f.read()
                except UnicodeDecodeError:
                    # Skip binary/non-UTF8 files
                    print(f"⚠️ Skipping non-UTF8 file: {rel_path}")
                    continue

                out.write(f"FILE_BEGIN {rel_path}\n")
                out.write(content.rstrip("\n") + "\n")
                out.write("FILE_END\n")

    print(f"✅ JBF-bundle written to: {out_path}")

def main():
    if len(sys.argv) != 3:
        print("Usage:")
        print(f"  python3 {sys.argv[0]} <root_dir> <bundle.txt>")
        print("\nExample:")
        print(f"  python3 {sys.argv[0]} ./chatGPT project_bundle.txt")
        sys.exit(1)

    root_dir = sys.argv[1]
    out_path = sys.argv[2]

    if not os.path.isdir(root_dir):
        print(f"❌ Directory does not exist: {root_dir}")
        sys.exit(1)

    pack_dir(root_dir, out_path)

if __name__ == "__main__":
    main()
