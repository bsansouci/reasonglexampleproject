/*
  Ok brain dump time.

  We should set things up so that each Node has in its context:
    - vertex data
    - position
    - scale
    - vertex buffer object

  We'll be caching the vertex data, so we only ever send it once (the first time it appears let's
  say) and then we'll be changing its position and scaling it. We will not be sending any more
  data.

  We write a new shader that handles multiplying by a matrix which represents translate AND scale.
  At each object we just call draw immediately (for now) by binding the VBO and sending the
  position / scale matrix thing.



  Once that's setup look into vertex array objects (-> webgl doesn't support those for some reason)

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
  attribute vec3 aVertexPosition;
  attribute vec4 aVertexColor;
  attribute vec2 aTextureCoord;

  uniform mat4 uPMatrix;
  uniform mat4 posMatrix;

  varying vec4 vColor;
  varying vec2 vTextureCoord;

  void main(void) {
    gl_Position = posMatrix * uPMatrix * vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
    vTextureCoord = aTextureCoord;
  }
|};

let fragmentShaderSource = {|
  varying vec4 vColor;
  varying vec2 vTextureCoord;

  uniform sampler2D uSampler;

  void main(void) {
    vec4 tex = texture2D(uSampler, vTextureCoord);
    gl_FragColor = tex * vColor;

    // Other attempts at blending the texture and the color. The one used is the best one so far.
    // gl_FragColor = vColor;
    // gl_FragColor = vec4(vColor.xyz/tex.a, tex.a);
    // gl_FragColor = tex + vColor * tex.a;
    // gl_FragColor = texture2D(uSampler, vTextureCoord) + vColor;
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
/*let initGl () => {*/

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

/* Gl.clearColor context 1.0 1.0 1.0 1.0; */
Gl.clear ::context mask::(Constants.color_buffer_bit lor Constants.depth_buffer_bit);


/** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
let camera = {projectionMatrix: Gl.Mat4.create ()};


/**
 * Those buffers are basically pointers to chunks of memory on the graphics card. They're used to store the
 * vertex and color data.
 */
let vertexBuffer1 = Gl.createBuffer context;

let vertexBuffer2 = Gl.createBuffer context;

let elementBuffer1 = Gl.createBuffer context;

let elementBuffer2 = Gl.createBuffer context;

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

let posMatrix = Gl.getUniformLocation context program "posMatrix";

Gl.uniformMatrix4fv ::context location::posMatrix value::(Gl.Mat4.create ());

let aTextureCoord = Gl.getAttribLocation ::context ::program name::"aTextureCoord";

Gl.enableVertexAttribArray ::context attribute::aTextureCoord;


/** This tells OpenGL that we're going to be using texture0. OpenGL imposes a limit on the number of
    texture we can manipulate at the same time. That limit depends on the device. We don't care as we'll just
    always use texture0. **/
Gl.activeTexture ::context RGLConstants.texture0;

let uSampler = Gl.getUniformLocation ::context ::program name::"uSampler";


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

Gl.enable ::context RGLConstants.depth_test;

Gl.blendFunc ::context RGLConstants.src_alpha RGLConstants.one_minus_src_alpha;

let minZBuffer = 0.;

let maxZBuffer = 1000.;

let defaultZBuffer = 500.;


/**
 * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
 * See this link for quick explanation of what this is.
 * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
 */
let doOrtho () =>
  Gl.Mat4.ortho
    out::camera.projectionMatrix
    left::0.
    right::(float_of_int (Gl.Window.getWidth window))
    bottom::(float_of_int (Gl.Window.getHeight window))
    top::0.
    near::(-. maxZBuffer)
    far::(minZBuffer +. 1.);

doOrtho ();

let resizeWindow () => {
  let width = Gl.Window.getWidth window;
  let height = Gl.Window.getHeight window;
  Gl.viewport ::context x::0 y::0 ::width ::height;
  doOrtho ()
};

let circularBufferSize = 6 * 100;

let vertexSize = 9;

type batchT = {
  mutable elementPtr: int,
  mutable vertexPtr: int,
  mutable currTex: Gl.textureT,
  vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt,
  elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt
};

