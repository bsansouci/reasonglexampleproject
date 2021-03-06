type fontT = {. "unitsPerEm": int, "ascender": int, "descender": int};

[@bs.module "opentype.js"] external load : (string, ('a, fontT) => unit) => unit = "load";

type contextT;

type dataT;

[@bs.send] external getImageData : (contextT, int, int, int, int) => dataT = "getImageData";

[@bs.get] external getData : dataT => array(int) = "data";

let getContext: unit => contextT = [%bs.raw
  {| function(_) {
    let canvas = document.createElement('canvas');
    // document.body.appendChild(canvas);
    canvas.width = 2048;
    canvas.height = 2048;

  let context = canvas.getContext('2d');
  context.clearRect(0, 0, canvas.width, canvas.height);
  return context;
}
|}
];

[@bs.send] external drawChar : (fontT, contextT, char, int, int, float) => unit = "draw";

[@bs.send] external drawText : (fontT, contextT, string, int, int, float) => unit = "draw";

[@bs.send] external getAdvanceWidth : (fontT, int, float) => int = "getAdvanceWidth";

/*type glyphSetT;*/
/*external getAllGlyphs : fontT => glyphSetT = "glyphs" [@@bs.get] ;*/
/*type pathT;*/
type glyphT = {. "xMin": int, "yMin": int, "xMax": int, "yMax": int, "advanceWidth": int};

[@bs.get] [@bs.return undefined_to_opt] external getUnicode : glyphT => option(int) = "unicode";

type boundingBoxT = {. "x1": int, "y1": int, "x2": int, "y2": int};

[@bs.send] external getGlyphBoundingBox : glyphT => boundingBoxT = "getBoundingBox";

[@bs.get] [@bs.scope "glyphs"] external getNumberOfGlyphs : fontT => int = "length";

[@bs.get_index] [@bs.scope ("glyphs", "glyphs")] external getGlyph : (fontT, int) => glyphT = "";

[@bs.send] external drawGlyph : (glyphT, contextT, int, int, int) => unit = "draw";

[@bs.send] external charToGlyph : (fontT, string) => glyphT = "";

[@bs.send] external getKerningValue : (fontT, glyphT, glyphT) => int = "";

type pathT;

[@bs.send] external getPath : (glyphT, int, int, int) => pathT = "getPath";

[@bs.send] external drawPath : (pathT, contextT) => unit = "draw";

[@bs.send] external getPathBoundingBox : pathT => boundingBoxT = "getBoundingBox";

let allCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+1234567890-={}|:\"<>?[]\\;',./ ";

let loadFont = (~fontSize, ~fontPath, ~id as _: int) => {
  let prevX = ref(4);
  let prevY = ref(0);
  let nextY = ref(0);
  let texLength = 2048;
  let chars = ref(Draw.IntMap.empty);
  let kerningMap = ref(Draw.IntPairMap.empty);
  let maxHeight = ref(0);
  let maxWidth = ref(0);
  let bigarrayTextData =
    Draw.Gl.Bigarray.create(Draw.Gl.Bigarray.Uint8, texLength * texLength * 4);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 0, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 1, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 2, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 3, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 4, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 5, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 6, 255);
  Draw.Gl.Bigarray.unsafe_set(bigarrayTextData, 7, 255);
  let ret = ref(None);
  load(
    fontPath,
    (err, font) =>
      if (Js.null == err) {
        let fontSize = int_of_float(fontSize);
        let scale = font##unitsPerEm;
        let ascender = font##ascender * fontSize / scale;
        let descender = font##descender * fontSize / scale;
        let bboxheight = ascender - descender;
        let context = getContext();
        String.iter(
          (c) => {
            let cString = String.make(1, c);
            let g = charToGlyph(font, cString);
            switch (getUnicode(g)) {
            | None => ()
            | Some(unicode) =>
              let path = getPath(g, prevX^, prevY^ + ascender, fontSize);
              let bbox = getPathBoundingBox(path);
              let advanceWidth = g##advanceWidth * fontSize / scale;
              let bboxwidth = bbox##x2 - bbox##x1 + advanceWidth;
              if (bboxwidth !== 0 && bboxheight !== 0) {
                maxHeight := max(maxHeight^, bbox##y2 - bbox##y1);
                maxWidth := max(maxWidth^, bboxwidth);
                /* Draw shifted downwards by `qscender` because that draw function is from the
                   baseline */
                drawPath(path, context);
                String.iter(
                  (c2) => {
                    let c2String = String.make(1, c2);
                    let g2 = charToGlyph(font, c2String);
                    let x = getKerningValue(font, g, g2);
                    let x = float_of_int @@ x * fontSize / scale;
                    if (abs_float(x) > 0.00001) {
                      kerningMap :=
                        Draw.IntPairMap.add((Char.code(c), Char.code(c2)), (x, 0.), kerningMap^)
                    }
                  },
                  allCharacters
                );
                chars :=
                  Draw.IntMap.add(
                    unicode,
                    Draw.{
                      atlasX: float_of_int(prevX^),
                      atlasY: float_of_int(prevY^),
                      height: float_of_int(bboxheight),
                      width: float_of_int(bboxwidth),
                      bearingX: float_of_int(0),
                      bearingY: float_of_int(ascender),
                      advance: float_of_int(advanceWidth)
                    },
                    chars^
                  );
                let allData =
                  getData(getImageData(context, prevX^, prevY^, bboxwidth, bboxheight));
                for (y in 0 to bboxheight - 1) {
                  for (x in 0 to bboxwidth - 1) {
                    let level = allData[4 * (y * bboxwidth + x) + 3];
                    let baIndex = (y + prevY^) * texLength + (x + prevX^);
                    Draw.Bigarray.unsafe_set(bigarrayTextData, 4 * baIndex, 255);
                    Draw.Bigarray.unsafe_set(bigarrayTextData, 4 * baIndex + 1, 255);
                    Draw.Bigarray.unsafe_set(bigarrayTextData, 4 * baIndex + 2, 255);
                    Draw.Bigarray.unsafe_set(bigarrayTextData, 4 * baIndex + 3, level)
                  }
                };
                prevX := prevX^ + bboxwidth + 10;
                nextY := max(nextY^, prevY^ + bboxheight);
                if (prevX^ + bboxwidth >= texLength) {
                  prevX := 4;
                  prevY := nextY^ + 10
                }
              } else {
                Js.log(("Skipping", cString, bboxwidth, bboxheight))
              }
            }
          },
          allCharacters
        );
        let textureBuffer = Draw.Gl.createTexture(Draw.context);
        Draw.Gl.bindTexture(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~texture=textureBuffer
        );
        Draw.Gl.texImage2D_RGBA(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~level=0,
          ~width=texLength,
          ~height=texLength,
          ~border=0,
          ~data=bigarrayTextData
        );
        Draw.Gl.texParameteri(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~pname=Draw.Constants.texture_mag_filter,
          ~param=Draw.Constants.linear
        );
        Draw.Gl.texParameteri(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~pname=Draw.Constants.texture_min_filter,
          ~param=Draw.Constants.linear
        );
        Draw.Gl.texParameteri(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~pname=Draw.Constants.texture_wrap_s,
          ~param=Draw.Constants.clamp_to_edge
        );
        Draw.Gl.texParameteri(
          ~context=Draw.context,
          ~target=Draw.Constants.texture_2d,
          ~pname=Draw.Constants.texture_wrap_t,
          ~param=Draw.Constants.clamp_to_edge
        );
        ret :=
          Some({
            Draw.chars: chars^,
            textureBuffer,
            textureWidth: float_of_int(texLength),
            textureHeight: float_of_int(texLength),
            kerning: kerningMap^,
            maxHeight: float_of_int(maxHeight^),
            maxWidth: float_of_int(maxWidth^)
          })
      } else {
        Js.log(err)
      }
  );
  ret
};
