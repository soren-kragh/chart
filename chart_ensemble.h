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
  friend class Main;
  friend class HTML;

public:

  Ensemble( void );
  ~Ensemble( void );

  bool Empty( void ) { return element_list.empty(); }

  Main* LastChart( void ) { return element_list.back().chart; }

  bool NewChart(
    uint32_t grid_row1, uint32_t grid_col1,
    uint32_t grid_row2, uint32_t grid_col2,
    Pos align_hor = Pos::Auto,
    Pos align_ver = Pos::Auto
  );

  void EnableHTML( bool enable = true ) { enable_html = enable; }

  void SetMargin( SVG::U margin ) { this->margin = margin; }
  void SetBorderWidth( SVG::U width ) { border_width = width; }
  SVG::Color* BorderColor( void ) { return &border_color; }
  SVG::Color* ForegroundColor( void ) { return &foreground_color; }
  SVG::Color* BackgroundColor( void ) { return &background_color; }

  // Padding around all elements.
  void SetPadding( SVG::U padding ) { this->padding = padding; }

  // Padding around elements in the grid,
  void SetGridPadding( SVG::U padding ) { grid_padding = padding; }

  void AddFootnote( std::string& txt );

  // Applies to the most recently added footnote.
  void SetFootnotePos( Pos pos );

  // A line above the footnotes.
  void SetFootnoteLine( bool footnote_line = true );

  // Footnote size scaling factor.
  void SetFootnoteSize( float size ) { footnote_size = size; }

  std::string Build( void );

private:

  SVG::Canvas* canvas;
  SVG::Group* top_g;

  bool enable_html = false;
  HTML* html_db = nullptr;

  SVG::Color foreground_color;
  SVG::Color background_color;
  SVG::Color border_color;
  SVG::U border_width = 0;
  SVG::U margin       = 0;
  SVG::U padding      = 8;
  SVG::U grid_padding = 4;

  SVG::U max_area_pad = 0;

  uint32_t grid_max_x = 0;
  uint32_t grid_max_y = 0;

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

  void InitGrid( void );

  void SolveGridSpace( std::vector< space_t >& space_list );

  void ComputeGrid( void );

  struct footnote_t {
    std::string txt;
    Pos pos;
  };
  std::vector< footnote_t > footnotes;
  bool footnote_line = false;
  float footnote_size = 1.0;

  void BuildFootnotes( void );

  void BuildBackground( void );

public:
  void DisplayGridSpace( std::vector< space_t >& space_list );
  void RenumberGridSpace( std::vector< space_t >& space_list );
  void Test( void );

};

}
