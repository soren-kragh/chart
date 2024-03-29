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
  void SetBW( bool bw = true );

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetFootnote( std::string txt );
  void SetFootnotePos( Pos pos );

  // Normally it will strive to place the series legends somewhere inside the
  // chart area, but if the legends obscure too much of the charts you may
  // specify a location outside the chart area.
  void SetLegendPos( Pos pos );

  Axis* AxisX( void ) { return axis_x; }
  Axis* AxisY( int n = 0 ) { return axis_y[ n ]; }

  Series* AddSeries( std::string name );

  SVG::Canvas* Build( void );

private:

  void AxisPrepare( void );

  uint32_t LegendCnt( void );
  void CalcLegendSize( SVG::Group* g, SVG::U& ch, SVG::U& tw, SVG::U& th );
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

  std::string title;
  std::string sub_title;
  std::string sub_sub_title;
  std::string footnote;
  Pos         footnote_pos;

  Pos legend_pos;

  SVG::U margin  = 5;
  SVG::U chart_w = 1200;
  SVG::U chart_h = 800;
  bool bw        = false;

  std::list< Series* > series_list;

  Axis* axis_x;
  Axis* axis_y[ 2 ];

  SVG::U legend_bx = 8;         // X-border around text in series legends.
  SVG::U legend_by = 4;         // Y-border around text in series legends.
  SVG::U legend_sx = 12;        // X-space between series legends.
  SVG::U legend_sy = 12;        // Y-space between series legends.
};

}
