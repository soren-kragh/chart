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

  Axis( int angle );

  void SetLogScale( bool log_scale = true );
  void SetNumberFormat( NumberFormat number_format );
  void ShowMinorNumbers( bool show_minor_mumbers = true );

  void SetRange( double min, double max, double orth_axis_cross );
  void SetRange( double min, double max );

  // For logarithmic scale, major refers to the power, usually 10.
  void SetTick( double major, int sub_divs = 0 );

  void SetGrid( bool major_enable = true, bool minor_enable = false );
  void SetNumberPos( Pos pos );
  void SetLabel( std::string label );
  void SetUnit( std::string unit );
  void SetUnitPos( Pos pos );

private:

  int angle;

  const double num_lo = 1e-300;
  const double num_hi = 1e+300;

  // Maximum number of decimals to show.
  const int precision = 10;

  // Correction for rounding errors in comparisons etc.
  const double cre = std::pow( double( 10 ), -precision ) / 2;

  int32_t digits;   // Before decimal point (includes any sign).
  int32_t decimals; // After decimal point;
  int32_t num_max_len;
  int32_t exp_max_len;
  int32_t ComputeDecimals( double v, bool update = false );
  int32_t NormalizeExponent( double& num );
  void ComputeNumFormat( void );

  std::string NumToStr( double v );
  SVG::Object* BuildNum( SVG::Group* g, double v, bool bold );

  void AutoTick( void );

  // Convert a value to an SVG coordinate.
  SVG::U Coor( double v );

  // Determine if value is valid.
  bool Valid( double v )
  {
    return (std::abs( v ) < num_hi && (!log_scale || v > num_lo));
  }

  void BuildTicsNumsLinear(
    Axis& orth_axis,
    std::vector< SVG::Object* >& axes_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g,
    SVG::U sx, SVG::U sy, SVG::U ex, SVG::U ey
  );
  void BuildTicsNumsLogarithmic(
    Axis& orth_axis,
    std::vector< SVG::Object* >& axes_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g,
    SVG::U sx, SVG::U sy, SVG::U ex, SVG::U ey
  );

  void Build(
    Axis& orth_axis,
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

  bool         log_scale;
  NumberFormat number_format;
  bool         number_format_auto;
  bool         show_minor_mumbers;
  bool         show_minor_mumbers_auto;

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
