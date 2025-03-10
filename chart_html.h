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

#include <map>
#include <chart_common.h>

namespace Chart {

class Main;
class Axis;
class Series;

class HTML
{
  friend class Axis;
  friend class Series;

public:

  HTML( Main* main ) : main( main ) {}

  void DefAxisX(
    int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );
  void DefAxisY(
    int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );

  // Specify if the chart X-axis is vertical.
  void SwapAxis( bool swap = true ) { axis_swap = swap; };

  void LegendPos( Series* series, const SVG::BoundaryBox& bb );
  void MoveLegends( SVG::U dx, SVG::U dy );

  void AddSnapPoint(
    Series* series,
    SVG::Point p, std::string_view tag_x, std::string_view tag_y
  );
  void AddSnapPoint(
    Series* series,
    SVG::Point p, uint32_t cat_idx, std::string_view tag_y
  );

  // Informs if all snap points are in line; for multiple bars per category
  // this will not be the case.
  void SetAllInline( bool all_inline ) { this->all_inline = all_inline; };

  std::string GenHTML( SVG::Canvas* canvas );

private:

  Main* main;

  bool all_inline = true;

  struct snap_point_t {
    uint32_t series_id;
    uint32_t cat_idx;
    SVG::Point p;
    std::string_view tag_x;
    std::string_view tag_y;
  };

  bool axis_swap = false;

  struct axis_t {
    Axis*        axis = nullptr;
    bool         is_cat;
    NumberFormat number_format;
    bool         number_sign;
    bool         logarithmic;
    double       val1;
    double       val2;
  };

  axis_t x_axis[ 2 ];
  axis_t y_axis[ 2 ];

  std::map< Series*, SVG::BoundaryBox > series_legend_map;

  std::vector< snap_point_t > snap_points;

};

}
