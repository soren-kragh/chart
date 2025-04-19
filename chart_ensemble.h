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
#include <chart_main.h>

namespace Chart {

class Ensemble
{

public:

  Ensemble( void );
  ~Ensemble( void );

private:

  struct chart_t {
    Main* chart = nullptr;
    SVG::BoundaryBox full_bb;
    SVG::BoundaryBox area_bb;
    SVG::AnchorX anchor_x = SVG::AnchorX::Mid;
    SVG::AnchorY anchor_y = SVG::AnchorY::Mid;
    uint32_t x1, y1;
    uint32_t x2, y2;
  };

  struct edge_t {
    SVG::U coor = 0;    // Coordinate of this core chart area edge
    SVG::U adj;         // Iterative adjustment to coor
    SVG::U pad;         // Padding caused by decorations outside core chart area
    bool pad_use = false;
  };

  struct space_t {
    edge_t e1;
    edge_t e2;
  };

  std::vector< chart_t > chart_list;
  std::vector< space_t > space_list_x;
  std::vector< space_t > space_list_y;

  void InitGrid( void );

  uint32_t SolveGridSpace( std::vector< space_t >& space_list );

public:
  void DisplayGridSpace( std::vector< space_t >& space_list );
  void RenumberGridSpace( std::vector< space_t >& space_list );
  void Test( void );

};

}
