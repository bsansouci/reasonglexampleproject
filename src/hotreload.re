/* See src/plugin.re for explanation. */
/*module Layout = Draw.Layout;

module Node = Draw.Node;

open Plugin;

let font40 = Font.loadFont fontSize::24. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let defaultColor = (0.3, 0.4, 0.9, 1.);

module M: PLUG = {
  let render () => {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a word" font40;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };
};

p := Some ((module M): (module PLUG));*/
