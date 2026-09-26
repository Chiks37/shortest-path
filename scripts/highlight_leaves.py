#!/usr/bin/env python3
"""Post-process Doxygen class-hierarchy SVGs: highlight leaf (final) classes.

Doxygen cannot color leaf nodes on its own. In the global hierarchy graph edges
go base -> derived, so a leaf is a node with no outgoing edges. Project
convention guarantees every leaf is `final` and every non-leaf is `Abstract*`,
so abstract template instantiations that happen to be topological leaves are
filtered out by name.

Usage: highlight_leaves.py [doxygen_out_dir]
"""

import glob
import os
import re
import sys

LEAF_FILL = "#d9f0d9"
LEAF_STROKE = "#1b7837"
EDGE_OLD = "midnightblue"

PART_RE = re.compile(r"(<!--\s*Node.*?-->)")
NODE_ID_RE = re.compile(r"<!--\s*(Node\d+)\s*-->")
EDGE_ID_RE = re.compile(r"<!--\s*(Node\d+)&#45;&gt;(Node\d+)\s*-->")
CLASS_RE = re.compile(
    r'<title>(Node\d+)</title>\s*<g id="a_node\d+"><a xlink:href="classSP_1_1([^."]+)\.html"'
)


def is_abstract(class_name):
    return class_name.startswith("Abstract") or class_name == "BaseAlgo"


def process(svg):
    node_class = dict(CLASS_RE.findall(svg))
    sources = {m.group(1) for m in EDGE_ID_RE.finditer(svg)}

    leaves = {
        nid
        for nid, cls in node_class.items()
        if nid not in sources and not is_abstract(cls)
    }
    if not leaves:
        return svg, 0

    parts = PART_RE.split(svg)
    for i in range(1, len(parts), 2):
        comment, body = parts[i], parts[i + 1]

        node_m = NODE_ID_RE.match(comment)
        if node_m and node_m.group(1) in leaves:
            parts[i + 1] = body.replace(
                'fill="none" stroke="black"',
                f'fill="{LEAF_FILL}" stroke="{LEAF_STROKE}"',
                1,
            )
            continue

        edge_m = EDGE_ID_RE.match(comment)
        if edge_m and edge_m.group(2) in leaves:
            parts[i + 1] = body.replace(EDGE_OLD, LEAF_STROKE)

    return "".join(parts), len(leaves)


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else "doxygen_out"
    svgs = glob.glob(os.path.join(out_dir, "inherit_graph_*.svg"))
    if not svgs:
        print(f"no inherit_graph_*.svg found in {out_dir}", file=sys.stderr)
        return 1

    total = 0
    for path in svgs:
        with open(path, encoding="utf-8") as f:
            patched, n = process(f.read())
        if n:
            with open(path, "w", encoding="utf-8") as f:
                f.write(patched)
            total += n
            print(f"{os.path.basename(path)}: {n} leaves highlighted")
    print(f"done, {total} leaves total")
    return 0


if __name__ == "__main__":
    sys.exit(main())
