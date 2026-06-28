#!/usr/bin/env python3
"""Round-trip correctness test for the multithreaded inverter.

Inverting an image is its own inverse at the pixel level, so:

    inv(inv(inv(x))) == inv(x)

stb's PNG encoder is deterministic for identical pixel data, therefore the PNG
produced by ONE inversion must be byte-for-byte identical to the PNG produced by
THREE inversions. We build a self-contained sample, run the program three times
and compare once.png with thrice.png.

Usage:
    verify_roundtrip.py <path-to-inverted_photo-exe> [tmpdir]
"""
import filecmp
import os
import subprocess
import sys
import tempfile


def make_sample(path: str, size: int = 64) -> None:
    width = height = size
    with open(path, "wb") as f:
        f.write(b"P6\n%d %d\n255\n" % (width, height))
        row = bytearray(width * 3)
        for y in range(height):
            for x in range(width):
                row[x * 3 + 0] = (x * 3) % 256
                row[x * 3 + 1] = (y * 5) % 256
                row[x * 3 + 2] = (x * y) % 256
            f.write(row)


def run(exe: str, src: str, dst: str) -> None:
    subprocess.run(
        [exe, src, dst, "--threads", "4", "--quiet", "--log", dst + ".log"],
        check=True,
    )


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: verify_roundtrip.py <exe> [tmpdir]", file=sys.stderr)
        return 2

    exe = sys.argv[1]
    tmp = sys.argv[2] if len(sys.argv) > 2 else tempfile.mkdtemp(prefix="invrt_")
    os.makedirs(tmp, exist_ok=True)

    src = os.path.join(tmp, "src.ppm")
    once = os.path.join(tmp, "once.png")
    twice = os.path.join(tmp, "twice.png")
    thrice = os.path.join(tmp, "thrice.png")

    make_sample(src)
    run(exe, src, once)     # once   = inv(src)
    run(exe, once, twice)   # twice  = inv(inv(src)) == src
    run(exe, twice, thrice)  # thrice = inv(src)

    if filecmp.cmp(once, thrice, shallow=False):
        print("PASS: inv(x) == inv(inv(inv(x)))  [once.png == thrice.png]")
        return 0

    print("FAIL: round-trip mismatch (once.png != thrice.png)", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
