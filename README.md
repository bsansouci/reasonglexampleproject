# ReasonglExampleProject
More complex Example project using `ReasonGL`.

This is a big example project showcasing how to:
- render text, textures and rectangles in GL
- handle events
- layout things using [Relayout](https://github.com/jordwalke/ReLayout)

This only builds on native currently because it depends on [camlimages](https://bitbucket.org/camlspotter/camlimages) which is a library that binds to the C lib freetype.

To install run `npm install`, then make sure you have [opam](https://opam.ocaml.org/doc/Install.html) and install `camlimages` (`opam install camlimages`).

To build `npm run build` and to start `npm run start`.

There's a full version of Relayout checked in the repo.