let batch1 = {
  elementPtr: 0,
  vertexPtr: 0,
  vertexArray: Gl.Bigarray.create Gl.Bigarray.Float32 (circularBufferSize * vertexSize),
  elementArray: Gl.Bigarray.create Gl.Bigarray.Uint16 circularBufferSize,
  currTex: nullTex
};

let batch2 = {
  elementPtr: 0,
  vertexPtr: 0,
  vertexArray: Gl.Bigarray.create Gl.Bigarray.Float32 (circularBufferSize * vertexSize),
  elementArray: Gl.Bigarray.create Gl.Bigarray.Uint16 circularBufferSize,
  currTex: nullTex
};

let curBatch = ref 0;

/*
 * This array packs all of the values that the shaders need: vertices, colors and texture coordinates.
 * We put them all in one as an optimization, so there are less back and forths between us and the GPU.
 *
 * The vertex array looks like:
 *
 * |<--------  9 * 4 bytes  ------->|
 *  --------------------------------
 * |  x  y  z  |  r  g  b  a  |  s  t  |  x2  y2  |  r2  g2  b2  a2  |  s2  t2  | ....
 *  --------------------------------
 * |             |              |
 * +- offset: 0 bytes, stride: 9 * 4 bytes (because we need to move by 8*4 bytes to get to the next x)
 *               |              |
 *               +- offset: 3 * 4 bytes, stride: 9 * 4 bytes
 *                              |
 *                              +- offset: (3 + 4) * 4 bytes, stride: 9 * 4 bytes
 *
 */
/* @Speed There are  couple things w could do to optimize this function.
     The first thing we could do is not reallocate arrays at each call.

     The third thing is to use triangles and element indices in conjunction with a larger
     buffer that we flush only a couple of times per frame. See Reprocessing which does that.


          Ben - August 19th 2017
   */
let drawGeometry
    vertexArray::(vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt)
    elementArray::(elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt)
    count::(count: int)
    textureBuffer::(textureBuffer: Gl.textureT)
    posMatrix::(pm: Gl.Mat4.t) => {
  if (!curBatch mod 2 === 0) {
    Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer1
  } else {
    Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer2
  };
  Gl.bufferData
    ::context target::Constants.array_buffer data::vertexArray usage::Constants.stream_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::3
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
    offset::(3 * 4);

  /** Texture */
  Gl.vertexAttribPointer
    ::context
    attribute::aTextureCoord
    size::2
    type_::Constants.float_
    normalize::false
    stride::(vertexSize * 4)
    offset::(7 * 4);

  /** */
  Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix;
  Gl.uniformMatrix4fv ::context location::posMatrix value::pm;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which
      is what texture0 represent.  **/
  Gl.uniform1i ::context location::uSampler val::0;

  /** */
  if (!curBatch mod 2 === 0) {
    Gl.bindBuffer ::context target::RGLConstants.element_array_buffer buffer::elementBuffer1
  } else {
    Gl.bindBuffer ::context target::RGLConstants.element_array_buffer buffer::elementBuffer2
  };

  /** Copy the `elementArray` into whatever buffer is in `element_array_buffer` **/
  Gl.bufferData
    ::context
    target::RGLConstants.element_array_buffer
    data::elementArray
    usage::RGLConstants.stream_draw;

  /** */
  Gl.bindTexture ::context target::RGLConstants.texture_2d texture::textureBuffer;

  /** Final call which actually does the "draw" **/
  Gl.drawElements
    ::context mode::Constants.triangles ::count type_::RGLConstants.unsigned_short offset::0
};

/*
 * Helper that will send the currently available data inside globalVertexArray.
 * This function assumes that the vertex data is stored as simple triangles.
 *
 * That function creates a new big array with a new size given the offset and len but does NOT copy the
 * underlying array of memory. So mutation done to that sub array will be reflected in the original one.
 */
let u = Gl.Mat4.create ();

