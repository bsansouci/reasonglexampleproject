/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Layout = Draw.Layout;

module Node = Draw.Node;

let font40 = Font.loadFont fontSize::40. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font36 = Font.loadFont fontSize::36. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font32 = Font.loadFont fontSize::32. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font28 = Font.loadFont fontSize::28. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font24 = Font.loadFont fontSize::24. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font20 = Font.loadFont fontSize::20. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font16 = Font.loadFont fontSize::16. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let font12 = Font.loadFont fontSize::12. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

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

type keyboardStateT = {
  mutable shiftIsDown: int,
  mutable altIsDown: int
};

let keyboardState = {shiftIsDown: 0, altIsDown: 0};

let defaultColor = (0.3, 0.4, 0.9, 1.);

type appStateT = {
  mutable color: (float, float, float, float),
  mutable inputText: string,
  mutable easterEgg: bool
};

let appState = {color: defaultColor, inputText: "this is not a word", easterEgg: false};

Random.init 0;

let maxHeight = Draw.getFontMaxHeight font24;

/*let baseline = Draw.getFontBaseline font24;*/
let render time => {
  /* Remember to clear the clear at each tick */
  Draw.clearScreen ();

  /** */
  let child0 = {
    /* FPS counter :) */
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText ("fps: " ^ string_of_int (int_of_float (1000. /. time +. 0.5))) font16;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child1 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font40;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child2 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font36;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child3 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font32;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child4 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font28;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** Easter egg, you can edit this one! */
  let child5 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText appState.inputText font24;
    let marginBottom = maxHeight -. textHeight;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight, marginBottom};
    let child1 =
      Layout.createNode
        withChildren::[||]
        andStyle::style
        Node.{texture: textureBuffer, backgroundColor: defaultColor};
    let style = Layout.{...defaultStyle, width: 2., height: maxHeight -. 6.};
    let cursor =
      Layout.createNode
        withChildren::[||]
        andStyle::style
        Node.{...nullContext, backgroundColor: appState.easterEgg ? Draw.red : Draw.noColor};
    let style = Layout.{...defaultStyle, flexDirection: Row, alignItems: AlignCenter};
    Layout.createNode
      withChildren::[|child1, cursor|]
      andStyle::style
      Node.{...nullContext, backgroundColor: Draw.noColor}
  };

  /** */
  let child6 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font20;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };
  let child7 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font16;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child8 = {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font12;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };

  /** */
  let child9 = {
    let innerChild = {
      let {Draw.width: textWidth, height: textHeight, textureBuffer} =
        Draw.drawText "Change colors when a click happens" font16;
      let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
      Layout.createNode
        withChildren::[||]
        andStyle::style
        Node.{...nullContext, backgroundColor: Draw.white, texture: textureBuffer}
    };
    let style =
      Layout.{
        ...defaultStyle,
        flexGrow: 1.,
        height: 100.,
        marginTop: 32.,
        justifyContent: JustifyCenter,
        alignItems: AlignCenter
      };
    Layout.createNode
      withChildren::[|innerChild|]
      andStyle::style
      Node.{...nullContext, backgroundColor: appState.color}
  };

  /** */
  let root = {
    let rootStyle =
      Layout.{
        ...defaultStyle,
        flexDirection: Column,
        paddingLeft: 24.,
        paddingRight: 24.,
        paddingTop: 24.,
        paddingBottom: 24.,
        width: float_of_int @@ Draw.getWindowWidth (),
        height: float_of_int @@ Draw.getWindowHeight ()
      };
    Layout.createNode
      withChildren::[|
        child0,
        child1,
        child2,
        child3,
        child4,
        child5,
        child6,
        child7,
        child8,
        child9
      |]
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
            appState.color = Draw.randomColor ()
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

let windowResize () => Draw.resizeWindow ();

let keyDown ::keycode ::repeat => {
  open Draw.Events;
  appState.easterEgg = true;
  let letter =
    switch keycode {
    | A => "a"
    | B => "b"
    | C => "c"
    | D => "d"
    | E => "e"
    | F => "f"
    | G => "g"
    | H => "h"
    | I => "i"
    | J => "j"
    | K => "k"
    | L => "l"
    | M => "m"
    | N => "n"
    | O => "o"
    | P => "p"
    | Q => "q"
    | R => "r"
    | S => "s"
    | T => "t"
    | U => "u"
    | V => "v"
    | W => "w"
    | X => "x"
    | Y => "y"
    | Z => "z"
    | Quote => "\""
    | Comma => ","
    | Minus when keyboardState.shiftIsDown == 0 => "-"
    | Minus when keyboardState.shiftIsDown > 0 => "_"
    | Equals when keyboardState.shiftIsDown == 0 => "="
    | Equals when keyboardState.shiftIsDown > 0 => "+"
    | Slash => "/"
    | Num_0 => "0"
    | Num_1 => "1"
    | Num_2 => "2"
    | Num_3 => "3"
    | Num_4 => "4"
    | Num_5 => "5"
    | Num_6 => "6"
    | Num_7 => "7"
    | Num_8 => "8"
    | Num_9 => "9"
    | Semicolon => ";"
    | Period => "."
    | Space => " "
    | _ => ""
    };
  switch keycode {
  | Backspace =>
    if (keyboardState.altIsDown > 0) {
      let rec iter i =>
        if (i == 0) {
          0
        } else if (appState.inputText.[i] == ' ') {
          i
        } else {
          iter (i - 1)
        };
      let cutoffPoint = iter (String.length appState.inputText - 1);
      if (cutoffPoint == 0) {
        appState.inputText = ""
      } else {
        appState.inputText = String.sub appState.inputText 0 cutoffPoint
      }
    } else if (
      String.length appState.inputText > 0
    ) {
      appState.inputText = String.sub appState.inputText 0 (String.length appState.inputText - 1)
    }
  | LeftShift
  | RightShift => keyboardState.shiftIsDown = keyboardState.shiftIsDown + 1
  | LeftAlt
  | RightAlt => keyboardState.altIsDown = keyboardState.altIsDown + 1
  | _ => ()
  };
  let letter =
    if (keyboardState.shiftIsDown > 0) {
      String.capitalize letter
    } else {
      letter
    };
  if (String.length letter > 0) {
    appState.inputText = appState.inputText ^ letter
  }
};

let keyUp ::keycode =>
  Draw.Events.(
    switch keycode {
    | LeftShift
    | RightShift => keyboardState.shiftIsDown = keyboardState.shiftIsDown - 1
    | LeftAlt
    | RightAlt => keyboardState.altIsDown = keyboardState.altIsDown - 1
    | _ => ()
    }
  );


/** Start the render loop. **/
Draw.render ::keyUp ::keyDown ::windowResize ::mouseMove ::mouseDown ::mouseUp render;
