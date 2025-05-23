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

#include <map>
#include <chart_common.h>

namespace Chart {

class Ensemble;
class Main;
class Axis;
class Series;

class HTML
{
public:

  HTML( Ensemble* ensemble ) : ensemble( ensemble ) {}

  void NewChart( Main* main );

  void DefAxisX(
    Main* main, int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );
  void DefAxisY(
    Main* main, int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );

  void LegendPos( Series* series, const SVG::BoundaryBox& bb );
  void MoveLegend( Series* series, SVG::U dx, SVG::U dy );
  void MoveLegends( Main* main, SVG::U dx, SVG::U dy );

  void AddSnapPoint(
    Series* series,
    SVG::Point p, std::string_view tag_x, std::string_view tag_y
  );
  void AddSnapPoint(
    Series* series,
    SVG::Point p, uint32_t cat_idx, std::string_view tag_y
  );

  std::string GenHTML( SVG::Canvas* canvas );

  Ensemble* ensemble = nullptr;
  std::vector< Main* > main_list;

  void GenChartData( Main* main, std::ostringstream& oss );

  std::map< Series*, SVG::BoundaryBox > series_legend_map;
};

}
