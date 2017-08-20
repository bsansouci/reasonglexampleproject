# ReasonGL Full Example
More complex Example project using `ReasonGL`.

This project is showing how to:
- render text, textures and arbitrary geometry in GL
- handle events
- layout things using [Relayout](https://github.com/jordwalke/ReLayout)

This only builds on native currently because it depends on [camlimages](https://bitbucket.org/camlspotter/camlimages) which is a library that binds to the C lib freetype. Everything needed is vendored inside the repo so we don't depend on opam.

To install run `npm install`.

To build `npm run build` and to start `npm run start`.

There's a full version of Relayout checked in the repo.
