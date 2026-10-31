#!/usr/bin/env python3
"""Redraws the eyes of the intro full-body pic so both match.

  python3 tools/player_sprites/fix_intro_face.py

The generated art had a plain 2x2 left eye next to a lidded, brown-iris
right eye (plus stray brow dots and a gray smudge on the lip). This mirrors
the right eye onto the left and centers the pair on the visible face.
Idempotent; build_battle_sprites.build_oak() also calls fix_face().
Indices are absolute values in oak_speech/red/pic.png (palette bank 4).
"""
import os

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PIC = os.path.join(ROOT, "graphics/oak_speech/red/pic.png")

SKIN = 91    # light skin
LID = 65     # black lid line
IRIS = (70, 68)  # right-eye iris, outer to inner (dark brown, near black)
WHITE = 95
SHADE = 79   # brown shade under the eye
TOOTH_SHADE = 89  # light shade right of the teeth

# (x, y) of stray pixels turned back into plain skin.
CLEAR = [(26, 17), (27, 17), (26, 18), (27, 18),  # old left eye
         (26, 15), (27, 15), (32, 15),            # stray brow dots
         (32, 22)]                                # gray smudge on the lip
EYE_AREA = (25, 37, 16, 18)  # x0, x1, y0, y1 wiped before redrawing eyes

# Eye layout (3 px wide): lid, iris + white, shade. The right eye is drawn
# at RIGHT_X; the left eye is its mirror image starting at LEFT_X. The
# visible face (skin left of the sideburn hair) is centered near x = 30, so
# the pair sits at x = 26..28 and 32..34 (one pixel left of the generated
# art's x = 27..29 / 33..35).
LEFT_X, RIGHT_X, TOP_Y = 26, 32, 16
EYE = {(0, 0): LID, (1, 0): LID, (2, 0): LID,
       (0, 1): IRIS[0], (1, 1): IRIS[1], (2, 1): WHITE,
       (0, 2): SHADE, (1, 2): SHADE, (2, 2): SKIN}

# Teeth stay 2 px at x = 30..31 with a shade on x = 32.
TEETH_Y = 23


def fix_face(im):
  """Applies the face fix in place to a 64x96 indexed pic image."""
  px = im.load()
  for x, y in CLEAR:
    px[x, y] = SKIN
  x0, x1, y0, y1 = EYE_AREA
  for y in range(y0, y1 + 1):
    for x in range(x0, x1 + 1):
      px[x, y] = SKIN
  for (dx, dy), v in EYE.items():
    px[RIGHT_X + dx, TOP_Y + dy] = v
    px[LEFT_X + 2 - dx, TOP_Y + dy] = v  # mirrored
  px[30, TEETH_Y] = px[31, TEETH_Y] = WHITE
  px[32, TEETH_Y] = TOOTH_SHADE


def main():
  im = Image.open(PIC)
  fix_face(im)
  im.save(PIC)
  print("fixed", os.path.relpath(PIC, ROOT))


if __name__ == "__main__":
  main()
