/*

  @Todo look into vertex array objects (-> webgl doesn't support those for some reason)

 */
module Constants = ReasonglInterface.Constants;

module Gl: ReasonglInterface.Gl.t = Reasongl.Gl;

module Events = Gl.Events;

module IntMap =
  Map.Make {
    type t = int;
    let compare = compare;
  };

module IntPairMap =
  Map.Make {
    type t = (int, int);
    let compare (a1, a2) (b1, b2) => {
      let first = compare a1 b1;
      if (first != 0) {
        first
      } else {
        compare a2 b2
      }
    };
  };

type glyphInfoT = {
  width: float,
  height: float,
  atlasX: float,
  atlasY: float,
  bearingX: float,
  bearingY: float,
  advance: float
};

type fontT = {
  /*face: Ftlow.face,*/
  chars: IntMap.t glyphInfoT,
  kerning: IntPairMap.t (float, float),
  textureBuffer: Gl.textureT,
  textureWidth: float,
  textureHeight: float
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
    Gl.getShaderParameter ::context shader::vertexShader paramName::Gl.Compile_status == 1;
  if compiledCorrectly {
    let fragmentShader = Gl.createShader context Constants.fragment_shader;
    Gl.shaderSource context fragmentShader fragmentShaderSource;
    Gl.compileShader context fragmentShader;
    let compiledCorrectly =
      Gl.getShaderParameter ::context shader::fragmentShader paramName::Gl.Compile_status == 1;
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
        print_endline @@ "Linking error: " ^ Gl.getProgramInfoLog ::context program;
        None
      }
    } else {
      print_endline @@ "Fragment shader error: " ^ Gl.getShaderInfoLog ::context fragmentShader;
      None
    }
  } else {
    print_endline @@ "Vertex shader error: " ^ Gl.getShaderInfoLog ::context vertexShader;
    None
  }
};


/** */
let vertexShaderSource = {|
  attribute vec2 aVertexPosition;
  attribute vec4 aVertexColor;
  attribute vec2 aTextureCoord;

  uniform mat4 uPMatrix;
  uniform vec4 posAndScaleVec;

  varying vec4 vColor;
  varying vec2 vTextureCoord;

  void main(void) {
    gl_Position = uPMatrix * vec4(posAndScaleVec.xy + posAndScaleVec.zw * aVertexPosition, 0., 1.0);
    vColor = aVertexColor;
    vTextureCoord = aTextureCoord;
  }
|};

let fragmentShaderSource = {|
  varying vec4 vColor;
  varying vec2 vTextureCoord;

  uniform sampler2D uSampler;

  void main(void) {
    gl_FragColor = texture2D(uSampler, vTextureCoord) * vColor;
  }
|};

type glCamera = {projectionMatrix: Gl.Mat4.t};

/* @Incomplete @Hack This is commented out because of the module Node.
   That module needs to expose a `nullContext` value which should (for performance reasons)
   contain an initialized but dummy texture so that we can use nullContext to render correctly.

   The reason I want a nullTex and not a Some/None, is that the latter would force me to switch
   on the texture within the render loop, throwing off path prediction and (maybe) affecting
   performance. I prefer to have a valid texture that is fully handled (like have a nil pointer
   in objc). This allows us to not have conditionals in the render loop.

   One potential solution is to modify the shader to receive another uniform indicating whether
   or not there's a texture ready to be rendered. I'm not sure of the performance implications of
   having conditions in the shader.

           Ben - August 19th 2017

      */
/*type envT = {
    context: Gl.contextT,
    nullTex: Gl.textureT,
    uSampler: Gl.uniformT,
    aVertexPosition: Gl.attributeT,
    aVertexColor: Gl.attributeT,
    aTextureCoord: Gl.attributeT,
    pMatrixUniform: Gl.uniformT,
    vertexBuffer: Gl.bufferT,
    colorBuffer: Gl.bufferT,
    textureBuffer: Gl.bufferT,
    window: Gl.Window.t,
    camera: glCamera
  };
  */

/** This initializes the window **/
let window = Gl.Window.init argv::Sys.argv;

let windowWidth = 900;

let windowHeight = 600;

Gl.Window.setWindowSize ::window width::windowWidth height::windowHeight;


/** Initialize the Gl context **/
let context = Gl.Window.getContext window;

Gl.viewport ::context x::0 y::0 width::windowWidth height::windowHeight;

let getWindowWidth () => Gl.Window.getWidth window;

let getWindowHeight () => Gl.Window.getHeight window;

