type refbox 'a 'b = {
  cont: 'a,
  ref: ref 'b
};

type t = refbox Ftlow.library unit;

let library = {cont: Ftlow.init (), ref: ref ()};

let done_face face => Ftlow.done_face face.cont;

Gc.finalise (fun v => Ftlow.close v.cont) library;

let new_face font idx => {
  let face = {cont: Ftlow.new_face library.cont font idx, ref: ref library};
  let info = Ftlow.face_info face.cont;
  Gc.finalise done_face face;
  (face, info)
};

let intfrac_of_float dotbits f => {
  let d = float (1 lsl dotbits);
  truncate (f *. d)
};

let intfrac6_of_float = intfrac_of_float 6;

let set_char_size face char_w char_h res_h res_v =>
  Ftlow.set_char_size face.cont (intfrac6_of_float char_w) (intfrac6_of_float char_h) res_h res_v;

let set_charmap face charmap => Ftlow.set_charmap face.cont charmap;

/* no idea what this is */
let csize = 100;

let debug = false;

let allCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+1234567890-={}|:\"<>?[]\\;',./ ";

/*let allCharacters = "AB";*/
let unicode_of_latin s => Array.init (String.length s) @@ (fun i => Char.code s.[i]);

let allCharactersEncoded = unicode_of_latin allCharacters;

let loadFont ::fontSize=24. ::fontPath ::id => {
  if debug {
    prerr_endline (Printf.sprintf "Processing %s" fontPath);
    prerr_endline "opening font..."
  };
  let (face, _info) = new_face fontPath id;
  set_char_size face fontSize fontSize csize csize;
  set_charmap face Freetype.{platform_id: 3, encoding_id: 1};
  let texLength = 2048;
  let bigarrayTextData =
    Draw.Gl.Bigarray.create Draw.Gl.Bigarray.Uint8 (texLength * texLength * 4);
  /* @Incomplete @Hack fill bigarray with 0s. We need to add Bigarray.fill to reasongl-interface
     to get this to work. */
  /*for i in 0 to (texLength * texLength - 1) {
      Draw.Gl.Bigarray.set bigarrayTextData (i * 4 + 0) 255;
      Draw.Gl.Bigarray.set bigarrayTextData (i * 4 + 1) 255;
      Draw.Gl.Bigarray.set bigarrayTextData (i * 4 + 2) 255;
      Draw.Gl.Bigarray.set bigarrayTextData (i * 4 + 3) 255
    };*/
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 0 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 1 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 2 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 3 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 4 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 5 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 6 255;
  Draw.Gl.Bigarray.unsafe_set bigarrayTextData 7 255;
  let prevX = ref 4;
  let prevY = ref 0;
  let nextY = ref 0;
  let chars = ref Draw.IntMap.empty;
  let kerningMap = ref Draw.IntPairMap.empty;
  let {Ftlow.has_kerning: has_kerning} = Ftlow.face_info face.cont;
  Array.iter
    (
      fun c => {
        ignore @@ Ftlow.render_char_raw face.cont c 0 Ftlow.Render_Normal;
        let glyphMetrics = Ftlow.get_glyph_metrics face.cont;
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
        prevX := !prevX + bitmapInfo.bitmap_width + 2;
        nextY := max !nextY (!prevY + bitmapInfo.bitmap_height) + 2
      }
    )
    allCharactersEncoded;
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
  {
    Draw.chars: !chars,
    textureBuffer,
    textureWidth: float_of_int texLength,
    textureHeight: float_of_int texLength,
    kerning: !kerningMap
  }
};
