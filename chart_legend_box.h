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

class LegendBox
{
  friend class Main;
  friend class Series;

private:

  SVG::BoundaryBox bb;

  uint32_t nx = 1;      // Number of columns.
  uint32_t sx = 0;      // Spare in last row.

  uint32_t collisions = 0;

};

}