Gl.clearColor context 1.0 1.0 1.0 1.0;

Gl.clear ::context mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);


/** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
let camera = {projectionMatrix: Gl.Mat4.create ()};


/**
 * Those buffers are basically pointers to chunks of memory on the graphics card. They're used to store the
 * vertex and color data.
 */
let vertexBuffer = Gl.createBuffer context;

let elementBuffer = Gl.createBuffer context;

let colorBuffer = Gl.createBuffer context;

let textureBuffer = Gl.createBuffer context;


/** Compiles the shaders and gets the program with the shaders loaded into **/
let program =
  switch (
    getProgram ::context vertexShader::vertexShaderSource fragmentShader::fragmentShaderSource
  ) {
  | None => failwith "Could not create the program and/or the shaders. Aborting."
  | Some program => program
  };

Gl.useProgram context program;


/** Get the attribs ahead of time to be used inside the render function **/
let aVertexPosition = Gl.getAttribLocation ::context ::program name::"aVertexPosition";

Gl.enableVertexAttribArray ::context attribute::aVertexPosition;

let aVertexColor = Gl.getAttribLocation ::context ::program name::"aVertexColor";

Gl.enableVertexAttribArray ::context attribute::aVertexColor;

let pMatrixUniform = Gl.getUniformLocation context program "uPMatrix";

Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix;

let posAndScaleVec = Gl.getUniformLocation context program "posAndScaleVec";

/*Gl.uniformMatrix4fv ::context location::posAndScaleVec value::(Gl.Bigarray.of_array Gl.Bigarray.Float32 [|255, 255|]);*/
/*let scaleVec = Gl.getUniformLocation context program "scaleVec";*/
/*Gl.uniformMatrix4fv ::context location::scaleVec value::(Gl.Mat4.create ());*/
let aTextureCoord = Gl.getAttribLocation ::context ::program name::"aTextureCoord";

Gl.enableVertexAttribArray ::context attribute::aTextureCoord;


/** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
    texture we can manipulate at the same time. That limit depends on the device. We don't care as we'll just
    always use texture0. **/
Gl.activeTexture ::context RGLConstants.texture0;

let uSampler = Gl.getUniformLocation ::context ::program name::"uSampler";

Gl.uniform1i ::context location::uSampler val::0;


/** Generate texture buffer that we'll use to pass image data around. **/
let nullTex = Gl.createTexture ::context;

Gl.bindTexture ::context target::RGLConstants.texture_2d texture::nullTex;

/* Load a dummy texture. This is because we're using the same shader for things with and without
      a texture. See the big comment near `envT` about using a nullTex vs Some/None.
   */
Gl.texImage2D_RGBA
  ::context
  target::RGLConstants.texture_2d
  level::0
  width::1
  height::1
  border::0
  data::(Gl.Bigarray.of_array Gl.Bigarray.Uint8 [|255, 255, 255, 255|]);

Gl.texParameteri
  ::context
  target::Constants.texture_2d
  pname::Constants.texture_mag_filter
  param::Constants.linear;

Gl.texParameteri
  ::context
  target::Constants.texture_2d
  pname::Constants.texture_min_filter
  param::Constants.linear;

Gl.texParameteri
  ::context
  target::Constants.texture_2d
  pname::Constants.texture_wrap_s
  param::Constants.clamp_to_edge;

Gl.texParameteri
  ::context
  target::Constants.texture_2d
  pname::Constants.texture_wrap_t
  param::Constants.clamp_to_edge;


/** Enable blend and tell OpenGL how to blend. */
Gl.enable ::context RGLConstants.blend;

let alpha_test = 0x0BC0;

Gl.disable ::context alpha_test;

let dither = 0x0BD0;

Gl.disable ::context dither;

let stencil_test = 0x0B90;

Gl.disable ::context stencil_test;

let fog = 0x0B60;

Gl.disable ::context fog;

Gl.disable ::context RGLConstants.depth_test;

Gl.blendFunc ::context RGLConstants.src_alpha RGLConstants.one_minus_src_alpha;

let minZBuffer = 0.;

let maxZBuffer = 1000.;


/**
 * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
 * See this link for quick explanation of what this is.
 * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
 */
let doOrtho () => {
  Gl.Mat4.ortho
    out::camera.projectionMatrix
    left::0.
    right::(float_of_int (Gl.Window.getWidth window))
    bottom::(float_of_int (Gl.Window.getHeight window))
    top::0.
    near::(-. maxZBuffer)
    far::(minZBuffer +. 1.);
  Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix
};

