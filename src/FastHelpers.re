module type t = {
  let unsafe_update_float32:
    'a => int => mul::float => add::float => unit;
};
