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

  Series( std::string name );
  ~Series( void );

  void SetType( SeriesType type );

  // Select primary (0) or secondary (1) Y-axis; default is primary.
  void SetAxisY( int axis_y_n );

  void SetStyle( int style );
  SVG::Color* Color( void ) { return &color; }
  void SetWidth( SVG::U width );
  void SetDash( SVG::U dash );
  void SetDash( SVG::U dash, SVG::U hole );

  void SetMarkerSize( SVG::U marker_size );
  void SetMarkerShape( MarkerShape marker_shape );

  // For series types where the X value is a string (all but XY and Scatter),
  // the x-value below is an index into Chart::Main::categoty_list.
  void Add( double x, double y );

  uint32_t Size( void ) { return datum_list.size(); }

private:

  void ApplyLineStyle( SVG::Object* obj );
  void ApplyFillStyle( SVG::Object* obj );
  void ApplyHoleStyle( SVG::Object* obj );

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
    SVG::Group* hole_g,
    SVG::Group* area_g,
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

  bool Inside(
    const SVG::Point p, const SVG::BoundaryBox& clip_box
  );

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
    const SVG::BoundaryBox& clip_box
  );

  std::string name;
  SeriesType type;
  int axis_y_n;

  std::vector< SVG::Color > color_list;
  SVG::Color color;
  SVG::U width;
  SVG::U dash;
  SVG::U hole;

  // Used for floating point precision issues.
  double e1 = 0;
  double e2 = 0;

  double bar_width = 0.90;
  double bar_cluster_width = 0.85;

  std::vector< Datum > datum_list;

  typedef struct {
    SVG::U x1;
    SVG::U y1;
    SVG::U x2;
    SVG::U y2;
  } MarkerDims;

  SVG::U      marker_size;
  MarkerShape marker_shape;
  bool        marker_show;
  bool        marker_hollow;
  SVG::U      marker_diameter;
  SVG::U      marker_radius;
  MarkerDims  marker_int;       // Interior dimension for hollow marker.
  MarkerDims  marker_out;       // Outer marker dimension.
  MarkerDims  marker_rim;       // Dimension of rim around marker.

  // Compute marker_* variables.
  void ComputeMarker( SVG::U rim = 0 );

  // Build marker based on marker_* variables.
  void BuildMarker( SVG::Group* g, const MarkerDims& m, SVG::Point p );
};

}