doOrtho ();

let resizeWindow () => {
  let width = Gl.Window.getWidth window;
  let height = Gl.Window.getHeight window;
  Gl.viewport ::context x::0 y::0 ::width ::height;
  doOrtho ()
};

let vertexSize = 8;

/*
 * This array packs all of the values that the shaders need: vertices, colors and texture coordinates.
 * We put them all in one as an optimization, so there are less back and forths between us and the GPU.
 *
 * The vertex array looks like:
 *
 * |<--------  8 * 4 bytes  ------->|
 *  --------------------------------
 * |  x  y  |  r  g  b  a  |  s  t  |  x2  y2  |  r2  g2  b2  a2  |  s2  t2  | ....
 *  --------------------------------
 * |        |              |
 * +- offset: 0 bytes, stride: 8 * 4 bytes (because we need to move by 8*4 bytes to get to the next x)
 *          |              |
 *          +- offset: 2 * 4 bytes, stride: 8 * 4 bytes
 *                         |
 *                         +- offset: (2 + 4) * 4 bytes, stride: 8 * 4 bytes
 *
 */
let drawGeometrySendData
    ::vertexBuffer
    ::elementBuffer
    vertexArray::(vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt)
    elementArray::(elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt)
    count::(count: int)
    textureBuffer::(textureBuffer: Gl.textureT)
    posVecData::((x, y): (float, float))
    scaleVecData::((width, height): (float, float)) => {
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
  Gl.bufferData
    ::context target::Constants.array_buffer data::vertexArray usage::Constants.static_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::2
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::0;

  /** Color */
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::(2 * 4);

  /** Texture */
  Gl.vertexAttribPointer
    ::context
    attribute::aTextureCoord
    size::2
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::(6 * 4);

  /** */
  Gl.uniform4f ::context location::posAndScaleVec v1::x v2::y v3::width v4::height;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which
      is what texture0 represent.  **/
  Gl.uniform1i ::context location::uSampler val::0;

  /** */
  Gl.bindBuffer ::context target::RGLConstants.element_array_buffer buffer::elementBuffer;

  /** Copy the `elementArray` into whatever buffer is in `element_array_buffer` **/
  Gl.bufferData
    ::context
    target::RGLConstants.element_array_buffer
    data::elementArray
    usage::RGLConstants.static_draw;

  /** */
  Gl.bindTexture ::context target::RGLConstants.texture_2d texture::textureBuffer;

  /** Final call which actually does the "draw" **/
  Gl.drawElements
    ::context mode::Constants.triangles ::count type_::RGLConstants.unsigned_short offset::0
};

let drawGeometry2
    count::(count: int)
    ::vertexBuffer
    ::elementBuffer
    textureBuffer::(textureBuffer: Gl.textureT)
    posVecData::((x, y): (float, float))
    scaleVecData::((width, height): (float, float)) => {
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::2
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::0;

  /** Color */
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::(2 * 4);

  /** Texture */
  Gl.vertexAttribPointer
    ::context
    attribute::aTextureCoord
    size::2
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::(6 * 4);

  /** */
  Gl.uniform4f ::context location::posAndScaleVec v1::x v2::y v3::width v4::height;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which
      is what texture0 represent.  **/
  Gl.uniform1i ::context location::uSampler val::0;

  /** */
  Gl.bindBuffer ::context target::RGLConstants.element_array_buffer buffer::elementBuffer;

  /** */
  Gl.bindTexture ::context target::RGLConstants.texture_2d texture::textureBuffer;

  /** Final call which actually does the "draw" **/
  Gl.drawElements
    ::context mode::Constants.triangles ::count type_::RGLConstants.unsigned_short offset::0
};

type vertexDataT = {
  mutable scalable: bool,
  mutable vertexArrayBuffer: Gl.bufferT,
  mutable elementArrayBuffer: Gl.bufferT,
  mutable vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt,
  mutable elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt,
  mutable count: int,
  mutable textureBuffer: Gl.textureT
};

/* Each node contains all possible values, it's probably faster than
   conditional on type. */
