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

#include <chart_common.h>

namespace Chart {

class Axis
{
  friend class Main;
  friend class Series;

public:

  Axis( int angle );

  // For AxisStyle::Edge, the edge is determined by SetNumberPos() and the
  // orth_axis_cross is ignored.
  void SetStyle( AxisStyle style );

  // Position of axis, will override orth_axis_cross.
  void SetPos( Pos pos );

  void SetLogScale( bool log_scale = true );
  void SetNumberFormat( NumberFormat number_format );
  void SetNumberUnit( const std::string& txt );
  void ShowMinorNumbers( bool show_minor_mumbers = true );

  void SetRange( double min, double max, double orth_axis_cross );
  void SetRange( double min, double max );

  // For logarithmic scale, major refers to the power, usually 10.
  void SetTick( double major, int sub_divs = 0 );

  void SetGridStyle( GridStyle gs );
  void SetGrid( bool major_enable = true, bool minor_enable = false );
  void SetNumberPos( Pos pos );
  void SetLabel( const std::string& txt );
  void SetSubLabel( const std::string& txt );
  void SetUnit( const std::string& txt );
  void SetUnitPos( Pos pos );

private:

  int angle;

  // Should axis be shown.
  bool show;

  const double num_lo = 1e-300;
  const double num_hi = 1e+300;

  // Maximum number of decimals to show.
  const int precision = 10;

  // NumberFormat::Fixed number lower limit.
  const double lim = std::pow( double( 10 ), -precision ) * 0.65;

  int32_t digits;   // Before decimal point (includes any sign).
  int32_t decimals; // After decimal point;
  int32_t num_max_len;
  int32_t exp_max_len;
  int32_t ComputeDecimals( double v, bool update = false );
  int32_t NormalizeExponent( double& num );
  void ComputeNumFormat( void );

  std::string NumToStr( double v );
  SVG::Group* BuildNum( SVG::Group* g, double v, bool bold );

  void LegalizeMinor( void );
  void LegalizeMajor( void );
  void LegalizeMinMax( void );

  // Convert a value to an SVG coordinate.
  SVG::U Coor( double v );

  // Determines if coordinates are so near as to be considered the same.
  bool CoorNear( SVG::U c1, SVG::U c2 );

  // Determine if value is valid.
  bool Valid( double v )
  {
    return (std::abs( v ) < num_hi && (!log_scale || v > num_lo));
  }

  void BuildTicksHelper(
    double v, SVG::U v_coor, int32_t sn, bool at_zero,
    SVG::U min_coor, SVG::U max_coor, SVG::U eps_coor,
    std::vector< SVG::Object* >& axis_objects,
    std::vector< SVG::Object* >& num_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g,
    SVG::U sx, SVG::U sy
  );
  void BuildTicksNumsLinear(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g,
    SVG::U sx, SVG::U sy
  );
  void BuildTicksNumsLogarithmic(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g,
    SVG::U sx, SVG::U sy
  );

  void Build(
    uint32_t phase,
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g, SVG::Group* label_g
  );

  void BuildLabel(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* label_g
  );

  SVG::U length;

  AxisStyle style;
  Pos       pos;
  GridStyle grid_style;

  SVG::U arrow_length = 10;
  SVG::U arrow_width = 10;

  SVG::U overhang = 2*arrow_length;
  SVG::U tick_major_len = 8;
  SVG::U tick_minor_len = 4;

  // Axis number spacing from major tick.
  SVG::U num_space_x = 2;
  SVG::U num_space_y = 3;

  bool         log_scale;
  NumberFormat number_format;
  std::string  number_unit;
  bool         show_minor_mumbers;
  bool         show_minor_mumbers_auto;

  // Use to determine to min/max of the associated series data.
  bool   data_def;
  double data_min;
  double data_max;

  double min;
  double max;
  double orth_axis_cross;
  double major;
  int    sub_divs;
  Pos    number_pos;
  bool   major_grid_enable;
  bool   minor_grid_enable;
  bool   grid_set;

  std::string label;
  std::string sub_label;
  std::string unit;
  Pos unit_pos;

  SVG::U orth_length;
  SVG::U orth_length_ext[ 2 ];
  AxisStyle orth_style[ 2 ];
  SVG::U orth_axis_coor[ 2 ];

  bool   at_orth_min;
  bool   at_orth_max;
  SVG::U at_orth_coor;
};

}
