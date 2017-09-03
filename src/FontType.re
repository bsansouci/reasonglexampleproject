module type t = {
  let loadFont: fontSize::float => fontPath::string => id::int => Draw.fontT;
};
