/*

  What is this?

  This is a more interesting and solid demo project to showcase the following technologies working together:
  - ReLayout (flexbox subset implemented in Reason)
  - ReasonGL (thin cross platform bindings to GL)
  - text rendering (small readable implementation)
  - hot reloading (currently all commented out because we haven't written the JS version. It still
    works in native if you comment things back in)

  So this is a demo, but a very realistic one. We're render 1000 UI elements on screen (1000 rectangles and 1000 pieces of text each containing a lot more rectangles), we're computing the whole layout each time a brick is destroyed and doing a bunch of vector math.

  There are many many more optimizations that could be made and we'd love help with that.


          Ben - September 5th 2017
 */
/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Load (Font: FontType.t) => {
  module Layout = Draw.Layout;
  module Node = Draw.Node;
  module Font = Font;
  module MainComponent = MainComponent.Load Font;

  /** */
  Random.init 0;

  /** */
  let font12 = Font.loadFont fontSize::24. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

  /** List of fps of previous frames used to */
  let lastCoupleOfFrames = ref [];
  let fpsTextData = Draw.drawTextImmediate 12. 20. "fps:60" Draw.black font12;

  /** Commented out type magic used for hot-reloading in native dev. */
  /*external magicalPoneys : 'a => 'b = "%identity";*/

  /** Main function called 60 times a second. */
  let render time => {
    /* Remember to clear the clear at each tick */
    Draw.clearScreen ();

    /** Magical hotreloading sauce. Currently commented out as explained at the top of the file */
    /*switch !Hotreloader.p {
      | Some s =>
        module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
        M.render time;
        ()
      | None => ()
      };*/

    /** Main render call */
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
      12. 20. ("fps:" ^ string_of_int fpscount) outContext::fpsTextData Draw.black font12
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

  /** event handlers calling MainComponent directly instead of using the Hotreloader because that
      doesn't work in JS right now. */
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
