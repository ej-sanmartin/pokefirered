#!/usr/bin/env python3
"""Builds the male-player overworld sheets from the concept art.

Run from anywhere:  python3 tools/player_sprites/build_player_sprites.py
Writes the red_*.png sheets + player.pal / player_reflection.pal.
Frame order follows pokefirered's sPicTable_Red* / object_event_anims.h.
"""
import os

import numpy as np
from PIL import Image

from concept_groups import group_scale, load_all
from gbaquant import (idx_to_rgba, kmeans_palette, mode_scale, quantize,
                      save_indexed, write_jasc)
from spritelib import place

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PEOPLE = os.path.join(ROOT, "graphics/object_events/pics/people")
PALS = os.path.join(ROOT, "graphics/object_events/palettes")
SEEDS = [(30, 34, 38), (238, 238, 238), (255, 197, 148), (232, 64, 64)]
BG = (131, 123, 164)  # vanilla index-0 placeholder color


def build_palette(groups):
  """15-color GBA palette shared by every overworld sheet."""
  px = []
  for rows in groups.values():
    for row in rows:
      for s in row:
        a = s["rgba"][..., 3] > 0
        px.append(s["rgba"][..., :3][a])
  px = np.concatenate(px)
  rng = np.random.default_rng(3)
  px = px[rng.choice(len(px), min(60000, len(px)), replace=False)]
  pal = kmeans_palette(px, 15, SEEDS)
  order = np.argsort(pal.sum(axis=1))  # darkest first -> outline is idx 1
  return pal[order]


def shrink(groups, name, pal):
  """Returns rows of palette-snapped GBA-size RGBA sprites for a group."""
  k = group_scale(name)
  out = []
  for row in groups[name]:
    r = []
    for s in row:
      h, w = s["rgba"].shape[:2]
      r.append(mode_scale(s["rgba"], pal, round(w * k), round(h * k),
                          outline_idx=1))
    out.append(r)
  return out


def trim_above_hair(spr):
  """Blanks rows above the first row with a solid run of dark hair pixels.

  Removes the wifi arc floating over the head in the Vs. Seeker frames.
  """
  out = spr.copy()
  dark = (spr[..., :3].sum(axis=2) < 330) & (spr[..., 3] > 0)
  top = int(np.argmax(dark.sum(axis=1) >= 5))
  out[:top] = 0
  return out


def main():
  groups = load_all()
  pal = build_palette(groups)
  print("palette:", [tuple(int(v) for v in c) for c in pal])
  sm = {n: shrink(groups, n, pal) for n in groups}
  ow, bike, fm, vf, vb, fish = (sm[n] for n in
                                ("ow", "bike", "fm", "vsfoot", "vsbike",
                                 "fish"))
  d, u, l = ow[0], ow[1], ow[2]  # down / up / left (right unused: h-flip)

  def p16(spr, base, **kw):
    return place(spr, 16, 32, base, **kw)

  # Overworld walk: idle (concept col 0) then two stride frames per
  # direction, picked for opposite leg phases.
  strides = ((d, 2, 4), (u, 2, 4), (l, 2, 4))
  idle = [p16(d[0], 30), p16(u[0], 30), p16(l[0], 30)]
  walk = []
  for spr, a, b in strides:
    walk += [p16(spr[a], 31), p16(spr[b], 31)]
  save(np.concatenate(idle + walk, axis=1), pal, "red_normal.png")

  # surf_run: 0-2 surf/mount idle, 3-11 run (idle,A,B per dir), 12-13 shake
  surf_idle = [p16(d[0], 29), p16(u[0], 29), p16(l[0], 29)]
  run = []  # no run art in the concept: reuse idle + stride frames
  for spr, a, b in strides:
    run += [p16(spr[0], 30), p16(spr[a], 31), p16(spr[b], 31)]
  shake = [walk[0], walk[1]]
  save(np.concatenate(surf_idle + run + shake, axis=1), pal,
       "red_surf_run.png")

  # item: field move 0-4, Vs Seeker 5-8 (open, close, active, open)
  f = [p16(s, 30) for row in fm for s in row]  # concept frames 1..6
  vopen, vact, vclose = (p16(s, 30) for s in vf[0])
  item = f[:5] + [vopen, vclose, vact, vopen]
  save(np.concatenate(item, axis=1), pal, "red_item.png")

  # bike (32x32): idle d/u/l then A/B per direction
  def p32(spr, base=31, **kw):
    return place(spr, 32, 32, base, **kw)
  bd, bu, bl = bike
  cells = [p32(bd[0]), p32(bu[0]), p32(bl[0]), p32(bd[1]), p32(bd[2]),
           p32(bu[1]), p32(bu[2]), p32(bl[1]), p32(bl[2])]
  save(np.concatenate(cells, axis=1), pal, "red_bike.png")

  # Vs Seeker bike: concept frames 1..6 (frame 3 has a wifi arc on top)
  vcells = [p32(trim_above_hair(s)) for s in vb[0]]
  save(np.concatenate(vcells, axis=1), pal, "red_vs_seeker_bike.png")

  # fishing: rows West(flip, concept faces right), North, South
  fcells = []
  for ri, row in enumerate(fish):
    for s in row:
      fcells.append(place(s, 32, 32, 30, cx=21 if ri == 0 else 11,
                          flip=(ri == 0), anchor="feet"))
  save(np.concatenate(fcells, axis=1), pal, "red_fish.png")

  write_palettes(pal)


def save(rgba_sheet, pal, name):
  """Quantizes a RGBA sheet to the shared palette and writes the PNG."""
  idx = quantize(rgba_sheet, pal)
  full = np.concatenate([[BG], pal]).astype(np.uint8)
  save_indexed(idx, full, os.path.join(PEOPLE, name))
  print("wrote", name, rgba_sheet.shape[1], "x", rgba_sheet.shape[0])


def write_palettes(pal):
  """player.pal plus a lightened water-reflection variant."""
  full = np.concatenate([[BG], pal]).astype(int)
  write_jasc(full, os.path.join(PALS, "player.pal"))
  refl = full + (255 - full) * 0.5
  refl[:, 2] = full[:, 2] + (255 - full[:, 2]) * 0.65
  refl[0] = (197, 255, 246)
  write_jasc(refl.round().astype(int), os.path.join(PALS,
                                                    "player_reflection.pal"))


if __name__ == "__main__":
  main()
