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

#include <chart_main.h>

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Main::Main( void )
{
  SetLegendPos( Pos::Auto );
  SetFootnotePos( Pos::Auto );
  axis_x = new Axis( true );
  axis_y[ 0 ] = new Axis( false );
  axis_y[ 1 ] = new Axis( false );
}

Main::~Main( void )
{
  for ( auto series : series_list ) {
    delete series;
  }
  delete axis_x;
  delete axis_y[ 0 ];
  delete axis_y[ 1 ];
}

///////////////////////////////////////////////////////////////////////////////

void Main::SetChartArea( SVG::U width, SVG::U height )
{
  chart_w = std::max( U( 100 ), width );
  chart_h = std::max( U( 100 ), height );
}

void Main::SetMargin( SVG::U margin )
{
  this->margin = margin;
}

void Main::SetTitle( const std::string& txt )
{
  title = txt;
}

void Main::SetSubTitle( const std::string& txt )
{
  sub_title = txt;
}

void Main::SetSubSubTitle( const std::string& txt )
{
  sub_sub_title = txt;
}

void Main::SetFootnote( std::string txt )
{
  footnote = txt;
}

void Main::SetFootnotePos( Pos pos )
{
  footnote_pos = pos;
}

void Main::SetLegendPos( Pos pos )
{
  legend_pos = pos;
}

Series* Main::AddSeries( SeriesType type )
{
  Series* series = new Series( type );
  int style = series_list.size() % 80;
  series->SetStyle( style );
  series_list.push_back( series );
  return series;
}

void Main::AddCategory( const std::string category )
{
  category_list.push_back( category );
}

///////////////////////////////////////////////////////////////////////////////

uint32_t Main::LegendCnt( void )
{
  uint32_t n = 0;
  for ( Series* series : series_list ) {
    if ( series->name.length() > 0 ) n++;
  }
  return n;
}

//-----------------------------------------------------------------------------

void Main::CalcLegendDims( Group* g, LegendDims& legend_dims )
{
  U gx = 8;     // X-gap between series legends.
  U gy = 8;     // Y-gap between series legends.

  legend_dims.ch = 0;
  legend_dims.mw = 0;
  legend_dims.cr = 0;
  legend_dims.ss = 0;
  legend_dims.ex = 0;
  legend_dims.tx = 0;
  legend_dims.dx = gx;
  legend_dims.dy = gy;
  legend_dims.sx = 0;
  legend_dims.sy = 0;
  legend_dims.mx = 16;
  legend_dims.my = 16;

  g->Add( new Text( "X" ) );
  BoundaryBox bb = g->Last()->GetBB();
  g->DeleteFront();
  U char_w = bb.max.x - bb.min.x;
  U char_h = bb.max.y - bb.min.y;

  legend_dims.ch = char_h;

  for ( Series* series : series_list ) {
    if ( series->name.length() == 0 ) continue;

    bool has_outline =
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
      && series->line_width > 0
      && !series->line_color.IsClear();

    if ( has_outline ) {
      legend_dims.mw = std::max( legend_dims.mw, series->line_width );
    }
  }

  U hw = legend_dims.mw / 2;
  U ox = char_h / 3;    // Text to outline X spacing.
  U oy = char_h / 5;    // Text to outline Y spacing.

  if ( legend_dims.mw > 0 ) {
    legend_dims.cr = hw + char_h / 4;
  }

  for ( int pass : { 1, 2, 3 } ) {
    pass = pass;        // To avoid warning.
    for ( Series* series : series_list ) {
      if ( series->name.length() == 0 ) continue;

      uint32_t max_lines = 1;
      uint32_t max_chars = 1;
      uint32_t cur_chars = 0;
      for ( char c : series->name ) {
        if ( c == '\n' ) {
          max_lines++;
          max_chars = std::max( max_chars, cur_chars );
          cur_chars = 0;
        } else {
          cur_chars++;
        }
      }
      max_chars = std::max( max_chars, cur_chars );
      U text_w = char_w * max_chars;
      U text_h = char_h * max_lines;

      Series::MarkerDims md;
      md.x1 = md.y1 = md.x2 = md.y2 = 0;
      series->ComputeMarker();
      if (
        series->marker_show &&
        series->type != SeriesType::Area &&
        series->type != SeriesType::StackedArea
      ) {
        md = series->marker_out;
        legend_dims.ss =
          std::max(
            +legend_dims.ss,
            std::min( md.x2 - md.x1, md.y2 - md.y1 ) / 2
          );
      }
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar ||
        series->type == SeriesType::Area ||
        series->type == SeriesType::StackedArea
      ) {
        legend_dims.ss =
          std::max( +legend_dims.ss, char_h * legend_area_id_fact / 2 );
        if ( !series->line_color.IsClear() ) {
          legend_dims.ss = std::max( +legend_dims.ss, 2 * series->line_width );
        }
        md.x1 = md.y1 = -legend_dims.ss;
        md.x2 = md.y2 = +legend_dims.ss;
      }

      bool has_outline =
        ( series->type == SeriesType::XY ||
          series->type == SeriesType::Line ||
          series->type == SeriesType::Lollipop
        )
        && series->line_width > 0
        && !series->line_color.IsClear();

      legend_dims.tx = std::max( +legend_dims.tx, md.x2 + ox );
      if ( has_outline ) {
        legend_dims.tx =
          std::max( +legend_dims.tx, series->line_width / 2 + ox );
      }

      legend_dims.ex = std::max( +legend_dims.ex, -md.x1 - hw );

      legend_dims.dx = std::max( +legend_dims.dx, gx - md.x1 - hw );

      legend_dims.sx =
        std::max(
          +legend_dims.sx,
          hw + legend_dims.tx + text_w + ox +
          (has_outline ? (series->line_width / 2 + hw) : 0)
        );
      legend_dims.sy =
        std::max(
          +legend_dims.sy,
          text_h +
          (has_outline ? (2 * (oy + series->line_width / 2 + hw)) : 0)
        );
      legend_dims.sy =
        std::max(
          +legend_dims.sy,
          (md.y2 - md.y1) +
          (has_outline ? (2 * (legend_dims.cr + hw)) : 0)
        );
    }
  }

  return;
}

