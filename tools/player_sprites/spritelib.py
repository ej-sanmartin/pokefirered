"""Helpers to pull sprites out of concept art and fit them to GBA limits.

Pure Pillow + numpy. Used by build_player_sprites.py.
"""
from collections import deque

import numpy as np
from PIL import Image

CHROMA = (0, 255, 41)


def bg_like(rgb, min_chan=185, max_spread=50):
  """Returns bool mask of light, low-saturation pixels (paper/stripes).

  Args:
    rgb: HxWx3 uint8 array.
  """
  mn = rgb.min(axis=2).astype(int)
  mx = rgb.max(axis=2).astype(int)
  return (mn >= min_chan) & ((mx - mn) <= max_spread)


def chroma_like(rgb, tol=70):
  """Returns bool mask of pixels close to the chroma green background."""
  d = np.abs(rgb.astype(int) - np.array(CHROMA)).sum(axis=2)
  # Also catch the darker/lighter greens image models drift to.
  g = rgb[..., 1].astype(int)
  greenish = (g > rgb[..., 0] + 60) & (g > rgb[..., 2] + 60)
  return (d < tol * 3) | greenish


def _flood(candidate):
  """Marks `candidate` pixels connected (4-way) to the array border."""
  h, w = candidate.shape
  seen = np.zeros_like(candidate)
  q = deque()
  for x in range(w):
    for y in (0, h - 1):
      if candidate[y, x] and not seen[y, x]:
        seen[y, x] = True
        q.append((y, x))
  for y in range(h):
    for x in (0, w - 1):
      if candidate[y, x] and not seen[y, x]:
        seen[y, x] = True
        q.append((y, x))
  while q:
    y, x = q.popleft()
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
      ny, nx = y + dy, x + dx
      if 0 <= ny < h and 0 <= nx < w and candidate[ny, nx] \
          and not seen[ny, nx]:
        seen[ny, nx] = True
        q.append((ny, nx))
  return seen


def components(mask):
  """Returns list of (area, ys, xs) 8-connected components of mask."""
  h, w = mask.shape
  seen = np.zeros_like(mask)
  out = []
  for y0 in range(h):
    for x0 in range(w):
      if not mask[y0, x0] or seen[y0, x0]:
        continue
      q = deque([(y0, x0)])
      seen[y0, x0] = True
      ys, xs = [], []
      while q:
        y, x = q.popleft()
        ys.append(y)
        xs.append(x)
        for dy in (-1, 0, 1):
          for dx in (-1, 0, 1):
            ny, nx = y + dy, x + dx
            if 0 <= ny < h and 0 <= nx < w and mask[ny, nx] \
                and not seen[ny, nx]:
              seen[ny, nx] = True
              q.append((ny, nx))
      out.append((len(ys), np.array(ys), np.array(xs)))
  return out


def tint_like(rgb, min_chan=150, tint=8):
  """Pale blue/green tinted pixels (battle stripes, platforms)."""
  mn = rgb.min(axis=2).astype(int)
  r = rgb[..., 0].astype(int)
  other = np.maximum(rgb[..., 1], rgb[..., 2]).astype(int)
  return (mn >= min_chan) & (other - r >= tint)


def _grow(mask, n):
  """Binary dilation by n pixels (4-neighborhood)."""
  for _ in range(n):
    g = mask.copy()
    g[1:] |= mask[:-1]
    g[:-1] |= mask[1:]
    g[:, 1:] |= mask[:, :-1]
    g[:, :-1] |= mask[:, 1:]
    mask = g
  return mask


def strip_halo(rgb, fg, reach=2):
  """Removes pale gray JPEG fringe hugging the outside of the sprite."""
  mn = rgb.min(axis=2).astype(int)
  spread = rgb.max(axis=2).astype(int) - mn
  pale = (spread < 45) & (mn >= 110)
  near_bg = _grow(~fg, reach)
  return fg & ~(pale & near_bg)


def foreground(rgb, mode="paper"):
  """Returns bool foreground mask for an RGB crop (border-connected bg off)."""
  if mode == "chroma":
    cand = chroma_like(rgb)
  elif mode == "tint":
    cand = tint_like(rgb) | bg_like(rgb, 215, 30)
  elif mode == "paper2":  # also eats pale shadow ellipses
    cand = bg_like(rgb, 165, 55)
  else:
    cand = bg_like(rgb)
  return strip_halo(rgb, ~_flood(cand))


