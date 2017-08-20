(***********************************************************************)
(*                                                                     *)
(*                           Objective Caml                            *)
(*                                                                     *)
(*            Fran�ois Pessaux, projet Cristal, INRIA Rocquencourt     *)
(*            Pierre Weis, projet Cristal, INRIA Rocquencourt          *)
(*            Jun Furuse, projet Cristal, INRIA Rocquencourt           *)
(*                                                                     *)
(*  Copyright 1999-2004,                                               *)
(*  Institut National de Recherche en Informatique et en Automatique.  *)
(*  Distributed only by permission.                                    *)
(*                                                                     *)
(***********************************************************************)

(* $Id: rgba32.ml,v 1.5 2009/07/04 03:39:28 furuse Exp $ *)

open Util

module E = struct
  open Color
  type t = Color.rgba
  let bytes_per_pixel = 4
  let get str pos =
    { color =
        { r = int_of_char str.[pos    ];
          g = int_of_char str.[pos + 1];
          b = int_of_char str.[pos + 2] };
      alpha = int_of_char str.[pos + 3] }
  let set str pos t =
    str << pos     & char_of_int t.color.r;
    str << pos + 1 & char_of_int t.color.g;
    str << pos + 2 & char_of_int t.color.b;
    str << pos + 3 & char_of_int t.alpha
  let make t =
    let str = Bytes.create bytes_per_pixel in
    set str 0 t;
    str
end

module RI = Genimage.MakeRawImage(E)

type rawimage = RI.t
type elt = Color.rgba
type t = {
  width : int;
  height : int;
  rawimage : RI.t;
  mutable infos : Info.info list;
 }

module C = struct
  type rawimage = RI.t
  type container = t
  let rawimage x = x.rawimage
  let create_default width height rawimage =
    { width = width;
      height = height;
      rawimage = rawimage;
      infos = []; }
  let create_duplicate src width height rawimage =
    { width = width;
      height = height;
      rawimage = rawimage;
      infos = src.infos; }
end

module IMAGE = Genimage.Make(RI)(C)

let create_with width height infos data =
  { width = width;
    height = height;
    rawimage = RI.create_with width height data;
    infos = infos; }

let create_with_scanlines width height infos data =
  { width = width;
    height = height;
    rawimage = RI.create_with_scanlines width height data;
    infos = infos; }

let rawimage = C.rawimage
let create = IMAGE.create
let make = IMAGE.make
let dump = IMAGE.dump
let unsafe_access = IMAGE.unsafe_access
let get_strip = IMAGE.get_strip
let set_strip = IMAGE.set_strip
let get_scanline = IMAGE.get_scanline
let set_scanline = IMAGE.set_scanline
let unsafe_get = IMAGE.unsafe_get
let unsafe_set = IMAGE.unsafe_set
let get = IMAGE.get
let set = IMAGE.set
let destroy = IMAGE.destroy
let copy = IMAGE.copy
let sub = IMAGE.sub
let blit = IMAGE.blit
let map = IMAGE.map
let blocks = IMAGE.blocks
let dump_block = IMAGE.dump_block

open Color

(* image resize with smoothing *)
(* good result for reducing *)
let resize_reduce prog img nw nh =
  let newimage = create nw nh in
  let xscale = float nw /. float img.width in
  let yscale = float nh /. float img.height in

  let xs = Array.init nw (fun x ->
    let sx = truncate (float x /. xscale) in
    let ex = truncate ((float x +. 0.99) /. xscale) in
    let dx = ex - sx + 1 in
    (sx, ex, dx)) in
  let ys = Array.init nh (fun y ->
    let sy = truncate (float y /. yscale) in
    let ey = truncate ((float y +. 0.99) /. yscale) in
    let dy = ey - sy + 1 in
    (sy, ey, dy)) in
  for x = 0 to nw - 1 do
    let sx, ex, dx = xs.(x) in
    for y = 0 to nh - 1 do
      let sy, ey, dy = ys.(y) in
      let size = dx * dy in
      let sr = ref 0
      and sg = ref 0
      and sb = ref 0
      and sa = ref 0 in
      for xx = sx to ex do
        for yy = sy to ey do
          let c = unsafe_get img xx yy in
          sr := !sr + c.color.r;
          sg := !sg + c.color.g;
          sb := !sb + c.color.b;
          sa := !sa + c.alpha
        done
      done;
      unsafe_set newimage x y
        { color = { r = !sr / size; g = !sg / size; b = !sb / size };
          alpha = !sa / size; }
    done;

    match prog with
    | Some p -> p (float (x + 1) /. float nw)
    | None -> ()
  done;
  newimage

