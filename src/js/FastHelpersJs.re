external get : 'a => int => 'b = "" [@@bs.get_index];

external set : 'a => int => 'b => unit = "" [@@bs.set_index];

let unsafe_update_float32 arr i ::mul ::add => set arr i (get arr i *. mul +. add);
