/* Commented out because it depends on Dynlink. To have that you need a global install of ocaml.
   If you'd like to try it, all you need is to install ocaml 4.02.3 using opam. Then uncomment
   the `ocamlfind-dependencies` in the bsconfig.json file. You can now build.
   */
module Layout = Draw.Layout;

module type DYNAMIC_MODULE = {let render: unit => Layout.node;};

let p = ref None;

let get_plugin () :(module DYNAMIC_MODULE) =>
  switch !p {
  | Some s => s
  | None => failwith "No plugin loaded"
  };
/*
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
 */
