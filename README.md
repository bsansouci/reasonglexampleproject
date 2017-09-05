# ReasonGL Full Example
More complex Example project using `ReasonGL`.

This project is showing how to:
- render text, textures and arbitrary geometry in GL
- handle events
- layout things using [Relayout](https://github.com/jordwalke/ReLayout)

Everything needed is vendored inside the repo so we don't depend on opam. We're using the C library freetype to parse the font file in native/bytecode version and using opentype.js for the web versio.

To install run `npm install`.

## Build

To build to js, run `npm run build` and to start `npm run start`. That'll spawn a simple python static server, just go to `localhost:8000` in your browser.

To build to bytecode, run `npm run build:bytecode` and to start `npm run start:bytecode`. This uses `ocamlc` which compiles to a bytecode representation ran in the ocaml VM.

To build to native, run `npm run build:native` and to start `npm run start:native`.
Native should be about 10x faster than bytecode at the expense of compilation speed.

There's a full version of Relayout checked in the repo, as well as freetype and a patched version of camlimages
