#!/bin/sh
# Zero Zero UFO for GFOS: host tests.
#   tests/host/run.sh [GFOPENALLEGRO/allegro4]
# Through gfopenallegro's allegro4 core on its fake acgame.library:
#   - test_data: every BIN/*.DAT loads at 8-bit, and every object name the
#     sources ask for (Datafile::getObject("NAME")) is in one of them;
#   - the whole game (SRC/ built as on GFOS, -D__amigaos__ with the header
#     names amiga/tools/dos_includes.py links) links on the host, so no
#     Allegro call is missing, and a smoke run in the game's data shows its
#     title menu (smoke_probe.c).
# Copyright (c) 2026 AmiCradle Team. MIT, as Zero Zero UFO.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
A4=${1:-"$ROOT/../gfopenallegro/allegro4"}
CC=${HOSTCC:-cc}
CXX=${HOSTCXX:-c++}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
mkdir -p "$T/a4" "$T/game" "$T/run/User"
# the core as gfopenallegro's own tests/run_tests.sh builds it: src/ less
# the Amiga-only a4_dumb_gm.c, with its fakes (acgame, gfopenmusic)
for f in "$A4"/src/a4_*.c "$A4"/tests/fake_*.c; do
    case "$f" in *a4_dumb_gm.c) continue ;; esac
    "$CC" -std=gnu99 -D_DEFAULT_SOURCE -w -I"$A4/include" -I"$A4/src" -I"$A4/tests" -c "$f" -o "$T/a4/$(basename "$f").o"
done
"$CC" -std=gnu99 -w -I"$A4/include" "$ROOT/tests/host/test_data.c" "$T"/a4/*.o -lm -o "$T/test_data"
"$T/test_data" "$ROOT/BIN" $(grep -rhoE 'getObject\("[^"]+"\)' "$ROOT"/SRC/*.CPP "$ROOT"/SRC/*.cpp | sed 's/getObject("//; s/")//' | sort -u)
[ "${HOST_LINK:-1}" = 0 ] && { echo "game: host link skipped (HOST_LINK=0)"; exit 0; }
python3 "$ROOT/amiga/tools/dos_includes.py" "$ROOT/INCLUDE" "$T/inc" "$ROOT/SRC" "$ROOT/INCLUDE"
n=0
for f in "$ROOT"/SRC/*.CPP "$ROOT"/SRC/*.cpp; do
    "$CXX" -std=gnu++17 -w -D__amigaos__ -I"$T/inc" -I"$A4/include" -c "$f" -o "$T/game/$(basename "$f").o"
    n=$((n + 1))
done
"$CC" -std=gnu99 -w -I"$A4/include" -I"$A4/tests" -c "$ROOT/tests/host/smoke_probe.c" -o "$T/probe.o"
"$CXX" -o "$T/zeroufo" "$T"/game/*.o "$T/probe.o" "$T"/a4/*.o -lm -lpthread
echo "game: $n sources link on the host"
for f in "$ROOT"/BIN/*.DAT "$ROOT"/BIN/*.dat; do cp "$f" "$T/run/$(basename "$f" | tr 'A-Z' 'a-z')"; done
(cd "$T/run" && AT_SMOKE_SECS=${AT_SMOKE_SECS:-12} AT_SMOKE_PPM=${AT_SMOKE_PPM:-} "$T/zeroufo" | grep '^smoke:')
