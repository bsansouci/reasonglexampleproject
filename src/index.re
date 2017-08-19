/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
module Constants = ReasonglInterface.Constants;

module Gl: ReasonglInterface.Gl.t = Reasongl.Gl;


/**
 * This program is an example of how to draw a square.
 * You can vary the number of vertices drawn, allowing you to draw triangles, squares and circles.
 */
type glCamera = {projectionMatrix: Gl.Mat4.t};

type glEnv = {
  camera: glCamera,
  window: Gl.Window.t,
  context: Gl.contextT
};


/**
 * Helper function which will initialize the shaders and attach them to the GL context.
 * Returns the program.
 */
let getProgram
    context::(context: Gl.contextT)
    vertexShader::(vertexShaderSource: string)
    fragmentShader::(fragmentShaderSource: string)
    :option Gl.programT => {
  let vertexShader = Gl.createShader context Constants.vertex_shader;
  Gl.shaderSource context vertexShader vertexShaderSource;
  Gl.compileShader context vertexShader;
  let compiledCorrectly =
    Gl.getShaderParameter
      ::context shader::vertexShader paramName::Gl.Compile_status == 1;
  if compiledCorrectly {
    let fragmentShader = Gl.createShader context Constants.fragment_shader;
    Gl.shaderSource context fragmentShader fragmentShaderSource;
    Gl.compileShader context fragmentShader;
    let compiledCorrectly =
      Gl.getShaderParameter
        ::context shader::fragmentShader paramName::Gl.Compile_status == 1;
    if compiledCorrectly {
      let program = Gl.createProgram context;
      Gl.attachShader ::context ::program shader::vertexShader;
      Gl.deleteShader ::context vertexShader;
      Gl.attachShader ::context ::program shader::fragmentShader;
      Gl.deleteShader ::context fragmentShader;
      Gl.linkProgram ::context program;
      let linkedCorrectly =
        Gl.getProgramParameter ::context ::program paramName::Gl.Link_status == 1;
      if linkedCorrectly {
        Some program
      } else {
        print_endline @@
        "Linking error: " ^ Gl.getProgramInfoLog ::context program;
        None
      }
    } else {
      print_endline @@
      "Fragment shader error: " ^ Gl.getShaderInfoLog ::context fragmentShader;
      None
    }
  } else {
    print_endline @@
    "Vertex shader error: " ^ Gl.getShaderInfoLog ::context vertexShader;
    None
  }
};


/**
 * Dumb vertex shader which take for input a vertex position and a vertex color and maps the point onto
 * the screen.
 * Fragment shader simply applies the color to the pixel.
 */
let vertexShaderSource = {|
  attribute vec3 aVertexPosition;
  attribute vec4 aVertexColor;

  uniform mat4 uPMatrix;

  varying vec4 vColor;

  void main(void) {
    gl_Position = uPMatrix * vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
  }
|};

let fragmentShaderSource = {|
  varying vec4 vColor;

  void main(void) {
    gl_FragColor = vColor;
  }
|};


/** This initializes the window **/
let window = Gl.Window.init argv::Sys.argv;

let windowSize = 600;

Gl.Window.setWindowSize ::window width::windowSize height::windowSize;


/** Initialize the Gl context **/
let context = Gl.Window.getContext window;

Gl.viewport ::context x::0 y::0 width::windowSize height::windowSize;

/* Gl.clearColor context 1.0 1.0 1.0 1.0; */
Gl.clear
  ::context mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);


/** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
let camera = {projectionMatrix: Gl.Mat4.create ()};


/**
 * Those buffers are basically pointers to chunks of memory on the graphics card. They're used to store the
 * vertex and color data.
 */
let vertexBuffer = Gl.createBuffer context;

let colorBuffer = Gl.createBuffer context;


/** Compiles the shaders and gets the program with the shaders loaded into **/
let program =
  switch (
    getProgram
      ::context
      vertexShader::vertexShaderSource
      fragmentShader::fragmentShaderSource
  ) {
  | None =>
    failwith "Could not create the program and/or the shaders. Aborting."
  | Some program => program
  };

Gl.useProgram context program;


/** Get the attribs ahead of time to be used inside the render function **/
let aVertexPosition =
  Gl.getAttribLocation ::context ::program name::"aVertexPosition";

Gl.enableVertexAttribArray ::context attribute::aVertexPosition;

let aVertexColor =
  Gl.getAttribLocation ::context ::program name::"aVertexColor";

