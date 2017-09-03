/* "What is going on here" you may ask.
   Well we kinda sorta profiled the app and noticed that ba_caml_XYZ was called a lot.
   This is an attempt at reducing the cost of those calls. We implemented our own C blit (which is
   just memcpy) and a little helper which will update a field from the Bigarray in one shot.
   It'll just do arr[i] = arr[i] * mul + add; which turns out to be cheaper than doing a get and a
   set because Bigarray has to acquire the global lock for some reason. Here we don't care because
   we're not doing any threading (thank god).


            Ben - August 28th 2017
      */
external unsafe_update_float32 :
  'a => int => mul::float => add::float => unit =
  "unsafe_update_float32" [@@noalloc];
