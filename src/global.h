#pragma once

std::string replace_first_occurence(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

inline float filter_gaussian(AtVector2 p, float width) {
  const float r = AiSqr(2.0 / width) * (AiSqr(p.x) + AiSqr(p.y));
  if (r > 1.0f) return 0.0;
  return AiFastExp(2 * -r);
}

inline float linear_interpolate(float perc, float a, float b){
    return a + perc * (b - a);
}