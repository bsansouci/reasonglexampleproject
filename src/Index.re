/* By the simple act of linking in the Draw module, window's created and GL gets initialized. */
module Layout = Draw.Layout;

module Node = Draw.Node;

let font = Font.loadFont "OpenSans-Regular.ttf";

let tick = ref 0.;

let render time => {
  /* Remember to clear the clear at each tick */
  Draw.clearScreen ();

  /** */
  let child1Style = Layout.{...defaultStyle, flexGrow: 1., top: 0., left: 0.};
  let child1 =
    Layout.createNode
      withChildren::[||] andStyle::child1Style Node.{...nullContext, backgroundColor: Draw.red};

  /** */
  let {Draw.width: width, height, textureBuffer} = Draw.drawText "this is not a word" font;

  /** */
  let child2Style = Layout.{...defaultStyle, flexGrow: 2., top: 0., left: 0.};
  let child2 =
    Layout.createNode
      withChildren::[||] andStyle::child2Style Node.{...nullContext, texture: textureBuffer};
  let rootStyle =
    Layout.{
      ...defaultStyle,
      flexDirection: Row,
      width: width *. 3. /. 2.,
      height,
      top: 100.,
      left: 8. +. !tick
    };

  /** */
  let root =
    Layout.createNode
      withChildren::[|child1, child2|]
      andStyle::rootStyle
      Node.{...nullContext, backgroundColor: Draw.blue};

  /** This will perform all of the Flexbox calculations and mutate the layouts to have left, top, width, height set. The positions are relative to the parent. */
  Layout.doLayoutNow root;

  /** This will traverse the layout tree and blit each item to the screen one by one. */
  Draw.traverseAndDraw root 0. 0.;

  /** */
  tick := !tick +. 1.
};


/** Start the render loop. **/
Draw.render render;
