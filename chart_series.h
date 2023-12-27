//
//  Copyright (C) 2023, S. Kragh
//
//  This file is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License for more details.
//  <http://www.gnu.org/licenses/>.
//

#pragma once

#include <svg_canvas.h>
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

  void SetStyle( int style );
  SVG::Color* Color( void ) { return &color; }
  void SetWidth( SVG::U width );
  void SetDash( SVG::U dash );
  void SetDash( SVG::U dash, SVG::U hole );
  void SetPointSize( SVG::U point_size );

  void Add( double x, double y );

  uint32_t Size( void ) { return datum_list.size(); }

private:

  void ApplyStyle( SVG::Object* obj );

  void UpdateLegendBoxes(
    std::vector< LegendBox >& lb_list,
    SVG::U x1, SVG::U y1,
    SVG::U x2, SVG::U y2
  );

  void Build(
    SVG::Group* g,
    Axis& x_axis,
    Axis& y_axis,
    std::vector< LegendBox >& lb_list
  );

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2
  );

  std::string name;

  std::vector< Datum > datum_list;

  std::vector< SVG::Color > color_list;
  SVG::Color color;
  SVG::U width;
  SVG::U dash;
  SVG::U hole;
  SVG::U point_size;

  // Box within which to display graph.
  SVG::BoundaryBox box;

  // Correction for rounding errors.
  double cre = 1e-6;
  double e1 = 0;
  double e2 = 0;

};

}
