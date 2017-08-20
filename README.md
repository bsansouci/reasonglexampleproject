# ReasonGL Full Example
More complex Example project using `ReasonGL`.

This project is showing how to:
- render text, textures and arbitrary geometry in GL
- handle events
- layout things using [Relayout](https://github.com/jordwalke/ReLayout)

This only builds on native currently because it depends on [camlimages](https://bitbucket.org/camlspotter/camlimages) which is a library that binds to the C lib freetype. You'll need to install `camlimages` through [opam](https://opam.ocaml.org/doc/Install.html) (`opam install camlimages`) and you'll need to install `freetype` through homebrew (`brew install freetype`).

To install all the non-global deps, run `npm install`.

To build `npm run build` and to start `npm run start`.

There's a full version of Relayout checked in the repo.
