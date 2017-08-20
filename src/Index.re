/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Layout = Draw.Layout;

module Node = Draw.Node;

let font24 = Font.loadFont fontSize::24. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let font20 = Font.loadFont fontSize::20. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let font16 = Font.loadFont fontSize::16. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let font12 = Font.loadFont fontSize::12. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

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

let defaultColor = (0.3, 0.4, 0.9, 1.);

type appStateT = {mutable num: int};

let appState = {num: 0};

let averageOverTime = ref [];

let render time => {
  Random.init appState.num;
  /* Remember to clear the clear at each tick */
  Draw.clearScreen ();
  averageOverTime := [1000. /. time, ...!averageOverTime];

  /** */
  let child0 = {
    /* FPS counter :) */
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText ("fps: " ^ string_of_int (int_of_float (1000. /. time))) font16;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child1 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font24;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child2 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font20;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };
  let child3 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font16;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child4 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font12;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child5 = {
    let innerChild = {
      let {Draw.width: width, height, textureBuffer} =
        Draw.drawText "Change colors when a click happens" font16;
      let style = Layout.{...defaultStyle, width, height};
      Layout.createNode
        withChildren::[||] andStyle::style Node.{...nullContext, texture: textureBuffer}
    };
    let style =
      Layout.{
        ...defaultStyle,
        width: 400.,
        height: 100.,
        marginTop: 32.,
        justifyContent: JustifyCenter,
        alignItems: AlignCenter
      };
    Layout.createNode
      withChildren::[|innerChild|]
      andStyle::style
      Node.{...nullContext, backgroundColor: Draw.randomColor ()}
  };

  /** */
  let root = {
    let rootStyle =
      Layout.{
        ...defaultStyle,
        flexDirection: Column,
        paddingLeft: 24.,
        paddingTop: 24.,
        width: float_of_int Draw.windowSize,
        height: float_of_int Draw.windowSize
      };
    Layout.createNode
      withChildren::[|child0, child1, child2, child3, child4, child5|]
      andStyle::rootStyle
      Node.{...nullContext, backgroundColor: (0.9, 0.9, 0.9, 1.)}
  };

  /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
  Layout.doLayoutNow root;

  /** Immediate-style event handling.
      This works kinda like a game engine. You check the state of the input every frame and act
      on it. By now the layout has been calculated, so we have up-to-date values and we can
      choose to change them if needed.
        */
  let (mouseX, mouseY) = !mouse;
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
            appState.num = appState.num + 1
          }
        }
      }
    )
    root.Layout.children;

  /** This will traverse the layout tree and blit each item to the screen one by one. */
  Draw.traverseAndDraw root 0. 0.;

  /** Reset the button state for having a way to check if a button was clicked for 1 frame. */
  mouseState.leftButton = {...mouseState.leftButton, isClicked: false};
  mouseState.rightButton = {...mouseState.leftButton, isClicked: false}
};

let mouseMove ::x ::y => mouse := (float_of_int x, float_of_int y);

let mouseDown ::button ::state ::x ::y =>
  switch button {
  | Draw.Events.LeftButton =>
    mouseState.leftButton = {state, x: float_of_int x, y: float_of_int y, isClicked: false}
  | Draw.Events.RightButton =>
    mouseState.rightButton = {state, x: float_of_int x, y: float_of_int y, isClicked: false}
  | _ => ()
  };

let mouseUp ::button ::state ::x ::y =>
  switch button {
  | Draw.Events.LeftButton =>
    mouseState.leftButton = {state, x: float_of_int x, y: float_of_int y, isClicked: true}
  | Draw.Events.RightButton =>
    mouseState.rightButton = {state, x: float_of_int x, y: float_of_int y, isClicked: true}
  | _ => ()
  };


/** Start the render loop. **/
Draw.render ::mouseMove ::mouseDown ::mouseUp render;

let total = List.fold_left (fun acc v => v +. acc) 0. !averageOverTime;

print_endline @@
"average " ^ string_of_float @@ total /. float_of_int (List.length !averageOverTime);