module Node = {
  type context = {
    /* Each field is mutable for performance reason. We want to encourage creating a pretty
       static tree that gets its nodes mutated and fully composited each frame. */
    mutable visible: bool,
    mutable isDataSentToGPU: bool,
    mutable allGLData: vertexDataT
  };
  /* @Hack we're using nullTex which is a value that we're assuming has
       been loaded already. This is very bad but it'll work for now.
             Ben - August 19th 2017
     */
  let nullContext = {
    visible: true,
    isDataSentToGPU: false,
    allGLData: {
      scalable: false,
      vertexArrayBuffer: vertexBuffer,
      elementArrayBuffer: elementBuffer,
      vertexArray: Gl.Bigarray.create Gl.Bigarray.Float32 0,
      elementArray: Gl.Bigarray.create Gl.Bigarray.Uint16 0,
      count: 0,
      textureBuffer: nullTex
      /*      posMatrix: Gl.Mat4.create ()*/
    }
  };
};

/* @Speed Boot up time can be improved by not calling set many times. */
let generateRectContext (r, g, b, a) => {
  let vertexArray = Gl.Bigarray.create Gl.Bigarray.Float32 (4 * vertexSize);
  let elementArray = Gl.Bigarray.create Gl.Bigarray.Uint16 6;
  let set = Gl.Bigarray.set;
  let texX = 0.;
  let texY = 0.;
  let texW = 1.;
  let texH = 1.;
  set vertexArray 0 1.;
  set vertexArray 1 1.;
  set vertexArray 2 r;
  set vertexArray 3 g;
  set vertexArray 4 b;
  set vertexArray 5 a;
  set vertexArray 6 (texX +. texW);
  set vertexArray 7 (texY +. texH);
  set vertexArray 8 0.;
  set vertexArray 9 1.;
  set vertexArray 10 r;
  set vertexArray 11 g;
  set vertexArray 12 b;
  set vertexArray 13 a;
  set vertexArray 14 texX;
  set vertexArray 15 (texY +. texH);
  set vertexArray 16 1.;
  set vertexArray 17 0.;
  set vertexArray 18 r;
  set vertexArray 19 g;
  set vertexArray 20 b;
  set vertexArray 21 a;
  set vertexArray 22 (texX +. texW);
  set vertexArray 23 texY;
  set vertexArray 24 0.;
  set vertexArray 25 0.;
  set vertexArray 26 r;
  set vertexArray 27 g;
  set vertexArray 28 b;
  set vertexArray 29 a;
  set vertexArray 30 texX;
  set vertexArray 31 texY;
  set elementArray 0 0;
  set elementArray 1 1;
  set elementArray 2 2;
  set elementArray 3 1;
  set elementArray 4 2;
  set elementArray 5 3;
  Node.{
    visible: true,
    isDataSentToGPU: false,
    allGLData: {
      scalable: true,
      vertexArrayBuffer: Gl.createBuffer context,
      elementArrayBuffer: Gl.createBuffer context,
      vertexArray,
      elementArray,
      count: 6,
      textureBuffer: nullTex
    }
  }
};

