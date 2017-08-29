module Layout = Draw.Layout;

module Node = Draw.Node;

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
module View = {
  let createElement ::style=Layout.defaultStyle ::color=Draw.white ::children () =>
    Layout.createNode
      withChildren::(Array.of_list children) andStyle::style (Draw.generateRectContext color);
};

module Text = {
  let createElement
      ::style=Layout.defaultStyle
      ::color=Draw.white
      ::text=""
      ::font=font7
      ::children
      ::context=?
      () =>
    switch context {
    | None =>
      Layout.createNode
        withChildren::(Array.of_list children)
        andStyle::style
        (Draw.generateTextContext text color font)
    | Some context =>
      Layout.createNode withChildren::(Array.of_list children) andStyle::style context
    };
};

let defaultColor = (0.6, 0.6, 0.9, 1.);

let colors = [|
  (1., 1., 0.4, 1.),
  (1., 204. /. 255., 0., 1.),
  (1., 0.6, 0., 1.),
  (1., 0., 0., 1.)
|];

let fonts = [|
  /*Font.loadFont fontSize::9. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0,*/
  Font.loadFont fontSize::9. fontPath::"assets/fonts/Anonymous_Pro.ttf" id::0
  /*Font.loadFont fontSize::9. fontPath::"assets/fonts/DroidSansMono.ttf" id::0,*/
  /*Font.loadFont fontSize::24. fontPath::"assets/fonts/Anonymous_Pro.ttf" id::0,*/
  /*Font.loadFont fontSize::28. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0,*/
  /*Font.loadFont fontSize::32. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0*/
|];

let width = float_of_int @@ Draw.getWindowWidth ();

let height = float_of_int @@ Draw.getWindowHeight ();

let totalTiles = 1000;

type stateT = array (string, Draw.fontT, (float, float, float, float));

let tiles: stateT =
  Array.init
    totalTiles
    (fun i => ("H", fonts.(i / 5 mod Array.length fonts), colors.(i / 5 mod Array.length colors)));

let ballV = ref (2., 2.);

let ballPos = ref (width /. 2. -. 10., height -. 50.);

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

let tileWidth = width /. 50.;

let tileHeight = height /. 60.;

let tileMargin = height /. 400.;

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
    width: width -. 200.,
    height
  };

let elementsNode =
  View.createElement
    style::rootstyle
    color::defaultColor
    children::(
      Array.to_list (
        Array.mapi
          (
            fun i (text, font, color) =>
              <View
                style=Layout.{
                        ...defaultStyle,
                        marginLeft: tileMargin,
                        marginRight: tileMargin,
                        marginTop: tileMargin,
                        marginBottom: tileMargin,
                        width: tileWidth,
                        height: tileHeight
                      }
                color>
                <Text text font />
              </View>
          )
          tiles
      )
    )
    ();

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
            top: height -. 15.,
            left: width /. 2. -. paddleWidth /. 2.
          }
  />;

let root = <View> elementsNode paddle </View>;

/*let lastFrameTime = ref 0.;*/
let leftIsPressed = ref false;

let rightIsPressed = ref false;

let rightPressed () => rightIsPressed := true;

let leftPressed () => leftIsPressed := true;

let rightReleased () => rightIsPressed := false;

let leftReleased () => leftIsPressed := false;

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
      paddle.style.left = width /. 2. -. paddleWidth /. 2.
    }
  );

let gameover = ref false;

let loseText = Draw.generateTextContext "YOU LOSE </3" Draw.red font48;

let timer = ref 3000.;

let mouse = ref (0., 0.);

type buttonStateT = {
  state: Draw.Events.stateT,
  x: float,
  y: float,
  isClicked: bool
};

type mouseStateT = {
  mutable leftButton: buttonStateT,
  mutable rightButton: buttonStateT
};

let mouseState = {
  leftButton: {state: Draw.Events.MouseUp, x: 0., y: 0., isClicked: false},
  rightButton: {state: Draw.Events.MouseUp, x: 0., y: 0., isClicked: false}
};

