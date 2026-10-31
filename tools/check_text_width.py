#!/usr/bin/env python3
"""Checks map dialogue in data/maps/*/text.inc for GBA text box limits.

Reports, for each `.string` label:
  - any character missing from charmap.txt
  - any line wider than the dialogue box (pixel width, FONT_MALE glyphs)
  - any page (text between \\p) with a third line that did not use \\l

Usage: tools/check_text_width.py [text.inc ...]
With no arguments, every data/maps/*/text.inc is checked.
Exit status is 1 if any problem is found.
"""

import glob
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
# Widest line found in the unmodified game text (FONT_MALE glyph widths).
MAX_LINE_PX = 201


def load_widths():
  """Returns {char code: pixel width} for FONT_MALE Latin glyphs."""
  src = open(os.path.join(ROOT, 'src/text.c'), encoding='utf-8').read()
  body = re.search(
      r'sFontMaleLatinGlyphWidths\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
  return [int(n) for n in re.findall(r'\d+', body)]


def load_charmap():
  """Returns {single character: byte code} parsed from charmap.txt."""
  table = {}
  pattern = re.compile(r"^'(\\'|[^']+)'\s*=\s*([0-9A-Fa-f]{2})\b")
  for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
    m = pattern.match(line)
    if m:
      char = "'" if m.group(1) == "\\'" else m.group(1)
      table[char] = int(m.group(2), 16)
  return table


def line_width(text, charmap, widths, problems):
  """Returns the pixel width of one rendered line of text."""
  total = 0
  for char in text:
    code = charmap.get(char)
    if code is None:
      problems.append('character %r not in charmap' % char)
    elif code < len(widths):
      total += widths[code]
  return total


def check_file(path, charmap, widths):
  """Prints problems for one text.inc and returns how many were found."""
  count = 0
  text = open(path, encoding='utf-8').read()
  labels = re.findall(r'^(\w+)::\n((?:[ \t]+\.string.*\n?)+)', text, re.M)
  for label, body in labels:
    joined = ''.join(re.findall(r'"(.*?)"\s*$', body, re.M)).rstrip('$')
    problems = []
    for page in joined.split('\\p'):
      rows = re.split(r'\\[nl]', page)
      seps = re.findall(r'\\([nl])', page)
      if len(rows) > 2 and 'n' in seps[1:]:
        problems.append('page has a third line without \\l: %r' % page)
      for row in rows:
        if '{' in row:  # Variable-width placeholder; cannot be checked.
          continue
        if line_width(row, charmap, widths, problems) > MAX_LINE_PX:
          problems.append('line too wide: %r' % row)
    for problem in problems:
      print('%s: %s: %s' % (os.path.relpath(path), label, problem))
    count += len(problems)
  return count


def main():
  widths, charmap = load_widths(), load_charmap()
  files = sys.argv[1:] or sorted(
      glob.glob(os.path.join(ROOT, 'data/maps/*/text.inc')))
  bad = sum(check_file(f, charmap, widths) for f in files)
  print('%d problem(s) in %d file(s)' % (bad, len(files)))
  return 1 if bad else 0


if __name__ == '__main__':
  sys.exit(main())
