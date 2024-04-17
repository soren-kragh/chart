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

  // The given size is the diameter of the marker in addition to the line width.
  void SetMarkerSize( SVG::U marker_size );

  void SetMarkerShape( MarkerShape marker_shape );

  void Add( double x, double y );

  uint32_t Size( void ) { return datum_list.size(); }

private:

  void ApplyStyle( SVG::Object* obj );

  void UpdateLegendBoxes(
    std::vector< LegendBox >& lb_list, SVG::Point p1, SVG::Point p2
  );

  void Build(
    SVG::Group* g,
    Axis* x_axis,
    Axis* y_axis,
    std::vector< LegendBox >& lb_list
  );

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
    SVG::BoundaryBox& box
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