let generateTextContext
    (s: string)
    color
    mutableThing::(mutableThing: option Node.context)=?
    ({textureBuffer, textureWidth, textureHeight, chars, kerning}: fontT) => {
  let vertexArray = Gl.Bigarray.create Gl.Bigarray.Float32 (4 * String.length s * vertexSize);
  let elementArray = Gl.Bigarray.create Gl.Bigarray.Uint16 (6 * String.length s);
  let vertexPtr = ref 0;
  let elementPtr = ref 0;
  let set = Gl.Bigarray.set;
  let addRectToBatch
      (x: float)
      (y: float)
      (width: float)
      (height: float)
      (texX: float)
      (texY: float)
      (texW: float)
      (texH: float)
      (r, g, b, a)
      (textureBuffer: Gl.textureT) => {
    let i = !vertexPtr;
    set vertexArray (i + 0) (x +. width);
    set vertexArray (i + 1) (y +. height);
    set vertexArray (i + 2) r;
    set vertexArray (i + 3) g;
    set vertexArray (i + 4) b;
    set vertexArray (i + 5) a;
    set vertexArray (i + 6) (texX +. texW);
    set vertexArray (i + 7) (texY +. texH);
    set vertexArray (i + 8) x;
    set vertexArray (i + 9) (y +. height);
    set vertexArray (i + 10) r;
    set vertexArray (i + 11) g;
    set vertexArray (i + 12) b;
    set vertexArray (i + 13) a;
    set vertexArray (i + 14) texX;
    set vertexArray (i + 15) (texY +. texH);
    set vertexArray (i + 16) (x +. width);
    set vertexArray (i + 17) y;
    set vertexArray (i + 18) r;
    set vertexArray (i + 19) g;
    set vertexArray (i + 20) b;
    set vertexArray (i + 21) a;
    set vertexArray (i + 22) (texX +. texW);
    set vertexArray (i + 23) texY;
    set vertexArray (i + 24) x;
    set vertexArray (i + 25) y;
    set vertexArray (i + 26) r;
    set vertexArray (i + 27) g;
    set vertexArray (i + 28) b;
    set vertexArray (i + 29) a;
    set vertexArray (i + 30) texX;
    set vertexArray (i + 31) texY;
    let ii = i / vertexSize;
    let j = !elementPtr;
    let elementArrayToMutate = elementArray;
    set elementArrayToMutate (j + 0) ii;
    set elementArrayToMutate (j + 1) (ii + 1);
    set elementArrayToMutate (j + 2) (ii + 2);
    set elementArrayToMutate (j + 3) (ii + 1);
    set elementArrayToMutate (j + 4) (ii + 2);
    set elementArrayToMutate (j + 5) (ii + 3);
    vertexPtr := i + 4 * vertexSize;
    elementPtr := j + 6
  };
  let offset = ref 0.;
  let prevChar = ref None;
  String.iter
    (
      fun c => {
        let code = Char.code c;
        switch (IntMap.find code chars) {
        | {width, height, atlasX, atlasY, bearingX, bearingY, advance} =>
          let (kerningOffsetX, kerningOffsetY) =
            switch !prevChar {
            | None => (0., 0.)
            | Some c =>
              switch (IntPairMap.find (c, code) kerning) {
              | v => v
              | exception Not_found => (0., 0.)
              }
            };
          addRectToBatch
            (!offset +. bearingX +. kerningOffsetX)
            (-. bearingY -. kerningOffsetY)
            width
            height
            (atlasX /. textureWidth)
            ((atlasY +. 1.) /. textureHeight)
            (width /. textureWidth)
            (height /. textureHeight)
            color
            textureBuffer;
          prevChar := Some code;
          offset := !offset +. advance
        | exception Not_found =>
          failwith (Printf.sprintf "Couldn't find character %c in atlas :(" c)
        }
      }
    )
    s;
  switch mutableThing {
  | None =>
    Node.{
      visible: true,
      isDataSentToGPU: false,
      allGLData: {
        scalable: false,
        vertexArrayBuffer: Gl.createBuffer context,
        elementArrayBuffer: Gl.createBuffer context,
        vertexArray: Gl.Bigarray.sub vertexArray offset::0 len::!vertexPtr,
        elementArray: Gl.Bigarray.sub elementArray offset::0 len::!elementPtr,
        count: !elementPtr,
        textureBuffer
      }
    }
  | Some dataBag =>
    dataBag.visible = true;
    dataBag.isDataSentToGPU = false;
    dataBag.allGLData.scalable = false;
    dataBag.allGLData.vertexArray = Gl.Bigarray.sub vertexArray offset::0 len::!vertexPtr;
    dataBag.allGLData.elementArray = Gl.Bigarray.sub elementArray offset::0 len::!elementPtr;
    dataBag.allGLData.count = !elementPtr;
    dataBag.allGLData.textureBuffer = textureBuffer;
    dataBag
  }
};

let drawTextImmediate
    (x: float)
    (y: float)
    (s: string)
    color
    mutableThing::(mutableThing: option Node.context)=?
    font => {
  let {
        Node.allGLData: {
          vertexArray,
          elementArray,
          count,
          textureBuffer,
          vertexArrayBuffer,
          elementArrayBuffer
        }
      } as data =
    generateTextContext s color ::?mutableThing font;
  drawGeometrySendData
    vertexBuffer::vertexArrayBuffer
    elementBuffer::elementArrayBuffer
    ::vertexArray
    ::elementArray
    ::count
    ::textureBuffer
    posVecData::(x, y)
    scaleVecData::(1., 1.);
  data
};

let drawCircle x y ::radius color::(r, g, b, a) pm => {

  /** Instantiate a list of points for the circle and bind to the circleBuffer. **/
  let circle_vertex = ref [];
  for i in 0 to 360 {
    let deg2grad = 3.14159 /. 180.;
    let degInGrad = float_of_int i *. deg2grad;
    circle_vertex := [cos degInGrad *. radius, sin degInGrad *. radius, ...!circle_vertex]
  };
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 (Array.of_list !circle_vertex))
    usage::Constants.stream_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::2
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;

  /** Instantiate color array **/
  let circle_colors = ref [];
  for _ in 0 to 360 {
    circle_colors := [r, g, b, a, ...!circle_colors]
  };
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::colorBuffer;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 (Array.of_list !circle_colors))
    usage::Constants.stream_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::0
    offset::0;
  Gl.uniform4f ::context location::posAndScaleVec v1::x v2::y v3::1. v4::1.;
  Gl.uniform1i ::context location::uSampler val::0;
  Gl.bindTexture ::context target::RGLConstants.texture_2d texture::nullTex;
  Gl.drawArrays ::context mode::Constants.triangle_fan first::0 count::360
};