let flushGlobalBatch textureBuffer =>
  if (!curBatch mod 2 === 0) {
    if (batch1.elementPtr > 0) {
      drawGeometry
        vertexArray::(Gl.Bigarray.sub batch1.vertexArray offset::0 len::batch1.vertexPtr)
        elementArray::(Gl.Bigarray.sub batch1.elementArray offset::0 len::batch1.elementPtr)
        count::batch1.elementPtr
        ::textureBuffer
        posMatrix::u;
      batch1.vertexPtr = 0;
      batch1.elementPtr = 0;
      batch1.currTex = textureBuffer;
      curBatch := !curBatch + 1
    }
  } else if (
    batch2.elementPtr > 0
  ) {
    drawGeometry
      vertexArray::(Gl.Bigarray.sub batch2.vertexArray offset::0 len::batch2.vertexPtr)
      elementArray::(Gl.Bigarray.sub batch2.elementArray offset::0 len::batch2.elementPtr)
      count::batch2.elementPtr
      ::textureBuffer
      posMatrix::u;
    batch2.vertexPtr = 0;
    batch2.elementPtr = 0;
    batch2.currTex = textureBuffer;
    curBatch := !curBatch + 1
  };

let maybeFlushBatch ::el ::vert textureBuffer =>
  if (!curBatch mod 2 === 0) {
    if (
      batch1.elementPtr + el >= circularBufferSize || batch1.vertexPtr + vert >= circularBufferSize
      /*|| batch1.elementPtr > 0 && batch1.currTex != textureBuffer*/
    ) {
      /*print_endline @@ Printf.sprintf "tex: %b" (batch1.currTex != textureBuffer);*/
      /*assert false;*/
      flushGlobalBatch textureBuffer
    }
  } else if (
    batch2.elementPtr + el >= circularBufferSize || batch2.vertexPtr + vert >= circularBufferSize
    /*|| batch2.elementPtr > 0 && batch2.currTex != textureBuffer*/
  ) {
    /*print_endline @@ Printf.sprintf "tex: %b" (batch2.currTex != textureBuffer);*/
    /*assert false;*/
    flushGlobalBatch textureBuffer
  };

let addRectToGlobalBatch = {
  let set = Gl.Bigarray.set;
  fun (x: float)
      (y: float)
      (z: float)
      (width: float)
      (height: float)
      (texX: float)
      (texY: float)
      (texW: float)
      (texH: float)
      (r, g, b, a)
      (textureBuffer: Gl.textureT) => {
    maybeFlushBatch el::6 vert::36 textureBuffer;
    let batch = !curBatch mod 2 === 0 ? batch1 : batch2;
    let i = batch.vertexPtr;
    let vertexArrayToMutate = batch.vertexArray;
    set vertexArrayToMutate (i + 0) (x +. width);
    set vertexArrayToMutate (i + 1) (y +. height);
    set vertexArrayToMutate (i + 2) z;
    set vertexArrayToMutate (i + 3) r;
    set vertexArrayToMutate (i + 4) g;
    set vertexArrayToMutate (i + 5) b;
    set vertexArrayToMutate (i + 6) a;
    set vertexArrayToMutate (i + 7) (texX +. texW);
    set vertexArrayToMutate (i + 8) (texY +. texH);
    set vertexArrayToMutate (i + 9) x;
    set vertexArrayToMutate (i + 10) (y +. height);
    set vertexArrayToMutate (i + 11) z;
    set vertexArrayToMutate (i + 12) r;
    set vertexArrayToMutate (i + 13) g;
    set vertexArrayToMutate (i + 14) b;
    set vertexArrayToMutate (i + 15) a;
    set vertexArrayToMutate (i + 16) texX;
    set vertexArrayToMutate (i + 17) (texY +. texH);
    set vertexArrayToMutate (i + 18) (x +. width);
    set vertexArrayToMutate (i + 19) y;
    set vertexArrayToMutate (i + 20) z;
    set vertexArrayToMutate (i + 21) r;
    set vertexArrayToMutate (i + 22) g;
    set vertexArrayToMutate (i + 23) b;
    set vertexArrayToMutate (i + 24) a;
    set vertexArrayToMutate (i + 25) (texX +. texW);
    set vertexArrayToMutate (i + 26) texY;
    set vertexArrayToMutate (i + 27) x;
    set vertexArrayToMutate (i + 28) y;
    set vertexArrayToMutate (i + 29) z;
    set vertexArrayToMutate (i + 30) r;
    set vertexArrayToMutate (i + 31) g;
    set vertexArrayToMutate (i + 32) b;
    set vertexArrayToMutate (i + 33) a;
    set vertexArrayToMutate (i + 34) texX;
    set vertexArrayToMutate (i + 35) texY;
    let ii = i / vertexSize;
    let j = batch.elementPtr;
    let elementArrayToMutate = batch.elementArray;
    set elementArrayToMutate (j + 0) ii;
    set elementArrayToMutate (j + 1) (ii + 1);
    set elementArrayToMutate (j + 2) (ii + 2);
    set elementArrayToMutate (j + 3) (ii + 1);
    set elementArrayToMutate (j + 4) (ii + 2);
    set elementArrayToMutate (j + 5) (ii + 3);
    batch.vertexPtr = i + 4 * vertexSize;
    batch.elementPtr = j + 6;
    batch.currTex = textureBuffer
  }
};

