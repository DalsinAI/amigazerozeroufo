# Zero Zero UFO for GFOS

Zero Zero UFO v0.2022.11.24 by Alvaro "Krono" Gonzalez Ferrer (Kronoman,
MIT), from the author's own https://github.com/alvarogonzalezferrer/zerozeroufo
at commit c43cc93aa3b209cb2b24cd0de93d27abff33271c, built for AmigaOS
3.2 / GFOS on a 68040, 68060 or 68080. No FPU is needed.

A side-scrolling arcade game written in 2022 for MS-DOS (third place in
Espacio TEC's retro game contest): you fly a UFO, lift cows with your
tractor beam to mend your armour, and fight off the army's tanks,
helicopters, planes and soldiers, wave after wave. It runs in 320 x 200
in 256 colours. The author calls it an unfinished alpha: the waves never
end.

Status: see the pull request (stove build and lab).

## What is in the drawer

- `ZeroZeroUFO`: the game.
- The `.dat` files: the author's datafiles (sprites, backgrounds, sounds,
  fonts), as released.
- `Libs/allegro4.library` and `Libs/acgame.library`: Allegro 4 for GFOS
  (gfopenallegro) on ACGame. The game opens them from its own `Libs/`.
- `User/`: your settings (`game.cfg`), made by the game. Updates and
  `GFOpenUpTool REMOVE` leave it alone.
- `Controls`, `ReadMe-ZeroZeroUFO` (the author's read-me),
  `Licences.txt`, `LICENSE.txt`.

## Playing

Run Install-ZeroZeroUFO (or copy the ZeroZeroUFO drawer where you want
it), then double-click ZeroZeroUFO, or start it from a Shell from
anywhere: the game makes its own drawer its current directory while it
runs. The keys are in `Controls`.

## How the port is made

- The code is the author's, built against gfopenallegro's Allegro 4 with
  68040 soft float (`amiga/Makefile.amigaos`). The game times itself with
  an Allegro timer; allegro4.library runs it from a task of its own.
- The DOS sources name their headers in any case; the cross compiler
  needs the exact names, so `amiga/tools/dos_includes.py` makes a link
  per spelling at build time. No source is renamed.
- The datafiles are read from the drawer, and `game.cfg` goes in
  `User/` (`SRC/HARDWARE.CPP`, marked `__amigaos__`).
- Sound is silent for now: gfopenallegro plays samples once ACGame has
  audio.

## Building

    make -f amiga/Makefile.amigaos PREFIX=<stove>/prefix A4=<gfopenallegro>/allegro4 A4OUT=... ACGOUT=...

or, in Cradle, stove_build with the os32-gcc16-open6 stove and target
package (`package/build-amiga.sh`), which also builds allegro4.library
and acgame.library at the commits `package/package.env` pins, and packs
ZeroZeroUFO-VERSION-68040.lha and the GFOpenUp part.

    tests/host/run.sh <gfopenallegro>/allegro4

runs the host tests: all 17 datafiles load at 8-bit and every one of
the 91 object names the sources ask for is in them; the whole game links
on the host against gfopenallegro; and a smoke run in the game's data
shows the title menu.

Designed, tested and delivered by The AmiCradle Team.
