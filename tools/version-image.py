#!/usr/bin/env python3
import argparse
import os
from PIL import Image


def main() -> None:
    parser = argparse.ArgumentParser(
        description="""\
Update the epicardium version-splash.  NOTE: You need to adjust
epicardium/main.c if you want to actually see your version splash!!
"""
    )

    parser.add_argument("image", help="Path to version image")

    args = parser.parse_args()

    im = Image.open(args.image)
    assert im.size[0] == 160, "Image must be 160 pixels wide"
    assert im.size[1] == 80, "Image must be 80 pixels high)"

    # find version-splash.h
    vsplash = os.path.join(os.path.dirname(__file__), "../epicardium/version-splash.h")

    with open(vsplash, "w") as f:
        tmp = """\
#pragma once
#include <stdint.h>

/* clang-format off */
const uint8_t version_splash[] = {
"""
        f.write(tmp)

        for x in range(im.size[1]):
            for y in range(im.size[0]):
                px = im.getpixel((y, x))

                px565 = ((px[0] >> 3) << 11) | ((px[1] >> 2) << 5) | (px[2] >> 3)
                byte_high = (px565 & 0xFF00) >> 8
                byte_low = px565 & 0xFF

                if y % 4 == 0:
                    f.write("\t")

                f.write("0x{:02x}, 0x{:02x},".format(byte_low, byte_high))

                if y % 4 == 3:
                    f.write("\n")
                else:
                    f.write(" ")

        f.write("};\n")


if __name__ == "__main__":
    main()
