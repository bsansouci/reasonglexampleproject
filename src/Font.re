/* no idea what this is */
let csize = 100;

let debug = false;

let loadFont ::fontSize=24. ::fontPath ::id => {
  if debug {
    prerr_endline (Printf.sprintf "Processing %s" fontPath);
    prerr_endline "opening font..."
  };
  let face = (new OFreetype.face) fontPath id;
  face#set_char_size fontSize fontSize csize csize;
  /*List.iter
    (
      fun cmap =>
        prerr_endline (
          Printf.sprintf
            "charmap: { platform_id = %d; encoding_id = %d}"
            cmap.platform_id
            cmap.encoding_id
        )
    )
    face#charmaps;*/
  try (face#set_charmap Freetype.{platform_id: 3, encoding_id: 1}) {
  | _ =>
    try (face#set_charmap Freetype.{platform_id: 3, encoding_id: 0}) {
    | _ => face#set_charmap (List.hd face#charmaps)
    }
  };
  face
};
