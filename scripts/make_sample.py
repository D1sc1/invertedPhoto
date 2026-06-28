#!/usr/bin/env python3
"""Generate a gradient sample image in binary PPM (P6) format.

PPM is used because it needs no third-party Python libraries, and stb_image
(used by the C++ program) can read it directly.

Usage:
    make_sample.py [output.ppm] [size]
"""
import os
import sys


def main() -> int:
    out = sys.argv[1] if len(sys.argv) > 1 else "assets/sample.ppm"
    size = int(sys.argv[2]) if len(sys.argv) > 2 else 256
    width = height = size

    parent = os.path.dirname(out)
    if parent:
        os.makedirs(parent, exist_ok=True)

    with open(out, "wb") as f:
        f.write(b"P6\n%d %d\n255\n" % (width, height))
        row = bytearray(width * 3)
        for y in range(height):
            for x in range(width):
                row[x * 3 + 0] = x % 256
                row[x * 3 + 1] = y % 256
                row[x * 3 + 2] = (x + y) % 256
            f.write(row)

    print(f"wrote {out} ({width}x{height} P6 PPM)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
