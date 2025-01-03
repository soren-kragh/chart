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

#include <svg_canvas.h>
#include <chart_common.h>
#include <chart_datum.h>
#include <chart_legend_box.h>

namespace Chart {

class Axis;

class Series
{
  friend class Main;

public:

  Series( SeriesType type );
  ~Series( void );

  void SetName( const std::string name );

  // Select primary (0) or secondary (1) Y-axis; default is primary.
  void SetAxisY( int axis_y_n );

  // Sets the base for area and bar type series.
  void SetBase( double base );

  void SetStyle( int style );

  // Line width also affects outline of hollow markers.
  SVG::Color* LineColor( void ) { return &line_color; }
  void SetLineWidth( SVG::U width );
  void SetLineDash( SVG::U dash );
  void SetLineDash( SVG::U dash, SVG::U hole );

  SVG::Color* FillColor( void ) { return &fill_color; }

  void SetMarkerSize( SVG::U size );
  void SetMarkerShape( MarkerShape shape );

  // For series types where the X-value is a string (all but XY and Scatter),
  // the X-value below is an index into Chart::Main::categoty_list. You should
  // never add numbers with a magnitude larger than mum_hi, as they could
  // otherwise be mistaken for the special values num_invalid and num_skip.
  // You can however explicitly add the special numbers num_invalid and num_skip.
  void Add( double x, double y );

  uint32_t Size( void ) { return datum_list.size(); }

private:

  void ApplyFillStyle( SVG::Object* obj );
  void ApplyLineStyle( SVG::Object* obj );
  void ApplyMarkStyle( SVG::Object* obj );
  void ApplyHoleStyle( SVG::Object* obj );

  bool Inside(
    const SVG::Point p, const SVG::BoundaryBox& clip_box
  );

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
    const SVG::BoundaryBox& clip_box
  );

  SVG::Point MoveInside(
    SVG::Point p, const SVG::BoundaryBox& clip_box
  );

  void UpdateLegendBoxes(
    std::vector< LegendBox >& lb_list,
    SVG::Point p1, SVG::Point p2,
    bool p1_inc = true, bool p2_inc = true
  );

  // Computes if the series must stack above base or below base:
  //    +1 : Stack above base.
  //    -1 : Stack below base.
  //     0 : No preferred stack direction.
  int GetStackDir( Axis* y_axis );

  void Build(
    SVG::Group* main_g,
    SVG::Group* area_fill_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos = nullptr,
    std::vector< double >* ofs_neg = nullptr,
    std::vector< SVG::Point >* pts_pos = nullptr,
    std::vector< SVG::Point >* pts_neg = nullptr
  );
  void BuildArea(
    const SVG::BoundaryBox& clip_box,
    SVG::Group* fill_g,
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos,
    std::vector< double >* ofs_neg,
    std::vector< SVG::Point >* pts_pos,
    std::vector< SVG::Point >* pts_neg
  );
  void BuildBar(
    const SVG::BoundaryBox& clip_box,
    SVG::Group* fill_g,
    SVG::Group* tbar_g,         // Used for thin bars
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos,
    std::vector< double >* ofs_neg
  );
  void BuildLine(
    const SVG::BoundaryBox& clip_box,
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list
  );

  SeriesType type;
  std::string name;
  int axis_y_n;
  double base;

  std::vector< SVG::Color > color_list;
  SVG::Color line_color;
  SVG::U line_width;
  SVG::U line_dash;
  SVG::U line_hole;

  SVG::Color fill_color;

  // Used for floating point precision issues.
  double e1 = 0;
  double e2 = 0;

  float bar_one_width = 1.00;
  float bar_all_width = 0.85;

  std::vector< Datum > datum_list;

  SVG::U      marker_size;
  MarkerShape marker_shape;

  typedef struct {
    SVG::U x1;
    SVG::U y1;
    SVG::U x2;
    SVG::U y2;
  } MarkerDims;

  // Derived marker variables:
  bool       marker_show;
  bool       marker_show_out;
  bool       marker_show_int;
  MarkerDims marker_int;        // Interior marker dimension.
  MarkerDims marker_out;        // Outer marker dimension.

  bool has_line;
  bool has_fill;

  // Compute derived marker_* variables and other visual properties.
  void DetermineVisualProperties( void );

  // Build marker based on marker_* variables.
  void BuildMarker( SVG::Group* g, const MarkerDims& m, SVG::Point p );
};

}