//-----------------------------------------------------------------------------

// Determine potential placement of series legends in chart interior.
void Main::CalcLegendBoxes(
  Group* g, std::vector< LegendBox >& lb_list,
  const std::vector< SVG::Object* >& axis_objects
)
{
  LegendDims legend_dims;
  CalcLegendDims( g, legend_dims );
  uint32_t lc = LegendCnt();

  auto add_lbs = [&](
    AnchorX anchor_x, AnchorY anchor_y, bool can_move_x, bool can_move_y
  )
  {
    uint32_t nx = (anchor_x == AnchorX::Mid) ? lc :  1;
    uint32_t ny = (anchor_x == AnchorX::Mid) ?  1 : lc;
    while ( nx > 0 && ny > 0 ) {
      g->Add(
        new Rect(
          0, 0,
          nx * legend_dims.sx + (nx - 1) * legend_dims.dx + 2 * legend_dims.mx +
          2 * legend_dims.ex,
          ny * legend_dims.sy + (ny - 1) * legend_dims.dy + 2 * legend_dims.my
        )
      );
      Object* obj = g->Last();
      U x = 0;
      U y = 0;
      if ( anchor_x == AnchorX::Mid ) x = chart_w / 2;
      if ( anchor_x == AnchorX::Max ) x = chart_w;
      if ( anchor_y == AnchorY::Mid ) y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h;
      obj->MoveTo( anchor_x, anchor_y, x, y );

      if ( can_move_y && anchor_y != AnchorY::Mid ) {
        while ( true ) {
          BoundaryBox obj_bb = obj->GetBB();
          U dy = 0;
          for ( auto ao : axis_objects ) {
            if ( !SVG::Collides( obj, ao ) ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            if ( anchor_y == AnchorY::Min ) {
              if ( ao_bb.max.y < (chart_h * 1 / 4) ) {
                dy = ao_bb.max.y - obj_bb.min.y;
                break;
              }
            } else {
              if ( ao_bb.min.y > (chart_h * 3 / 4) ) {
                dy = ao_bb.min.y - obj_bb.max.y;
                break;
              }
            }
          }
          if ( dy == 0 ) break;
          obj->Move( 0, dy );
        }
      }

      if ( can_move_x && anchor_x != AnchorX::Mid ) {
        while ( true ) {
          BoundaryBox obj_bb = obj->GetBB();
          U dx = 0;
          for ( auto ao : axis_objects ) {
            if ( !SVG::Collides( obj, ao ) ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            if ( anchor_x == AnchorX::Min ) {
              if ( ao_bb.max.x < (chart_w * 1 / 4) ) {
                dx = ao_bb.max.x - obj_bb.min.x;
                break;
              }
            } else {
              if ( ao_bb.min.x > (chart_w * 3 / 4) ) {
                dx = ao_bb.min.x - obj_bb.max.x;
                break;
              }
            }
          }
          if ( dx == 0 ) break;
          obj->Move( dx, 0 );
        }
      }

      if ( !Collides( obj, axis_objects ) ) {
        LegendBox lb;
        lb.bb = obj->GetBB();
        if (
          ( anchor_x != AnchorX::Mid ||
            (lb.bb.max.x - lb.bb.min.x) > (lb.bb.max.y - lb.bb.min.y)
          ) &&
          lb.bb.min.x > -epsilon && lb.bb.max.x < chart_w + epsilon &&
          lb.bb.min.y > -epsilon && lb.bb.max.y < chart_h + epsilon
        ) {
          lb.nx = nx;
          lb.sp = nx * ny - lc;
          lb.bb.min.x += 1; lb.bb.min.y += 1;
          lb.bb.max.x -= 1; lb.bb.max.y -= 1;
          lb_list.push_back( lb );
        }
      }
      g->DeleteFront();
      if ( anchor_x == AnchorX::Mid ) {
        if ( ny == lc ) break;
        uint32_t onx = nx;
        while ( ny < lc && onx == nx ) {
          ny++;
          nx = (lc + ny - 1) / ny;
        }
      } else {
        if ( nx == lc ) break;
        uint32_t ony = ny;
        while ( nx < lc && ony == ny ) {
          nx++;
          ny = (lc + nx - 1) / nx;
        }
      }
    }
  };

  bool dual_y = axis_y[ 0 ]->show && axis_y[ 1 ]->show;

  AnchorX ax1 = AnchorX::Max;
  AnchorX ax2 = AnchorX::Min;
  AnchorY ay1 = AnchorY::Max;
  AnchorY ay2 = AnchorY::Min;
  if ( axis_x->angle == 0 ) {
    if ( axis_y[ 0 ]->orth_coor_is_max ) std::swap( ax1, ax2 );
    if ( axis_x->orth_coor_is_max ) std::swap( ay1, ay2 );
  } else {
    if ( axis_x->orth_coor_is_max ) std::swap( ax1, ax2 );
    if ( axis_y[ 0 ]->orth_coor_is_max ) std::swap( ay1, ay2 );
  }

  for ( bool can_move : { false, true } ) {
    if ( !dual_y ) add_lbs( ax1, ay1, can_move, can_move );
    if ( dual_y && axis_x->angle != 0 ) {
      add_lbs( ax1, AnchorY::Mid, can_move, can_move );
      add_lbs( ax2, AnchorY::Mid, can_move, can_move );
    }
    add_lbs( AnchorX::Mid, ay1, can_move, can_move );
    add_lbs( AnchorX::Mid, ay2, can_move, can_move );
    if ( dual_y ) add_lbs( ax1, ay1, can_move, can_move );
    add_lbs( ax1, ay2, can_move, can_move );
    add_lbs( ax2, ay1, can_move, can_move );
    add_lbs( ax2, ay2, can_move, can_move );
    if ( can_move ) {
      add_lbs( AnchorX::Mid, AnchorY::Mid, false, false );
      if ( !dual_y ) {
        add_lbs( ax1, AnchorY::Mid, true, false );
        add_lbs( ax2, AnchorY::Mid, true, false );
      }
    }
  }

/*
  for ( auto lb : lb_list ) {
    g->Add( new Rect( lb.bb.min, lb.bb.max ) );
    g->Last()->Attr()->SetLineWidth( 0.5 );
    g->Last()->Attr()->FillColor()->Clear();
    g->Last()->Attr()->LineColor()->Set( ColorName::black );
  }
*/
}

//-----------------------------------------------------------------------------

void Main::BuildLegend( Group* g, int nx )
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
  LegendDims legend_dims;
  CalcLegendDims( g, legend_dims );
  int ny = (LegendCnt() + nx - 1) / nx;

  Point r1{
    -legend_dims.mx / 2 - legend_dims.ex, +legend_dims.my / 2
  };
  Point r2{
    legend_dims.ex +
    +(nx * legend_dims.sx + (nx - 1) * legend_dims.dx + legend_dims.mx / 2),
    -(ny * legend_dims.sy + (ny - 1) * legend_dims.dy + legend_dims.my / 2)
  };
  g->Add( new Rect( r1, r2, 4 ) );
  g->Last()->Attr()->LineColor()->Set( ColorName::black );
  g->Last()->Attr()->SetLineWidth( 1 );

  int n = 0;
  for ( Series* series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    U px = (n % nx) * +(legend_dims.sx + legend_dims.dx);
    U py = (n / nx) * -(legend_dims.sy + legend_dims.dy);
    Point marker_p{ px + legend_dims.mw/2, py - legend_dims.sy/2 };

    U line_w = 0;
    if ( !series->line_color.IsClear() ) {
      line_w = std::max( line_w, series->line_width );
    }

    bool has_outline =
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
      && line_w > 0;

    if ( has_outline ) {
      g->Add(
        new Rect(
          px + legend_dims.mw/2,
          py - legend_dims.mw/2,
          px - legend_dims.mw/2 + legend_dims.sx,
          py + legend_dims.mw/2 - legend_dims.sy,
          legend_dims.cr
        )
      );
      series->ApplyLineStyle( g->Last() );
    }

    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea
    ) {
      marker_p.y -= (series->marker_out.y1 + series->marker_out.y2) / 2;
      series->BuildMarker( g, series->marker_out, marker_p );
      series->ApplyMarkStyle( g->Last() );
      if ( series->marker_hollow ) {
        series->BuildMarker( g, series->marker_int, marker_p );
        series->ApplyFillStyle( g->Last() );
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
        }
      } else {
        series->ApplyMarkStyle( g->Last() );
      }
    }

    int lines = 1;
    for ( char c : series->name ) if ( c == '\n' ) lines++;
    px += legend_dims.mw / 2 + legend_dims.tx;
    py -= (legend_dims.sy - lines * legend_dims.ch) / 2;
    std::string s;
    for ( char c : series->name ) {
      if ( c == '\n' ) {
        g->Add( new Text( px, py, s ) );
        py -= legend_dims.ch;
        s = "";
      } else {
        s += c;
      }
    }
    if ( s.length() > 0 ) {
      g->Add( new Text( px, py, s ) );
    }
    n++;
  }
}

