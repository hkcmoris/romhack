# Maplebrook bench

`bench.aseprite` is the editable source. Export its single 32×16 frame as
`bench.png` in indexed color mode, preserving transparency. The sprite uses
existing outdoor palette 10; do not add colors or change that shared palette.

Rebuild the native tiles with `python tools/virelia/build_maplebrook_props.py`
and validate with `python tools/validate_virelia_tilesets.py`. The two bench
metatiles retain their grass underlay, footprint, and layer attributes.
