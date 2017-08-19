let csize = 100.0;

let debug = false;

let loadFont font => {
  if debug {
    prerr_endline (Printf.sprintf "Processing %s" font);
    prerr_endline "opening font..."
  };
  let face = (new OFreetype.face) font 0;
  face#set_char_size csize csize 24 24;
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