module M: Hotreloader.DYNAMIC_MODULE = {
  let render time => {
    /*lastFrameTime := time;*/
    /* Remember to clear the screen at each tick */
    Draw.clearScreen ();
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
              c.style.marginRight = c.style.marginRight -. 1. > 0. ? c.style.marginRight -. 1. : 0.
            }
          } else if (
            !gameover &&
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

    /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
    Layout.doLayoutNow root;

    /** This will traverse the layout tree and blit each item to the screen one by one. */
    Draw.traverseAndDraw root 0. 0.;

    /** Move ball */
    let (ballX, ballY) = !ballPos;
    let r = tileMargin *. 3.;

    /** Immediate draw. */
    Draw.drawCircleImmediate ballX ballY radius::r color::Draw.white;

    /** */
    let (ballVX, ballVY) = !ballV;
    let nextX = ballVX +. ballX;
    let nextY = ballVY +. ballY;
    let (nextX, nextY) =
      if (!timer > 0.) {
        Draw.drawRectImmediate
          (width /. 2. -. 75. /. 2.) (height /. 2. -. 155. /. 2.) 65. 90. defaultColor;
        ignore @@
        Draw.drawTextImmediate
          (width /. 2. -. 50. /. 2.)
          (height /. 2. -. 20. /. 2.)
          (Printf.sprintf "%d" (int_of_float (ceil @@ !timer /. 1000.)))
          (0.3, 0.9, 0.2, 1.)
          font48;
        timer := !timer -. time;
        (ballX, ballY)
      } else {
        (ballVX +. ballX, ballVY +. ballY)
      };
    let prevPaddleX = paddle.style.left;
    if (!leftIsPressed && not !rightIsPressed) {
      paddle.style.left = paddle.layout.left -. paddleSpeed;
      root.isDirty = true
    } else if (
      !rightIsPressed && not !leftIsPressed
    ) {
      paddle.style.left = paddle.layout.left +. paddleSpeed;
      root.isDirty = true
    };
    let (nextX, nextY) =
      if (!totalHidden === totalTiles) {
        ignore @@
        Draw.drawTextImmediate
          (width /. 2. -. 400. /. 2.)
          (height /. 2. -. 20. /. 2.)
          "YOU WIN <3"
          (0.8, 0.2, 0.8, 1.)
          font48;
        (ballX, ballY)
      } else {
        (nextX, nextY)
      };
    if
      Layout.(
        nextX -. r < elementsNode.layout.left ||
        nextX +. r > elementsNode.layout.left +. elementsNode.layout.width
      ) {
      ballV := (-. ballVX, ballVY)
    } else if (
      nextY -. r < elementsNode.layout.top
    ) {
      ballV := (ballVX, -. ballVY)
    } else if (
      nextY +. r > elementsNode.layout.top +. elementsNode.layout.height
    ) {
      gameover := true;
      let randX = Random.float 7.;
      let randY = Random.float 7.;
      ignore @@
      Draw.drawRectImmediate
        (width /. 2. -. 485. /. 2.) (height /. 2. -. 155. /. 2.) 470. 100. defaultColor;
      {
        let (mx, my) = !mouse;
        let left = width /. 2. -. 200. /. 2.;
        let top = height /. 2. +. 50. /. 2.;
        let color =
          if (mx > left && mx < left +. 200. && my > top && my < top +. 70.) {
            if mouseState.leftButton.isClicked {
              gameover := false;
              timer := 3000.;
              ballPos := (width /. 2. -. 10., height -. 50.);
              ballV := (2., 2.);
              elementsNode.children =
                Array.mapi
                  (
                    fun i (text, font, color) =>
                      <View
                        style=Layout.{
                                ...defaultStyle,
                                marginLeft: tileMargin,
                                marginRight: tileMargin,
                                marginTop: tileMargin,
                                marginBottom: tileMargin,
                                width: tileWidth,
                                height: tileHeight
                              }
                        color>
                        <Text text font />
                      </View>
                  )
                  tiles;
              (0.3, 0.5, 1., 1.)
            } else {
              (0.1, 0.4, 1., 1.)
            }
          } else {
            (0.1, 0.3, 0.7, 1.)
          };
        ignore @@ Draw.drawRectImmediate left top 200. 70. color;
        ignore @@
        Draw.drawTextImmediate
          (width /. 2. -. 170. /. 2.) (height /. 2. +. 150. /. 2.) "restart" Draw.green font38
      };
      ignore @@
      Draw.drawTextImmediate
        (width /. 2. -. 450. /. 2. +. randX)
        (height /. 2. -. 20. /. 2. +. randY)
        "YOU LOSE </3"
        Draw.red
        font48;
      ()
    } else {
      let collided = ref false;
      {

        /** */
        let {Layout.context: context, style, layout: {top, left, width, height}} = paddle;
        if context.visible {
          let topLeft = (left, top);
          let topRight = (left +. width, top);
          let bottomRight = (left +. width, top +. height);
          let bottomLeft = (left, top +. height);
          if (segmentIntersection (ballX -. r, ballY) (nextX, nextY) topRight bottomRight) {
            collided := true;
            if (paddle.style.left -. prevPaddleX > 0.) {
              ballV := (-. ballVX, -. ballVY)
            } else {
              ballV := (-. ballVX, ballVY)
            }
          } else if (
            segmentIntersection (ballX +. r, ballY) (nextX, nextY) topLeft bottomLeft
          ) {
            collided := true;
            if (paddle.style.left -. prevPaddleX < 0.) {
              ballV := (-. ballVX, -. ballVY)
            } else {
              ballV := (-. ballVX, ballVY)
            }
          } else if (
            segmentIntersection (ballX, ballY -. r) (nextX, nextY) topLeft topRight
          ) {
            collided := true;
            if (paddle.style.left -. prevPaddleX < 0.) {
              ballV := (ballVX -. Random.float 0.05, -. ballVY -. Random.float 1.0)
            } else {
              ballV := (ballVX, -. ballVY)
            }
          } else if (
            segmentIntersection (ballX, ballY +. r) (nextX, nextY) bottomLeft bottomRight
          ) {
            collided := true;
            ballV := (ballVX, -. ballVY)
          }
        }
      };
      /*elementsNode.isDirty = true;
        context.visible = false*/

      /** */
      let parentLeft = elementsNode.layout.left;
      let parentTop = elementsNode.layout.top;
      Array.iter
        (
          fun curr => {
            let {Layout.context: context, style, layout: {top, left, width, height}} = curr;
            if context.visible {
              let topLeft = (parentLeft +. left, parentTop +. top);
              let topRight = (parentLeft +. left +. width, parentTop +. top);
              let bottomRight = (parentLeft +. left +. width, parentTop +. top +. height);
              let bottomLeft = (parentLeft +. left, parentTop +. top +. height);
              if (segmentIntersection (ballX, ballY) (nextX, nextY) topRight bottomRight) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                elementsNode.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft bottomLeft
              ) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                elementsNode.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft topRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                elementsNode.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) bottomLeft bottomRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                elementsNode.isDirty = true;
                context.visible = false
              }
            }
          }
        )
        elementsNode.children;
      if (not !collided) {
        ballPos := (nextX, nextY)
      }
    }
    /*if (time -. 0.001 > 1000. /. 60.) {
        print_endline @@ "slooooow"
      }*/
    /*print_endline @@ "------------------------------------------------------------";*/
    /*Gc.print_stat stdout*/
  };
  let keyDown ::keycode ::repeat =>
    Draw.Events.(
      switch keycode {
      | Right => rightPressed ()
      | Left => leftPressed ()
      | _ => ()
      }
    );
  let keyUp ::keycode =>
    Draw.Events.(
      switch keycode {
      | Right => rightReleased ()
      | Left => leftReleased ()
      | _ => ()
      }
    );
  let mouseMove ::x ::y => mouse := (float_of_int x, float_of_int y);
  let mouseUp ::button ::state ::x ::y =>
    switch button {
    | Draw.Events.LeftButton =>
      mouseState.leftButton = {state, x: float_of_int x, y: float_of_int y, isClicked: false}
    | Draw.Events.RightButton =>
      mouseState.rightButton = {state, x: float_of_int x, y: float_of_int y, isClicked: false}
    | _ => ()
    };
  let mouseDown ::button ::state ::x ::y =>
    switch button {
    | Draw.Events.LeftButton =>
      mouseState.leftButton = {state, x: float_of_int x, y: float_of_int y, isClicked: true}
    | Draw.Events.RightButton =>
      mouseState.rightButton = {state, x: float_of_int x, y: float_of_int y, isClicked: true}
    | _ => ()
    };
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
