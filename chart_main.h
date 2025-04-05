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
#include <chart_label.h>
#include <chart_tag.h>
#include <chart_html.h>
#include <chart_series.h>
#include <chart_axis.h>
#include <chart_legend_box.h>

namespace Chart {

class Main
{
  friend class HTML;

public:

  Main( void );
  ~Main( void );

  void EnableHTML( bool enable = true ) { enable_html = enable; }

  void SetBorderWidth( SVG::U width );
  void SetMargin( SVG::U margin );
  void SetChartArea( SVG::U width, SVG::U height );
  void SetChartBox( bool chart_box = true );

  SVG::Color* BorderColor( void ) { return &border_color; }
  SVG::Color* BackgroundColor( void ) { return &background_color; }
  SVG::Color* ChartAreaColor( void ) { return &chart_area_color; }
  SVG::Color* AxisColor( void ) { return &axis_color; }
  SVG::Color* TextColor( void ) { return &text_color; }

  // Specify alternative background color of title and legend frames.
  SVG::Color* FrameColor( void ) { return &frame_color; }

  void SetLetterSpacing(
    float width_adj, float height_adj = 1.0, float baseline_adj = 1.0
  );

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetTitlePos( Pos pos_x, Pos pos_y = Pos::Top );
  void SetTitleInside( bool inside = true );
  void SetTitleSize( float size ) { title_size = size; }

  // Force the title frame to be drawn or not instead of it being determined
  // automatically.
  void SetTitleFrame( bool enable = true );

  void AddFootnote( std::string& txt );

  // Applies to the most recently added footnote.
  void SetFootnotePos( Pos pos );

  // A line above the footnotes.
  void SetFootnoteLine( bool footnote_line = true );

  // Footnote size scaling factor.
  void SetFootnoteSize( float size ) { footnote_size = size; }

  void SetLegendHeading( const std::string& txt );

  // Force the legend frame to be drawn or not instead of it being determined
  // automatically.
  void SetLegendFrame( bool enable = true );

  // Normally it will strive to place the series legends somewhere inside the
  // chart area, but if the legends obscure too much of the charts you may
  // specify a location outside the chart area.
  void SetLegendPos( Pos pos );

  // Specify if line style legends are shown with an outline around the legend
  // text, or with a small line segment in front of the legend text.
  void SetLegendOutline( bool outline );

  // Legend text size scaling factor.
  void SetLegendSize( float size ) { legend_size = size; }

  // Specify the relative width of bars (0.0 to 1.0) and the relative width (0.0
  // to 1.0) of all bars belonging to the same X-value.
  void SetBarWidth( float one_width, float all_width );

  // Set extra start/end margin in units of bar buckets.
  void SetBarMargin( float margin );

  Axis* AxisX( void ) { return axis_x; }
  Axis* AxisY( int n = 0 ) { return axis_y[ n ]; }

  // A default style a automatically assigned new series, but style properties
  // can subsequently be changed.
  Series* AddSeries( SeriesType type );

  // Add categories for string based X-values.
  void AddCategory( const std::string& category );

  std::string Build( void );

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

  uint32_t LegendCnt( void );
  void CalcLegendDims( SVG::Group* g, LegendDims& legend_dims );
  void CalcLegendBoxes(
    SVG::Group* g, std::vector< LegendBox >& lb_list,
    const std::vector< SVG::Object* >& avoid_objects
  );
  void BuildLegends( SVG::Group* g, int nx, bool framed );
  void PlaceLegends(
    std::vector< SVG::Object* >& avoid_objects,
    const std::vector< LegendBox >& lb_list,
    SVG::Group* legend_g
  );

  // Compute the category stride, i.e. the minimum distance between non empty
  // string categories.
  int CatStrideEmpty( void );

  void AxisPrepare( SVG::Group* tag_g );

  void SeriesPrepare(
    std::vector< LegendBox >* lb_list
  );

  void BuildSeries(
    SVG::Group* below_axes_g,
    SVG::Group* above_axes_g,
    SVG::Group* tag_g
  );

  void AddTitle(
    SVG::Group* chart_g,
    std::vector< SVG::Object* >& avoid_objects
  );

  void AddFootnotes(
    SVG::Group* chart_g
  );

  // Create an invisible rectangle around the chart area with extra margin to
  // account for markers and/or lines which due to their width may spill out of
  // of the chart area. Doing this ensures consistent chart dimensions
  // independent of the data values close to the edge of the chart area.
  void AddChartMargin(
    SVG::Group* chart_g, bool do_area_margin
  );

  // Transfer various information to the HTML object (html_db).
  void PrepareHTML( void );

  bool enable_html = false;

  SVG::Color border_color;
  SVG::Color background_color;
  SVG::Color chart_area_color;
  SVG::Color axis_color;
  SVG::Color text_color;
  SVG::Color frame_color;

  float width_adj;
  float height_adj;
  float baseline_adj;

  std::string title;
  std::string sub_title;
  std::string sub_sub_title;
  Pos         title_pos_x;
  Pos         title_pos_y;
  bool        title_inside;
  float       title_size;
  bool        title_frame;
  bool        title_frame_specified;

  uint32_t bar_tot = 0;
  uint32_t lol_tot = 0;

  struct footnote_t {
    std::string txt;
    Pos pos;
  };
  std::vector< footnote_t > footnotes;
  bool footnote_line = false;
  float footnote_size = 1.0;

  std::string legend_heading;
  bool        legend_frame;
  bool        legend_frame_specified;
  Pos         legend_pos;
  bool        legend_outline;
  float       legend_size;

  SVG::U border_width = 0;
  SVG::U margin       = 5;
  SVG::U chart_w      = 1000;
  SVG::U chart_h      = 600;
  bool   chart_box    = false;

  float bar_one_width = 1.00;
  float bar_all_width = 0.85;
  float bar_margin    = 0.00;

  Label* label_db;
  Tag* tag_db;
  HTML* html_db;

  std::vector< Series* > series_list;

  std::vector< std::string > category_list;

  Axis* axis_x;
  Axis* axis_y[ 2 ];
};

}
