module Load (Font: FontType.t) => {
  module Layout = Draw.Layout;
  module Node = Draw.Node;

  /** */
  let font7 = Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;
  let font48 = Font.loadFont fontSize::48. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;
  let font38 = Font.loadFont fontSize::38. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;
  /*type kindT =
      | Text
      | Rect;

    type viewType = {
      style: Layout.cssStyle,
      children: list viewType,
      kind: kindT,
      color: (float, float, float, float),
      text: string
    };*/
  /*let globalRoot = ref None;*/
  /*let rec createHierarchy root => {
      let context =
        switch root.kind {
        | Text => Draw.generateTextContext root.text Draw.white font7
        | Rect => Draw.generateRectContext root.color
        };
      Layout.createNode
        withChildren::(Array.of_list (List.map createHierarchy root.children))
        andStyle::root.style
        context
    };
    */
  /* For anyone looking at this code, this does NOT handle tree differences after the first
     frame.
     Please implement it for me.
              Ben - August 27th 2017
     */
  /*let rec doTheRenderThing (root: viewType) node::(node: option Layout.node)=? () =>
      switch node {
      | None => globalRoot := Some (createHierarchy root)
      | Some node =>
        if (node.Layout.style != root.style) {
          node.Layout.style = root.style
        };
        List.iteri (fun i c => doTheRenderThing c node::node.Layout.children.(i) ()) root.children
      /*if (node.context != root.context) {
          node.context = root.context
        };*/
      /*node.children = root.children*/
      };

    let doTheRenderThing root => {
      let node = !globalRoot;
      doTheRenderThing root ::?node ()
    };
    */

  /** We need to have default style be a function because each node have mutable style. If we just
      used DefaultStyle we'd be mutating the same shared one! */
  let makeDefaultStyle () => Layout.{...defaultStyle, positionType: Relative};
  module View = {
    let createElement ::style=(makeDefaultStyle ()) ::color=Draw.white ::children () =>
      Layout.createNode
        withChildren::(Array.of_list children) andStyle::style (Draw.generateRectContext color);
  };
  module Text = {
    let createElement
        ::style=(makeDefaultStyle ())
        ::color=Draw.white
        ::text=""
        ::font=font7
        ::children
        ::context=?
        () =>
      switch context {
      | None =>
        let context = Draw.generateTextContext text color font;
        /*print_endline @@
          Printf.sprintf "width: %f, height: %f" context.textInfo.width font.maxHeight;*/
        Layout.createNode
          withChildren::(Array.of_list children)
          andStyle::style
          andMeasure::(
            fun _node _width _measureModeWidth _height _measureModeHeight =>
              switch !font {
              | None => {
                  Layout.width: context.textInfo.width /. Draw.pixelScale,
                  height: _height
                }
              | Some {maxHeight} => {
                  Layout.width: context.textInfo.width /. Draw.pixelScale,
                  height: maxHeight /. Draw.pixelScale
                }
              }
          )
          context
      | Some context =>
        Layout.createNode withChildren::(Array.of_list children) andStyle::style context
      };
  };

  /** */
  let defaultColor = (0.6, 0.6, 0.9, 1.);
  let colors = [|
    (1., 1., 0.4, 1.),
    (1., 204. /. 255., 0., 1.),
    (1., 0.6, 0., 1.),
    (1., 0., 0., 1.)
  |];

  /** */
  let fonts = [|
    Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0
    /*Font.loadFont fontSize::9. fontPath::"assets/fonts/Anonymous_Pro.ttf" id::0*/
    /*Font.loadFont fontSize::9. fontPath::"assets/fonts/DroidSansMono.ttf" id::0,*/
    /*Font.loadFont fontSize::24. fontPath::"assets/fonts/Anonymous_Pro.ttf" id::0,*/
    /*Font.loadFont fontSize::28. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0,*/
    /*Font.loadFont fontSize::32. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0*/
  |];

  /** */
  let windowWidth = float_of_int @@ Draw.getWindowWidth ();
  let windowHeight = float_of_int @@ Draw.getWindowHeight ();
  let totalTiles = 1000;
  let tiles =
    Array.init
      totalTiles
      (
        fun i => ("H", fonts.(i / 5 mod Array.length fonts), colors.(i / 5 mod Array.length colors))
      );

  /** */
  let segmentIntersection (x1, y1) (x2, y2) (bx1, by1) (bx2, by2) => {
    let s1_x = x2 -. x1;
    let s1_y = y2 -. y1;
    let s2_x = bx2 -. bx1;
    let s2_y = by2 -. by1;
    let s = (-. s1_y *. (x1 -. bx1) +. s1_x *. (y1 -. by1)) /. (-. s2_x *. s1_y +. s1_x *. s2_y);
    let t = (s2_x *. (y1 -. by1) -. s2_y *. (x1 -. bx1)) /. (-. s2_x *. s1_y +. s1_x *. s2_y);
    if (s >= 0. && s <= 1. && t >= 0. && t <= 1.) {
      true
    } else {
      false
    }
  };

  /** */
  let tileWidth = windowWidth /. 40.;
  let tileHeight = windowHeight /. 80.;
  let tileMargin = windowHeight /. 400.;
  let rootstyle =
    Layout.{
      ...defaultStyle,
      paddingTop: 40.,
      paddingLeft: 40.,
      paddingRight: 40.,
      justifyContent: JustifyFlexStart,
      flexDirection: Row,
      flexWrap: CssWrap,
      marginLeft: 100.,
      width: windowWidth -. 200.,
      height: windowHeight
    };
  let makeChildren () =>
    Array.map
      (
        fun (text, font, color) =>
          <View
            style=Layout.{
                    ...defaultStyle,
                    marginLeft: tileMargin,
                    marginRight: tileMargin,
                    marginTop: tileMargin,
                    marginBottom: tileMargin,
                    width: tileWidth,
                    height: tileHeight,
                    justifyContent: JustifyCenter,
                    alignItems: AlignCenter
                  }
            color>
            <Text text font color=Draw.black />
          </View>
      )
      tiles;
  let elementsNode =
    View.createElement
      style::rootstyle color::defaultColor children::(Array.to_list (makeChildren ())) ();
  let paddleWidth = 60.;
  let paddleSpeed = 9.;
  let paddle =
    <View
      color=Draw.red
      style=Layout.{
              ...defaultStyle,
              positionType: Absolute,
              width: paddleWidth,
              height: 9.,
              top: windowHeight -. 15.,
              left: windowWidth /. 2. -. paddleWidth /. 2.
            }
    />;
  let timerNode =
    <View
      color=defaultColor
      style=Layout.{
              ...defaultStyle,
              positionType: Absolute,
              width: 65.,
              height: 90.,
              justifyContent: JustifyCenter,
              alignItems: AlignCenter
            }>
      <Text text="3" color=(0.3, 0.9, 0.2, 1.) font=font48 />
    </View>;
  let loseNode =
    <View
      color=defaultColor
      style=Layout.{
              ...defaultStyle,
              positionType: Absolute,
              justifyContent: JustifySpaceBetween,
              alignItems: AlignCenter,
              paddingTop: 24.,
              paddingLeft: 24.,
              paddingRight: 24.,
              paddingBottom: 12.
            }>
      <Text text="YOU LOSE </3" color=Draw.red font=font48 />
      <View
        style=Layout.{
                ...defaultStyle,
                justifyContent: JustifyCenter,
                alignItems: AlignCenter,
                paddingLeft: 12.,
                paddingRight: 12.,
                paddingBottom: 12.,
                marginTop: 24.
              }>
        <Text text="restart" color=Draw.green font=font38 />
      </View>
    </View>;
  loseNode.context.visible = false;
  let winNode =
    <View
      color=defaultColor
      style=Layout.{
              ...defaultStyle,
              positionType: Absolute,
              justifyContent: JustifySpaceBetween,
              alignItems: AlignCenter,
              paddingTop: 24.,
              paddingLeft: 24.,
              paddingRight: 24.,
              paddingBottom: 12.
            }>
      <Text text="YOU WIN <3" color=(0.8, 0.2, 0.8, 1.) font=font48 />
    </View>;
  winNode.context.visible = false;
  let root = <View> elementsNode paddle timerNode loseNode winNode </View>;
  /*let lastFrameTime = ref 0.;*/
  /*let alarm =
    Gc.create_alarm (
      fun () =>
        if (!lastFrameTime -. 0.001 > totalTiles. /. 60.) {
          print_endline @@ "------------------------------\nGC: " ^ string_of_float !lastFrameTime
        }
    );*/
  /*let c = Gc.get ();*/
  /*c.verbose = 0x001 + 0x002 + 0x004 + 0x010 + 0x200;*/
  /*c.space_overhead = 800;*/
  /*Gc.set c;*/
  Draw.onWindowResize :=
    Some (
      fun () => {
        let width = float_of_int @@ Draw.getWindowWidth ();
        let height = float_of_int @@ Draw.getWindowHeight ();
        rootstyle.Layout.width = width -. 200.;
        rootstyle.Layout.height = height;
        paddle.style.top = height -. 15.;
        paddle.style.left = width /. 2. -. paddleWidth /. 2.;
        loseNode.style.left = width /. 2. -. loseNode.layout.width /. 2.;
        loseNode.style.top = height /. 2. -. loseNode.layout.height /. 2.;
        winNode.style.left = width /. 2. -. winNode.layout.width /. 2.;
        winNode.style.top = height /. 2. -. winNode.layout.height /. 2.
      }
    );
  type buttonStateT = {
    state: Draw.Events.stateT,
    x: float,
    y: float,
    isClicked: bool
  };
  type vec2 = {
    mutable x: float,
    mutable y: float
  };
  type mouseStateT = {
    mutable pos: vec2,
    mutable leftButton: buttonStateT,
    mutable rightButton: buttonStateT
  };
  type keyboardStateT = {
    mutable leftIsDown: bool,
    mutable rightIsDown: bool
  };
  /*module M: Hotreloader.DYNAMIC_MODULE = {*/
  module M = {
    type stateT = {
      mutable gameover: bool,
      mutable timer: float,
      mutable tiles: array (string, Draw.fontT, (float, float, float, float)),
      mutable ballV: vec2,
      mutable ballPos: vec2,
      mutable keyboard: keyboardStateT,
      mutable mouseState: mouseStateT
    };
    let appState = {
      gameover: false,
      timer: 3000.,
      tiles,
      ballV: {x: 2., y: 2.},
      ballPos: {x: windowWidth /. 2. -. 10., y: windowHeight -. 50.},
      keyboard: {leftIsDown: false, rightIsDown: false},
      mouseState: {
        pos: {x: 0., y: 0.},
        leftButton: {state: Draw.Events.MouseUp, x: 0., y: 0., isClicked: false},
        rightButton: {state: Draw.Events.MouseUp, x: 0., y: 0., isClicked: false}
      }
    };
    /* @Hack for hot reloading. */
    let setAppState newAppState => {
      appState.gameover = newAppState.gameover;
      appState.timer = newAppState.timer;
      appState.tiles = newAppState.tiles;
      appState.ballV = newAppState.ballV;
      appState.ballPos = newAppState.ballPos;
      appState.keyboard = newAppState.keyboard;
      appState.mouseState.pos = newAppState.mouseState.pos;
      appState.mouseState.leftButton = newAppState.mouseState.leftButton;
      appState.mouseState.rightButton = newAppState.mouseState.rightButton
    };
    let render time => {
      /*lastFrameTime := time;*/

      /** Remember to clear the screen at each tick */
      Draw.clearScreen ();

      /** */
      let totalHidden = ref 0;
      let i = ref 0;
      Array.iter
        (
          fun c => {
            if (not c.Layout.context.visible) {
              totalHidden := !totalHidden + 1;
              if (c.style.width > 0.) {
                c.style.width = c.style.width -. 0.6;
                c.style.marginLeft = c.style.marginLeft -. 1. > 0. ? c.style.marginLeft -. 1. : 0.;
                c.style.marginRight =
                  c.style.marginRight -. 1. > 0. ? c.style.marginRight -. 1. : 0.
              };
              switch c.children {
              | [|textNode|] => textNode.style.width = 0.
              | _ => assert false
              }
            } else if (
              appState.gameover &&
              !i <
              int_of_float @@
              (rootstyle.width -. rootstyle.paddingLeft -. rootstyle.paddingRight) /. (
                c.layout.width +. c.style.marginRight +. c.style.marginLeft
              )
            ) {
              c.style.height = c.style.height +. Random.float 4.0;
              i := !i + 1
            } else {
              i := !i + 1
            };
            /* @Speed comment this out to get performance back... */
            elementsNode.isDirty = true
          }
        )
        elementsNode.children;

      /** Timer */
      if (appState.timer > 0.) {
        /** timer countdown */
        timerNode.style.left = windowWidth /. 2. -. 75. /. 2.;
        timerNode.style.top = windowHeight /. 2. -. 155. /. 2.;
        let text = Printf.sprintf "%d" (int_of_float (ceil @@ appState.timer /. 1000.));
        switch timerNode.children {
        | [|textNode|] =>
          textNode.context = Draw.generateTextContext text (0.3, 0.9, 0.2, 1.) font48
        | _ => assert false
        };
        root.isDirty = true
      } else if
        timerNode.context.visible {
        timerNode.context.visible = false
      };

      /** Losing state */
      if appState.gameover {
        loseNode.style.left = windowWidth /. 2. -. loseNode.layout.width /. 2.;
        loseNode.style.top = windowHeight /. 2. -. loseNode.layout.height /. 2.;
        loseNode.context.visible = true;
        loseNode.isDirty = true;

        /** Static UI tree has benefits, like allowing pattern matching on it! */
        switch loseNode.children {
        | [|message, restartButton|] =>
          let randX = Random.float 7. -. 3.;
          let randY = Random.float 7. -. 3.;
          message.style.left = randX;
          message.style.top = randY;
          message.isDirty = true;
          let {x: mx, y: my} = appState.mouseState.pos;
          let left = loseNode.style.left +. restartButton.layout.left;
          let top = loseNode.style.top +. restartButton.layout.top;
          let color =
            if (
              mx > left &&
              mx < left +. restartButton.layout.width &&
              my > top && my < top +. restartButton.layout.height
            ) {
              /* If the user clicks on the Restart button we reset the state of the game */
              if appState.mouseState.leftButton.isClicked {
                appState.gameover = false;
                appState.timer = 3000.;
                appState.ballPos.x = windowWidth /. 2. -. 10.;
                appState.ballPos.y = windowHeight -. 50.;
                appState.ballV.x = 2.;
                appState.ballV.y = 2.;
                loseNode.context.visible = false;
                timerNode.context.visible = true;
                elementsNode.children = makeChildren ();
                winNode.context.visible = false;
                (0.3, 0.5, 1., 1.)
              } else {
                (0.1, 0.4, 1., 1.)
              }
            } else {
              (0.1, 0.3, 0.7, 1.)
            };
          restartButton.context = Draw.generateRectContext outContext::restartButton.context color;
          restartButton.isDirty = true
        | _ => assert false
        };
        root.isDirty = true
      };

      /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
      Layout.doLayoutNow root;

      /** This will traverse the layout tree and blit each item to the screen one by one. */
      Draw.traverseAndDraw root 0. 0.;

      /** Move ball */
      let {x: ballX, y: ballY} = appState.ballPos;
      let r = tileMargin *. 3.;

      /** Immediate draw. */
      Draw.drawCircleImmediate ballX ballY radius::r color::Draw.white;

      /** Event handling and manipulation */
      let {x: ballVX, y: ballVY} = appState.ballV;
      let (nextX, nextY) =
        if (appState.timer > 0.) {
          appState.timer = appState.timer -. time;
          (ballX, ballY)
        } else {
          (ballVX +. ballX, ballVY +. ballY)
        };
      if (appState.keyboard.leftIsDown && not appState.keyboard.rightIsDown) {
        paddle.style.left = paddle.layout.left -. paddleSpeed;
        root.isDirty = true
      } else if (
        appState.keyboard.rightIsDown && not appState.keyboard.leftIsDown
      ) {
        paddle.style.left = paddle.layout.left +. paddleSpeed;
        root.isDirty = true
      };
      let (nextX, nextY) =
        if (!totalHidden === totalTiles) {
          winNode.context.visible = true;
          winNode.style.left = windowWidth /. 2. -. winNode.layout.width /. 2.;
          winNode.style.top = windowHeight /. 2. -. winNode.layout.height /. 2.;
          (ballX, ballY)
        } else {
          (nextX, nextY)
        };
      if
        Layout.(
          nextX -. r < elementsNode.layout.left ||
          nextX +. r > elementsNode.layout.left +. elementsNode.layout.width
        ) {
        appState.ballV.x = -. ballVX;
        appState.ballV.y = ballVY
      } else if (
        nextY -. r < elementsNode.layout.top
      ) {
        appState.ballV.x = ballVX;
        appState.ballV.y = -. ballVY
      } else if (
        nextY +. r > elementsNode.layout.top +. elementsNode.layout.height
      ) {
        appState.gameover = true
      } else {
        let collided = ref false;
        {

          /** */
          let {Layout.context: context, layout: {top, left, width, height}} = paddle;
          if context.visible {
            let topLeft = (left, top);
            let topRight = (left +. width, top);
            let bottomRight = (left +. width, top +. height);
            let bottomLeft = (left, top +. height);
            if (segmentIntersection (ballX -. r, ballY) (nextX, nextY) topRight bottomRight) {
              collided := true;
              appState.ballV.x = -. ballVX;
              appState.ballV.y = ballVY
            } else if (
              segmentIntersection (ballX +. r, ballY) (nextX, nextY) topLeft bottomLeft
            ) {
              collided := true;
              appState.ballV.x = -. ballVX;
              appState.ballV.y = ballVY
            } else if (
              segmentIntersection (ballX, ballY -. r) (nextX, nextY) topLeft topRight
            ) {
              collided := true;
              appState.ballV.x = ballVX;
              appState.ballV.y = -. ballVY -. Random.float 0.5
            } else if (
              segmentIntersection (ballX, ballY +. r) (nextX, nextY) bottomLeft bottomRight
            ) {
              collided := true;
              appState.ballV.x = ballVX;
              appState.ballV.y = -. ballVY
            }
          }
        };

        /** */
        let parentLeft = elementsNode.layout.left;
        let parentTop = elementsNode.layout.top;
        Array.iter
          (
            fun curr => {
              let {Layout.context: context, layout: {top, left, width, height}} = curr;
              if context.visible {
                let topLeft = (parentLeft +. left, parentTop +. top);
                let topRight = (parentLeft +. left +. width, parentTop +. top);
                let bottomRight = (parentLeft +. left +. width, parentTop +. top +. height);
                let bottomLeft = (parentLeft +. left, parentTop +. top +. height);
                if (segmentIntersection (ballX -. r, ballY) (nextX, nextY) topRight bottomRight) {
                  collided := true;
                  appState.ballV.x = -. ballVX;
                  appState.ballV.y = ballVY;
                  elementsNode.isDirty = true;
                  context.visible = false
                } else if (
                  segmentIntersection (ballX +. r, ballY) (nextX, nextY) topLeft bottomLeft
                ) {
                  collided := true;
                  appState.ballV.x = -. ballVX;
                  appState.ballV.y = ballVY;
                  elementsNode.isDirty = true;
                  context.visible = false
                } else if (
                  segmentIntersection (ballX, ballY -. r) (nextX, nextY) topLeft topRight
                ) {
                  collided := true;
                  appState.ballV.x = ballVX;
                  appState.ballV.y = -. ballVY;
                  elementsNode.isDirty = true;
                  context.visible = false
                } else if (
                  segmentIntersection (ballX, ballY +. r) (nextX, nextY) bottomLeft bottomRight
                ) {
                  collided := true;
                  appState.ballV.x = ballVX;
                  appState.ballV.y = -. ballVY;
                  elementsNode.isDirty = true;
                  context.visible = false
                }
              }
            }
          )
          elementsNode.children;
        if (not !collided) {
          appState.ballPos.x = nextX;
          appState.ballPos.y = nextY
        }
      }
      /*if (time -. 0.001 > 1000. /. 60.) {
          print_endline @@ "slooooow"
        }*/
      /*print_endline @@ "------------------------------------------------------------";*/
      /*Gc.print_stat stdout*/
    };
    let keyDown ::keycode repeat::_ =>
      Draw.Events.(
        switch keycode {
        | Right => appState.keyboard.rightIsDown = true
        | Left => appState.keyboard.leftIsDown = true
        | _ => ()
        }
      );
    let keyUp ::keycode =>
      Draw.Events.(
        switch keycode {
        | Right => appState.keyboard.rightIsDown = false
        | Left => appState.keyboard.leftIsDown = false
        | _ => ()
        }
      );
    let mouseMove ::x ::y => {
      appState.mouseState.pos.x = float_of_int x;
      appState.mouseState.pos.y = float_of_int y
    };
    let mouseUp ::button ::state ::x ::y =>
      switch button {
      | Draw.Events.LeftButton =>
        appState.mouseState.leftButton = {
          state,
          x: float_of_int x,
          y: float_of_int y,
          isClicked: false
        }
      | Draw.Events.RightButton =>
        appState.mouseState.rightButton = {
          state,
          x: float_of_int x,
          y: float_of_int y,
          isClicked: false
        }
      | _ => ()
      };
    let mouseDown ::button ::state ::x ::y =>
      switch button {
      | Draw.Events.LeftButton =>
        appState.mouseState.leftButton = {
          state,
          x: float_of_int x,
          y: float_of_int y,
          isClicked: true
        }
      | Draw.Events.RightButton =>
        appState.mouseState.rightButton = {
          state,
          x: float_of_int x,
          y: float_of_int y,
          isClicked: true
        }
      | _ => ()
      };
  };
  /*Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));*/
};
