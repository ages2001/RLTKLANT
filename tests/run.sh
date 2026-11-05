#!/bin/bash
# Host checks of the PCI / USB id tables, the chip identification and the
# INF models against the Linux drivers they follow (see test_idtables.c).
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
CC=${CC:-gcc}
OUT="${TMPDIR:-/tmp}/rltk_t_idtables"
$CC -std=c99 -Wall -Wextra -Werror -include string.h -I"$HERE/../src" \
    "$HERE/test_idtables.c" "$HERE/../src/fwimages.c" -o "$OUT"
"$OUT"
python3 "$HERE/../tools/mkinfids.py" --check
