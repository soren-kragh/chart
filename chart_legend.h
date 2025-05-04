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
#include <chart_label.h>
#include <chart_series.h>
#include <chart_legend_box.h>

namespace Chart {

class Legend
{
  friend class Main;

public:

  Legend( void );
  ~Legend( void );

private:

  typedef struct {
    SVG::U ch;  // Character height.
    SVG::U ow;  // Max outline width.
    SVG::U cr;  // Outline corner radius.
    SVG::U mw;  // Marker width.
    SVG::U mh;  // Marker height.
    SVG::U ss;  // Symbol size "radius" (including markers).
    SVG::U lx;  // Left extra X caused by symbol left overhang.
    SVG::U rx;  // Right extra X caused by symbol left overhang.
    SVG::U tx;  // Text indentation relative to center of symbol/marker.
    SVG::U dx;  // Delta between individual legends in X direction.
    SVG::U dy;  // Delta between individual legends in Y direction.
    SVG::U sx;  // Size in X direction.
    SVG::U sy;  // Size in Y direction.
    SVG::U mx;  // Legend box margin in X direction.
    SVG::U my;  // Legend box margin in Y direction.
    SVG::U hx;  // Heading X width.
    SVG::U hy;  // Heading Y width.
  } LegendDims;

  static uint32_t LegendCnt(
    std::vector< Series* >& series_list
  );

  static void CalcLegendDims(
    std::vector< Series* >& series_list,
    const std::string& legend_heading,
    bool framed, bool legend_outline,
    SVG::Group* g, Legend::LegendDims& legend_dims
  );

  static void BuildLegends(
    Ensemble* ensemble,
    std::vector< Series* >& series_list,
    const std::string& legend_heading,
    bool legend_outline,
    SVG::Color* frame_line_color,
    SVG::Color* frame_fill_color,
    SVG::Group* g, int nx, bool framed
  );

};

}
