module type DYNAMIC_MODULE = {
  let render: float => unit;
  let keyDown: keycode::Draw.Events.keycodeT => repeat::bool => unit;
  let keyUp: keycode::Draw.Events.keycodeT => unit;
  let mouseMove: x::int => y::int => unit;
  let mouseDown:
    button::Draw.Events.buttonStateT => state::Draw.Events.stateT => x::int => y::int => unit;
  let mouseUp:
    button::Draw.Events.buttonStateT => state::Draw.Events.stateT => x::int => y::int => unit;
};

let p = ref None;

let get_plugin () :(module DYNAMIC_MODULE) =>
  switch !p {
  | Some s => s
  | None => failwith "No plugin loaded"
  };

let load_plug fname => {
  let fname = Dynlink.adapt_filename fname;
  if (Sys.file_exists fname) {
    try (Dynlink.loadfile fname) {
    | Dynlink.Error err as e =>
      print_endline ("ERROR loading plugin: " ^ Dynlink.error_message err);
      raise e
    | _ => failwith "Unknow error while loading plugin"
    }
  } else {
    failwith "Plugin file does not exist"
  }
};

let (ic, oc) = Unix.open_process "which ocamlc";

let buf = Buffer.create 64;

try (
  while true {
    Buffer.add_channel buf ic 1
  }
) {
| End_of_file => ()
};

Unix.close_process (ic, oc);

let last_st_mtime = ref 0.;

let ocaml = Dynlink.is_native ? "ocamlopt" : "ocamlc";

let extension = Dynlink.is_native ? "cmxs" : "cmo";

let shared = Dynlink.is_native ? "-shared" : "-c";

let folder = Dynlink.is_native ? "native" : "bytecode";

let ocamlPath =
  if (Buffer.contents buf == "") {
    "node_modules/bs-platform/vendor/ocaml/" ^ ocaml
  } else {
    ocaml
  };

let checkRebuild () => {
  let {Unix.st_mtime: st_mtime} = Unix.stat "src/Child1.re";
  if (st_mtime > !last_st_mtime) {
    print_endline "Rebuilding hotloaded module";
    /* @Incomplete Check the error code sent back. Also don't do this, use stdout/stderr. */
    let _ =
      Unix.system @@
      Printf.sprintf
        "%s %s -w -40 -I lib/bs/%s/src -I node_modules/ReasonglInterface/lib/bs/%s/src  -I lib/bs/%s/vendor/ReLayout/src -pp './node_modules/bs-platform/bin/refmt.exe --print binary' -o lib/bs/%s/src/Child1.%s -impl src/Child1.re 2>&1"
        ocamlPath
        shared
        folder
        folder
        folder
        folder
        extension;
    /*Unix.system "./node_modules/.bin/bsb";*/
    load_plug @@ Printf.sprintf "lib/bs/%s/src/Child1.%s" folder extension;
    last_st_mtime := st_mtime;
    print_endline "----------------------"
  }
};