let fg = Images.{color: {r: 255, g: 255, b: 255}, alpha: 255};

let bg = Images.{color: {r: 0, g: 0, b: 0}, alpha: 0};

/*type ourOwnTextureT = {
    width: float,
    height: float,
    textureBuffer: Gl.textureT
  };*/
/* I don't know if this a hack or not anymore...
   I used to not memoize the text that we'd render, but then the current demo ran at 10fps.
   So I got annoyed and wrote this and now it's 60fps but feels a bit like cheating.

   Memory over time will grow etc etc... We should revisit thing some day.


       Ben - August 19th 2017
   */
/*module FontCompare = {
    type t = (string, OFreetype.face);
    let compare (a1, a2) (b1, b2) =>
      if (a2 === b2) {
        String.compare a1 b1
      } else {
        1
      };
  };
  */
/*module MemoizedText = Map.Make FontCompare;*/
/*let memoizedText = ref MemoizedText.empty;*/
/*let getFontMaxHeight face => {
    let (x1, y1, x2, y2) =
      face#size (Fttext.unicode_of_latin "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    let plus = 8.;
    y2 -. y1 +. plus +. 1.
  };

  let getFontBaseline face => {
    let (x1, y1, x2, y2) = face#size (Fttext.unicode_of_latin "m");
    let plus = 8.;
    y2 -. y1 +. plus
  };
  */
let measureText (s: string) ({textureBuffer, textureWidth, textureHeight, chars, kerning}: fontT) => {
  let offset = ref 0.;
  let prevChar = ref None;
  let maxBitmapHeight = ref 0.;
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
          /*          addRectToGlobalBatch
                      (x +. !offset +. bearingX +. kerningOffsetX)
                      (y -. bearingY -. kerningOffsetY)
                      defaultZBuffer
                      width
                      height
                      (atlasX /. textureWidth)
                      ((atlasY +. 1.) /. textureHeight)
                      (width /. textureWidth)
                      (height /. textureHeight)
                      color
                      textureBuffer;*/
          prevChar := Some code;
          offset := !offset +. advance;
          maxBitmapHeight := max !maxBitmapHeight height
        | exception Not_found =>
          failwith (Printf.sprintf "Couldn't find character %c in atlas :(" c)
        }
      }
    )
    s;
  (!offset, !maxBitmapHeight)
};

type vertexDataT = {
  vertexArrayBuffer: Gl.bufferT,
  elementArrayBuffer: Gl.bufferT,
  vertexArray: Gl.Bigarray.t float Gl.Bigarray.float32_elt,
  elementArray: Gl.Bigarray.t int Gl.Bigarray.int16_unsigned_elt,
  count: int,
  textureBuffer: Gl.textureT,
  posMatrix: Gl.Mat4.t
};

