module Layout = Draw.Layout;

module Node = Draw.Node;

let font7 = Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

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
  let createElement ::style ::color=Draw.white ::children () => {
    let ctx = Draw.generateRectContext color;
    ctx.allGLData.textureBuffer = font7.textureBuffer;
    Layout.createNode withChildren::(Array.of_list children) andStyle::style ctx
  };
};

module Text = {
  let createElement ::style ::color=Draw.white ::text="" ::font=font7 ::children ::context=? () =>
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
  font7,
  Font.loadFont fontSize::10. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0,
  Font.loadFont fontSize::14. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0,
  Font.loadFont fontSize::24. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0
|];

type stateT = array (string, Draw.fontT, (float, float, float, float));

let tiles: stateT =
  Array.init
    1000
    (fun i => (".", fonts.(i / 5 mod Array.length fonts), colors.(i / 5 mod Array.length colors)));

let ballV = ref (4., 4.);

let ballPos = ref (200., 200.);

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

let width = float_of_int @@ Draw.getWindowWidth ();

let height = float_of_int @@ Draw.getWindowHeight ();

let tileWidth = width /. 50.;

let tileHeight = height /. 60.;

let tileMargin = height /. 400.;

/*let style =
  Layout.{
    ...defaultStyle,
    marginLeft: tileMargin,
    marginRight: tileMargin,
    marginTop: tileMargin,
    marginBottom: tileMargin,
    width: tileWidth,
    height: tileHeight
  };*/
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

let root =
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
                <Text style=Layout.defaultStyle text font />
              </View>
          )
          tiles
      )
    )
    ();

module M: Hotreloader.DYNAMIC_MODULE = {
  let render time => {
    /* Remember to clear the screen at each tick */
    Draw.clearScreen ();
    let width = float_of_int @@ Draw.getWindowWidth ();
    let height = float_of_int @@ Draw.getWindowHeight ();
    let tileWidth = width /. 50.;
    let tileHeight = height /. 60.;
    let tileMargin = height /. 400.;
    rootstyle.width = width -. 200.;
    rootstyle.height = height;
    Array.iter
      (
        fun c => {
          if (not c.Layout.context.visible && c.style.width > 0.) {
            c.style.width = c.style.width -. 0.6;
            c.style.marginLeft = 0.;
            c.style.marginLeft = c.style.marginLeft -. 0.6 > 0. ? c.style.marginLeft -. 0.6 : 0.;
            c.style.marginRight = c.style.marginRight -. 0.6 > 0. ? c.style.marginRight -. 0.6 : 0.
          };
          /* @Speed comment this out to get performance back... */
          root.isDirty = true
        }
      )
      root.children;

    /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
    Layout.doLayoutNow root;

    /** This will traverse the layout tree and blit each item to the screen one by one. */
    Draw.traverseAndDraw root 0. 0.;
    Draw.flushGlobalBatch ();

    /** Move ball */
    let (ballX, ballY) = !ballPos;
    let r = tileMargin *. 3.;

    /** Immediate draw. */
    Draw.drawCircleImmediate ballX ballY radius::r color::Draw.white (Draw.Gl.Mat4.create ());

    /** */
    let (ballVX, ballVY) = !ballV;
    let (nextX, nextY) = (ballVX +. ballX, ballVY +. ballY);
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
          fun (
                {Layout.context: context, style, layout: {top, left, width, height} as layout} as curr
              ) =>
            if context.visible {
              let topLeft = (parentLeft +. left, parentTop +. top);
              let topRight = (parentLeft +. left +. width, parentTop +. top);
              let bottomRight = (parentLeft +. left +. width, parentTop +. top +. height);
              let bottomLeft = (parentLeft +. left, parentTop +. top +. height);
              if (segmentIntersection (ballX, ballY) (nextX, nextY) topRight bottomRight) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                root.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft bottomLeft
              ) {
                collided := true;
                ballV := (-. ballVX, ballVY);
                root.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) topLeft topRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                root.isDirty = true;
                context.visible = false
              } else if (
                segmentIntersection (ballX, ballY) (nextX, nextY) bottomLeft bottomRight
              ) {
                collided := true;
                ballV := (ballVX, -. ballVY);
                root.isDirty = true;
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
  Draw.flushGlobalBatch ();
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
