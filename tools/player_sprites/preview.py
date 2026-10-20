#!/usr/bin/env python3
"""Writes tools/player_sprites/previews/overworld_sheets.png: all rebuilt player sheets, scaled."""
import os
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
P = os.path.join(ROOT, "graphics/object_events/pics/people")
SHEETS = ["red_normal", "red_surf_run", "red_item", "red_bike",
          "red_vs_seeker_bike", "red_fish"]


def main(scale=4):
  ims = []
  for n in SHEETS:
    im = Image.open(os.path.join(P, n + ".png")).convert("RGBA")
    ims.append(im.resize((im.width * scale, im.height * scale),
                         Image.NEAREST))
  w = max(i.width for i in ims)
  h = sum(i.height + 6 for i in ims)
  out = Image.new("RGBA", (w, h), (150, 190, 130, 255))
  y = 0
  for im in ims:
    out.alpha_composite(im, (0, y))
    y += im.height + 6
  os.makedirs(os.path.join(ROOT, "tools/player_sprites/previews"), exist_ok=True)
  out.save(os.path.join(ROOT, "tools/player_sprites/previews/overworld_sheets.png"))
  print(out.size)


if __name__ == "__main__":
  main()