let generateTextVertexData
    (s: string)
    color
    ({textureBuffer, textureWidth, textureHeight, chars, kerning}: fontT) => {
  let vertexArray = Gl.Bigarray.create Gl.Bigarray.Float32 (6 * 1000 * vertexSize);
  let elementArray = Gl.Bigarray.create Gl.Bigarray.Uint16 (6 * 1000);
  let vertexPtr = ref 0;
  let elementPtr = ref 0;
  let set = Gl.Bigarray.set;
  let addRectToBatch
      (x: float)
      (y: float)
      (z: float)
      (width: float)
      (height: float)
      (texX: float)
      (texY: float)
      (texW: float)
      (texH: float)
      (r, g, b, a)
      (textureBuffer: Gl.textureT) => {
    /* @HACK This might segfault because we're not checking if we're going beyond the max size  */
    let i = !vertexPtr;
    set vertexArray (i + 0) (x +. width);
    set vertexArray (i + 1) (y +. height);
    set vertexArray (i + 2) z;
    set vertexArray (i + 3) r;
    set vertexArray (i + 4) g;
    set vertexArray (i + 5) b;
    set vertexArray (i + 6) a;
    set vertexArray (i + 7) (texX +. texW);
    set vertexArray (i + 8) (texY +. texH);
    set vertexArray (i + 9) x;
    set vertexArray (i + 10) (y +. height);
    set vertexArray (i + 11) z;
    set vertexArray (i + 12) r;
    set vertexArray (i + 13) g;
    set vertexArray (i + 14) b;
    set vertexArray (i + 15) a;
    set vertexArray (i + 16) texX;
    set vertexArray (i + 17) (texY +. texH);
    set vertexArray (i + 18) (x +. width);
    set vertexArray (i + 19) y;
    set vertexArray (i + 20) z;
    set vertexArray (i + 21) r;
    set vertexArray (i + 22) g;
    set vertexArray (i + 23) b;
    set vertexArray (i + 24) a;
    set vertexArray (i + 25) (texX +. texW);
    set vertexArray (i + 26) texY;
    set vertexArray (i + 27) x;
    set vertexArray (i + 28) y;
    set vertexArray (i + 29) z;
    set vertexArray (i + 30) r;
    set vertexArray (i + 31) g;
    set vertexArray (i + 32) b;
    set vertexArray (i + 33) a;
    set vertexArray (i + 34) texX;
    set vertexArray (i + 35) texY;
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
            (50. +. !offset +. bearingX +. kerningOffsetX)
            (50. -. bearingY -. kerningOffsetY)
            defaultZBuffer
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
  {
    vertexArray: Gl.Bigarray.sub vertexArray offset::0 len::!vertexPtr,
    elementArray: Gl.Bigarray.sub elementArray offset::0 len::!elementPtr,
    count: !elementPtr,
    textureBuffer,
    posMatrix: Gl.Mat4.create ()
  }
};

let drawText
    (x: float)
    (y: float)
    (z: float)
    (s: string)
    color
    ({textureBuffer, textureWidth, textureHeight, chars, kerning}: fontT) => {
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
          addRectToGlobalBatch
            (x +. !offset +. bearingX +. kerningOffsetX)
            (y -. bearingY -. kerningOffsetY)
            z
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
    s
};

let drawCircle x y ::radius color::(r, g, b, a) => {

  /** Instantiate a list of points for the circle and bind to the circleBuffer. **/
  let circle_vertex = ref [];
  for i in 0 to 360 {
    let deg2grad = 3.14159 /. 180.;
    let degInGrad = float_of_int i *. deg2grad;
    circle_vertex := [
      cos degInGrad *. radius +. x,
      sin degInGrad *. radius +. y,
      0.,
      ...!circle_vertex
    ]
  };
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer1;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 (Array.of_list !circle_vertex))
    usage::Constants.static_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::3
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
  Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix;
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

let noColor = (0., 0., 0., 0.);

let randomColor () => (Random.float 1., Random.float 1., Random.float 1., 1.);

type textT = {
  mutable font: fontT,
  text: string
};

/* Each node contains all possible values, it's probably faster than
   conditional on type. */
module Node = {
  type context = {
    /* Each field is mutable for performance reason. We want to encourage creating a pretty
       static tree that gets its nodes mutated and fully composited each frame. */
    /*mutable texture: Gl.textureT,*/
    /*mutable backgroundColor: (float, float, float, float),*/
    mutable visible: bool,
    mutable allGLData: vertexDataT
    /*mutable text: option textT*/
  };
  /* @Hack we're using nullTex which is a value that we're assuming has
       been loaded already. This is very bad but it'll work for now.
             Ben - August 19th 2017
     */
  let nullContext = {
    visible: true,
    allGLData: {
      vertexArrayBuffer: vertexBuffer1,
      elementArrayBuffer: elementBuffer1,
      vertexArray: Gl.Bigarray.create Gl.Bigarray.Float32 0,
      elementArray: Gl.Bigarray.create Gl.Bigarray.Uint16 0,
      count: 0,
      textureBuffer: nullTex,
      posMatrix: Gl.Mat4.create ()
    }
  };
};

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

/*let rec traverseAndDraw nodes left top depth::depth=defaultZBuffer => {
  let allChildren = ref [];
  let l = Array.length nodes;
  for i in (l - 1) downto 0 {
    let node = Array.get nodes i;
    if node.context.visible {
      let absoluteLeft = left +. node.layout.left;
      let absoluteTop = top +. node.layout.top;
      addRectToGlobalBatch
        (floor absoluteLeft)
        (floor absoluteTop)
        depth
        node.layout.width
        node.layout.height
        0.
        0.
        1.
        1.
        node.context.Node.backgroundColor
        node.context.Node.texture;
        allChildren := [Array.get nodes (l - i - 1), ...!allChildren];
      }
  };
  traverseAndDraw !allChildren */
  
  let rec traverseAndDraw ::depth=defaultZBuffer root left top =>
  Layout.(
    if root.context.visible {
      let absoluteLeft = left +. root.layout.left;
      let absoluteTop = top +. root.layout.top;
      switch root.context.Node.allGLData {
      | Some {vertexArray, elementArray, count, textureBuffer, posMatrix} =>
        drawGeometry ::vertexArray ::elementArray ::count ::textureBuffer ::posMatrix
      /*drawText
        (floor absoluteLeft)
        (floor absoluteTop)
        depth
        text
        root.context.Node.backgroundColor
        font*/
      | None =>
        switch root.context.text {
        | None =>
          addRectToGlobalBatch
            (floor absoluteLeft)
            (floor absoluteTop)
            depth
            root.layout.width
            root.layout.height
            0.
            0.
            (0.5 /. 2048.)
            (0.5 /. 2048.)
            root.context.Node.backgroundColor
            root.context.Node.texture
        | Some {text, font} =>
          drawText
            (floor absoluteLeft)
            (floor absoluteTop)
            depth
            text
            root.context.Node.backgroundColor
            font
        }
      /*print_endline @@ "None allGLData doing nothing for now"*/
      };
      Array.iter
        (fun child => traverseAndDraw child absoluteLeft absoluteTop depth::(depth +. 1.))
        root.children
    }
  );

/*let rec traverseAndDraw ::depth=defaultZBuffer root left top =>
  Layout.(
    if root.context.visible {
      let absoluteLeft = left +. root.layout.left;
      let absoluteTop = top +. root.layout.top;
      switch root.context.Node.allGLData {
      | Some {vertexArray, elementArray, count, textureBuffer, posMatrix} =>
        drawGeometry ::vertexArray ::elementArray ::count ::textureBuffer ::posMatrix
      /*drawText
        (floor absoluteLeft)
        (floor absoluteTop)
        depth
        text
        root.context.Node.backgroundColor
        font*/
      | None =>
        switch root.context.text {
        | None =>
          addRectToGlobalBatch
            (floor absoluteLeft)
            (floor absoluteTop)
            depth
            root.layout.width
            root.layout.height
            0.
            0.
            (0.5 /. 2048.)
            (0.5 /. 2048.)
            root.context.Node.backgroundColor
            root.context.Node.texture
        | Some {text, font} =>
          drawText
            (floor absoluteLeft)
            (floor absoluteTop)
            depth
            text
            root.context.Node.backgroundColor
            font
        }
      /*print_endline @@ "None allGLData doing nothing for now"*/
      };
      Array.iter
        (fun child => traverseAndDraw child absoluteLeft absoluteTop depth::(depth +. 1.))
        root.children
    }
  );

let traverseAndDraw root left top => {
  traverseAndDraw root left top;
  if (!curBatch mod 2 === 0) {
    flushGlobalBatch batch1.currTex
  } else {
    flushGlobalBatch batch2.currTex
  }
};*/
