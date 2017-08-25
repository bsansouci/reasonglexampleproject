/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Layout = Draw.Layout;

module Node = Draw.Node;

module Font = Font;

let ocamlPath = Hotreloader.ocamlPath;

/*let font40 = Font.loadFont fontSize::40. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font36 = Font.loadFont fontSize::36. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font32 = Font.loadFont fontSize::32. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font28 = Font.loadFont fontSize::28. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font24 = Font.loadFont fontSize::24. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font20 = Font.loadFont fontSize::20. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

  let font16 = Font.loadFont fontSize::16. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;*/
/*let font12 = Font.loadFont fontSize::12. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;*/
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

let font12 = Font.loadFont fontSize::12. fontPath::"assets/fonts/OpenSans-Regular.ttf" id::0;

let render time => {
  /* Remember to clear the clear at each tick */
  Draw.clearScreen ();

  /** Magical src/Child1.re has hot-reloading hooked up */
  switch !Hotreloader.p {
  | Some s =>
    module M = (val (s: (module Hotreloader.DYNAMIC_MODULE)));
    M.render ()
  | None => ()
  };

  /** Happy FPS counter. */
  Draw.drawText
    5. 50. 1. ("fps: " ^ string_of_int (int_of_float (1000. /. time +. 0.5))) Draw.white font12;

  /** @Hack For hotreloading. */
  Hotreloader.checkRebuild ()
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
    /* @Incomplete How on earth do we support different keyboard layouts? This is my layout on a
       QWERTY US macbook air.
       */
    | Tab => "  " /* Tab is 2 space. What ya gonna do about it HUH. */
    | Quote when keyboardState.shiftIsDown == 0 => "'"
    | Quote when keyboardState.shiftIsDown > 0 => "\""
    | Comma when keyboardState.shiftIsDown == 0 => ","
    | Comma when keyboardState.shiftIsDown > 0 => "<"
    | Minus when keyboardState.shiftIsDown == 0 => "-"
    | Minus when keyboardState.shiftIsDown > 0 => "_"
    | Equals when keyboardState.shiftIsDown == 0 => "="
    | Equals when keyboardState.shiftIsDown > 0 => "+"
    | Slash when keyboardState.shiftIsDown == 0 => "/"
    | Slash when keyboardState.shiftIsDown > 0 => "?"
    | Num_0 when keyboardState.shiftIsDown == 0 => "0"
    | Num_0 when keyboardState.shiftIsDown > 0 => ")"
    | Num_1 when keyboardState.shiftIsDown == 0 => "1"
    | Num_1 when keyboardState.shiftIsDown > 0 => "!"
    | Num_2 when keyboardState.shiftIsDown == 0 => "2"
    | Num_2 when keyboardState.shiftIsDown > 0 => "@"
    | Num_3 when keyboardState.shiftIsDown == 0 => "3"
    | Num_3 when keyboardState.shiftIsDown > 0 => "#"
    | Num_4 when keyboardState.shiftIsDown == 0 => "4"
    | Num_4 when keyboardState.shiftIsDown > 0 => "$"
    | Num_5 when keyboardState.shiftIsDown == 0 => "5"
    | Num_5 when keyboardState.shiftIsDown > 0 => "%"
    | Num_6 when keyboardState.shiftIsDown == 0 => "6"
    | Num_6 when keyboardState.shiftIsDown > 0 => "^"
    | Num_7 when keyboardState.shiftIsDown == 0 => "7"
    | Num_7 when keyboardState.shiftIsDown > 0 => "&"
    | Num_8 when keyboardState.shiftIsDown == 0 => "8"
    | Num_8 when keyboardState.shiftIsDown > 0 => "*"
    | Num_9 when keyboardState.shiftIsDown == 0 => "9"
    | Num_9 when keyboardState.shiftIsDown > 0 => "("
    | Semicolon when keyboardState.shiftIsDown == 0 => ";"
    | Semicolon when keyboardState.shiftIsDown > 0 => ":"
    | Period when keyboardState.shiftIsDown == 0 => "."
    | Period when keyboardState.shiftIsDown > 0 => ">"
    | OpenBracket when keyboardState.shiftIsDown == 0 => "["
    | OpenBracket when keyboardState.shiftIsDown > 0 => "{"
    | CloseBracket when keyboardState.shiftIsDown == 0 => "]"
    | CloseBracket when keyboardState.shiftIsDown > 0 => "}"
    | Backslash when keyboardState.shiftIsDown == 0 => "\\"
    | Backslash when keyboardState.shiftIsDown > 0 => "|"
    | Backtick when keyboardState.shiftIsDown == 0 => "`"
    | Backtick when keyboardState.shiftIsDown > 0 => "~"
    | Space => " "
    | _ => ""
    };
  switch keycode {
  | Backspace =>
    if (String.length appState.inputText > 0) {
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
        if (cutoffPoint <= 0) {
          appState.inputText = ""
        } else {
          appState.inputText = String.sub appState.inputText 0 cutoffPoint
        }
      } else {
        appState.inputText = String.sub appState.inputText 0 (String.length appState.inputText - 1)
      }
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
