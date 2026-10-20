"""Where each sprite group lives inside the two concept images.

Boxes are (x0, y0, x1, y1) in concept-image pixels. `scale` converts concept
pixels to GBA pixels (tuned so the hair/head width matches across groups).
"""
import os

from PIL import Image

from spritelib import find_sprites, sort_grid

HERE = os.path.dirname(os.path.abspath(__file__))
C1 = os.path.join(HERE, "concept", "concept1_overworld_portrait.jpg")
C2 = os.path.join(HERE, "concept", "concept2_activities_battle.jpg")

# name: (image, box, mode, min_area, dilate, keep_frac, scale)
GROUPS = {
    "ow": (C1, (440, 38, 828, 358), "paper", 400, 0, 0.0, 1 / 3),
    "bike": (C2, (55, 38, 268, 312), "paper2", 500, 0, 0.0, 1 / 3),
    "fm": (C2, (772, 40, 1016, 232), "paper", 500, 2, 0.2, 0.36),
    "vsfoot": (C2, (15, 392, 285, 520), "paper", 500, 0, 0.0, 0.29),
    "vsbike": (C2, (300, 392, 680, 520), "paper", 500, 0, 0.0, 1 / 3),
    "fish": (C2, (768, 358, 1018, 525), "paper", 300, 0, 0.0, 0.42),
}


def load_group(name):
  """Returns rows (list of lists) of sprite dicts for a GROUPS entry."""
  path, box, mode, min_area, dilate, keep_frac, _ = GROUPS[name]
  img = Image.open(path)
  sprites = find_sprites(img, box, min_area=min_area, mode=mode,
                         dilate=dilate, keep_frac=keep_frac)
  sprites = [s for s in sprites if s["rgba"].shape[0] > 30]  # drop labels
  return sort_grid(sprites, row_tol=40)


def group_scale(name):
  """Concept-px -> GBA-px scale for a group."""
  return GROUPS[name][6]


def load_all():
  """Returns {group: rows} for every overworld-palette group."""
  return {name: load_group(name) for name in GROUPS}
