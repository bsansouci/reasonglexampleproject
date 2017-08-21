module Layout = Draw.Layout;

module type DYNAMIC_MODULE = {let render: unit => Layout.node;};

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

let ocamlPath =
  if (Buffer.contents buf == "") {
    "node_modules/bs-platform/vendor/ocaml/ocamlc"
  } else {
    "ocamlc"
  };
