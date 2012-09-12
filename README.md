overbrew
========

*by Overkill (Andrew G. Crowell)*

A collection of primitive editor/converter tools for making assets used with homebrew ROM development, written in C++ and Qt.

These are going to be mashed together pretty haphazardly. The goal is low-effort UI that just gets the job done.

Contents
--------

Here are the tools in this collection so far:

* chrbrew: Convert image files into binary tile/character sets.
* palbrew: Make 15-bit RGB color palettes.
* spritebrew: Create metasprites, comprised of several hardware sprite parts, which reference palette and chr data.
* tilebrew: Create tileset of metatiles, comprised of 2x2 blocks of hardware tiles, which reference palette and chr data.
* textbrew: Convert text snippets into a tile encoding (with control characters) that can be printed to screen.
* databrew: Create tables of data, which can be stored as binary blobs. Spreadsheet style.
* worldbrew: Stitch some assets together: a tilemap, a tileset, some sprites, probably other stuff. Output some program code that will embed all the assets.

SOME OTHER IDEAS MAYBE

* Use tiled for making maps with the metatiles, single tile layer. Maybe object layers for metadata, like entity placement.