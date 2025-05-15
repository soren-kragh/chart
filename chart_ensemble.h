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
#include <chart_grid.h>

namespace Chart {

class Ensemble
{
  friend class Main;
  friend class Legend;
  friend class HTML;

public:

  Ensemble( void );
  ~Ensemble( void );

  bool Empty( void ) { return grid.element_list.empty(); }

  Main* LastChart( void ) { return grid.element_list.back().chart; }

  bool NewChart(
    uint32_t grid_row1, uint32_t grid_col1,
    uint32_t grid_row2, uint32_t grid_col2,
    Pos align_hor = Pos::Auto,
    Pos align_ver = Pos::Auto
  );

  void SetLetterSpacing(
    float width_adj, float height_adj = 1.0, float baseline_adj = 1.0
  );

  void EnableHTML( bool enable = true ) { enable_html = enable; }

  void SetHeading( const std::string& txt );
  void SetSubHeading( const std::string& txt );
  void SetSubSubHeading( const std::string& txt );
  void SetHeadingPos( Pos pos ) { heading_pos = pos; }
  void SetHeadingSize( float size ) { heading_size = size; }

  // A line below the heading.
  void SetHeadingLine( bool line = true ) { heading_line = line; }

  void SetMargin( SVG::U margin ) { this->margin = margin; }
  void SetBorderWidth( SVG::U width ) { border_width = width; }
  SVG::Color* BorderColor( void ) { return &border_color; }
  SVG::Color* ForegroundColor( void ) { return &foreground_color; }
  SVG::Color* BackgroundColor( void ) { return &background_color; }

  // Padding around all elements.
  void SetPadding( SVG::U padding ) { this->padding = padding; }

  // Padding around elements in the grid; a negative value means that
  // only the core chart areas are considered when laying out the grid.
  void SetGridPadding( SVG::U padding ) { grid_padding = padding; }

  void SetLegendHeading( const std::string& txt );
  void SetLegendFrame( bool enable = true );
  void SetLegendPos( Pos pos );
  void SetLegendSize( float size );
  SVG::Color* LegendColor( void ) { return &legend_color; }

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

  float width_adj    = 1.0;
  float height_adj   = 1.0;
  float baseline_adj = 1.0;

  SVG::Color foreground_color;
  SVG::Color background_color;
  SVG::Color border_color;
  SVG::U border_width = 0;
  SVG::U margin       = 0;
  SVG::U padding      = 8;
  SVG::U grid_padding = 4;

  SVG::U max_area_pad = 0;

  Grid grid;

  void InitGrid( void );
  void ComputeGrid( void );

  std::string heading;
  std::string sub_heading;
  std::string sub_sub_heading;
  Pos         heading_pos  = Pos::Center;
  bool        heading_line = false;
  float       heading_size = 1.0;

  Legend*    legend_obj;
  bool       legend_frame;
  bool       legend_frame_specified;
  SVG::Color legend_color;

  struct footnote_t {
    std::string txt;
    Pos pos;
  };
  std::vector< footnote_t > footnotes;
  bool footnote_line = false;
  float footnote_size = 1.0;

  SVG::BoundaryBox TopBB( void );

  void BuildLegends( void );
  void BuildHeading( void );
  void BuildFootnotes( void );
  void BuildBackground( void );

};

}