let resize_enlarge prog img nw nh =
  let newimage = create nw nh in
  let xscale = float nw /. float img.width in
  let yscale = float nh /. float img.height in

  let ww = truncate (ceil xscale)
  and wh = truncate (ceil yscale) in

  let weight =
    Array.init ww (fun x ->
      Array.init wh (fun y ->
        let x0 = x - ww / 2
        and y0 = y - wh / 2 in
        let x1 = x0 + ww - 1
        and y1 = y0 + wh - 1 in
        Array.init 3 (fun xx ->
          Array.init 3 (fun yy ->
            let mx0 = (xx - 1) * ww
            and my0 = (yy - 1) * wh in
            let mx1 = mx0 + ww - 1
            and my1 = my0 + wh - 1 in

            let cx0 = if x0 < mx0 then mx0 else x0 in
            let cy0 = if y0 < my0 then my0 else y0 in
            let cx1 = if x1 > mx1 then mx1 else x1 in
            let cy1 = if y1 > my1 then my1 else y1 in

            let dx = cx1 - cx0 + 1
            and dy = cy1 - cy0 + 1 in
            let dx = if dx < 0 then 0 else dx
            and dy = if dy < 0 then 0 else dy in
            dx * dy)))) in

  let wsum =
    Array.init ww (fun x ->
      Array.init wh (fun y ->
        let sum = ref 0 in
        Array.iter
          (Array.iter (fun w ->  sum := !sum + w))
          weight.(x).(y);
        if !sum = 0 then failwith "resize_enlarge wsum";
        !sum)) in

  let xs = Array.init img.width (fun x ->
    let sx = truncate (float x *. xscale) in
    let ex = truncate (float (x + 1) *. xscale) - 1 in
    let dx = ex - sx + 1 in
    if dx > ww then failwith "resize_enlarge";
    (sx, ex, dx)) in
  let ys = Array.init img.height (fun y ->
    let sy = truncate (float y *. yscale) in
    let ey = truncate (float (y + 1) *. yscale) - 1 in
    let dy = ey - sy + 1 in
    if dy > wh then failwith "resize_enlarge";
    (sy, ey, dy)) in

  let query c x y =
    if x < 0 || y < 0 || x >= img.width || y >= img.height
    then c
    else unsafe_get img x y in

  for y = 0 to img.height - 1 do

    let sy, _ey, dy = ys.(y) in

    for x = 0 to img.width - 1 do
      let sx, _ex, dx = xs.(x) in

      let colors =
        let c = unsafe_get img x y in
        Array.init 3 (fun dx ->
          Array.init 3 (fun dy ->
            query c (x + dx - 1) (y + dy - 1))) in

      for xx = 0 to dx - 1 do
        for yy = 0 to dy - 1 do
          let sr = ref 0
          and sg = ref 0
          and sb = ref 0
          and sa = ref 0 in
          let weight = weight.(xx).(yy) in
          let wsum = wsum.(xx).(yy) in
          for xxx = 0 to 2 do
            for yyy = 0 to 2 do
              let c = colors.(xxx).(yyy) in
              sr := !sr + c.color.r * weight.(xxx).(yyy);
              sg := !sg + c.color.g * weight.(xxx).(yyy);
              sb := !sb + c.color.b * weight.(xxx).(yyy);
              sa := !sa + c.alpha * weight.(xxx).(yyy);
            done
          done;
          unsafe_set newimage (sx + xx) (sy + yy)
            {color= {r = !sr / wsum; g = !sg / wsum; b = !sb / wsum};
             alpha = !sa / wsum}
        done
      done
    done;

    match prog with
    | Some p -> p (float (y + 1) /. float img.height)
    | None -> ()

  done;
  newimage

let resize prog img nw nh =
  let xscale = float nw /. float img.width in
  let yscale = float nh /. float img.height in
  if xscale >= 1.0 && yscale >= 1.0 then resize_enlarge prog img nw nh else
  if xscale <= 1.0 && yscale <= 1.0 then resize_reduce prog img nw nh
  else resize_reduce prog img nw nh


(*
(* image resize with smoothing *)
let resize prog img nw nh =
  let newimage = create nw nh in
  let xscale = float nw /. float img.width in
  let yscale = float nh /. float img.height in
  for y = 0 to nh - 1 do
    for x = 0 to nw - 1 do
      let sx = truncate (float x /. xscale)
      and sy = truncate (float y /. yscale)
      in
      let ex = truncate ((float x +. 0.99) /. xscale)
      and ey = truncate ((float y +. 0.99) /. yscale)
      in
(*
      let ex = if ex >= img.width then img.width - 1 else ex
      and ey = if ey >= img.height then img.height - 1 else ey
      in
*)
      let size = (ex - sx + 1) * (ey - sy + 1) in
      let sr = ref 0
      and sg = ref 0
      and sb = ref 0
      and sa = ref 0
      in
      for xx = sx to ex do
          for yy = sy to ey do
            let c = unsafe_get img xx yy in
            sr := !sr + c.color.r;
            sg := !sg + c.color.g;
            sb := !sb + c.color.b;
          sa := !sa + c.alpha
          done
      done;
      unsafe_set newimage x y { color = { r = (!sr/size);
                                          g = (!sg/size);
                                          b = (!sb/size) };
                                alpha = (!sa/size) }
    done;

    match prog with
      Some p -> p (float (y + 1) /. float img.height)
    | None -> ()

  done;
  newimage

*)
