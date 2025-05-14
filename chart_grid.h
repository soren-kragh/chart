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

#include <chart_common.h>

namespace Chart {

class Main;

class Grid
{
public:

  Grid( void );
  ~Grid( void );

  struct element_t {
    Main* chart = nullptr;
    SVG::BoundaryBox full_bb;
    SVG::BoundaryBox area_bb;
    uint32_t grid_x1 = 0;
    uint32_t grid_y1 = 0;
    uint32_t grid_x2 = 0;
    uint32_t grid_y2 = 0;
    bool anchor_x_defined = false;
    bool anchor_y_defined = false;
    SVG::AnchorX anchor_x = SVG::AnchorX::Mid;
    SVG::AnchorY anchor_y = SVG::AnchorY::Mid;
  };

  struct edge_t {
    SVG::U coor;        // Coordinate of this core chart area edge
    SVG::U adj;         // Iterative adjustment to coor
    SVG::U pad;         // Padding caused by decorations outside core chart area
    bool pad_use;
  };

  struct space_t {
    edge_t e1;
    edge_t e2;
  };

  std::vector< element_t > element_list;

  std::vector< space_t > space_list_x;
  std::vector< space_t > space_list_y;

  void SolveSpace( std::vector< space_t >& space_list );

  void DisplayGridSpace( std::vector< space_t >& space_list );
  void RenumberGridSpace( std::vector< space_t >& space_list );
  void Test( void );

};

}
