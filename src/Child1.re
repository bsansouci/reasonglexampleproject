module Layout = Draw.Layout;

module Node = Draw.Node;

let font7 = Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let defaultColor = (0.6, 0.6, 0.9, 1.);

let colors = [|
  (0.2, 0.2, 0.9, 1.),
  (0.4, 0.7, 0.3, 1.),
  (0.3, 0.3, 0.1, 1.),
  (0.7, 0.4, 0.3, 1.)
|];

let tiles = Array.init 1000 (fun i => Draw.generateTextContext "Hello!" Draw.white font7);

let ballV = ref (4., 4.);

let ballPos = ref (300., 300.);

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

let root = {
  let children: array Layout.node =
    Array.mapi
      (
        fun i (context: Node.context) =>
          Layout.createNode
            withChildren::[|
              Layout.createNode withChildren::[||] andStyle::Layout.defaultStyle context
            |]
            andStyle::Layout.defaultStyle
            (Draw.generateRectContext colors.(i / 5 mod Array.length colors))
      )
      tiles;
  Layout.createNode
    withChildren::children andStyle::Layout.defaultStyle (Draw.generateRectContext defaultColor)
};

module M: Hotreloader.DYNAMIC_MODULE = {
  let render time => {
    /* Remember to clear the screen at each tick */
    Draw.clearScreen ();
    let tileWidth = (float_of_int @@ Draw.getWindowWidth ()) /. 20.;
    let tileHeight = (float_of_int @@ Draw.getWindowHeight ()) /. 30.;
    let tileMargin = (float_of_int @@ Draw.getWindowHeight ()) /. 400.;
    let rootstyle =
      Layout.{
        ...defaultStyle,
        paddingTop: 40.,
        paddingLeft: 40.,
        paddingRight: 40.,
        justifyContent: JustifyCenter,
        flexDirection: Row,
        flexWrap: CssWrap,
        marginLeft: 100.,
        width: float_of_int @@ Draw.getWindowWidth () - 200,
        height: float_of_int @@ Draw.getWindowHeight ()
      };
    Array.iteri
      (
        fun i child => {
          /*let visible = root.children.(i).context.visible;*/
          let visible = true;
          child.Layout.style =
            Layout.{
              ...defaultStyle,
              marginLeft: visible ? tileMargin : 0.,
              marginRight: visible ? tileMargin : 0.,
              marginTop: tileMargin,
              marginBottom: tileMargin,
              width: visible ? tileWidth : 0.,
              height: visible ? tileHeight : 0.
            }
        }
      )
      root.children;
    root.Layout.style = rootstyle;

    /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
    Layout.doLayoutNow root;

    /** This will traverse the layout tree and blit each item to the screen one by one. */
    Draw.traverseAndDraw root 0. 0.;

    /** Move ball */
    let (ballX, ballY) = !ballPos;
    let r = tileMargin *. 3.;

    /** Immediate draw. */
    Draw.drawCircle ballX ballY radius::r color::Draw.white (Draw.Gl.Mat4.create ());

    /** */
    let (ballVX, ballVY) = !ballV;
    let (nextX, nextY) = (
      ballVX *. time /. 16.66666666 +. ballX,
      ballVY *. time /. 16.66666666 +. ballY
    );
    if Layout.(nextX -. r < root.layout.left || nextX +. r > root.layout.left +. root.layout.width) {
      ballV := (-. ballVX, ballVY)
    } else if (
      nextY -. r < root.layout.top || nextY +. r > root.layout.top +. root.layout.height
    ) {
      ballV := (ballVX, -. ballVY)
    } else {
      let collided = ref false;
      let parentLeft = root.layout.left;
      let parentTop = root.layout.top;
      Array.iter
        (
          fun {Layout.context: context, layout: {top, left, width, height}} =>
            if context.visible {
              let topLeft = (parentLeft +. left, parentTop +. top);
              let topRight = (parentLeft +. left +. width, parentTop +. top);
              let bottomRight = (parentLeft +. left +. width, parentTop +. top +. height);
              let bottomLeft = (parentLeft +. left, parentTop +. top +. height);
              if (segmentIntersection (ballX, ballY) (nextX, nextY) topRight bottomRight) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft bottomLeft
              ) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft topRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) bottomLeft bottomRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                context.visible = false
              }
            }
        )
        root.children;
      if (not !collided) {
        ballPos := (nextX, nextY)
      }
    }
  };
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