/* Commented out because not used. */
/*type _imageT = {
    textureBuffer: Gl.textureT,
    img: Gl.imageT,
    height: int,
    width: int
  };

  type imageT = ref (option _imageT);

  let loadImage filename :imageT => {
    let imageRef = ref None;
    Gl.loadImage
      ::filename
      callback::(
        fun imageData =>
          switch imageData {
          | None => print_endline ("Could not load image '" ^ filename ^ "'.") /* TODO: handle this better? */
          | Some img =>
            let textureBuffer = Gl.createTexture ::context;
            let height = Gl.getImageHeight img;
            let width = Gl.getImageWidth img;
            imageRef := Some {img, textureBuffer, height, width};
            Gl.bindTexture
              ::context target::Constants.texture_2d texture::textureBuffer;
            Gl.texImage2DWithImage
              ::context target::Constants.texture_2d level::0 image::img;
            Gl.texParameteri
              ::context
              target::Constants.texture_2d
              pname::Constants.texture_mag_filter
              param::Constants.linear;
            Gl.texParameteri
              ::context
              target::Constants.texture_2d
              pname::Constants.texture_min_filter
              param::Constants.linear;
            Gl.texParameteri
              ::context
              target::Constants.texture_2d
              pname::Constants.texture_wrap_s
              param::Constants.clamp_to_edge;
            Gl.texParameteri
              ::context
              target::Constants.texture_2d
              pname::Constants.texture_wrap_t
              param::Constants.clamp_to_edge
          }
      )
      ();
    imageRef
  };*/
let render ::keyUp ::keyDown ::windowResize ::mouseMove ::mouseDown ::mouseUp r =>
  Gl.render
    ::keyUp ::keyDown ::windowResize ::mouseDown ::mouseUp ::mouseMove ::window displayFunc::r ();

let clearScreen () =>
  Gl.clear ::context mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);

let red = (1., 0., 0., 1.);

let green = (0., 1., 0., 1.);

let blue = (0., 0., 1., 1.);

let white = (1., 1., 1., 1.);

let black = (0., 0., 0., 1.);

let noColor = (0., 0., 0., 0.);

let randomColor () => (Random.float 1., Random.float 1., Random.float 1., 1.);

module Layout = {
  module Encoding = FloatEncoding;
  module Layout = Layout.Create Node Encoding;
  module LayoutPrint = LayoutPrint.Create Node Encoding;
  module LayoutSupport = Layout.LayoutSupport;

  /** Including this to get the types! */
  include LayoutSupport.LayoutTypes;

  /** */
  let createNode = LayoutSupport.createNode;
  let defaultStyle = LayoutSupport.defaultStyle;
  let doLayoutNow root => Layout.layoutNode root Encoding.cssUndefined Encoding.cssUndefined Ltr;
};

let rec traverseAndDraw root left top =>
  Layout.(
    if root.context.visible {
      let absoluteLeft = left +. root.layout.left;
      let absoluteTop = top +. root.layout.top;
      let {
        scalable,
        vertexArray,
        elementArray,
        count,
        textureBuffer,
        vertexArrayBuffer,
        elementArrayBuffer
      } =
        root.context.Node.allGLData;
      let scaleVecData =
        if scalable {
          (root.layout.width, root.layout.height)
        } else {
          (1.0, 1.0)
        };
      if (not root.context.Node.isDataSentToGPU) {
        drawGeometrySendData
          vertexBuffer::vertexArrayBuffer
          elementBuffer::elementArrayBuffer
          ::vertexArray
          ::elementArray
          ::count
          ::textureBuffer
          posVecData::(floor absoluteLeft, floor absoluteTop)
          ::scaleVecData;
        root.context.Node.isDataSentToGPU = true
      } else {
        drawGeometry2
          vertexBuffer::vertexArrayBuffer
          elementBuffer::elementArrayBuffer
          ::count
          ::textureBuffer
          posVecData::(floor absoluteLeft, floor absoluteTop)
          ::scaleVecData
      };
      Array.iter (fun child => traverseAndDraw child absoluteLeft absoluteTop) root.children
    }
  );
