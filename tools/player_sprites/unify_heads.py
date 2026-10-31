#!/usr/bin/env python3
"""Makes the player's head identical across all frames of a direction.

  python3 tools/player_sprites/unify_heads.py

The concept-art pipeline shrinks every frame on its own, so hair texture and
eyes differ from frame to frame (shimmer) and the two eyes never match. This
takes one head per direction from the idle frames of red_normal, symmetrizes
the eyes of the front head, and pastes it over the head of every walk / run /
bike frame of that direction, at the position that frame already uses (so
the walking bob and body poses are unchanged). Edits the PNGs in place and
is idempotent. build_player_sprites.py runs it as its last step.
"""
import os

import numpy as np
from PIL import Image

from gbaquant import save_indexed

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PEOPLE = os.path.join(ROOT, "graphics/object_events/pics/people")

# sheet -> (frame width, view of each frame). "-" frames are left alone.
D, U, L = "down", "up", "left"
SHEETS = {
    "red_normal": (16, [D, U, L, D, D, U, U, L, L]),
    "red_surf_run": (16, [D, U, L, D, D, D, U, U, U, L, L, L, D, D]),
    "red_bike": (32, [D, U, L, D, D, U, U, L, L]),
}
# Head height in rows, from the first opaque row of the idle frame.
HEAD_ROWS = {D: 13, U: 12, L: 15}
IDLE_FRAME = {D: 0, U: 1, L: 2}  # in red_normal
MIN_MATCH = 0.7  # silhouette overlap needed to treat a frame's head as a match
KEEP_ROWS = 2    # last head rows only overwrite where the new head is opaque

# Front-head eyes in the idle frame (x, y): lid, white, iris. Left eye is
# x = 5..6, right eye its mirror (x = 10..9) around the face midline.
EYE_LID, EYE_WHITE, EYE_IRIS = 1, 15, 8
EYE_Y = (19, 20)


def read_sheet(name):
  """Returns (indexed pixel array, palette list) of a people sheet."""
  im = Image.open(os.path.join(PEOPLE, name + ".png"))
  return np.array(im), im.getpalette()[:48]


def symmetrize_eyes(cell):
  """Redraws both eyes of an idle front frame as a mirrored pair."""
  top, bot = EYE_Y
  for x, mx in ((5, 10), (6, 9)):
    for xx in (x, mx):
      cell[top, xx] = EYE_LID
  cell[bot, 5] = cell[bot, 10] = EYE_WHITE
  cell[bot, 6] = cell[bot, 9] = EYE_IRIS


def canonical_heads():
  """Returns {view: (rows x 16 head array)} from red_normal idle frames."""
  sheet, _ = read_sheet("red_normal")
  heads = {}
  for view, f in IDLE_FRAME.items():
    cell = sheet[:, f * 16:(f + 1) * 16].copy()
    if view == D:
      symmetrize_eyes(cell)
    top = int(np.nonzero(cell.any(axis=1))[0][0])
    heads[view] = cell[top:top + HEAD_ROWS[view]]
  return heads


def find_head(cell, head):
  """Finds the best silhouette overlap of `head` inside `cell`.

  Returns (score, y, x): intersection-over-union of the opaque masks.
  """
  n = head.shape[0]
  mask = head > 0
  best = (0.0, 0, 0)
  for y in range(cell.shape[0] - n + 1):
    for x in range(cell.shape[1] - 15):
      win = cell[y:y + n, x:x + 16] > 0
      score = (win & mask).sum() / max((win | mask).sum(), 1)
      if score > best[0]:
        best = (score, y, x)
  return best


def paste_head(cell, head, y, x):
  """Pastes `head` at (y, x); the last rows only paint opaque pixels."""
  n = head.shape[0]
  for t in range(n):
    row = head[t]
    if t >= n - KEEP_ROWS:
      keep = row > 0
      cell[y + t, x:x + 16][keep] = row[keep]
    else:
      cell[y + t, x:x + 16] = row


def run():
  """Unifies heads in every sheet listed in SHEETS."""
  heads = canonical_heads()
  for name, (fw, views) in SHEETS.items():
    sheet, pal = read_sheet(name)
    for f, view in enumerate(views):
      cell = sheet[:, f * fw:(f + 1) * fw]
      score, y, x = find_head(cell, heads[view])
      if score < MIN_MATCH:
        print(f"  {name}[{f}] ({view}): kept, head overlap {score:.2f}")
        continue
      paste_head(cell, heads[view], y, x)
    save_indexed(sheet, np.array(pal).reshape(-1, 3),
                 os.path.join(PEOPLE, name + ".png"))
    print("unified heads in", name)


if __name__ == "__main__":
  run()
