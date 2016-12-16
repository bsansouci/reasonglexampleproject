/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
module Constants = Reglinterface.Constants;

/* module Make (Gl: Gl.t) => { */
/* Setting up the Gl utils functions */
type glCamera = {projectionMatrix: Gl.Mat4.t};

type glEnv = {camera: glCamera, window: Gl.Window.t, gl: Gl.contextT};

let getProgram
    gl::(gl: Gl.contextT)
    vertexShader::(vertexShaderSource: string)
    fragmentShader::(fragmentShaderSource: string)
    :option Gl.programT => {
  let vertexShader = Gl.createShader gl Constants.vertex_shader;
  Gl.shaderSource gl vertexShader vertexShaderSource;
  Gl.compileShader gl vertexShader;
  let compiledCorrectly =
    Gl.getShaderParameter context::gl shader::vertexShader paramName::Gl.Compile_status == 1;
  if compiledCorrectly {
    let fragmentShader = Gl.createShader gl Constants.fragment_shader;
    Gl.shaderSource gl fragmentShader fragmentShaderSource;
    Gl.compileShader gl fragmentShader;
    let compiledCorrectly =
      Gl.getShaderParameter context::gl shader::fragmentShader paramName::Gl.Compile_status == 1;
    if compiledCorrectly {
      let program = Gl.createProgram gl;
      Gl.attachShader context::gl ::program shader::vertexShader;
      Gl.deleteShader context::gl shader::vertexShader;
      Gl.attachShader context::gl ::program shader::fragmentShader;
      Gl.deleteShader context::gl shader::fragmentShader;
      Gl.linkProgram gl program;
      let linkedCorrectly =
        Gl.getProgramParameter context::gl ::program paramName::Gl.Link_status == 1;
      if linkedCorrectly {
        Some program
      } else {
        print_endline @@ "Linking error: " ^ Gl.getProgramInfoLog context::gl ::program;
        None
      }
    } else {
      print_endline @@
      "Fragment shader error: " ^ Gl.getShaderInfoLog context::gl shader::fragmentShader;
      None
    }
  } else {
    print_endline @@
    "Vertex shader error: " ^ Gl.getShaderInfoLog context::gl shader::vertexShader;
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
let gl = Gl.Window.getContext window;

Gl.viewport context::gl x::0 y::0 width::windowSize height::windowSize;

/* Gl.clearColor gl 1.0 1.0 1.0 1.0; */
Gl.clear context::gl mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);


/** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
let camera = {projectionMatrix: Gl.Mat4.create ()};

let vertexBuffer = Gl.createBuffer gl;

let colorBuffer = Gl.createBuffer gl;


/** compiler shaders and get the program with the shaders loaded into **/
let program =
  switch (getProgram ::gl vertexShader::vertexShaderSource fragmentShader::fragmentShaderSource) {
  | None => failwith "Could not create the program and/or the shaders. Aborting."
  | Some program => program
  };

Gl.useProgram gl program;


/** Get the attribs ahead of time to be used inside the render function **/
let aVertexPosition = Gl.getAttribLocation context::gl ::program name::"aVertexPosition";

Gl.enableVertexAttribArray context::gl attribute::aVertexPosition;

let aVertexColor = Gl.getAttribLocation context::gl ::program name::"aVertexColor";

Gl.enableVertexAttribArray context::gl attribute::aVertexColor;

let pMatrixUniform = Gl.getUniformLocation gl program "uPMatrix";

Gl.uniformMatrix4fv context::gl location::pMatrixUniform value::camera.projectionMatrix;


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


/**
 * Render simply draws a rectangle.
 */
let render time => {
  /* 0,0 is the bottom left corner */
  let x = 150;
  let y = 150;
  let width = 300;
  let height = 300;

  /** Setup vertices to be sent to the GPU **/
  let square_vertices = [|
    float_of_int @@ x + width,
    float_of_int @@ y + height,
    0.0,
    float_of_int x,
    float_of_int @@ y + height,
    0.0,
    float_of_int @@ x + width,
    float_of_int y,
    0.0,
    float_of_int x,
    float_of_int y,
    0.0
  |];
  Gl.bindBuffer context::gl target::Constants.array_buffer buffer::vertexBuffer;
  Gl.bufferData
    context::gl
    target::Constants.array_buffer
    data::(Gl.Float32 square_vertices)
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    context::gl
    attribute::aVertexPosition
    size::3
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;

  /** Setup colors to be sent to the GPU **/
  let (r, g, b) = (1.0, 0., 0.);
  let square_colors = ref [];
  for i in 0 to 3 {
    square_colors := [r, g, b, 1., ...!square_colors]
  };
  Gl.bindBuffer context::gl target::Constants.array_buffer buffer::colorBuffer;
  Gl.bufferData
    context::gl
    target::Constants.array_buffer
    data::(Gl.Float32 (Array.of_list !square_colors))
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    context::gl
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;
  Gl.uniformMatrix4fv context::gl location::pMatrixUniform value::camera.projectionMatrix;

  /** Final call which actually does the "draw" **/
  Gl.drawArrays context::gl mode::Constants.triangle_strip first::0 count::4
};

/* let start () =>  */
Gl.render ::window displayFunc::render ();
/* }; */
