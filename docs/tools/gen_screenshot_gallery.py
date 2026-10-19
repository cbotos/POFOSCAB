#!/usr/bin/env python3
"""Generates a static HTML gallery of MAME test screenshots, grouped by
test name (subdirectory), for the Sphinx docs test_results page.

Usage: gen_screenshot_gallery.py <screenshots_dir> <output_html>
"""
import html
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 1

    src_dir = Path(sys.argv[1])
    out_path = Path(sys.argv[2])

    groups = {}
    for png in sorted(src_dir.rglob('*.png')):
        group = png.parent.relative_to(src_dir).as_posix()
        groups.setdefault(group, []).append(png.relative_to(src_dir).as_posix())

    sections = []
    for group in sorted(groups):
        figures = '\n'.join(
            f'<figure><img src="{html.escape(rel)}" loading="lazy">'
            f'<figcaption>{html.escape(Path(rel).name)}</figcaption></figure>'
            for rel in groups[group]
        )
        sections.append(
            f'<section><h2>{html.escape(group)}</h2>'
            f'<div class="grid">{figures}</div></section>'
        )

    out_path.write_text(f"""<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>MAME test screenshots</title>
<style>
body {{ font-family: sans-serif; margin: 1.5em; }}
.grid {{ display: flex; flex-wrap: wrap; gap: 1em; }}
figure {{ margin: 0; max-width: 320px; }}
img {{ max-width: 100%; border: 1px solid #ccc; }}
figcaption {{ font-size: 0.85em; color: #555; text-align: center; }}
</style>
</head>
<body>
<h1>MAME test screenshots</h1>
{''.join(sections) if sections else '<p>No screenshots in this run.</p>'}
</body>
</html>
""")
    return 0


if __name__ == '__main__':
    sys.exit(main())
