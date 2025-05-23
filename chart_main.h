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
#include <chart_legend.h>
#include <chart_legend_box.h>

namespace Chart {

class Ensemble;

class Main
{
  friend class HTML;
  friend class Ensemble;

public:

  Main( Ensemble* ensemble, SVG::Group* svg_g );
  ~Main( void );

  SVG::Group* GetGroup( void ) { return svg_g; }

  // Used to move the completed chart (i.e. after Build()) to its
  // final position in the grid,
  void Move( SVG::U dx, SVG::U dy );

  void SetChartArea( SVG::U width, SVG::U height );
  void SetChartBox( bool chart_box = true );

  SVG::Color* ChartAreaColor( void ) { return &chart_area_color; }
  SVG::Color* AxisColor( void ) { return &axis_color; }
  SVG::Color* TextColor( void ) { return &text_color; }

  // Specify alternative background color of framed title and legend frames.
  SVG::Color* FrameColor( void ) { return &frame_color; }

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetTitlePos( Pos pos_x, Pos pos_y = Pos::Top );
  void SetTitleInside( bool inside = true );
  void SetTitleSize( float size ) { title_size = size; }

  // Force the title frame to be drawn or not instead of it being determined
  // automatically.
  void SetTitleFrame( bool enable = true );

  void SetLegendHeading( const std::string& txt );

  // Force the legend frame to be drawn or not instead of it being determined
  // automatically.
  void SetLegendFrame( bool enable = true );

  // Normally it will strive to place the series legends somewhere inside the
  // chart area, but if the legends obscure too much of the charts you may
  // specify a location outside the chart area.
  void SetLegendPos( Pos pos );

  // Legend text size scaling factor.
  void SetLegendSize( float size );

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

  void Build( void );

private:

  Ensemble* ensemble = nullptr;
  SVG::Group* svg_g = nullptr;
  SVG::U g_dx = 0;
  SVG::U g_dy = 0;

  SVG::U chart_w   = 1000;
  SVG::U chart_h   = 600;
  bool   chart_box = false;

  void CalcLegendBoxes(
    SVG::Group* g, std::vector< LegendBox >& lb_list,
    const std::vector< SVG::Object* >& avoid_objects
  );
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

  void BuildTitle(
    std::vector< SVG::Object* >& avoid_objects
  );

  // Get the padding around the core chart area required to account for markers
  // and/or lines which due to their width may spill out of of the chart area.
  SVG::U GetAreaPadding( void );

  // Transfer various information to the HTML object (ensemble->html_db).
  void PrepareHTML( void );

  SVG::Color chart_area_color;
  SVG::Color axis_color;
  SVG::Color text_color;
  SVG::Color frame_color;

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

  Legend* legend_obj;
  bool    legend_frame;
  bool    legend_frame_specified;

  float bar_one_width = 1.00;
  float bar_all_width = 0.85;
  float bar_margin    = 0.00;

  Label* label_db;
  Tag* tag_db;

  std::vector< Series* > series_list;

  std::vector< std::string > category_list;

  Axis* axis_x;
  Axis* axis_y[ 2 ];

  // Used by HTML class.
  struct html_t {
    struct snap_point_t {
      uint32_t series_id;
      uint32_t cat_idx;
      SVG::Point p;
      std::string_view tag_x;
      std::string_view tag_y;
    };

    std::vector< snap_point_t > snap_points;

    // Informs if all snap points are in line; for multiple bars per category
    // this will not be the case.
    bool all_inline = true;

    // Specify if the chart X-axis is vertical.
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
  };
  html_t html;

};

}
