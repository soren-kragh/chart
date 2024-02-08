//
//  MIT No Attribution License
//
//  Copyright 2024, Soren Kragh
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the
//  “Software”), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so.
//

#pragma once

#include <svg_canvas.h>

namespace Chart {

  // Correction factor for floating point precision issues in comparisons etc.
  const double epsilon = 1e-6;

  typedef enum { Auto, Center, Left, Right, Top, Bottom, Above, Below } Pos;

  typedef enum { Fixed, Scientific, Magnitude } NumberFormat;

  void TextBG(
    SVG::Group* g, const SVG::BoundaryBox& bb, SVG::U h
  );

  // Return newly created group object or nullptr if new_g is false.
  SVG::Group* MultiLineText(
    SVG::Group* g,
    bool new_g,
    const std::string txt,
    SVG::U size,
    bool bg,              // Add background.
    bool bg_truncate,     // Truncate leading/trailing white-space.
    bool bg_per_line      // Per line background.
  );

  SVG::Group* MultiLineText(
    SVG::Group* g, const std::string txt, SVG::U size = 0
  );

  SVG::Group* Label(
    SVG::Group* g, const std::string txt, SVG::U size = 0
  );

  bool Collides(
    SVG::Object* obj, const std::vector< SVG::Object* >& objects,
    SVG::U margin_x, SVG::U margin_y,
    SVG::BoundaryBox& bb
  );
  bool Collides(
    SVG::Object* obj, const std::vector< SVG::Object* >& objects,
    SVG::U margin_x, SVG::U margin_y
  );

}