def find_sprites(img, box, min_area=300, mode="paper", dilate=0,
                 keep_frac=0.0):
  """Finds sprites inside box.

  Returns a list of dicts: {bbox:(x0,y0,x1,y1) in image coords,
  rgba: HxWx4 uint8 crop} sorted top-to-bottom, left-to-right (row-major,
  rows clustered by y-center).
  """
  x0, y0, x1, y1 = box
  rgb = np.array(img.convert("RGB").crop(box))
  fg = foreground(rgb, mode)
  grow = fg
  for _ in range(dilate):
    g = grow.copy()
    g[1:] |= grow[:-1]
    g[:-1] |= grow[1:]
    g[:, 1:] |= grow[:, :-1]
    g[:, :-1] |= grow[:, 1:]
    grow = g
  found = []
  for area, ys, xs in components(grow):
    if area < min_area:
      continue
    ty0, ty1, tx0, tx1 = ys.min(), ys.max(), xs.min(), xs.max()
    m = np.zeros_like(fg)
    m[ys, xs] = True
    m &= fg
    if keep_frac > 0:  # drop detached bits (sparkles, arcs) inside the blob
      parts = components(m)
      big = max(p[0] for p in parts)
      m = np.zeros_like(fg)
      for pa, py, px in parts:
        if pa >= keep_frac * big:
          m[py, px] = True
    crop = rgb[ty0:ty1 + 1, tx0:tx1 + 1].copy()
    a = (m[ty0:ty1 + 1, tx0:tx1 + 1] * 255).astype(np.uint8)
    rgba = np.dstack([crop, a])
    found.append({
        "bbox": (x0 + tx0, y0 + ty0, x0 + tx1 + 1, y0 + ty1 + 1),
        "rgba": rgba,
        "area": area,
    })
  return found


def sort_grid(sprites, row_tol=30):
  """Sorts sprites into rows (by y-center) then columns. Returns rows list."""
  def cy(s):
    return (s["bbox"][1] + s["bbox"][3]) / 2

  def cx(s):
    return (s["bbox"][0] + s["bbox"][2]) / 2
  rows = []
  for s in sorted(sprites, key=cy):
    if rows and abs(cy(s) - np.mean([cy(r) for r in rows[-1]])) < row_tol:
      rows[-1].append(s)
    else:
      rows.append([s])
  return [sorted(r, key=cx) for r in rows]


def scale_rgba(rgba, factor):
  """Alpha-aware area downscale so background never bleeds into edges."""
  h, w = rgba.shape[:2]
  nw, nh = max(1, round(w * factor)), max(1, round(h * factor))
  a = rgba[..., 3:4].astype(np.float32) / 255.0
  prem = np.dstack([rgba[..., :3].astype(np.float32) * a, a])
  chans = []
  for c in range(4):
    im = Image.fromarray(prem[..., c], mode="F")
    chans.append(np.array(im.resize((nw, nh), Image.BOX)))
  prem = np.dstack(chans)
  alpha = prem[..., 3:4]
  rgb = np.where(alpha > 1e-4, prem[..., :3] / np.maximum(alpha, 1e-4), 0)
  out = np.dstack([rgb, alpha * 255]).clip(0, 255).astype(np.uint8)
  out[..., 3] = np.where(out[..., 3] >= 128, 255, 0)
  return out


def place(rgba, fw, fh, base_y, cx=None, flip=False, shift_x=0,
          anchor="bbox"):
  """Places sprite on a fw x fh transparent cell, feet at row base_y.

  anchor "bbox" centers the opaque bbox on cx (default cell middle);
  anchor "feet" centers the bottom 3 rows on cx (stable when rods or arms
  stick out).
  """
  if flip:
    rgba = rgba[:, ::-1]
  cell = np.zeros((fh, fw, 4), np.uint8)
  ys, xs = np.nonzero(rgba[..., 3])
  x0, x1, y0, y1 = xs.min(), xs.max() + 1, ys.min(), ys.max() + 1
  spr = rgba[y0:y1, x0:x1]
  h, w = spr.shape[:2]
  if cx is None:
    cx = fw / 2
  ax = w / 2
  if anchor == "feet":
    fx = np.nonzero(spr[-3:, :, 3])[1]
    ax = fx.mean() + 0.5
  left = int(round(cx - ax)) + shift_x
  top = base_y + 1 - h
  for yy in range(h):
    for xx in range(w):
      X, Y = left + xx, top + yy
      if 0 <= X < fw and 0 <= Y < fh and spr[yy, xx, 3]:
        cell[Y, X] = spr[yy, xx]
  return cell
