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
public:

  Ensemble( void );
  ~Ensemble( void );

  Main* last_chart = nullptr;

  bool Empty( void ) { return last_chart == nullptr; }

  Main* LastChart( void ) { return last_chart; }

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

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetTitlePos( Pos pos ) { title_pos = pos; }
  void SetTitleSize( float size ) { title_size = size; }

  // A line below the title.
  void SetTitleLine( bool line = true ) { title_line = line; }

  void SetMargin( SVG::U margin ) { this->margin = margin; }
  void SetBorderWidth( SVG::U width ) { border_width = width; }
  SVG::Color* BorderColor( void ) { return &border_color; }
  SVG::Color* ForegroundColor( void ) { return &foreground_color; }
  SVG::Color* BackgroundColor( void ) { return &background_color; }

  // Padding around all elements.
  void SetPadding( SVG::U padding ) { this->padding = padding; }

  // Padding around elements in the grid; a negative value for the grid_padding
  // means that only the core chart areas are considered when laying out the
  // grid.
  void SetGridPadding( SVG::U grid_padding, SVG::U area_padding )
  {
    this->grid_padding = grid_padding;
    this->area_padding = area_padding;
  }

  void SetLegendHeading( const std::string& txt );
  void SetLegendFrame( bool enable = true );
  void SetLegendPos( Pos pos );
  bool SetLegendPos(
    uint32_t grid_row1, uint32_t grid_col1,
    uint32_t grid_row2, uint32_t grid_col2,
    Chart::Pos align_hor, Chart::Pos align_ver
  );
  void SetLegendSize( float size );
  SVG::Color* LegendColor( void ) { return &legend_color; }

  void AddFootnote( std::string& txt );

  // Applies to the most recently added footnote.
  void SetFootnotePos( Pos pos );

  // A line above the footnotes.
  void SetFootnoteLine( bool footnote_line = true );

  // Footnote size scaling factor.
  void SetFootnoteSize( float size ) { footnote_size = size; }

  void MoveCharts( void );
  std::string Build( void );

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
  SVG::U area_padding = 0;

  SVG::U max_area_pad = 0;

  Grid grid;

  void InitGrid( void );
  void SolveGrid( void );

  std::string title;
  std::string sub_title;
  std::string sub_sub_title;
  Pos         title_pos  = Pos::Center;
  bool        title_line = false;
  float       title_size = 1.0;

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
  void BuildTitle( void );
  void BuildFootnotes( void );
  void BuildBackground( void );

};

}
