type fontT = Js.t {. unitsPerEm : int};

external load : string => ('a => fontT => unit) => unit = "load" [@@bs.module "opentype.js"];

type contextT;

type dataT;

external getImageData : contextT => int => int => int => int => dataT = "getImageData" [@@bs.send];

external getData : dataT => array int = "data" [@@bs.get];

let getContext: unit => contextT = [%bs.raw
  {| function() {
  if (!this.canvas) {
    this.canvas = document.createElement('canvas');
    this.canvas.width = 2048;
    this.canvas.height = 2048;
  }

  let context = this.canvas.getContext('2d');
  context.clearRect(0, 0, this.canvas.width, this.canvas.height);
  return context;
}
|}
];

external drawChar : fontT => contextT => char => int => int => float => unit = "draw" [@@bs.send];

external drawText : fontT => contextT => string => int => int => float => unit =
  "draw" [@@bs.send];

external getAdvanceWidth : fontT => int => float => int = "getAdvanceWidth" [@@bs.send];

/*type glyphSetT;*/
/*external getAllGlyphs : fontT => glyphSetT = "glyphs" [@@bs.get] ;*/
/*type pathT;*/
type glyphT = Js.t {. xMin : int, yMin : int, xMax : int, yMax : int, advanceWidth : int};

external getUnicode : glyphT => option int = "unicode" [@@bs.get] [@@bs.return undefined_to_opt];

type boundingBoxT = Js.t {. x1 : int, y1 : int, x2 : int, y2 : int};

external getGlyphBoundingBox : glyphT => boundingBoxT = "getBoundingBox" [@@bs.send];

external getNumberOfGlyphs : fontT => int = "length" [@@bs.get] [@@bs.scope "glyphs"];

external getGlyph : fontT => int => glyphT = "" [@@bs.get_index] [@@bs.scope ("glyphs", "glyphs")];

external drawGlyph : glyphT => contextT => int => int => int => unit = "draw" [@@bs.send];

external charToGlyph : fontT => string => glyphT = "" [@@bs.send];

/*type pathT;

  external getPath : glyphT => int => int => float => pathT = "getPath" [@@bs.send];

  external drawPath : pathT => contextT => unit = "draw" [@@bs.send];

  external getPathBoundingBox : pathT => boundingBoxT = "getBoundingBox" [@@bs.send];
  */
let allCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+1234567890-={}|:\"<>?[]\\;',./ ";

/*let allCharacters = "AB";*/
/*let unicode_of_latin s => Array.init (String.length s) @@ (fun i => Char.code s.[i]);*/
/*let allCharactersEncoded = unicode_of_latin allCharacters;*/
let loadFont ::fontSize ::fontPath id::(_: int) => {
  let prevX = ref 4;
  let prevY = ref 0;
  let nextY = ref 0;
  let texLength = 2048;
  let chars = ref Draw.IntMap.empty;
  let kerningMap = ref Draw.IntPairMap.empty;
  let maxHeight = ref 0;
  let maxWidth = ref 0;
  let bigarrayTextData =
    Draw.Gl.Bigarray.create Draw.Gl.Bigarray.Uint8 (texLength * texLength * 4);
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 0 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 1 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 2 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 3 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 4 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 5 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 6 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 7 255;
  let ret = ref None;
  load
    fontPath
    (
      fun err font =>
        {
          let fontSize = int_of_float fontSize;
          let scale = font##unitsPerEm;
          let context = getContext ();
          String.iter
            (
              fun c => {
                let g = charToGlyph font (String.make 1 c);
                switch (getUnicode g) {
                | None => ()
                | Some unicode =>
                  let bboxwidth = (g##xMax - g##xMin) * fontSize / scale;
                  let bboxheight = (g##yMax - g##yMin) * fontSize / scale;
                  if (bboxwidth !== 0 && bboxheight !== 0) {
                    maxHeight := max !maxHeight bboxheight;
                    maxWidth := max !maxWidth bboxwidth;
                    let advanceWidth = g##advanceWidth * fontSize / scale;
                    if (!prevX + bboxwidth >= texLength) {
                      prevX := 4;
                      prevY := !nextY
                    };
                    drawGlyph g context !prevX !prevY fontSize;
                    chars :=
                      Draw.IntMap.add
                        unicode
                        Draw.{
                          atlasX: float_of_int !prevX,
                          atlasY: float_of_int !prevY,
                          height: float_of_int bboxheight,
                          width: float_of_int bboxwidth,
                          bearingX: float_of_int 0,
                          bearingY: float_of_int 0,
                          advance: float_of_int advanceWidth
                        }
                        !chars;
                    let allData =
                      getData (getImageData context !prevX !prevY bboxwidth bboxheight);
                    for baIndex in 0 to ((Array.length allData - 1) / 4) {
                      Draw.Bigarray.unsafe_set bigarrayTextData (4 * baIndex) 255;
                      Draw.Bigarray.unsafe_set bigarrayTextData (4 * baIndex + 1) 255;
                      Draw.Bigarray.unsafe_set bigarrayTextData (4 * baIndex + 2) 255;
                      Draw.Bigarray.unsafe_set
                        bigarrayTextData (4 * baIndex + 3) allData.(baIndex + 3)
                    };
                    prevX := !prevX + bboxwidth + 10;
                    nextY := max !nextY (!prevY + bboxheight) + 10
                  }
                | _ => Js.log "Skipping"
                }
              }
            )
            allCharacters;
          let textureBuffer = Draw.Gl.createTexture Draw.context;
          Draw.Gl.bindTexture
            context::Draw.context target::Draw.Constants.texture_2d texture::textureBuffer;
          Draw.Gl.texImage2D_RGBA
            context::Draw.context
            target::Draw.Constants.texture_2d
            level::0
            width::texLength
            height::texLength
            border::0
            data::bigarrayTextData;
          Draw.Gl.texParameteri
            context::Draw.context
            target::Draw.Constants.texture_2d
            pname::Draw.Constants.texture_mag_filter
            param::Draw.Constants.linear;
          Draw.Gl.texParameteri
            context::Draw.context
            target::Draw.Constants.texture_2d
            pname::Draw.Constants.texture_min_filter
            param::Draw.Constants.linear;
          Draw.Gl.texParameteri
            context::Draw.context
            target::Draw.Constants.texture_2d
            pname::Draw.Constants.texture_wrap_s
            param::Draw.Constants.clamp_to_edge;
          Draw.Gl.texParameteri
            context::Draw.context
            target::Draw.Constants.texture_2d
            pname::Draw.Constants.texture_wrap_t
            param::Draw.Constants.clamp_to_edge;
          ret :=
            Some {
              Draw.chars: !chars,
              textureBuffer,
              textureWidth: float_of_int texLength,
              textureHeight: float_of_int texLength,
              kerning: !kerningMap,
              maxHeight: float_of_int !maxHeight,
              maxWidth: float_of_int !maxWidth
            }
        }
        /*Array.iter
          (
            fun c => {
              /*Font.draw (ctx, text, x, y, fontSize, options)*/
              drawChar font c !prevX !prevY fontSize;

              ignore @@ Ftlow.render_char_raw face.cont c 0 Ftlow.Render_Normal;
                let glyphMetrics = Ftlow.get_glyph_metrics face.cont;
                let bbox = Ftlow.glyph_get_bbox face.cont;
                maxHeight := max !maxHeight ((bbox.ymax - bbox.ymin) / 64);
                maxWidth := max !maxWidth ((bbox.xmax - bbox.xmin) / 64);
                let bitmapInfo = Ftlow.get_bitmap_info face.cont;
                if (!prevX + bitmapInfo.bitmap_width >= texLength) {
                  prevX := 4;
                  prevY := !nextY
                };
                if has_kerning {
                  let code = Ftlow.get_char_index face.cont c;
                  Array.iter
                    (
                      fun c2 => {
                        let code2 = Ftlow.get_char_index face.cont c2;
                        let (x, y) = Ftlow.get_kerning face.cont code code2;
                        let (x, y) = (float_of_int x /. 64., float_of_int y /. 64.);
                        if (abs_float x > 0.00001 || abs_float y > 0.00001) {
                          kerningMap := Draw.IntPairMap.add (c, c2) (x, y) !kerningMap
                        }
                      }
                    )
                    allCharactersEncoded
                };
                chars :=
                  Draw.IntMap.add
                    c
                    Draw.{
                      atlasX: float_of_int !prevX,
                      atlasY: float_of_int !prevY,
                      height: float_of_int bitmapInfo.bitmap_height,
                      width: float_of_int bitmapInfo.bitmap_width,
                      bearingX: float_of_int glyphMetrics.gm_hori.bearingx /. 64.,
                      bearingY: float_of_int glyphMetrics.gm_hori.bearingy /. 64.,
                      advance: float_of_int glyphMetrics.gm_hori.advance /. 64.
                    }
                    !chars;
                for y in 0 to (bitmapInfo.bitmap_height - 1) {
                  for x in 0 to (bitmapInfo.bitmap_width - 1) {
                    let level = Ftlow.read_bitmap face.cont x y;
                    let y = bitmapInfo.bitmap_height - y;
                    let baIndex = (y + !prevY) * texLength + (x + !prevX);
                    Draw.Gl.Bigarray.unsafe_set bigarrayTextData (4 * baIndex) 255;
                    Draw.Gl.Bigarray.unsafe_set bigarrayTextData (4 * baIndex + 1) 255;
                    Draw.Gl.Bigarray.unsafe_set bigarrayTextData (4 * baIndex + 2) 255;
                    Draw.Gl.Bigarray.unsafe_set bigarrayTextData (4 * baIndex + 3) level
                  }
                };
                prevX := !prevX + bitmapInfo.bitmap_width + 10;
                nextY := max !nextY (!prevY + bitmapInfo.bitmap_height) + 10
          })
          allCharactersEncoded*/
    );
  ret
  /*opentype.load('fonts/Roboto-Black.ttf', function(err, font) {
        if (err) {
             alert('Font could not be loaded: ' + err);
        } else {
            var ctx = document.getElementById('canvas').getContext('2d');
            // Construct a Path object containing the letter shapes of the given text.
            // The other parameters are x, y and fontSize.
            // Note that y is the position of the baseline.
            var path = font.getPath('Hello, World!', 0, 150, 72);
            // If you just want to draw the text you can also use font.draw(ctx, text, x, y, fontSize).
            path.draw(ctx);
        }
    });*/
};