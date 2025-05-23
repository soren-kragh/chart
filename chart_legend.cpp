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
#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Legend::Legend( Ensemble* ensemble )
{
  this->ensemble = ensemble;
}

Legend::~Legend( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Legend::Add( Series* series )
{
  for ( Series* s : series_list ) {
    if ( Series::SameLegend( s, series ) ) {
      series->same_legend_series = s->same_legend_series;
      s->same_legend_series = series;
      return;
    }
  }
  series_list.push_back( series );
  return;
}

uint32_t Legend::Cnt( void )
{
  return series_list.size();
}

////////////////////////////////////////////////////////////////////////////////

void Legend::CalcLegendDims(
  bool framed,
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

  if ( !heading.empty() ) {
    Label::CreateLabel( g, heading, char_h * 1.2 );
    BoundaryBox bb = g->Last()->GetBB();
    g->DeleteFront();
    legend_dims.hx = bb.max.x - bb.min.x;
    legend_dims.hy = bb.max.y - bb.min.y + char_h / 2;
  }

  U ox = char_h / 3;    // Text to outline X spacing.
  U oy = char_h / 5;    // Text to outline Y spacing.

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if ( series->line_width > char_h * 0.8 ) {
      // No outline if it is too fat.
      series->legend_outline = false;
    }
    bool has_outline =
      series->legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;
    if ( has_outline ) {
      legend_dims.ow = std::max( legend_dims.ow, series->line_width );
    }
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

  U line_symbol_width = -1;
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
      series->has_line && !series->legend_outline &&
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
    ) {
      if ( line_symbol_width < 0 ) {
        line_symbol_width = 2.8 * char_w;
      }
      legend_dims.ss = std::max( +legend_dims.ss, series->line_width / 2 );
      line_symbol_width =
        std::max(
          +line_symbol_width, 3 * series->line_dash + 2 * series->line_hole
        );
      line_symbol_width = std::max( +line_symbol_width, 3 * series->line_width );
      line_symbol_width = std::max( +line_symbol_width, 3 * legend_dims.mw );
    }
  }
  if ( line_symbol_width < 0 ) {
    line_symbol_width = 0;
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
      series->legend_outline &&
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

void Legend::BuildLegends(
  bool framed,
  SVG::Color* frame_line_color,
  SVG::Color* frame_fill_color,
  Group* g, int nx
)
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
  Legend::LegendDims legend_dims;
  CalcLegendDims( framed, g, legend_dims );
  int ny = (Cnt() + nx - 1) / nx;

  {
    U mx = framed ? legend_dims.mx : U( 0 );
    U my = framed ? legend_dims.my : U( 0 );
    U w = nx * legend_dims.sx + (nx - 1) * legend_dims.dx;
    U h = ny * legend_dims.sy + (ny - 1) * legend_dims.dy;
    w += legend_dims.lx + legend_dims.rx;
    U ey = legend_dims.hy;
    U ex = std::max( 0.0, legend_dims.hx - w );
    Point r1{
      -mx / 2 - legend_dims.lx - ex / 2,
      +my / 2 + ey
    };
    Point r2{
      r1.x + w + ex + mx,
      r1.y - h - ey - my
    };
    g->Add( new Rect( r1, r2, framed ? box_spacing : U( 0 ) ) );
    if ( framed ) {
      g->Last()->Attr()->LineColor()->Set( frame_line_color );
      g->Last()->Attr()->SetLineWidth( 1 );
      if ( frame_fill_color->IsDefined() ) {
        g->Last()->Attr()->FillColor()->Set( frame_fill_color );
      }
    } else {
      g->Last()->Attr()->FillColor()->Clear();
      g->Last()->Attr()->LineColor()->Clear();
      g->Last()->Attr()->SetLineWidth( 0 );
    }
    if ( !heading.empty() ) {
      Object* obj = Label::CreateLabel( g, heading, legend_dims.ch * 1.2 );
      obj->MoveTo( AnchorX::Mid, AnchorY::Max, (r1.x + r2.x)/2, r1.y - my/2 );
    }
  }

  int n = 0;
  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    U px = (n % nx) * +(legend_dims.sx + legend_dims.dx);
    U py = (n / nx) * -(legend_dims.sy + legend_dims.dy);
    Point marker_p{ px + legend_dims.ow/2, py - legend_dims.sy/2 };

    if ( ensemble->enable_html ) {
      BoundaryBox bb;
      bb.min.x = px - legend_dims.lx;
      bb.min.y = py - legend_dims.sy;
      bb.max.x = px + legend_dims.rx + legend_dims.sx;
      bb.max.y = py;
      for ( Series* s = series; s != nullptr; s = s->same_legend_series ) {
        ensemble->html_db->LegendPos( s, bb );
      }
    }

    U line_w = series->line_width;
    if ( !series->has_line ) line_w = 0;

    bool has_outline =
      series->legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

    if ( has_outline ) {
      g->Add(
        new Rect(
          px + legend_dims.ow/2,
          py - legend_dims.ow/2,
          px - legend_dims.ow/2 + legend_dims.sx,
          py + legend_dims.ow/2 - legend_dims.sy,
          legend_dims.cr
        )
      );
      series->ApplyLineStyle( g->Last() );
    }

    if (
      series->has_line && !series->legend_outline &&
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
    ) {
      g->Add(
        new Line(
          marker_p.x - legend_dims.ow/2 - legend_dims.lx, marker_p.y,
          marker_p.x + legend_dims.ow/2 + legend_dims.lx, marker_p.y
        )
      );
      series->ApplyLineStyle( g->Last() );
    }

    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea &&
      series->marker_shape != MarkerShape::LineX &&
      series->marker_shape != MarkerShape::LineY
    ) {
      marker_p.y -= (series->marker_out.y1 + series->marker_out.y2) / 2;
      if ( series->marker_show_out ) {
        series->BuildMarker( g, series->marker_out, marker_p );
        series->ApplyMarkStyle( g->Last() );
      }
      if ( series->marker_show_int ) {
        series->BuildMarker( g, series->marker_int, marker_p );
        series->ApplyHoleStyle( g->Last() );
      }
    }

    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      bool has_interior = legend_dims.ss > line_w + 1;
      Point p1{ marker_p.x - legend_dims.ss, marker_p.y - legend_dims.ss };
      Point p2{ marker_p.x + legend_dims.ss, marker_p.y + legend_dims.ss };
      {
        Point c1{ p1 };
        Point c2{ p2 };
        U db = std::min( 1.0, line_w / 2 );
        c1.x += db; c2.x -= db;
        c1.y += db; c2.y -= db;
        g->Add( new Rect( c1, c2 ) );
      }
      if ( has_interior ) {
        series->ApplyFillStyle( g->Last() );
        if ( line_w > 0 ) {
          p1.x += line_w / 2;
          p1.y += line_w / 2;
          p2.x -= line_w / 2;
          p2.y -= line_w / 2;
          g->Add( new Rect( p1, p2 ) );
          series->ApplyLineStyle( g->Last() );
          g->Last()->Attr()->SetLineJoin( LineJoin::Sharp );
        }
      } else {
        series->ApplyMarkStyle( g->Last() );
      }
    }

    int lines = 1;
    for ( char c : series->name ) if ( c == '\n' ) lines++;
    px += legend_dims.ow / 2 + legend_dims.tx;
    py -= (legend_dims.sy - lines * legend_dims.ch) / 2;
    std::string s;
    auto cit = series->name.cbegin();
    while ( cit != series->name.cend() ) {
      auto oit = cit;
      if ( Text::UTF8_CharAdv( series->name, cit ) ) {
        if ( *oit != '\n' ) s.append( oit, cit );
        if ( *oit == '\n' || cit == series->name.cend() ) {
          if ( !s.empty() ) {
            g->Add( new Text( px, py, s ) );
          }
          py -= legend_dims.ch;
          s = "";
        }
      }
    }

    n++;
  }
}

////////////////////////////////////////////////////////////////////////////////