Gl.enableVertexAttribArray ::context attribute::aVertexColor;

let pMatrixUniform = Gl.getUniformLocation context program "uPMatrix";

Gl.uniformMatrix4fv
  ::context location::pMatrixUniform value::camera.projectionMatrix;


/**
 * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
 * See this link for quick explanation of what this is.
 * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
 */
Gl.Mat4.ortho
  out::camera.projectionMatrix
  left::0.
  right::(float_of_int (Gl.Window.getWidth window))
  bottom::0.
  top::(float_of_int (Gl.Window.getHeight window))
  near::0.
  far::100.;

let drawRect x y width height (r, g, b, a) => {
  let square_vertices = [|
    x +. width,
    y +. height,
    0.0,
    x,
    y +. height,
    0.0,
    x +. width,
    y,
    0.0,
    x,
    y,
    0.0
  |];
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 square_vertices)
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::3
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;

  /** Setup colors to be sent to the GPU **/
  /*let r = 1.;
    let g = 0.;
    let b = 0.;
    let a = 1.;*/
  let square_colors = [|r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a|];
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::colorBuffer;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 square_colors)
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;
  Gl.uniformMatrix4fv
    ::context location::pMatrixUniform value::camera.projectionMatrix;

  /** Final call which actually does the "draw" **/
  Gl.drawArrays ::context mode::Constants.triangle_strip first::0 count::4
};

module Node = {
  type context = unit;
  let nullContext = ();
};

module Encoding = FloatEncoding;

module Layout = Layout.Create Node Encoding;

module LayoutPrint = LayoutPrint.Create Node Encoding;

module LayoutSupport = Layout.LayoutSupport;

let root_child0_child0_style =
  LayoutSupport.LayoutTypes.{
    ...LayoutSupport.defaultStyle,
    flexGrow: 1.,
    /*height: 2000,*/
    top: 0.,
    left: 0.
  };

let root_child0_child0 =
  LayoutSupport.createNode
    withChildren::[||] andStyle::root_child0_child0_style ();

let root_child0_child1_style =
  LayoutSupport.LayoutTypes.{
    ...LayoutSupport.defaultStyle,
    flexGrow: 2.,
    /*height: 2000,*/
    top: 0.,
    left: 0.
  };

let root_child0_child1 =
  LayoutSupport.createNode
    withChildren::[||] andStyle::root_child0_child1_style ();

let root_style =
  LayoutSupport.LayoutTypes.{
    ...LayoutSupport.defaultStyle,
    flexDirection: Row,
    width: 200.,
    height: 100.,
    top: 100.,
    left: 100.
  };

/*let root =
  ref (LayoutSupport.createNode
    withChildren::[|root_child0_child0, root_child0_child1|]
    andStyle::root_style
    ());*/
/*LayoutPrint.printCssNode (
    root,
    {printLayout: true, printChildren: true, printStyle: true}
  );*/
let red = (1., 0., 0., 1.);

let green = (0., 1., 0., 1.);

let blue = (0., 0., 1., 1.);

let randomColor () => (Random.float 1., Random.float 1., Random.float 1., 1.);

let rec traverseAndDraw root left top => {
  open LayoutSupport.LayoutTypes;
  drawRect
    (left +. root.layout.left)
    (top +. root.layout.top)
    root.layout.width
    root.layout.height
    (randomColor ());
  Array.iter
    (fun child => traverseAndDraw child root.layout.left root.layout.top)
    root.children
};

let tick = ref 0.;


/**
 * Render simply draws a rectangle.
 */
let render time => {
  Random.init 0;
  Gl.clear ::context mask::Constants.color_buffer_bit;
  let root_style =
    LayoutSupport.LayoutTypes.{
      ...LayoutSupport.defaultStyle,
      flexDirection: Row,
      width: 200.,
      height: 100.,
      top: 100.,
      left: 100. +. !tick
    };
  let root =
    LayoutSupport.createNode
      withChildren::[|root_child0_child0, root_child0_child1|]
      andStyle::root_style
      ();
  /* 0,0 is the bottom left corner */
  Layout.layoutNode
    root
    Encoding.cssUndefined
    Encoding.cssUndefined
    LayoutSupport.LayoutTypes.Ltr;
  traverseAndDraw root 0. 0.;
  tick := !tick +. 1.
};


/** Start the render loop. **/
Gl.render ::window displayFunc::render ();
