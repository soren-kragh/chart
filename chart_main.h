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

#include <list>

#include <chart_common.h>
#include <chart_series.h>
#include <chart_axis.h>
#include <chart_legend_box.h>

namespace Chart {

class Main
{

public:

  Main( void );
  ~Main( void );

  void SetChartArea( SVG::U width, SVG::U height );
  void SetMargin( SVG::U margin );

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetFootnote( std::string txt );
  void SetFootnotePos( Pos pos );

  // Normally it will strive to place the series legends somewhere inside the
  // chart area, but if the legends obscure too much of the charts you may
  // specify a location outside the chart area.
  void SetLegendPos( Pos pos );

  // Specify the relative width of bars (0.0 to 1.0) and the relative width (0.0
  // to 1.0) of all bars belonging to the same X-value.
  void SetBarWidth( float one_width, float all_width );

  Axis* AxisX( void ) { return axis_x; }
  Axis* AxisY( int n = 0 ) { return axis_y[ n ]; }

  // A default style a automatically assigned new series, but style properties
  // can subsequently be changed.
  Series* AddSeries( SeriesType type );

  // Add categories for string based X-values.
  void AddCategory( const std::string category );

  SVG::Canvas* Build( void );

private:

  typedef struct {
    SVG::U ch;  // Character height.
    SVG::U mw;  // Max outline width.
    SVG::U cr;  // Outline corner radius.
    SVG::U ss;  // Symbol size.
    SVG::U ex;  // Extra X caused by symbol left overhang.
    SVG::U tx;  // Text indentation relative to center of symbol/marker.
    SVG::U dx;  // Delta between individual legends in X direction.
    SVG::U dy;  // Delta between individual legends in Y direction.
    SVG::U sx;  // Size in X direction.
    SVG::U sy;  // Size in Y direction.
    SVG::U mx;  // Legend box margin in X direction.
    SVG::U my;  // Legend box margin in Y direction.
  } LegendDims;

  void AxisPrepare( void );

  uint32_t LegendCnt( void );
  void CalcLegendDims( SVG::Group* g, LegendDims& legend_dims );
  void CalcLegendBoxes(
    SVG::Group* g, std::vector< LegendBox >& lb_list,
    const std::vector< SVG::Object* >& axis_objects
  );
  void BuildLegend( SVG::Group* g, int nx );
  void PlaceLegend(
    const std::vector< SVG::Object* >& axis_objects,
    const std::vector< LegendBox >& lb_list,
    SVG::Group* legend_g
  );

  // Compute the category stride, i.e. the minimum distance between non empty
  // string categories.
  int CategoryStride( void );

  void BuildSeries(
    SVG::Group* below_axes_g,
    SVG::Group* above_axes_g,
    std::vector< LegendBox >& lb_list
  );

  // Create an invisible rectangle around the chart area with extra margin to
  // account for markers and/or lines which due to their width may spill out of
  // of the chart area. Doing this ensures consistent chart dimensions
  // independent of the data values close to the edge of the chart area.
  void AddChartMargin(
    SVG::Group* chart_g
  );

  std::string title;
  std::string sub_title;
  std::string sub_sub_title;
  std::string footnote;
  Pos         footnote_pos;

  Pos legend_pos;

  SVG::U margin  = 5;
  SVG::U chart_w = 1000;
  SVG::U chart_h = 700;

  float bar_one_width = 1.00;
  float bar_all_width = 0.85;

  std::vector< Series* > series_list;

  std::vector< std::string > category_list;

  Axis* axis_x;
  Axis* axis_y[ 2 ];

  // When shoving the legend ID for a series having an area, this defines the
  // size relative to the characters.
  double legend_area_id_fact = 1.2;
};

}
