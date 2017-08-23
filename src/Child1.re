module Layout = Draw.Layout;

module Node = Draw.Node;

let font40TextBufferThingData =
  Font.loadFont fontSize::24. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;
  
let font24TextBufferThingData =
  Font.loadFont fontSize::40. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

/*let font10 = Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;*/
let defaultColor = (0.6, 0.6, 0.9, 1.);

let invisibleColor = (0.0, 0.0, 0.0, 0.0);

let colors = [|
  (0.2, 0.2, 0.9, 1.),
  (0.4, 0.7, 0.3, 1.),
  (0.3, 0.3, 0.1, 1.),
  (0.7, 0.4, 0.3, 1.)
|];

/*let tiles =
  Array.init
    100
    (
      fun i => (
        Draw.drawText "test" font10,
        Node.{
          ...nullContext,
          visible: true,
          backgroundColor: colors.(i / 5 mod Array.length colors)
        }
      )
    );*/
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

module M: Hotreloader.DYNAMIC_MODULE = {
  let render () => {
    /* Remember to clear the screen at each tick */
    Draw.clearScreen ();
    let tileWidth = (float_of_int @@ Draw.getWindowWidth ()) /. 20.;
    let tileHeight = (float_of_int @@ Draw.getWindowHeight ()) /. 30.;
    let tileMargin = (float_of_int @@ Draw.getWindowHeight ()) /. 400.;
    let rootstyle =
      Layout.{
        ...defaultStyle,
        /*  width: textWidth,
            height: textHeight,*/
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
    /*let emptyArray = [||];*/
    Draw.drawText 100. 100. "Hey Yitong why is this so bad " Draw.white font40TextBufferThingData;
    Draw.drawText 100. 200. "Hey Yitong why is this so bad " Draw.white font24TextBufferThingData;
    Draw.drawRect
      100.
      100.
      300.
      1.
      0.
      0.
      (1. /. 2048.)
      (1. /. 2048.)
      Draw.red
      font40TextBufferThingData.textureBuffer;
    ()
    /*let children: array Layout.node =
      Array.map
        (
          fun ({Draw.textureBuffer: textureBuffer, width, height}, context) =>
            Layout.createNode
              withChildren::[|
                Layout.createNode
                  withChildren::emptyArray
                  andStyle::{...Layout.defaultStyle, width, height}
                  Node.{...nullContext, backgroundColor: Draw.white, texture: textureBuffer}
              |]
              andStyle::
                Layout.{
                  ...defaultStyle,
                  marginLeft: tileMargin,
                  marginRight: tileMargin,
                  marginTop: tileMargin,
                  marginBottom: tileMargin,
                  width: tileWidth,
                  height: tileHeight
                }
              context
        )
        tiles;*/
    /*let root =
      Layout.createNode
        withChildren::children
        andStyle::rootstyle
        Node.{...nullContext, backgroundColor: defaultColor};*/
    /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
    /*Layout.doLayoutNow root;*/
    /** Immediate-style event handling.
        This works kinda like a game engine. You check the state of the input every frame and act
        on it. By now the layout has been calculated, so we have up-to-date values and we can
        choose to change them if needed.
          */
    /*let (mouseX, mouseY) = !mouse;
      Array.iter
        (
          fun child => {
            let {Layout.top: top, left, width, height} = child.Layout.layout;
            if (mouseX > left && mouseX < left +. width && mouseY > top && mouseY < top +. height) {
              if (mouseState.leftButton.state == Draw.Events.MouseDown) {
                child.Layout.context.Node.backgroundColor = (0.9, 0.4, 0.9, 1.)
              } else {
                child.Layout.context.Node.backgroundColor = (0.9, 0.4, 0.3, 1.)
              };
              if mouseState.leftButton.isClicked {
                appState.color = Draw.randomColor ()
              }
            }
          }
        )
        root.Layout.children;*/
    /** This will traverse the layout tree and blit each item to the screen one by one. */
    /*Draw.traverseAndDraw root 0. 0.;*/
    /* Move ball */
    /*let (ballX, ballY) = !ballPos;
      let r = tileMargin *. 5.;
      Draw.drawCircle ballX ballY r (0., 0., 0., 1.);
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
            fun {Layout.context: context, layout: {top, left, width, height}} =>
              /*let insideX = Layout.(nextX +. r > left +. r && nextX +. r < left +. width -. r);
                let insideY = Layout.(nextY +. r > top +. r && nextY +. r < top +. height -. r);
                if (insideX && insideY && not !collided) {
                  collided := true;
                  ballV := (-. ballVX, ballVY)
                }*/
              if context.visible {
                let topLeft = (parentLeft +. left, parentTop +. top);
                let topRight = (parentLeft +. left +. width, parentTop +. top);
                let bottomRight = (parentLeft +. left +. width, parentTop +. top +. height);
                let bottomLeft = (parentLeft +. left, parentTop +. top +. height);
                Draw.drawRect left top 10. 10. (1., 1., 1., 1.) Node.nullContext.texture;
                /*Draw.drawRect (left +. width) top 10. 10. (1., 1., 0.3, 1.) Node.nullContext.texture;*/
                if (segmentIntersection (ballX, ballY) (nextX, nextY) topRight bottomRight) {
                  collided := true;
                  ballV := (-. ballVX, ballVY);
                  context.visible = false;
                  context.backgroundColor = invisibleColor
                } else if (
                  segmentIntersection (ballX, ballY) (nextX, nextY) topLeft bottomLeft
                ) {
                  collided := true;
                  ballV := (-. ballVX, ballVY);
                  context.visible = false;
                  context.backgroundColor = invisibleColor
                } else if (
                  segmentIntersection (ballX, ballY) (nextX, nextY) topLeft topRight
                ) {
                  collided := true;
                  ballV := (ballVX, -. ballVY);
                  context.visible = false;
                  context.backgroundColor = invisibleColor
                } else if (
                  segmentIntersection (ballX, ballY) (nextX, nextY) bottomLeft bottomRight
                ) {
                  collided := true;
                  ballV := (ballVX, -. ballVY);
                  context.visible = false;
                  context.backgroundColor = invisibleColor
                }
              }
          )
          children;
        if (not !collided) {
          ballPos := (nextX, nextY)
        }
      }*/
  };
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
