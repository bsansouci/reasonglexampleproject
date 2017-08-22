module Layout = Draw.Layout;

module Node = Draw.Node;

module M: Hotreloader.DYNAMIC_MODULE = {
  let render () => Draw.clearScreen ();
};

Hotreloader.p := Some ((module M): (module Hotreloader.DYNAMIC_MODULE));
