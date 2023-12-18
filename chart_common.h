//
//  Copyright (C) 2023, S. Kragh
//
//  This file is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License for more details.
//  <http://www.gnu.org/licenses/>.
//

#pragma once

#include <svg_canvas.h>

namespace Chart {

  typedef enum { Auto, Center, Left, Right, Top, Bottom, Above, Below } Pos;

  typedef enum { Fixed, Scientific, Magnitude } NumberFormat;

  SVG::Object* Label(
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
