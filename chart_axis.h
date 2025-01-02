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

  Axis( bool x_axis );

  // The angle may be 0 or 90 and the X- and Y-axes must be orthogonal.
  void SetAngle( int angle );

  // Reverse the axis direction.
  void SetReverse( bool reverse = true );

  void SetStyle( AxisStyle style );

  // Position of axis, will override orth_axis_cross. If pos is Pos::Base, then
  // axis_y_n indicates which Y-axis base is refers to.
  void SetPos( Pos pos, int axis_y_n = 0 );

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

  // Should axis be shown.
  bool show;

  bool x_axis;
  int  angle;
  bool category_axis;
  bool reverse;

  // Axis is one axis of a dual Y-axis.
  bool y_dual;

  // Orthogonal axis is dual, implies that this axis is the X-axis.
  bool orth_dual;

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

  // Determine if value is valid.
  bool Valid( double v )
  {
    return (std::abs( v ) <= num_hi && (!log_scale || v >= num_lo));
  }

  // Determine if value is a skipped data point.
  bool Skip( double v )
  {
    return v == num_skip;
  }

  void BuildTicksHelper(
    double v, SVG::U v_coor, int32_t sn, bool at_zero,
    SVG::U min_coor, SVG::U max_coor, SVG::U eps_coor,
    std::vector< SVG::Object* >& axis_objects,
    std::vector< SVG::Object* >& num_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g
  );
  void BuildTicksNumsLinear(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g
  );
  void BuildTicksNumsLogarithmic(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g
  );

  void BuildCategories(
    const std::vector< std::string >& category_list,
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* cat_g, SVG::Group* major_g
  );

  void BuildUnit(
    SVG::Group* unit_g,
    std::vector< SVG::Object* >& axis_objects
  );

  void Build(
    const std::vector< std::string >& category_list,
    uint32_t phase,
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
    SVG::Group* line_g, SVG::Group* num_g, SVG::Group* unit_g
  );

  void BuildLabel(
    std::vector< SVG::Object* >& axis_objects,
    SVG::Group* label_g
  );

  SVG::U length;

  // Indicates that we have a chart box.
  bool chart_box;

  AxisStyle style;
  Pos       pos;
  GridStyle grid_style;

  // If pos is Pos::Base, this variable indicates which y-axis it refers to, in
  // which case orth_axis_cross of that y-axis is where the base is at.
  int pos_base_axis_y_n;

  SVG::U arrow_length = 10;
  SVG::U arrow_width = 10;

  SVG::U overhang = 3*arrow_length;
  SVG::U tick_major_len = 8;
  SVG::U tick_minor_len = 4;

  // Axis number spacing from major tick.
  SVG::U num_space_x = 3;
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
  bool   data_min_is_base;
  bool   data_max_is_base;

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

  SVG::U    orth_length;
  AxisStyle orth_style[ 2 ];
  SVG::U    orth_axis_coor[ 2 ];        // Coordinates where orthogonal axes
                                        // cross this axis.
  bool      orth_reverse[ 2 ];

  SVG::U orth_coor;         // Coordinate of where this axis crosses the
                            // orthogonal axis.
  bool   orth_coor_is_min;  // This axis placed at min orthogonal
                            // coordinate (0).
  bool   orth_coor_is_max;  // This axis placed at max orthogonal
                            // coordinate (orth_length).

  // The coordinate of the category "number" line, which is always at one of
  // the four sides.
  SVG::U cat_coor;

  // The minimum distance between non empty string categories.
  int category_stride;
};

}
