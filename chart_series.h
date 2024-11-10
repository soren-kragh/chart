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

  // For series types where the X value is a string (all but XY and Scatter),
  // the x-value below is an index into Chart::Main::categoty_list.
  void Add( double x, double y );

  uint32_t Size( void ) { return datum_list.size(); }

private:

  void ApplyLineStyle( SVG::Object* obj );
  void ApplyMarkStyle( SVG::Object* obj );
  void ApplyFillStyle( SVG::Object* obj );

  void UpdateLegendBoxes(
    std::vector< LegendBox >& lb_list, SVG::Point p1, SVG::Point p2
  );

  void Build(
    SVG::Group* g1,
    SVG::Group* g2,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos = nullptr,
    std::vector< double >* ofs_neg = nullptr
  );
  void BuildArea(
    const SVG::BoundaryBox& clip_box,
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* fill_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos,
    std::vector< double >* ofs_neg
  );
  void BuildBar(
    const SVG::BoundaryBox& clip_box,
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* fill_g,
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
    SVG::Group* fill_g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list
  );

  bool Inside(
    const SVG::Point p, const SVG::BoundaryBox& clip_box
  );

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
    const SVG::BoundaryBox& clip_box
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

  double bar_width = 0.85;
  double bar_cluster_width = 0.85;

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
  bool       marker_hollow;
  SVG::U     marker_diameter;
  SVG::U     marker_radius;
  MarkerDims marker_int;        // Interior dimension for hollow marker.
  MarkerDims marker_out;        // Outer marker dimension.
  MarkerDims marker_rim;        // Dimension of rim around marker.

  // Compute derived marker_* variables.
  void ComputeMarker( SVG::U rim = 0 );

  // Build marker based on marker_* variables.
  void BuildMarker( SVG::Group* g, const MarkerDims& m, SVG::Point p );
};

}
