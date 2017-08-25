module Layout = Draw.Layout;

module Node = Draw.Node;

let datOneTextureWePassAround =
  Font.loadFont fontSize::12. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

/*let font24TextBufferThingData =
    Font.loadFont fontSize::40. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;
  */
/*let font10 = Font.loadFont fontSize::7. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;*/
let defaultColor = (0.6, 0.6, 0.9, 1.);

let invisibleColor = (0.0, 0.0, 0.0, 0.0);

let colors = [|
  (0.2, 0.2, 0.9, 1.),
  (0.4, 0.7, 0.3, 1.),
  (0.3, 0.3, 0.1, 1.),
  (0.7, 0.4, 0.3, 1.)
|];

let tiles =
  Array.init
    17000
    (
      fun i => {
        let vertexData =
          Draw.generateTextVertexData "Hello sailor sailor!" Draw.white datOneTextureWePassAround;
        /*Draw.Gl.Mat4.translate vertexData.posMatrix vertexData.posMatrix [|0.1, -.0.2, 0.1, 0.|];*/
        Node.{
          ...nullContext,
          /*visible: true,*/
          /*text: Some {Draw.text: "Hello world world", font: datOneTextureWePassAround},*/
          backgroundColor: colors.(i / 5 mod Array.length colors),
          /*cachedVertexData: Some vertexData*/
          /*texture: datOneTextureWePassAround.textureBuffer*/
        }
      }
    );

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

let tileWidth = (float_of_int @@ Draw.getWindowWidth ()) /. 200.;

let tileHeight = (float_of_int @@ Draw.getWindowHeight ()) /. 300.;

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

let emptyArray = [||];

let children: array Layout.node =
  Array.map
    (
      fun (context: Node.context) =>
        /*switch context.text {*/
        /*| None =>*/
        Layout.createNode
          withChildren::[|
            /* Layout.createNode
               withChildren::emptyArray
               andStyle::{...Layout.defaultStyle, width, height}
               context*/
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
        /*Node.{
            ...nullContext,
            backgroundColor: Draw.white,
            texture: datOneTextureWePassAround.textureBuffer
          }*/
        /*| Some {text, font} => assert false*/
        /*let (width, height) = Draw.measureText text font;*/
        /*let c = {...context, text: context.text};*/
        /*context.text = None;*/
        /*Layout.createNode
          withChildren::[|
            /*Layout.createNode
              withChildren::emptyArray
              andStyle::{
                ...Layout.defaultStyle,
                width,
                height,
                /*marginTop: 10.,*/
                /*marginLeft: 4.*/
              }
              context*/
          |]
          andStyle::
            Layout.{
              ...defaultStyle,
              marginLeft: tileMargin,
              marginRight: tileMargin,
              marginTop: tileMargin,
              marginBottom: tileMargin,
              width: width,
              height: height
            }
          context*/
        /* Node.{
             ...nullContext,
             visible: context.visible,
             backgroundColor: context.backgroundColor,
             texture: datOneTextureWePassAround.textureBuffer
           }*/
        /*}*/
    )
    tiles;

let root =
  Layout.createNode
    withChildren::children
    andStyle::rootstyle
    Node.{
      ...nullContext,
      backgroundColor: defaultColor
      /*visible: false,*/
      /*texture: datOneTextureWePassAround.textureBuffer*/
    };


/** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
Layout.doLayoutNow root;

module M: Hotreloader.DYNAMIC_MODULE = {
  let render () => {
    /* Remember to clear the screen at each tick */
    Draw.clearScreen ();
    Array.iteri
      (
        fun i {Node.cachedVertexData: cachedVertexData} =>
          switch cachedVertexData {
          | None => assert false
          | Some {vertexArray, elementArray, count, textureBuffer, posMatrix} =>
            let m = Draw.Gl.Mat4.create ();
            Draw.Gl.Mat4.translate
              m
              posMatrix
              [|0.4 *. float_of_int i /. 100., (-0.3) *. float_of_int i /. 4., 0.1, 0.|];
            Draw.drawGeometry ::vertexArray ::elementArray ::count ::textureBuffer posMatrix::m
          }
      )
      tiles;
    /** This will traverse the layout tree and blit each item to the screen one by one. */
    /*Draw.traverseAndDraw root 0. 0.*/
    /* Move ball */
    /*let (ballX, ballY) = !ballPos;
      let r = tileMargin *. 5.;
      Draw.addRectToGlobalBatch
        ballX
        ballY
        999.0
        r
        r
        0.
        0.
        (0.5 /. 2048.)
        (0.5 /. 2048.)
        Draw.white
        datOneTextureWePassAround.textureBuffer;
      /*Draw.drawCircle ballX ballY r (0., 0., 0., 1.);*/
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
                /*Draw.drawRect left top 10. 10. (1., 1., 1., 1.) Node.nullContext.texture;*/
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
      };*/
    /*Draw.flushGlobalBatch datOneTextureWePassAround.textureBuffer*/
  };
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
