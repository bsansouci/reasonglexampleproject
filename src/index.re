/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
open Reglinterface;
module Make (Gl: Gl.t) => {
  /* Setting up the Gl utils functions */
  type glCamera = {projectionMatrix: Gl.Mat4.t, modelViewMatrix: Gl.Mat4.t};
  type glEnv = {camera: glCamera, window: Gl.Window.t, gl: Gl.contextT};

  /**
   * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
   * See this link for quick explanation of what this is.
   * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
   */
  let setProjection (window: Gl.Window.t) (camera: glCamera) =>
    Gl.Mat4.ortho
      out::camera.projectionMatrix
      left::0.
      right::(float_of_int (Gl.Window.getWidth window))
      bottom::0.
      top::(float_of_int (Gl.Window.getHeight window))
      near::0.
      far::100.;
  let resetCamera (camera: glCamera) => Gl.Mat4.identity camera.modelViewMatrix;
  let buildGlEnv window::(window: Gl.Window.t) :glEnv => {
    let gl = Gl.Window.getContext window;
    let glCamera = {projectionMatrix: Gl.Mat4.create (), modelViewMatrix: Gl.Mat4.create ()};
    let env = {camera: glCamera, window, gl};
    let canvasWidth = Gl.Window.getWidth window;
    let canvasHeight = Gl.Window.getHeight window;
    Gl.viewport gl 0 0 canvasWidth canvasHeight;
    Gl.clearColor gl 0.0 0.0 0.0 1.0;
    Gl.clear gl (Constants.color_buffer_bit lor Constants.depth_buffer_bit);
    env
  };
  let getProgram
      env::(env: glEnv)
      vertexShader::vertexShaderSource
      fragmentShader::fragmentShaderSource
      :option Gl.programT => {
    let vertexShader = Gl.createShader env.gl Constants.vertex_shader;
    Gl.shaderSource env.gl vertexShader vertexShaderSource;
    Gl.compileShader env.gl vertexShader;
    let compiledCorrectly =
      Gl.getShaderParameter context::env.gl shader::vertexShader paramName::Gl.Compile_status == 1;
    if compiledCorrectly {
      let fragmentShader = Gl.createShader env.gl Constants.fragment_shader;
      Gl.shaderSource env.gl fragmentShader fragmentShaderSource;
      Gl.compileShader env.gl fragmentShader;
      let compiledCorrectly =
        Gl.getShaderParameter context::env.gl shader::fragmentShader paramName::Gl.Compile_status == 1;
      if compiledCorrectly {
        let program = Gl.createProgram env.gl;
        Gl.attachShader context::env.gl program::program shader::vertexShader;
        Gl.deleteShader context::env.gl shader::vertexShader;
        Gl.attachShader context::env.gl program::program shader::fragmentShader;
        Gl.deleteShader context::env.gl shader::fragmentShader;
        Gl.linkProgram env.gl program;
        let linkedCorrectly =
          Gl.getProgramParameter context::env.gl program::program paramName::Gl.Link_status == 1;
        if linkedCorrectly {
          Some program
        } else {
          print_endline @@
          "Linking error: " ^ Gl.getProgramInfoLog context::env.gl program::program;
          None
        }
      } else {
        print_endline @@
        "Fragment shader error: " ^ Gl.getShaderInfoLog context::env.gl shader::fragmentShader;
        None
      }
    } else {
      print_endline @@
      "Vertex shader error: " ^ Gl.getShaderInfoLog context::env.gl shader::vertexShader;
      None
    }
  };
  let vertexShaderSource = {|
     attribute vec3 aVertexPosition;
     attribute vec4 aVertexColor;

     uniform mat4 uMVMatrix;
     uniform mat4 uPMatrix;

     varying vec4 vColor;

     void main(void) {
       gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
       vColor = aVertexColor;
     }
  |};
  let fragmentShaderSource = {|
     varying vec4 vColor;

     void main(void) {
       gl_FragColor = vColor;
     }
    |};
  let window = Gl.Window.init argv::Sys.argv;
  let windowSize = 600;
  Gl.Window.setWindowSize window::window width::windowSize height::windowSize;
  let {gl, camera, window} as env = buildGlEnv window::window;
  let vertexBuffer = Gl.createBuffer gl;
  let colorBuffer = Gl.createBuffer gl;
  let program =
    switch (
      getProgram env::env vertexShader::vertexShaderSource fragmentShader::fragmentShaderSource
    ) {
    | None => failwith "Could not create the program and/or the shaders. Aborting."
    | Some program => program
    };
  Gl.useProgram gl program;
  let aVertexPosition = Gl.getAttribLocation gl program "aVertexPosition";
  Gl.enableVertexAttribArray gl aVertexPosition;
  let aVertexColor = Gl.getAttribLocation gl program "aVertexColor";
  Gl.enableVertexAttribArray gl aVertexColor;
  let pMatrixUniform = Gl.getUniformLocation gl program "uPMatrix";
  Gl.uniformMatrix4fv context::gl location::pMatrixUniform value::camera.projectionMatrix;
  let mvMatrixUniform = Gl.getUniformLocation gl program "uMVMatrix";
  Gl.uniformMatrix4fv context::gl location::mvMatrixUniform value::camera.modelViewMatrix;
  setProjection window camera;
  let render time => {

    /** Draw rect **/
    resetCamera camera;
    let x = 0;
    let y = 0;
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
    Gl.vertexAttribPointer gl aVertexPosition 3 Constants.float_ false 0 0;
    let (r, g, b) = (1.0, 0., 0.);

    /** Setup colors to be sent to the GPU **/
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
    Gl.vertexAttribPointer gl aVertexColor 4 Constants.float_ false 0 0;
    Gl.uniformMatrix4fv gl pMatrixUniform camera.projectionMatrix;
    Gl.uniformMatrix4fv gl mvMatrixUniform camera.modelViewMatrix;
    Gl.drawArrays gl Constants.triangle_strip 0 4
  };
  let start () => Gl.render window::window displayFunc::render ();
};
