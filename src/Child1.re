module Layout = Draw.Layout;

module Node = Draw.Node;

let font40 = Font.loadFont fontSize::24. fontPath::"assets/fonts/DroidSansMono.ttf" id::0;

let defaultColor = (0.3, 0.4, 0.9, 1.);

module M: Hotreloader.DYNAMIC_MODULE = {
  let render () => {
    let {Draw.width: textWidth, height: textHeight, textureBuffer} =
      Draw.drawText "this is not a world" font40;
    let style = Layout.{...defaultStyle, width: textWidth, height: textHeight};
    Layout.createNode
      withChildren::[||]
      andStyle::style
      Node.{texture: textureBuffer, backgroundColor: defaultColor}
  };
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
