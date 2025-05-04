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

#include <chart_legend.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Legend::Legend( void )
{
}

Legend::~Legend( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Legend::CalcLegendDims(
  std::vector< Series* >& series_list,
  const std::string& legend_heading,
  bool framed, bool legend_outline,
  Group* g, Legend::LegendDims& legend_dims
)
{
  legend_dims.ch = 0;
  legend_dims.ow = 0;
  legend_dims.cr = 0;
  legend_dims.mw = 0;
  legend_dims.mh = 0;
  legend_dims.ss = 0;
  legend_dims.lx = 0;
  legend_dims.rx = 0;
  legend_dims.tx = 0;
  legend_dims.dx = 8;
  legend_dims.dy = 4;
  legend_dims.sx = 0;
  legend_dims.sy = 0;
  legend_dims.mx = framed ? (2 * box_spacing) : (1 * box_spacing);
  legend_dims.my = framed ? (2 * box_spacing) : (1 * box_spacing);
  legend_dims.hx = 0;
  legend_dims.hy = 0;

  g->Add( new Text( "X" ) );
  BoundaryBox bb = g->Last()->GetBB();
  g->DeleteFront();
  U char_w = bb.max.x - bb.min.x;
  U char_h = bb.max.y - bb.min.y;

  if ( !legend_heading.empty() ) {
    Label::CreateLabel( g, legend_heading, char_h * 1.2 );
    BoundaryBox bb = g->Last()->GetBB();
    g->DeleteFront();
    legend_dims.hx = bb.max.x - bb.min.x;
    legend_dims.hy = bb.max.y - bb.min.y + char_h / 2;
  }

  U ox = char_h / 3;    // Text to outline X spacing.
  U oy = char_h / 5;    // Text to outline Y spacing.

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    bool has_outline =
      legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;
    if ( has_outline ) {
      legend_dims.ow = std::max( legend_dims.ow, series->line_width );
    }
  }

  // No outline if it is too fat.
  if ( legend_dims.ow > char_h * 0.8 ) {
    legend_outline = false;
    legend_dims.ow = 0;
  }
  U how = legend_dims.ow / 2;

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea &&
      series->marker_shape != MarkerShape::LineX &&
      series->marker_shape != MarkerShape::LineY
    ) {
      Series::MarkerDims md = series->marker_out;
      legend_dims.mw = std::max( +legend_dims.mw, md.x2 - md.x1 );
      legend_dims.mh = std::max( +legend_dims.mh, md.y2 - md.y1 );
    }
  }

  legend_dims.ss = std::max( legend_dims.mw, legend_dims.mh ) / 2;

  U line_symbol_width = legend_outline ? 0 : (2.8 * char_w);

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      if ( series->has_fill || series->has_line ) {
        legend_dims.ss = std::max( +legend_dims.ss, (char_h + 8) / 2 );
      }
      if ( series->has_line ) {
        legend_dims.ss = std::max( +legend_dims.ss, 2 * series->line_width );
        legend_dims.ss =
          std::max(
            +legend_dims.ss, (series->line_dash + series->line_hole) * 0.75
          );
      }
    }
    if (
      series->has_line && !legend_outline &&
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
    ) {
      legend_dims.ss = std::max( +legend_dims.ss, series->line_width / 2 );
      line_symbol_width =
        std::max(
          +line_symbol_width, 3 * series->line_dash + 2 * series->line_hole
        );
      line_symbol_width = std::max( +line_symbol_width, 3 * series->line_width );
      line_symbol_width = std::max( +line_symbol_width, 3 * legend_dims.mw );
    }
  }

  legend_dims.ch = char_h;
  legend_dims.lx = std::max( +legend_dims.lx, legend_dims.ss - how );
  legend_dims.lx = std::max( +legend_dims.lx, line_symbol_width / 2 - how );
  legend_dims.dx += legend_dims.lx;
  legend_dims.tx = how + legend_dims.lx + ox;

  if ( how > 0 ) {
    legend_dims.cr = how + char_h / 4;
  }

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;

    uint32_t max_lines = 1;
    uint32_t max_chars = 1;
    {
      uint32_t cur_chars = 0;
      auto it = series->name.cbegin();
      while ( it != series->name.cend() ) {
        auto c = *it;
        if ( Text::UTF8_CharAdv( series->name, it ) ) {
          if ( c == '\n' ) {
            max_lines++;
            max_chars = std::max( max_chars, cur_chars );
            cur_chars = 0;
          } else {
            cur_chars++;
          }
        }
      }
      max_chars = std::max( max_chars, cur_chars );
    }
    U text_w = char_w * max_chars;
    U text_h = char_h * max_lines;

    bool has_outline =
      legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

    if ( has_outline ) legend_dims.rx = legend_dims.lx;

    legend_dims.sx =
      std::max(
        +legend_dims.sx,
        2 * how + legend_dims.lx + ox + text_w + ox +
        (has_outline ? (2 * how) : 0)
      );

    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        text_h +
        (has_outline ? (2 * (oy + series->line_width / 2 + how)) : 0)
      );
    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        has_outline
        ? (legend_dims.mh + 2*(legend_dims.cr + how))
        : (2 * legend_dims.ss)
      );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