//-----------------------------------------------------------------------------

void Main::PlaceLegend(
  const std::vector< SVG::Object* >& axis_objects,
  const std::vector< LegendBox >& lb_list,
  Group* legend_g
)
{
  if ( LegendCnt() == 0 ) return;

  if ( legend_pos == Pos::Auto ) {
    LegendBox best_lb;
    bool best_lb_defined = false;
    for ( const LegendBox& lb : lb_list ) {
      if ( !best_lb_defined ) {
        best_lb = lb;
        best_lb_defined = true;
      }
      if (
        lb.weight1 < best_lb.weight1 ||
        ( lb.weight1 == best_lb.weight1 &&
          ( lb.weight2 < best_lb.weight2 ||
            (lb.weight2 == best_lb.weight2 && lb.sp < best_lb.sp)
          )
        )
      ) {
        best_lb = lb;
      }
    }
    if ( best_lb_defined ) {
      BuildLegend( legend_g->AddNewGroup(), best_lb.nx );
      legend_g->Last()->MoveTo(
        AnchorX::Mid, AnchorY::Mid,
        (best_lb.bb.min.x + best_lb.bb.max.x) / 2,
        (best_lb.bb.min.y + best_lb.bb.max.y) / 2
      );
    } else {
      legend_pos = Pos::Bottom;
    }
  }

  if ( legend_pos == Pos::Auto ) return;

  LegendDims legend_dims;
  CalcLegendDims( legend_g, legend_dims );

  if ( legend_pos == Pos::Left || legend_pos == Pos::Right ) {

    U avail_h = chart_h;
    uint32_t nx = 1;
    while ( 1 ) {
      uint32_t ny = (LegendCnt() + nx - 1) / nx;
      U need_h =
        ny * legend_dims.sy + (ny - 1) * legend_dims.dy + 2 * legend_dims.my;
      if ( need_h > avail_h && ny > 1 ) {
        nx++;
        continue;
      }
      break;
    }
    BuildLegend( legend_g->AddNewGroup(), nx );
    Object* legend = legend_g->Last();

    U x = 0 - legend_dims.mx;
    Dir dir = Dir::Left;
    AnchorX anchor_x = AnchorX::Max;
    if ( legend_pos == Pos::Right ) {
      x = chart_w + legend_dims.mx;
      dir = Dir::Right;
      anchor_x = AnchorX::Min;
    }
    AnchorY best_anchor_y{ AnchorY::Mid };
    U best_x{ 0 };
    U best_y{ 0 };
    for ( auto anchor_y : { AnchorY::Max, AnchorY::Mid, AnchorY::Min } ) {
      U y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h - legend_dims.my;
      if ( anchor_y == AnchorY::Min ) y = legend_dims.my;
      legend->MoveTo( anchor_x, anchor_y, x, y );
      MoveObj( dir, legend, axis_objects, legend_dims.mx, legend_dims.my );
      BoundaryBox bb = legend->GetBB();
      if (
        anchor_y == AnchorY::Max ||
        ((legend_pos == Pos::Left) ? (bb.min.x > best_x) : (bb.min.x < best_x))
      ) {
        best_anchor_y = anchor_y;
        best_x = bb.min.x;
        best_y = y;
      }
    }
    legend->MoveTo( anchor_x, best_anchor_y, x, best_y );
    MoveObj( dir, legend, axis_objects, legend_dims.mx, legend_dims.my );

  } else {

    U avail_w = chart_w;
    uint32_t nx = LegendCnt();
    uint32_t ny = 1;
    while ( 1 ) {
      nx = (LegendCnt() + ny - 1) / ny;
      U need_w =
        nx * legend_dims.sx + (nx - 1) * legend_dims.dx + 2 * legend_dims.mx;
      if ( need_w > avail_w && nx > 1 ) {
        ny++;
        continue;
      }
      break;
    }
    BuildLegend( legend_g->AddNewGroup(), nx );
    Object* legend = legend_g->Last();

    U x = chart_w / 2;
    U y = 0 - legend_dims.my;
    legend->MoveTo( AnchorX::Mid, AnchorY::Max, x, y );
    MoveObj( Dir::Down, legend, axis_objects, legend_dims.mx, legend_dims.my );

  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Main::AxisPrepare( void )
{
  if ( axis_x->angle == 0 ) {
    axis_x->angle = 0;
    axis_y[ 0 ]->angle = 90;
    axis_y[ 1 ]->angle = 90;
  } else {
    axis_x->angle = 90;
    axis_y[ 0 ]->angle = 0;
    axis_y[ 1 ]->angle = 0;
  }

  if ( category_list.size() > 0 ) {
    axis_x->category_axis = true;
    axis_x->log_scale = false;
    axis_x->min = -0.5;
    axis_x->max = axis_x->min + category_list.size();
    axis_x->orth_axis_cross = axis_x->min;
    axis_x->reverse = (axis_x->angle != 0);
  }

  axis_x->data_def = false;
  axis_x->data_min = axis_x->log_scale ? 10 : 0;
  axis_x->data_max = axis_x->log_scale ? 10 : 0;
  for ( auto a : axis_y ) {
    a->data_def = false;
    a->data_min = a->log_scale ? 10 : 0;
    a->data_max = a->log_scale ? 10 : 0;
  }
  {
    std::vector< double > ofs_pos[ 2 ][ 2 ];
    std::vector< double > ofs_neg[ 2 ][ 2 ];
    bool first[ 2 ][ 2 ] = { { true, true }, { true, true } };
    for ( Series* series : series_list ) {
      bool stackable =
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar ||
        series->type == SeriesType::StackedArea;
      int type_n = (series->type == SeriesType::StackedArea) ? 1 : 0;
      int axis_n = series->axis_y_n;
      if ( stackable ) {
        if ( first[ type_n ][ axis_n ] || series->type == SeriesType::Bar ) {
          ofs_pos[ type_n ][ axis_n ].assign( category_list.size(), series->base );
          ofs_neg[ type_n ][ axis_n ].assign( category_list.size(), series->base );
        }
        first[ type_n ][ axis_n ] = false;
      }
      Axis* ax = axis_x;
      Axis* ay = axis_y[ axis_n ];
      if (
        stackable ||
        series->type == SeriesType::Lollipop ||
        series->type == SeriesType::Area
      ) {
        double y = series->base;
        if ( ay->Valid( y ) ) {
          if ( !ay->data_def || ay->data_min > y ) ay->data_min = y;
          if ( !ay->data_def || ay->data_max < y ) ay->data_max = y;
          ay->data_def = true;
        }
      }
      for ( auto& datum : series->datum_list ) {
        double x = datum.x;
        double y = datum.y;
        if ( stackable ) {
          size_t i = x;
          y -= series->base;
          if ( y < 0 ) {
            y += ofs_neg[ type_n ][ axis_n ].at( i );
            ofs_neg[ type_n ][ axis_n ][ i ] = y;
          } else {
            y += ofs_pos[ type_n ][ axis_n ].at( i );
            ofs_pos[ type_n ][ axis_n ][ i ] = y;
          }
        }
        if ( !ax->Valid( x ) ) continue;
        if ( !ay->Valid( y ) ) continue;
        if ( !ax->data_def || ax->data_min > x ) ax->data_min = x;
        if ( !ax->data_def || ax->data_max < x ) ax->data_max = x;
        if ( !ay->data_def || ay->data_min > y ) ay->data_min = y;
        if ( !ay->data_def || ay->data_max < y ) ay->data_max = y;
        ax->data_def = true;
        ay->data_def = true;
      }
    }
  }

  // Show the Y-axis if series data has been associated to the given Y-axis.
  for ( auto a : axis_y ) {
    a->show = a->show || a->data_def;
  }

  // If we only show the secondary axis, then swap the roles.
  if ( !axis_y[ 0 ]->show && axis_y[ 1 ]->show ) {
    std::swap( axis_y[ 0 ], axis_y[ 1 ] );
  }

  // Always show X-axis and primary Y-axis
  axis_x->show = true;
  axis_y[ 0 ]->show = true;

  bool dual_y = axis_y[ 0 ]->show && axis_y[ 1 ]->show;

  if ( axis_x->category_axis ) {
    if ( axis_x->angle == 0 ) {
      if ( axis_x->pos != Pos::Top ) axis_x->pos = Pos::Bottom;
      if ( axis_y[ 0 ]->pos != Pos::Right ) axis_y[ 0 ]->pos = Pos::Left;
    } else {
      if ( axis_x->pos != Pos::Right && axis_x->pos != Pos::Left ) {
        axis_x->pos = axis_y[ 0 ]->reverse ? Pos::Right : Pos::Left;
      }
      if ( axis_y[ 0 ]->pos != Pos::Top ) axis_y[ 0 ]->pos = Pos::Bottom;
    }
    axis_x->number_pos = axis_x->pos;
    if ( axis_x->style == AxisStyle::Auto ) {
      axis_x->style = AxisStyle::None;
    }
    if ( axis_x->style != AxisStyle::None ) {
      axis_x->style = AxisStyle::Edge;
    }
    for ( auto a : axis_y ) {
      if ( a->style == AxisStyle::Auto ) {
        a->style = AxisStyle::None;
      }
    }
  }

  axis_x->orth_dual = dual_y;
  axis_y[ 0 ]->y_dual = dual_y;
  axis_y[ 1 ]->y_dual = dual_y;

  for ( int i : { 0, 1 } ) {
    axis_x->orth_style[ i ] = axis_y[ dual_y ? i : 0 ]->style;
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
  }

  for ( int i : { 0, 1 } ) {
    axis_x->orth_reverse[ i ] = axis_y[ dual_y ? i : 0 ]->reverse;
    axis_y[ i ]->orth_reverse[ 0 ] = axis_x->reverse;
    axis_y[ i ]->orth_reverse[ 1 ] = axis_x->reverse;
  }

  axis_x->LegalizeMinMax();
  for ( auto a : axis_y ) a->LegalizeMinMax();

  // Edge style forces cross point to be at min or max.
  if ( axis_x->style == AxisStyle::Edge ) {
    axis_y[ 0 ]->orth_axis_cross =
      (axis_y[ 0 ]->orth_axis_cross < axis_y[ 0 ]->max)
      ? axis_y[ 0 ]->min
      : axis_y[ 0 ]->max;
  }
  if ( axis_y[ 0 ]->style == AxisStyle::Edge ) {
    axis_x->orth_axis_cross =
      (axis_x->orth_axis_cross < axis_x->max)
      ? axis_x->min
      : axis_x->max;
  }

  if (
    (axis_x->angle == 0)
    ? (axis_x->pos == Pos::Bottom)
    : (axis_x->pos == Pos::Left)
  ) {
    axis_y[ 0 ]->orth_axis_cross =
      axis_y[ 0 ]->reverse ? axis_y[ 0 ]->max : axis_y[ 0 ]->min;
  }
  if (
    (axis_x->angle == 0)
    ? (axis_x->pos == Pos::Top)
    : (axis_x->pos == Pos::Right)
  ) {
    axis_y[ 0 ]->orth_axis_cross =
      axis_y[ 0 ]->reverse ? axis_y[ 0 ]->min : axis_y[ 0 ]->max;
  }
  if (
    (axis_y[ 0 ]->angle == 0)
    ? (axis_y[ 0 ]->pos == Pos::Bottom)
    : (axis_y[ 0 ]->pos == Pos::Left)
  ) {
    axis_x->orth_axis_cross = axis_x->reverse ? axis_x->max : axis_x->min;
  }
  if (
    (axis_y[ 0 ]->angle == 0)
    ? (axis_y[ 0 ]->pos == Pos::Top)
    : (axis_y[ 0 ]->pos == Pos::Right)
  ) {
    axis_x->orth_axis_cross = axis_x->reverse ? axis_x->min : axis_x->max;
  }

  axis_x->orth_axis_coor[ 0 ] =
  axis_x->orth_axis_coor[ 1 ] = axis_x->Coor( axis_x->orth_axis_cross );
  for ( auto a : axis_y ) {
    a->orth_axis_coor[ 0 ] =
    a->orth_axis_coor[ 1 ] = a->Coor( a->orth_axis_cross );
  }
  if ( dual_y ) {
    axis_x->orth_axis_coor[ 0 ] = 0;
    axis_x->orth_axis_coor[ 1 ] = axis_x->length;
  }
  axis_y[ 1 ]->orth_axis_coor[ 0 ] = axis_y[ 0 ]->orth_axis_coor[ 0 ];
  axis_y[ 1 ]->orth_axis_coor[ 1 ] = axis_y[ 0 ]->orth_axis_coor[ 1 ];

  if ( axis_x->style == AxisStyle::Auto ) {
    axis_x->style =
      ( dual_y &&
        ( axis_y[ 0 ]->orth_axis_cross == axis_y[ 0 ]->min ||
          axis_y[ 0 ]->orth_axis_cross == axis_y[ 0 ]->max
        )
      )
      ? AxisStyle::Edge
      : AxisStyle::Arrow;
  }
  for ( auto a : axis_y ) {
    if ( a->style == AxisStyle::Auto ) {
      a->style = dual_y ? AxisStyle::Edge : AxisStyle::Arrow;
    }
  }

  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
    axis_x->orth_style[ i ] = axis_y[ dual_y ? i : 0 ]->style;
  }

  axis_x->orth_coor = axis_y[ 0 ]->orth_axis_coor[ 0 ];
  axis_x->orth_coor_is_min =
    axis_y[ 0 ]->CoorNear( axis_x->orth_coor, 0 );
  axis_x->orth_coor_is_max =
    axis_y[ 0 ]->CoorNear( axis_x->orth_coor, axis_y[ 0 ]->length );
  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_coor = axis_x->orth_axis_coor[ i ];
    axis_y[ i ]->orth_coor_is_min =
      axis_x->CoorNear( axis_y[ i ]->orth_coor, 0 );
    axis_y[ i ]->orth_coor_is_max =
      axis_x->CoorNear( axis_y[ i ]->orth_coor, axis_x->length );
  }

  if ( dual_y ) {
    auto has_grid = [&]( int i ) {
      return
        axis_y[ i ]->major_grid_enable ||
        axis_y[ i ]->minor_grid_enable;
    };
    if ( has_grid( 0 ) && has_grid( 1 ) && !axis_y[ 1 ]->grid_set ) {
      axis_y[ 1 ]->SetGrid( false );
    }
    if ( has_grid( 1 ) && has_grid( 0 ) && !axis_y[ 0 ]->grid_set ) {
      axis_y[ 0 ]->SetGrid( false );
    }
    if ( has_grid( 0 ) && has_grid( 1 ) ) {
      for ( int i : { 0, 1 } ) {
        if (
          axis_y[ i ]->grid_style == GridStyle::Auto &&
          axis_y[ 1 - i ]->grid_style != GridStyle::Auto
        ) {
          axis_y[ i ]->grid_style =
            (axis_y[ 1 - i ]->grid_style == GridStyle::Dash)
            ? GridStyle::Solid
            : GridStyle::Dash;
        }
      }
      if (
        axis_y[ 0 ]->grid_style == GridStyle::Auto &&
        axis_y[ 1 ]->grid_style == GridStyle::Auto
      ) {
        axis_y[ 0 ]->grid_style = GridStyle::Dash;
        axis_y[ 1 ]->grid_style = GridStyle::Solid;
      }
    }
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Main::BuildSeries(
  SVG::Group* chartbox_g1,
  SVG::Group* chartbox_g2,
  std::vector< LegendBox >& lb_list
)
{
  Group* g1 = chartbox_g1->AddNewGroup();
  Group* g2 = chartbox_g2->AddNewGroup();

  uint32_t bar_tmp[ 2 ] = { 0, 0 };
  uint32_t lol_tot = 0;
  for ( Series* series : series_list ) {
    if ( series->type == SeriesType::Lollipop ) {
      lol_tot++;
    }
    if ( series->type == SeriesType::Bar ) {
      bar_tmp[ series->axis_y_n ]++;
    }
    if ( series->type == SeriesType::StackedBar ) {
      if ( bar_tmp[ series->axis_y_n ] == 0 ) bar_tmp[ series->axis_y_n ]++;
    }
  }
  uint32_t bar_tot = bar_tmp[ 0 ] + bar_tmp[ 1 ];

  {
    std::vector< double > ofs_pos[ 2 ];
    std::vector< double > ofs_neg[ 2 ];
    bool first[ 2 ] = { true, true };
    for ( auto type : { SeriesType::StackedArea, SeriesType::Area } ) {
      for ( Series* series : series_list ) {
        if ( series->type == type ) {
          if ( first[ series->axis_y_n ] || series->type == SeriesType::Area ) {
            ofs_pos[ series->axis_y_n ].assign( category_list.size(), series->base );
            ofs_neg[ series->axis_y_n ].assign( category_list.size(), series->base );
          }
          first[ series->axis_y_n ] = false;
          Group* series_g1 = g1->AddNewGroup();
          Group* series_g2 = g2->AddNewGroup();
          if ( series->type == SeriesType::StackedArea ) {
            g2->FrontToBack();
          }
          series->Build(
            series_g1, series_g2,
            axis_x, axis_y[ series->axis_y_n ], lb_list,
            0, 1,
            &ofs_pos[ series->axis_y_n ], &ofs_neg[ series->axis_y_n ]
          );
        }
      }
    }
  }

  {
    std::vector< double > ofs_pos[ 2 ];
    std::vector< double > ofs_neg[ 2 ];
    uint32_t bar_num[ 2 ] = { 0, 0 };
    uint32_t bar_cur = 0;
    bool first[ 2 ] = { true, true };
    for ( Series* series : series_list ) {
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar
      ) {
        if ( first[ series->axis_y_n ] || series->type == SeriesType::Bar ) {
          ofs_pos[ series->axis_y_n ].assign( category_list.size(), series->base );
          ofs_neg[ series->axis_y_n ].assign( category_list.size(), series->base );
        }
        if ( series->type == SeriesType::Bar || first[ series->axis_y_n ] ) {
          if ( !first[ 0 ] || !first[ 1 ] ) {
            bar_cur++;
            bar_num[ series->axis_y_n ] = bar_cur;
          }
        }
        Group* series_g = g1->AddNewGroup();
        series->Build(
          series_g, nullptr,
          axis_x, axis_y[ series->axis_y_n ], lb_list,
          bar_num[ series->axis_y_n ], bar_tot,
          &ofs_pos[ series->axis_y_n ], &ofs_neg[ series->axis_y_n ]
        );
        first[ series->axis_y_n ] = false;
      }
    }
  }

  {
    uint32_t lol_num = 0;
    for ( Series* series : series_list ) {
      if ( series->type == SeriesType::Lollipop ) {
        Group* series_g = g1->AddNewGroup();
        series->Build(
          series_g, nullptr,
          axis_x, axis_y[ series->axis_y_n ], lb_list,
          lol_num, lol_tot
        );
        lol_num++;
      }
    }
  }

  for ( Series* series : series_list ) {
    if (
      series->type == SeriesType::XY ||
      series->type == SeriesType::Line ||
      series->type == SeriesType::Scatter ||
      series->type == SeriesType::Point
    ) {
      Group* series_g = g1->AddNewGroup();
      series->Build(
        series_g, nullptr,
        axis_x, axis_y[ series->axis_y_n ], lb_list, 0, 1
      );
    }
  }

  return;
}

//------------------------------------------------------------------------------

Canvas* Main::Build( void )
{
  Canvas* canvas = new Canvas();

  Group* chart_g = canvas->TopGroup()->AddNewGroup();
  chart_g->Attr()->TextFont()->SetFamily(
    "DejaVu Sans Mono,Consolas,Menlo,Courier New"
  );
  chart_g->Attr()->FillColor()->Set( ColorName::white );
  chart_g->Attr()->LineColor()->Clear();
  chart_g->Add( new Rect( 0, 0, chart_w, chart_h ) );

  Group* chartbox_g2  = chart_g->AddNewGroup();
  Group* grid_minor_g = chart_g->AddNewGroup();
  Group* grid_major_g = chart_g->AddNewGroup();
  Group* grid_zero_g  = chart_g->AddNewGroup();

  Group* axes_line_g  = chart_g->AddNewGroup();
  Group* chartbox_g1  = chart_g->AddNewGroup();
  Group* axes_num_g   = chart_g->AddNewGroup();
  Group* axes_label_g = chart_g->AddNewGroup();
  Group* legend_g     = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( ColorName::black );
  axes_line_g->Attr()->SetLineCap( LineCap::Square );

  chartbox_g1->Attr()->FillColor()->Clear();
  chartbox_g2->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->TextFont()->SetSize( 14 );
  axes_num_g->Attr()->LineColor()->Clear();

  legend_g->Attr()->TextFont()->SetSize( 14 );

  std::vector< SVG::Object* > axis_objects;

  axis_x->length      = (axis_x->angle == 0) ? chart_w : chart_h;
  axis_x->orth_length = (axis_x->angle == 0) ? chart_h : chart_w;
  for ( auto a : axis_y ) {
    a->length      = (a->angle == 0) ? chart_w : chart_h;
    a->orth_length = (a->angle == 0) ? chart_h : chart_w;
  }

  AxisPrepare();

  for ( uint32_t phase : {0, 1} ) {
    axis_x->Build(
      category_list,
      phase,
      axis_objects,
      grid_minor_g, grid_major_g, grid_zero_g,
      axes_line_g, axes_num_g, axes_label_g
    );
    for ( int i : { 1, 0 } ) {
      std::vector< std::string > empty;
      axis_y[ i ]->Build(
        empty,
        phase,
        axis_objects,
        grid_minor_g, grid_major_g, grid_zero_g,
        axes_line_g, axes_num_g, axes_label_g
      );
    }
  }

  axis_x->BuildLabel( axis_objects, axes_label_g );
  for ( auto a : axis_y ) {
    a->BuildLabel( axis_objects, axes_label_g );
  }

  // Do title.
  {
    U space_x = 40;
    U space_y = 10;
    std::vector< SVG::Object* > title_objs;
    U y = chart_h + space_y;
    if ( sub_sub_title != "" ) {
      Object* obj = MultiLineText( chart_g, sub_sub_title, 14 );
      obj->MoveTo( AnchorX::Mid, AnchorY::Min, chart_w / 2, y );
      title_objs.push_back( obj );
      BoundaryBox bb = obj->GetBB();
      y += bb.max.y - bb.min.y + 3;
    }
    if ( sub_title != "" ) {
      Object* obj = MultiLineText( chart_g, sub_title, 20 );
      obj->MoveTo( AnchorX::Mid, AnchorY::Min, chart_w / 2, y );
      title_objs.push_back( obj );
      BoundaryBox bb = obj->GetBB();
      y += bb.max.y - bb.min.y + 3;
    }
    if ( title != "" ) {
      Object* obj = MultiLineText( chart_g, title, 36 );
      obj->MoveTo( AnchorX::Mid, AnchorY::Min, chart_w / 2, y );
      title_objs.push_back( obj );
      BoundaryBox bb = obj->GetBB();
      y += bb.max.y - bb.min.y;
    }
    MoveObjs( Dir::Up, title_objs, axis_objects, space_x, space_y );
    y = 0;
    for ( auto obj : title_objs ) {
      y = std::max( y, obj->GetBB().max.y );
    }
    y = chart_g->GetBB().max.y - y;
    if ( y > 0 ) {
      for ( auto obj : title_objs ) {
        obj->Move( 0, y );
      }
    }
  }

/*
  {
    for ( auto obj : axis_objects ) {
      if ( obj->Empty() ) continue;
      BoundaryBox bb = obj->GetBB();
      chart_g->Add( new Rect( bb.min, bb.max ) );
      chart_g->Last()->Attr()->FillColor()->Clear();
      chart_g->Last()->Attr()->SetLineWidth( 1 );
      chart_g->Last()->Attr()->LineColor()->Set( ColorName::blue );
    }
  }
*/

  std::vector< LegendBox > lb_list;
  CalcLegendBoxes( legend_g, lb_list, axis_objects );

  BuildSeries( chartbox_g1, chartbox_g2, lb_list );

  PlaceLegend( axis_objects, lb_list, legend_g );

  if ( footnote != "" ) {
    BoundaryBox bb = chart_g->GetBB();
    U x = bb.min.x + 15;
    U y = bb.min.y - 15;
    AnchorX a = AnchorX::Min;
    MultiLineText( chart_g, footnote, 14 );
    if ( footnote_pos == Pos::Center ) {
      x = chart_w / 2;
      a = AnchorX::Mid;
    }
    if ( footnote_pos == Pos::Right ) {
      x = bb.max.x - 15;
      a = AnchorX::Max;
    }
    chart_g->Last()->MoveTo( a, AnchorY::Max, x, y );
  }

  BoundaryBox bb = chart_g->GetBB();
  chart_g->Add(
    new Rect(
      bb.min.x - margin, bb.min.y - margin,
      bb.max.x + margin, bb.max.y + margin
    )
  );
  chart_g->Last()->Attr()->LineColor()->Clear();
  chart_g->FrontToBack();

  return canvas;
}

///////////////////////////////////////////////////////////////////////////////
