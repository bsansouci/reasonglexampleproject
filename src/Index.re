/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Load (Font: FontType.t) => {
  module Layout = Draw.Layout;
  module Node = Draw.Node;
  module Font = Font;
  module MainComponent = MainComponent.Load Font;
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
  external magicalPoneys : 'a => 'b = "%identity";
  let render time => {
    /* Remember to clear the clear at each tick */
    Draw.clearScreen ();

    /** Magical src/MainComponent.re has hot-reloading hooked up */
    /*switch !Hotreloader.p {
      | Some s =>
        module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
        M.render time;
        ()
      | None => ()
      };*/
    MainComponent.M.render time;

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
      int_of_float (
        List.fold_left (fun acc v => v < acc ? v : acc) 60. !lastCoupleOfFrames +. 0.5
      );
    ignore @@
    Draw.drawTextImmediate
      12. 20. ("fps: " ^ string_of_int fpscount) outContext::fpsTextData Draw.black font12
    /** @Hack For hotreloading. We get the previous module's state and set it on the new module
        loaded. Also relies on mutation! But shrug. */
    /*switch !Hotreloader.p {
      | Some s =>
        module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
        let state = M.appState;
        if (Hotreloader.checkRebuild ()) {
          switch !Hotreloader.p {
          | Some s =>
            module M2 = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
            /* We know the types align, but the type checker's dubious. So we pet it a little. */
            M2.setAppState (magicalPoneys M.appState);
            ()
          | None => ()
          }
        }
      | None => ignore @@ Hotreloader.checkRebuild ()
      }*/
  };
  let mouseMove ::x ::y => MainComponent.M.mouseMove ::x ::y;
  /*switch !Hotreloader.p {
    | Some s =>
      module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
      M.mouseMove ::x ::y
    | None => ()
    };*/
  let mouseDown ::button ::state ::x ::y => MainComponent.M.mouseDown ::button ::state ::x ::y;
  /*switch !Hotreloader.p {
    | Some s =>
      module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
      M.mouseDown ::button ::state ::x ::y
    | None => ()
    };*/
  let mouseUp ::button ::state ::x ::y => MainComponent.M.mouseUp ::button ::state ::x ::y;
  /*switch !Hotreloader.p {
    | Some s =>
      module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
      M.mouseUp ::button ::state ::x ::y
    | None => ()
    };*/
  let windowResize () => Draw.resizeWindow ();
  let keyDown ::keycode ::repeat => MainComponent.M.keyDown ::keycode ::repeat;
  /*switch !Hotreloader.p {
    | Some s =>
      module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
      M.keyDown ::keycode ::repeat
    | None => ()
    };*/
  let keyUp ::keycode => MainComponent.M.keyUp ::keycode;
  /*switch !Hotreloader.p {
    | Some s =>
      module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
      M.keyUp ::keycode
    | None => ()
    };*/

  /** Start the render loop. **/
  Draw.render ::keyUp ::keyDown ::windowResize ::mouseMove ::mouseDown ::mouseUp render;
};
