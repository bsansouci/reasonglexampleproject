(***********************************************************************)
(*                                                                     *)
(*                           Objective Caml                            *)
(*                                                                     *)
(*            Damien Doligez, projet Para, INRIA Rocquencourt          *)
(*                                                                     *)
(*  Copyright 1997 Institut National de Recherche en Informatique et   *)
(*  en Automatique.  All rights reserved.  This file is distributed    *)
(*  under the terms of the GNU Library General Public License, with    *)
(*  the special exception on linking described in file ../LICENSE.     *)
(*                                                                     *)
(***********************************************************************)

(* $Id: mylazy.ml,v 1.2 2004/09/23 07:20:20 weis Exp $ *)

(* Module [Lazy]: deferred computations *)

type 'a status =
  | Delayed of (unit -> 'a)
  | Value of 'a
  | Exception of exn


type 'a t = 'a status ref

exception Undefined

let make f = ref (Delayed f)
let make_val v = ref (Value v)

let force l =
  match !l with
  | Value v -> v
  | Exception e -> raise e
  | Delayed f ->
      l := Exception Undefined;
      try let v = f () in l := Value v; v
      with e -> l := Exception e; raise e

