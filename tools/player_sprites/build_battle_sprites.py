#!/usr/bin/env python3
"""Builds the male-player battle/UI art from the concept images.

  python3 tools/player_sprites/build_battle_sprites.py

Writes: trainers/back_pics/red_back_pic.png (+palette), front_pics/
red_front_pic.png (+palette), region_map/player_icon_red.png (+palette),
oak_speech/red/pic.png (+pal.pal). Uses the generated Oak portrait in
tools/player_sprites/generated/ if present.
"""
import os

import numpy as np
from PIL import Image

from concept_groups import C1, C2
from gbaquant import (kmeans_palette, mode_scale, quantize, save_indexed,
                      write_jasc)
from spritelib import components, find_sprites, sort_grid

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
G = os.path.join(ROOT, "graphics")
HERE = os.path.dirname(os.path.abspath(__file__))
BG = (115, 197, 164)  # index-0 placeholder color used by vanilla files
SEEDS = [(30, 34, 38), (238, 238, 238), (255, 197, 148), (232, 64, 64)]


def palette_from(sprites, k, seed_extra=()):
  """k-color GBA palette (darkest first) from a list of RGBA arrays."""
  px = np.concatenate([s[..., :3][s[..., 3] > 0] for s in sprites])
  rng = np.random.default_rng(5)
  px = px[rng.choice(len(px), min(60000, len(px)), replace=False)]
  pal = kmeans_palette(px, k, list(SEEDS) + list(seed_extra))
  return pal[np.argsort(pal.sum(axis=1))]


def write_sprite(idx, pal, png, pal_file=None, bg=BG):
  """Saves indexed PNG (idx 0 transparent) and optional JASC palette."""
  full = np.concatenate([[bg], pal]).astype(np.uint8)
  save_indexed(idx, full, png)
  if pal_file:
    write_jasc(full, pal_file)
  print("wrote", os.path.relpath(png, ROOT), idx.shape[::-1])


def load_one(path, box, mode, min_area, dilate=0):
  """Largest single sprite found in box."""
  sp = find_sprites(Image.open(path), box, min_area=min_area, mode=mode,
                    dilate=dilate)
  return max(sp, key=lambda s: s["area"])


def fit(rgba, pal, w, h, margin_bottom=0, margin_top=0):
  """Scales rgba (keeping aspect) to fit w x h and bottom-aligns it.

  Returns (h, w) palette-index array, 0 = transparent.
  """
  sh, sw = rgba.shape[:2]
  k = min(w / sw, (h - margin_top - margin_bottom) / sh)
  nw, nh = max(1, round(sw * k)), max(1, round(sh * k))
  small = mode_scale(rgba, pal, nw, nh, outline_idx=1)
  idx = quantize(small, pal)
  out = np.zeros((h, w), np.uint8)
  left = (w - nw) // 2
  top = h - margin_bottom - nh
  out[top:top + nh, left:left + nw] = idx
  return out


def build_back():
  """5 throw frames, each cropped from its concept cell, 64x64 apiece."""
  box = (12, 572, 858, 712)
  sp = [s for s in find_sprites(Image.open(C2), box, min_area=3000,
                                mode="tint") if s["rgba"].shape[0] > 60]
  sp = sort_grid(sp, row_tol=60)[0]
  assert len(sp) == 5, len(sp)
  cell_w, win_y0, size = 169.5, 542, 170
  canvases = []
  for i, s in enumerate(sp):
    cv = np.zeros((size, size, 4), np.uint8)
    wx0 = int(round(12 + cell_w * i))
    x0, y0, x1, y1 = s["bbox"]
    ox, oy = x0 - wx0, y0 - win_y0
    h, w = s["rgba"].shape[:2]
    cx0, cy0 = max(0, -ox), max(0, -oy)
    cx1, cy1 = min(w, size - ox), min(h, size - oy)
    cv[oy + cy0:oy + cy1, ox + cx0:ox + cx1] = s["rgba"][cy0:cy1, cx0:cx1]
    canvases.append(cv)
  pal = palette_from(canvases, 15)
  frames = [quantize(mode_scale(c, pal, 64, 64, outline_idx=1), pal)
            for c in canvases]
  sheet = np.concatenate(frames, axis=0)
  write_sprite(sheet, pal, os.path.join(G, "trainers/back_pics/"
                                        "red_back_pic.png"),
               os.path.join(G, "trainers/palettes/red_back_pic.pal"))


def drop_green(rgba, keep_frac=0.2):
  """Removes green platform pixels and detached specks from a sprite."""
  r, g, b = (rgba[..., i].astype(int) for i in range(3))
  out = rgba.copy()
  out[(g > r + 12) & (g >= b - 5), 3] = 0
  parts = components(out[..., 3] > 0)
  big = max(p[0] for p in parts)
  for area, ys, xs in parts:
    if area < keep_frac * big:
      out[ys, xs, 3] = 0
  return out


def build_front():
  """Battle-trainer pose -> 64x64 trainer front pic."""
  s = drop_green(load_one(C1, (10, 400, 262, 590), "tint", 2000)["rgba"])
  ys, xs = np.nonzero(s[..., 3])
  s = s[ys.min():ys.max() + 1, xs.min():xs.max() + 1]
  pal = palette_from([s], 15)
  idx = fit(s, pal, 64, 64, margin_bottom=1, margin_top=1)
  write_sprite(idx, pal, os.path.join(G, "trainers/front_pics/"
                                      "red_front_pic.png"),
               os.path.join(G, "trainers/palettes/red.pal"))


def build_map_icon():
  """16x16 region-map head icon."""
  s = load_one(C2, (868, 562, 1018, 606), "tint", 100)["rgba"]
  h, w = s.shape[:2]
  side = max(h, w)
  sq = np.zeros((side, side, 4), np.uint8)
  oy, ox = (side - h) // 2, (side - w) // 2
  sq[oy:oy + h, ox:ox + w] = s
  pal = palette_from([sq], 15)
  idx = quantize(mode_scale(sq, pal, 16, 16, outline_idx=1), pal)
  write_sprite(idx, pal, os.path.join(G, "region_map/player_icon_red.png"),
               os.path.join(G, "region_map/player_icon_red.pal"))


def build_oak():
  """64x96 intro portrait from the generated full-body image (8bpp)."""
  path = os.path.join(HERE, "generated", "oak_portrait_gen.jpg")
  if not os.path.exists(path):
    print("skip oak portrait (no generated image)")
    return
  img = Image.open(path)
  s = max(find_sprites(img, (0, 0) + img.size, min_area=5000,
                       mode="chroma"), key=lambda s: s["area"])["rgba"]
  pal = palette_from([s], 31)
  idx = fit(s, pal, 64, 96, margin_bottom=2, margin_top=2)
  idx = np.where(idx > 0, idx + 64, 0).astype(np.uint8)  # bank 4 @ index 64
  full = np.zeros((256, 3), np.uint8)
  full[0] = BG
  full[65:96] = pal
  full[64] = BG
  im = Image.fromarray(idx, mode="P")
  im.putpalette(full.flatten().tolist())
  out = os.path.join(G, "oak_speech/red/pic.png")
  im.save(out)
  write_jasc(np.concatenate([[BG], pal]).astype(int),
             os.path.join(G, "oak_speech/red/pal.pal"))
  print("wrote", os.path.relpath(out, ROOT), idx.shape[::-1])


if __name__ == "__main__":
  build_back()
  build_front()
  build_map_icon()
  build_oak()
