/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Layout = Draw.Layout;

module Node = Draw.Node;

module Font = Font;

let ocamlPath = Hotreloader.ocamlPath;

Random.init 0;

let font12 = Font.loadFont fontSize::12. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let lastCoupleOfFrames = ref [];

let fpsTextData =
  Draw.drawTextImmediate
    12.
    20.
    (
      "fps: " ^
      string_of_int (
        int_of_float (
          List.fold_left (+.) 0. !lastCoupleOfFrames /. (
            float_of_int @@ List.length !lastCoupleOfFrames
          ) +. 0.5
        )
      )
    )
    Draw.black
    font12;

let render time => {
  /* Remember to clear the clear at each tick */
  Draw.clearScreen ();

  /** Magical src/Child1.re has hot-reloading hooked up */
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.render time
  | None => ()
  };

  /** Happy FPS counter that smoothes things out. */
  let fps = 1000. /. time;
  if (List.length !lastCoupleOfFrames > 10) {
    switch !lastCoupleOfFrames {
    | [_, ...rest] => lastCoupleOfFrames := rest @ [fps]
    | _ => assert false
    }
  } else {
    lastCoupleOfFrames := !lastCoupleOfFrames @ [fps]
  };
  let fpscount =
    int_of_float (List.fold_left (fun acc v => v < acc ? v : acc) 60. !lastCoupleOfFrames +. 0.5);
  ignore @@
  Draw.drawTextImmediate
    12. 20. ("fps: " ^ string_of_int fpscount) mutableThing::fpsTextData Draw.black font12;

  /** @Hack For hotreloading. */
  Hotreloader.checkRebuild ()
};

let mouseMove ::x ::y =>
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.mouseMove ::x ::y
  | None => ()
  };

let mouseDown ::button ::state ::x ::y =>
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.mouseDown ::button ::state ::x ::y
  | None => ()
  };

let mouseUp ::button ::state ::x ::y =>
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.mouseUp ::button ::state ::x ::y
  | None => ()
  };

let windowResize () => Draw.resizeWindow ();

let keyDown ::keycode ::repeat =>
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.keyDown ::keycode ::repeat
  | None => ()
  };

let keyUp ::keycode =>
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.keyUp ::keycode
  | None => ()
  };


/** Start the render loop. **/
Draw.render ::keyUp ::keyDown ::windowResize ::mouseMove ::mouseDown ::mouseUp render;
