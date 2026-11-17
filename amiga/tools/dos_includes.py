#!/usr/bin/env python3
"""Zero Zero UFO for GFOS: the DOS sources name their headers in any case
(b17.h for B17.h, map.h for MAP.H), which DOS and AmigaDOS take but a cross
compiler on Linux does not. This makes OUT_DIR with one link per spelling
the sources use, to the header in HEADER_DIR that has it.

  dos_includes.py HEADER_DIR OUT_DIR SOURCE_DIR...

Copyright (c) 2026 AmiCradle Team. MIT, as Zero Zero UFO.
"""
import os
import re
import sys

SOURCES = (".C", ".CPP", ".H", ".HPP")


def main(hdr, out, scan):
    hdr = os.path.abspath(hdr)
    files = {f.lower(): f for f in os.listdir(hdr) if os.path.isfile(os.path.join(hdr, f))}
    wanted = set()
    for d in scan:
        for f in os.listdir(d):
            if f.upper().endswith(SOURCES):
                with open(os.path.join(d, f), encoding="latin-1") as fh:
                    wanted.update(re.findall(r'^\s*#\s*include\s*"([^"/]+)"', fh.read(), re.M))
    os.makedirs(out, exist_ok=True)
    made = 0
    for name in sorted(wanted):
        real = files.get(name.lower())
        if real is None:
            continue  # a library's header (allegro.h), found on the include path
        link = os.path.join(out, name)
        if os.path.lexists(link):
            os.remove(link)
        os.symlink(os.path.join(hdr, real), link)
        made += 1
    with open(os.path.join(out, ".done"), "w") as fh:
        fh.write("%d\n" % made)
    print("dos_includes: %d header names linked in %s" % (made, out))


if __name__ == "__main__":
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2], sys.argv[3:])
