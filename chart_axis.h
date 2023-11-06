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

#include <chart_common.h>

namespace Chart {

class Axis
{
  friend class Main;
  friend class Series;

public:

  Axis( void );

  void SetRange( double min, double max, double orth_axis_cross );
  void SetTick( double major, int sub_divs = 0 );
  void SetGrid( bool major_enable = true, bool minor_enable = false );
  void SetNumberPos( Pos pos );
  void SetLabel( std::string label );
  void SetUnit( std::string unit );
  void SetUnitPos( Pos pos );

private:

  void AutoTick( void );

  void Build(
    int angle, const Axis& orth_axis,
    std::vector< SVG::Object* >& axes_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g, SVG::Group* label_g
  );

  SVG::U length;

  SVG::U arrow_length = 10;
  SVG::U arrow_width = 10;

  SVG::U overhang = 2*arrow_length;
  SVG::U tick_major_len = 8;
  SVG::U tick_minor_len = 4;

  double min;
  double max;
  double orth_axis_cross;
  double major;
  int    sub_divs;
  Pos    number_pos;
  bool   major_grid_enable;
  bool   minor_grid_enable;

  std::string label;
  std::string unit;
  Pos unit_pos;

};

}
