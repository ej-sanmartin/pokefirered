"""GBA palette math, quantizing and indexed-PNG / JASC output."""
import numpy as np
from PIL import Image


def to_gba5(c):
  """Snaps an 8-bit channel to the nearest GBA 5-bit value (as 8-bit)."""
  c5 = np.clip(np.round(np.asarray(c, float) * 31 / 255), 0, 31)
  return np.round(c5 * 255 / 31).astype(int)


def kmeans_palette(pixels, k, seed_colors=(), iters=25, weights=None):
  """Weighted k-means on RGB pixels (N x 3). Returns k x 3 uint8 GBA colors."""
  rng = np.random.default_rng(7)
  px = pixels.astype(np.float32)
  cents = [np.array(c, np.float32) for c in seed_colors]
  while len(cents) < k:  # kmeans++ style farthest-point seeding
    d = np.min([((px - c) ** 2).sum(axis=1) for c in cents], axis=0) \
        if cents else np.ones(len(px))
    p = d / d.sum() if d.sum() > 0 else None
    cents.append(px[rng.choice(len(px), p=p)])
  cents = np.array(cents[:k])
  for _ in range(iters):
    d = ((px[:, None, :] - cents[None]) ** 2).sum(axis=2)
    lab = d.argmin(axis=1)
    for i in range(k):
      sel = lab == i
      if sel.any():
        cents[i] = px[sel].mean(axis=0)
  return to_gba5(cents).astype(np.uint8)


def quantize(rgba_cell, palette):
  """Maps RGBA cell to palette indices (1..n); transparent -> 0."""
  h, w = rgba_cell.shape[:2]
  idx = np.zeros((h, w), np.uint8)
  rgb = rgba_cell[..., :3].astype(np.int32)
  pal = palette.astype(np.int32)
  d = ((rgb[:, :, None, :] - pal[None, None]) ** 2).sum(axis=3)
  best = d.argmin(axis=2) + 1
  idx = np.where(rgba_cell[..., 3] > 0, best, 0).astype(np.uint8)
  return idx


def save_indexed(idx, palette16, path):
  """Writes an indexed PNG (index 0 = transparent) with a 16-color palette."""
  im = Image.fromarray(idx, mode="P")
  flat = []
  for c in palette16:
    flat.extend(int(v) for v in c)
  flat += [0] * (768 - len(flat))
  im.putpalette(flat)
  im.info["transparency"] = 0
  im.save(path, transparency=0)


def write_jasc(palette16, path):
  """Writes a JASC .pal file."""
  lines = ["JASC-PAL", "0100", str(len(palette16))]
  lines += [f"{int(r)} {int(g)} {int(b)}" for r, g, b in palette16]
  with open(path, "w") as f:
    f.write("\r\n".join(lines) + "\r\n")


def mode_scale(rgba, palette, nw, nh, outline_idx=None, outline_bias=0.34):
  """Palette-first block-mode downscale to exactly nw x nh.

  Each source pixel is snapped to `palette` (indices 1..n), then every
  target pixel takes the most common index in its source block. A block is
  transparent if over half of it is. Keeps pixel art crisp (no blending).
  Returns RGBA uint8 (nh, nw, 4) built from the palette.
  """
  h, w = rgba.shape[:2]
  idx = quantize(rgba, palette)
  ys = np.linspace(0, h, nh + 1).round().astype(int)
  xs = np.linspace(0, w, nw + 1).round().astype(int)
  out = np.zeros((nh, nw), np.uint8)
  n = len(palette) + 1
  for j in range(nh):
    for i in range(nw):
      blk = idx[ys[j]:max(ys[j + 1], ys[j] + 1),
                xs[i]:max(xs[i + 1], xs[i] + 1)].ravel()
      cnt = np.bincount(blk, minlength=n)
      if cnt[0] * 2 > blk.size:
        continue
      cnt[0] = 0
      if outline_idx and cnt[outline_idx] >= outline_bias * blk.size:
        out[j, i] = outline_idx
      else:
        out[j, i] = cnt.argmax()
  return idx_to_rgba(out, palette)


def idx_to_rgba(idx, palette):
  """Converts index array (0 = transparent) to RGBA using palette[idx-1]."""
  rgba = np.zeros(idx.shape + (4,), np.uint8)
  m = idx > 0
  rgba[m, :3] = np.asarray(palette)[idx[m] - 1]
  rgba[m, 3] = 255
  return rgba
