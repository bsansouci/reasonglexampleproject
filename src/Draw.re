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

  varying vec4 vColor;
  varying vec2 vTextureCoord;

  void main(void) {
    gl_Position = uPMatrix * vec4(aVertexPosition, 0.0, 1.0);
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

let resizeWindow () => {
  let width = Gl.Window.getWidth window;
  let height = Gl.Window.getHeight window;
  Gl.viewport ::context x::0 y::0 ::width ::height;
  Gl.Mat4.ortho
    out::camera.projectionMatrix
    left::0.
    right::(float_of_int width)
    bottom::(float_of_int height)
    top::0.
    near::0.
    far::100.
};


/**
 * Those buffers are basically pointers to chunks of memory on the graphics card. They're used to store the
 * vertex and color data.
 */
let vertexBuffer = Gl.createBuffer context;

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

Gl.blendFunc ::context RGLConstants.src_alpha RGLConstants.one_minus_src_alpha;


/**
 * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
 * See this link for quick explanation of what this is.
 * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
 */
Gl.Mat4.ortho
  out::camera.projectionMatrix
  left::0.
  right::(float_of_int (Gl.Window.getWidth window))
  bottom::(float_of_int (Gl.Window.getHeight window))
  top::0.
  near::0.
  far::100.;

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
 * |           |              |
 * +- offset: 0 bytes, stride: 8 * 4 bytes (because we need to move by 8*4 bytes to get to the next x)
 *             |              |
 *             +- offset: 3 * 4 bytes, stride: 8 * 4 bytes
 *                            |
 *                            +- offset: (3 + 4) * 4 bytes, stride: 8 * 4 bytes
 *
 */
/* @Speed There are  couple things w could do to optimize this function.
     The first thing we could do is not reallocate arrays at each call.

     The third thing is to use triangles and element indices in conjunction with a larger
     buffer that we flush only a couple of times per frame. See Reprocessing which does that.


          Ben - August 19th 2017
   */
let drawRect
    (x: float)
    (y: float)
    (width: float)
    (height: float)
    (texX: float)
    (texY: float)
    (texW: float)
    (texH: float)
    (r, g, b, a)
    (texture: Gl.textureT) => {
  let packedVertexData = [|
    x +. width,
    y +. height,
    r,
    g,
    b,
    a,
    texX +. texW,
    texY +. texH,
    x,
    y +. height,
    r,
    g,
    b,
    a,
    texX,
    texY +. texH,
    x +. width,
    y,
    r,
    g,
    b,
    a,
    texX +. texW,
    texY,
    x,
    y,
    r,
    g,
    b,
    a,
    texX,
    texY
  |];
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
  Gl.bufferData
    ::context
    target::Constants.array_buffer
    data::(Gl.Bigarray.of_array Gl.Bigarray.Float32 packedVertexData)
    usage::Constants.stream_draw;
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexPosition
    size::2
    type_::Constants.float_
    normalize::false
    stride::(8 * 4)
    offset::0;

  /** Color */
  Gl.vertexAttribPointer
    ::context
    attribute::aVertexColor
    size::4
    type_::Constants.float_
    normalize::false
    stride::(8 * 4)
    offset::(2 * 4);

  /** Texture */
  Gl.vertexAttribPointer
    ::context
    attribute::aTextureCoord
    size::2
    type_::Constants.float_
    normalize::false
    stride::(8 * 4)
    offset::(6 * 4);
  Gl.uniformMatrix4fv ::context location::pMatrixUniform value::camera.projectionMatrix;

  /** Tell OpenGL about what the uniform called `uSampler` is pointing at, here it's given 0 which
      is what texture0 represent.  **/
  Gl.uniform1i ::context location::uSampler val::0;
  Gl.bindTexture ::context target::RGLConstants.texture_2d ::texture;

  /** Final call which actually does the "draw" **/
  Gl.drawArrays ::context mode::Constants.triangle_strip first::0 count::4
};

let fg = Images.{color: {r: 255, g: 255, b: 255}, alpha: 255};

let bg = Images.{color: {r: 0, g: 0, b: 0}, alpha: 0};

type ourOwnTextureT = {
  width: float,
  height: float,
  textureBuffer: Gl.textureT
};

/* I don't know if this a hack or not anymore...
   I used to not memoize the text that we'd render, but then the current demo ran at 10fps.
   So I got annoyed and wrote this and now it's 60fps but feels a bit like cheating.

   Memory over time will grow etc etc... We should revisit thing some day.


       Ben - August 19th 2017
   */
module FontCompare = {
  type t = (string, OFreetype.face);
  let compare (a1, a2) (b1, b2) =>
    if (a2 === b2) {
      String.compare a1 b1
    } else {
      1
    };
};

module MemoizedText = Map.Make FontCompare;

/*let memoizedText = ref MemoizedText.empty;*/
let getFontMaxHeight face => {
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

let drawText
    (x: float)
    (y: float)
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
          drawRect
            (x +. !offset +. bearingX +. kerningOffsetX)
            (y -. bearingY -. kerningOffsetY)
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

/*if (String.length s == 0) {
    {width: 0., height: 0., textureBuffer: nullTex}
  } else {
    try (MemoizedText.find (s, face) !memoizedText) {
    | Not_found =>*/
/* @Dumb Copy pasted from https://bitbucket.org/camlspotter/camlimages/src/b18e82a3d840c458f5db3f33309dd2d6e97bef91/examples/ttfimg/ttfimg.ml?at=default&fileviewer=file-view-default */
/*let plus = 8;
  let encoded = Fttext.unicode_of_latin s;
  let (x1, y1, x2, y2) = face#size encoded;
  let h = truncate (ceil y2) - truncate y1 + 1 + plus;
  let rgba = (new OImages.rgba32_filled) (truncate (x2 -. x1) + plus) h bg;
  OFreetype.draw_text
    face
    (
      fun org level => {
        let level' = 255 - level;
        Images.{
          color: {
            r: (org.color.r * level' + fg.color.r * level) / 255,
            g: (org.color.g * level' + fg.color.g * level) / 255,
            b: (org.color.b * level' + fg.color.b * level) / 255
          },
          alpha: (org.alpha * level' + fg.alpha * level) / 255
        }
      }
    )
    (rgba :> OImages.map Images.rgba)
    (plus / 2 - truncate x1)
    (truncate y2 + plus / 2)
    encoded;

  /** Custom code starts here. */
  let data = rgba#dump;
  let length = String.length data;
  /* @Speed We shouldn't have to allocate a new array of chars here, this is a waste.
     To fix this we might have to vendor camlimages and make them use a bigarray... */
  let bigarrayTextData = Gl.Bigarray.create Gl.Bigarray.Char length;
  for i in 0 to (length / 4 - 1) {
    Gl.Bigarray.set bigarrayTextData (4 * i) (char_of_int 255);
    Gl.Bigarray.set bigarrayTextData (4 * i + 1) (char_of_int 255);
    Gl.Bigarray.set bigarrayTextData (4 * i + 2) (char_of_int 255);

    /** What if we just take the alpha? What are you gonna do about it. HUH */
    Gl.Bigarray.set bigarrayTextData (4 * i + 3) (Bytes.get data (4 * i + 3))
  };
  let imageWidth = rgba#width;
  let imageHeight = rgba#height;*/
/* @Speed we shouldn't be creating a textureBuffer at EVERY FRAME. That's
       pretty hardcore. We should make this function take one and let the user
       figure out how they want to do their thing.
                  Ben - August 19th 2017
   /*       */
    let imageWidth = 2048;
    let imageHeight = 2048;
    let textureBuffer = Gl.createTexture context;
    Gl.bindTexture ::context target::Constants.texture_2d texture::textureBuffer;
    Gl.texImage2D_RGBA
      ::context
      target::Constants.texture_2d
      level::0
      width::imageWidth
      height::imageHeight
      border::0
      data::bigarrayTextData;
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
    let ret = {width: float_of_int imageWidth, height: float_of_int imageHeight, textureBuffer};
    /*memoizedText := MemoizedText.add (s, face) ret !memoizedText;*/
    ret*/
/*}*/
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
  Gl.bindBuffer ::context target::Constants.array_buffer buffer::vertexBuffer;
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

let clearScreen () => Gl.clear ::context mask::Constants.color_buffer_bit;

let red = (1., 0., 0., 1.);

let green = (0., 1., 0., 1.);

let blue = (0., 0., 1., 1.);

let white = (1., 1., 1., 1.);

let noColor = (0., 0., 0., 0.);

let randomColor () => (Random.float 1., Random.float 1., Random.float 1., 1.);

/* Each node contains all possible values, it's probably faster than
   conditional on type. */
module Node = {
  type context = {
    /* Each field is mutable for performance reason. We want to encourage creating a pretty
       static tree that gets its nodes mutated and fully composited each frame. */
    mutable texture: Gl.textureT,
    mutable backgroundColor: (float, float, float, float),
    mutable visible: bool
  };
  /* @Hack we're using nullTex which is a value that we're assuming has
       been loaded already. This is very bad but it'll work for now.
             Ben - August 19th 2017
     */
  let nullContext = {texture: nullTex, backgroundColor: noColor, visible: true};
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

let rec traverseAndDraw root left top =>
  Layout.(
    if root.context.visible {
      let absoluteLeft = left +. root.layout.left;
      let absoluteTop = top +. root.layout.top;
      drawRect
        (floor absoluteLeft)
        (floor absoluteTop)
        root.layout.width
        root.layout.height
        0.
        0.
        1.
        1.
        root.context.Node.backgroundColor
        root.context.Node.texture;
      Array.iter (fun child => traverseAndDraw child absoluteLeft absoluteTop) root.children
    }
  );
