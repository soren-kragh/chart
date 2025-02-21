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
  frame_color.Set( ColorName::maroon );
  background_color.Set( ColorName::white );
  chart_area_color.Clear();
  axis_color.Set( ColorName::black );
  text_color.Set( ColorName::black );
  width_adj    = 1.0;
  height_adj   = 1.0;
  baseline_adj = 1.0;
  SetLegendPos( Pos::Auto );
  legend_color.Clear();
  label_db = new Label();
  tag_db = new Tag();
  axis_x      = new Axis( true , label_db );
  axis_y[ 0 ] = new Axis( false, label_db );
  axis_y[ 1 ] = new Axis( false, label_db );
}

Main::~Main( void )
{
  for ( auto series : series_list ) {
    delete series;
  }
  delete axis_x;
  delete axis_y[ 0 ];
  delete axis_y[ 1 ];
  delete tag_db;
  delete label_db;
}

///////////////////////////////////////////////////////////////////////////////

void Main::SetFrameWidth( SVG::U width )
{
  this->frame_width = width;
}

void Main::SetMargin( SVG::U margin )
{
  this->margin = margin;
}

void Main::SetChartArea( SVG::U width, SVG::U height )
{
  chart_w = std::max( U( 100 ), width );
  chart_h = std::max( U( 100 ), height );
}

void Main::SetChartBox( bool chart_box )
{
  this->chart_box = chart_box;
}

void Main::SetLetterSpacing(
  float width_adj, float height_adj, float baseline_adj
)
{
  this->width_adj    = width_adj;
  this->height_adj   = height_adj;
  this->baseline_adj = baseline_adj;
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

void Main::SetTitlePos( Pos pos )
{
  this->title_pos = pos;
}

void Main::SetTitleInside( bool inside )
{
  this->title_inside = inside;
}

void Main::AddFootnote(std::string& txt)
{
  footnotes.emplace_back( footnote_t{ txt, Pos::Left } );
}

void Main::SetFootnotePos( Pos pos )
{
  if ( !footnotes.empty() ) {
    footnotes.back().pos = pos;
  }
}

void Main::SetFootnoteLine( bool footnote_line )
{
  this->footnote_line = footnote_line;
}

void Main::SetLegendPos( Pos pos )
{
  legend_pos = pos;
}

void Main::SetBarWidth( float one_width, float all_width )
{
  bar_one_width = one_width;
  bar_all_width = all_width;
}

Series* Main::AddSeries( SeriesType type )
{
  Series* series = new Series( type );
  int style = series_list.size() % 80;
  series->SetStyle( style );
  series_list.push_back( series );
  return series;
}

void Main::AddCategory( const std::string& category )
{
  category_list.push_back( category );
}

///////////////////////////////////////////////////////////////////////////////

uint32_t Main::LegendCnt( void )
{
  uint32_t n = 0;
  for ( auto series : series_list ) {
    if ( series->name.length() > 0 ) n++;
  }
  return n;
}

//-----------------------------------------------------------------------------

void Main::CalcLegendDims( Group* g, LegendDims& legend_dims )
{
  U gx = 8;     // X-gap between series legends.
  U gy = 4;     // Y-gap between series legends.

  legend_dims.ch = 0;
  legend_dims.mw = 0;
  legend_dims.cr = 0;
  legend_dims.mh = 0;
  legend_dims.ss = 0;
  legend_dims.lx = 0;
  legend_dims.rx = 0;
  legend_dims.tx = 0;
  legend_dims.dx = gx;
  legend_dims.dy = gy;
  legend_dims.sx = 0;
  legend_dims.sy = 0;
  legend_dims.mx = 2 * box_spacing;
  legend_dims.my = 2 * box_spacing;

  g->Add( new Text( "X" ) );
  BoundaryBox bb = g->Last()->GetBB();
  g->DeleteFront();
  U char_w = bb.max.x - bb.min.x;
  U char_h = bb.max.y - bb.min.y;

  U ox = char_h / 3;    // Text to outline X spacing.
  U oy = char_h / 5;    // Text to outline Y spacing.

  for ( auto series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    bool has_outline =
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;
    if ( has_outline ) {
      legend_dims.mw = std::max( legend_dims.mw, series->line_width );
    }
  }

  U hmw = legend_dims.mw / 2;

  for ( auto series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea
    ) {
      Series::MarkerDims md = series->marker_out;
      legend_dims.mh = std::max( +legend_dims.mh, md.y2 - md.y1 );
      legend_dims.ss = std::max( +legend_dims.ss, -md.x1 );
      legend_dims.ss = std::max( +legend_dims.ss, +md.x2 );
    }
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      legend_dims.ss =
        std::max( +legend_dims.ss, (char_h + 8) / 2 );
      if ( !series->line_color.IsClear() ) {
        legend_dims.ss = std::max( +legend_dims.ss, 2 * series->line_width );
      }
      if ( series->line_dash > 0 ) {
        legend_dims.ss =
          std::max(
            +legend_dims.ss, (series->line_dash + series->line_hole) * 0.75
          );
      }
    }
  }

  legend_dims.ch = char_h;
  legend_dims.lx = std::max( +legend_dims.lx, legend_dims.ss - hmw );
  legend_dims.dx += legend_dims.lx;
  legend_dims.tx = hmw + legend_dims.lx + ox;

  if ( hmw > 0 ) {
    legend_dims.cr = hmw + char_h / 4;
  }

  for ( auto series : series_list ) {
    if ( series->name.length() == 0 ) continue;

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
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

    if ( has_outline ) legend_dims.rx = legend_dims.lx;

    legend_dims.sx =
      std::max(
        +legend_dims.sx,
        2 * hmw + legend_dims.lx + ox + text_w + ox +
        (has_outline ? (2 * hmw) : 0)
      );

    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        text_h +
        (has_outline ? (2 * (oy + series->line_width / 2 + hmw)) : 0)
      );
    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        has_outline
        ? (legend_dims.mh + 2*(legend_dims.cr + hmw))
        : (2 * legend_dims.ss)
      );
  }

  return;
}

//-----------------------------------------------------------------------------

// Determine potential placement of series legends in chart interior.
void Main::CalcLegendBoxes(
  Group* g, std::vector< LegendBox >& lb_list,
  const std::vector< SVG::Object* >& avoid_objects
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
          legend_dims.lx + legend_dims.rx,
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
        U old_y = coor_hi;
        while ( true ) {
          BoundaryBox obj_bb = obj->GetBB();
          if ( obj_bb.min.y == old_y ) break;
          old_y = obj_bb.min.y;
          U dy = 0;
          for ( auto ao : avoid_objects ) {
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
        U old_x = coor_hi;
        while ( true ) {
          BoundaryBox obj_bb = obj->GetBB();
          if ( obj_bb.min.x == old_x ) break;
          old_x = obj_bb.min.x;
          U dx = 0;
          for ( auto ao : avoid_objects ) {
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

      if ( !Collides( obj, avoid_objects ) ) {
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

void Main::BuildLegends( Group* g, int nx, bool framed )
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
  LegendDims legend_dims;
  CalcLegendDims( g, legend_dims );
  int ny = (LegendCnt() + nx - 1) / nx;

  {
    U mx = framed ? legend_dims.mx : U( 0 );
    U my = framed ? legend_dims.my : U( 0 );
    Point r1{
      -mx / 2 - legend_dims.lx, +my / 2
    };
    Point r2{
      legend_dims.rx +
      +(nx * legend_dims.sx + (nx - 1) * legend_dims.dx + mx / 2),
      -(ny * legend_dims.sy + (ny - 1) * legend_dims.dy + my / 2)
    };
    g->Add( new Rect( r1, r2, framed ? box_spacing : U( 0 ) ) );
    if ( framed ) {
      g->Last()->Attr()->LineColor()->Set( &axis_color );
      g->Last()->Attr()->SetLineWidth( 1 );
      if ( !LegendColor()->IsClear() ) {
        g->Last()->Attr()->FillColor()->Set( LegendColor() );
      }
    } else {
      g->Last()->Attr()->FillColor()->Clear();
      g->Last()->Attr()->LineColor()->Clear();
      g->Last()->Attr()->SetLineWidth( 0 );
    }
  }

  int n = 0;
  for ( auto series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    U px = (n % nx) * +(legend_dims.sx + legend_dims.dx);
    U py = (n / nx) * -(legend_dims.sy + legend_dims.dy);
    Point marker_p{ px + legend_dims.mw/2, py - legend_dims.sy/2 };

    U line_w = series->line_width;
    if ( !series->has_line ) line_w = 0;

    bool has_outline =
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

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
    px += legend_dims.mw / 2 + legend_dims.tx;
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

//-----------------------------------------------------------------------------

void Main::PlaceLegends(
  std::vector< SVG::Object* >& avoid_objects,
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
      BuildLegends( legend_g->AddNewGroup(), best_lb.nx, true );
      legend_g->Last()->MoveTo(
        AnchorX::Mid, AnchorY::Mid,
        (best_lb.bb.min.x + best_lb.bb.max.x) / 2,
        (best_lb.bb.min.y + best_lb.bb.max.y) / 2
      );
      return;
    } else {
      legend_pos = Pos::Bottom;
    }
  }

  LegendDims legend_dims;
  CalcLegendDims( legend_g, legend_dims );

  if ( legend_pos == Pos::Left || legend_pos == Pos::Right ) {

    U mx = legend_dims.mx;
    U my = 10;

    U avail_h = chart_h;
    uint32_t nx = 1;
    while ( 1 ) {
      uint32_t ny = (LegendCnt() + nx - 1) / nx;
      U need_h = ny * legend_dims.sy + (ny - 1) * legend_dims.dy;
      if ( need_h > avail_h && ny > 1 ) {
        nx++;
        continue;
      }
      break;
    }
    BuildLegends( legend_g->AddNewGroup(), nx, false );
    Object* legend = legend_g->Last();

    U x = 0 - mx;
    Dir dir = Dir::Left;
    AnchorX anchor_x = AnchorX::Max;
    if ( legend_pos == Pos::Right ) {
      x = chart_w + mx;
      dir = Dir::Right;
      anchor_x = AnchorX::Min;
    }
    AnchorY best_anchor_y{ AnchorY::Max };
    U best_x{ 0 };
    U best_y{ 0 };
    bool best_found = false;
    for ( auto anchor_y : { AnchorY::Max, AnchorY::Mid, AnchorY::Min } ) {
      U y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h;
      if ( anchor_y == AnchorY::Min ) y = 0;
      legend->MoveTo( anchor_x, anchor_y, x, y );
      MoveObj( dir, legend, avoid_objects, mx, my );
      BoundaryBox bb = legend->GetBB();
      if (
        !best_found ||
        ((legend_pos == Pos::Right) ? (bb.min.x < best_x) : (bb.min.x > best_x))
      ) {
        best_anchor_y = anchor_y;
        best_x = bb.min.x;
        best_y = y;
        best_found = true;
      }
    }
    legend->MoveTo( anchor_x, best_anchor_y, x, best_y );
    MoveObj( dir, legend, avoid_objects, mx, my );
    avoid_objects.push_back( legend );

  } else {

    U mx = 40;
    U my = legend_dims.my / 2;

    U avail_w = chart_w;
    uint32_t nx = LegendCnt();
    uint32_t ny = 1;
    while ( 1 ) {
      nx = (LegendCnt() + ny - 1) / ny;
      U need_w = nx * legend_dims.sx + (nx - 1) * legend_dims.dx;
      if ( need_w > avail_w && nx > 1 ) {
        ny++;
        continue;
      }
      break;
    }
    BuildLegends( legend_g->AddNewGroup(), nx, false );
    Object* legend = legend_g->Last();

    U y = 0 - my;
    Dir dir = Dir::Down;
    AnchorY anchor_y = AnchorY::Max;
    if ( legend_pos == Pos::Top ) {
      y = chart_h + my;
      dir = Dir::Up;
      anchor_y = AnchorY::Min;
    }
    AnchorX best_anchor_x{ AnchorX::Mid };
    U best_x{ 0 };
    U best_y{ 0 };
    bool best_found = false;
    for ( auto anchor_x : { AnchorX::Mid, AnchorX::Min, AnchorX::Max } ) {
      U x = chart_w / 2;
      if ( anchor_x == AnchorX::Max ) x = chart_w;
      if ( anchor_x == AnchorX::Min ) x = 0;
      legend->MoveTo( anchor_x, anchor_y, x, y );
      MoveObj( dir, legend, avoid_objects, mx, my );
      BoundaryBox bb = legend->GetBB();
      if (
        !best_found ||
        ((legend_pos == Pos::Top) ? (bb.min.y < best_y) : (bb.min.y > best_y))
      ) {
        best_anchor_x = anchor_x;
        best_x = x;
        best_y = bb.min.y;
        best_found = true;
      }
    }
    legend->MoveTo( best_anchor_x, anchor_y, best_x, y );
    MoveObj( dir, legend, avoid_objects, mx, my );
    avoid_objects.push_back( legend );

  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

int Main::CategoryStride( void )
{
  int stride = -1;
  {
    int s = 1;
    for ( const auto& cat : category_list ) {
      if ( cat.empty() ) {
        s++;
      } else {
        if ( stride < 0 ) {
          stride = 0;
        } else {
          if ( stride == 0 || s < stride ) stride = s;
        }
        s = 1;
      }
    }
  }
  if ( stride < 1 ) stride = 1;
  return stride;
}

///////////////////////////////////////////////////////////////////////////////

void Main::AxisPrepare( SVG::Group* tag_g )
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

  bool category_axis = false;
  for ( auto series : series_list ) {
    if (
      series->type != SeriesType::XY &&
      series->type != SeriesType::Scatter
    )
      category_axis = true;
  }

  if ( category_axis ) {
    bool no_bar = true;
    for ( auto series : series_list ) {
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar ||
        series->type == SeriesType::Lollipop
      )
        no_bar = false;
    }
    axis_x->category_axis = true;
    axis_x->log_scale = false;
    axis_x->min = (no_bar && !category_list.empty()) ? 0.0 : -0.5;
    axis_x->max =
      axis_x->min
      + std::max( category_list.size(), size_t( 1 ) )
      - ((axis_x->min < 0) ? 0 : 1);
    axis_x->orth_axis_cross = axis_x->min;
    axis_x->reverse = axis_x->reverse ^ (axis_x->angle != 0);
    axis_x->category_stride = CategoryStride();
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
    for ( auto series : series_list ) {
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
      series->DetermineMinMax(
        ofs_pos[ type_n ][ axis_n ],
        ofs_neg[ type_n ][ axis_n ]
      );
      if ( series->def_x ) {
        Axis* ax = series->axis_x;
        if ( !ax->data_def || ax->data_min > series->min_x ) {
          ax->data_min = series->min_x;
        }
        if ( !ax->data_def || ax->data_max < series->max_x ) {
          ax->data_max = series->max_x;
        }
        ax->data_def = true;
      }
      if ( series->def_y ) {
        Axis* ay = series->axis_y;
        if ( !ay->data_def || ay->data_min > series->min_y ) {
          ay->data_min = series->min_y;
          ay->data_min_is_base = series->min_y_is_base;
        }
        if ( !ay->data_def || ay->data_max < series->max_y ) {
          ay->data_max = series->max_y;
          ay->data_max_is_base = series->max_y_is_base;
        }
        ay->data_def = true;
      }
    }
  }

  // Show the Y-axis if series data has been associated to the given Y-axis.
  for ( auto a : axis_y ) {
    a->show = a->show || a->data_def;
  }

  // Legalize axis_x->pos_base_axis_y_n.
  {
    if ( axis_x->pos_base_axis_y_n < 0 ) axis_x->pos_base_axis_y_n = 0;
    if ( axis_x->pos_base_axis_y_n > 1 ) axis_x->pos_base_axis_y_n = 1;
    int sn = 0;
    for ( int i : { 1, 0 } ) {
      if ( axis_y[ i ]->show ) sn = i;
    }
    if ( !axis_y[ axis_x->pos_base_axis_y_n ]->show ) {
      axis_x->pos_base_axis_y_n = sn;
      axis_y[ sn ]->show = true;
    }
  }

  // If we only show the secondary axis, then swap the roles.
  if ( !axis_y[ 0 ]->show && axis_y[ 1 ]->show ) {
    std::swap( axis_y[ 0 ], axis_y[ 1 ] );
    for ( auto series : series_list ) {
      series->axis_y_n = 0;
    }
    axis_x->pos_base_axis_y_n = 0;
  }

  // Always show X-axis and primary Y-axis
  axis_x->show = true;
  axis_y[ 0 ]->show = true;

  bool dual_y = axis_y[ 0 ]->show && axis_y[ 1 ]->show;

  if ( axis_x->category_axis ) {
    if ( axis_x->pos != Pos::Base ) {
      if ( axis_x->angle == 0 ) {
        if ( axis_x->pos != Pos::Top && axis_x->pos != Pos::Bottom ) {
          axis_x->pos = Pos::Auto;
        }
      } else {
        if ( axis_x->pos != Pos::Right && axis_x->pos != Pos::Left ) {
          axis_x->pos = Pos::Auto;
        }
      }
    }
    if ( axis_x->pos == Pos::Auto || axis_x->pos == Pos::Base ) {
      int base_def[ 2 ] = { 0, 0 };
      double base[ 2 ];
      for ( auto series : series_list ) {
        if (
          series->type == SeriesType::Lollipop ||
          series->type == SeriesType::Bar ||
          series->type == SeriesType::StackedBar ||
          series->type == SeriesType::Area ||
          series->type == SeriesType::StackedArea
        ) {
          if ( base_def[ series->axis_y_n ] == 2 ) continue;
          if ( base_def[ series->axis_y_n ] == 1 ) {
            if ( series->base != base[ series->axis_y_n ] ) {
              base_def[ series->axis_y_n ] = 2;
            }
            continue;
          }
          base_def[ series->axis_y_n ] = 1;
          base[ series->axis_y_n ] = series->base;
        }
      }
      if ( axis_x->pos == Pos::Base ) {
        int i = axis_x->pos_base_axis_y_n;
        if ( base_def[ i ] == 1 ) {
          axis_y[ i ]->orth_axis_cross = base[ i ];
        } else {
          axis_x->pos = Pos::Auto;
        }
      } else {
        for ( int i = 0; i < 2; i++ ) {
          if ( base_def[ i ] == 1 ) {
            axis_y[ i ]->orth_axis_cross = base[ i ];
            axis_x->pos = Pos::Base;
            axis_x->pos_base_axis_y_n = i;
            break;
          }
        }
      }
    }
    if ( axis_x->angle == 0 ) {
      if ( axis_x->pos != Pos::Base ) {
        if ( axis_x->pos != Pos::Top ) axis_x->pos = Pos::Bottom;
      }
      // Assuming not dual:
      if ( axis_y[ 0 ]->pos != Pos::Right ) axis_y[ 0 ]->pos = Pos::Left;
    } else {
      if ( axis_x->pos != Pos::Base ) {
        if ( axis_x->pos != Pos::Right && axis_x->pos != Pos::Left ) {
          axis_x->pos = axis_y[ 0 ]->reverse ? Pos::Right : Pos::Left;
        }
      }
      // Assuming not dual:
      if ( axis_y[ 0 ]->pos != Pos::Top ) axis_y[ 0 ]->pos = Pos::Bottom;
    }
    if ( axis_x->style == AxisStyle::Auto ) {
      axis_x->style =
        (axis_x->pos == Pos::Base) ? AxisStyle::Line : AxisStyle::None;
    }
    if ( axis_x->style != AxisStyle::Line ) {
      axis_x->style = AxisStyle::None;
    }
    for ( auto a : axis_y ) {
      if ( a->style == AxisStyle::Auto ) {
        a->style = AxisStyle::None;
      }
    }
  } else {
    if ( axis_x->pos == Pos::Base ) axis_x->pos = Pos::Auto;
  }
  if ( axis_x->pos != Pos::Base ) {
    axis_x->pos_base_axis_y_n = 0;
  }
  for ( auto a : axis_y ) {
    if ( a->pos == Pos::Base ) a->pos = Pos::Auto;
    a->pos_base_axis_y_n = 0;
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

  axis_x->LegalizeMinMax( nullptr, nullptr );
  for ( auto a : axis_y ) a->LegalizeMinMax( tag_g, &series_list );

  if ( axis_x->pos == Pos::Base ) {
    int i = axis_x->pos_base_axis_y_n;
    if (
      axis_y[ i ]->orth_axis_cross < axis_y[ i ]->min ||
      axis_y[ i ]->orth_axis_cross > axis_y[ i ]->max
    )
      axis_x->style = AxisStyle::None;
  }

  // Edge style forces cross point to be at min or max.
  if ( axis_x->style == AxisStyle::Edge ) {
    for ( auto a : axis_y ) {
      a->orth_axis_cross = (a->orth_axis_cross < a->max) ? a->min : a->max;
    }
  }

  // Assuming not dual:
  if ( axis_y[ 0 ]->style == AxisStyle::Edge ) {
    axis_x->orth_axis_cross =
      (axis_x->orth_axis_cross < axis_x->max)
      ? axis_x->min
      : axis_x->max;
  }

  // Assuming not dual:
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

  if ( axis_x->pos == Pos::Base ) {
    int x = axis_x->pos_base_axis_y_n;
    axis_x->orth_coor = axis_y[ x ]->orth_axis_coor[ 0 ];
  } else {
    axis_x->orth_coor = axis_y[ 0 ]->orth_axis_coor[ 0 ];
  }
  {
    int x = (axis_x->pos == Pos::Base) ? axis_x->pos_base_axis_y_n : 0;
    auto a = axis_y[ x ];
    for ( int i : { 0, 1 } ) {
      axis_y[ i ]->orth_axis_coor[ 0 ] = a->orth_axis_coor[ 0 ];
      axis_y[ i ]->orth_axis_coor[ 1 ] = a->orth_axis_coor[ 1 ];
    }
  }

  if ( axis_x->style == AxisStyle::Auto ) {
    axis_x->style =
      ( dual_y &&
        ( axis_y[ 0 ]->orth_axis_cross == axis_y[ 0 ]->min ||
          axis_y[ 0 ]->orth_axis_cross == axis_y[ 0 ]->max
        )
      )
      ? AxisStyle::Edge
      : (chart_box ? AxisStyle::Edge : AxisStyle::Arrow);
  }
  for ( auto a : axis_y ) {
    if ( a->style == AxisStyle::Auto ) {
      a->style =
        dual_y
        ? AxisStyle::Edge
        : (chart_box ? AxisStyle::Edge : AxisStyle::Arrow);
    }
  }

  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
    axis_x->orth_style[ i ] = axis_y[ dual_y ? i : 0 ]->style;
  }

  axis_x->orth_coor = axis_y[ 0 ]->orth_axis_coor[ 0 ];
  axis_x->orth_coor_is_min = CoorNear( axis_x->orth_coor, 0 );
  axis_x->orth_coor_is_max = CoorNear( axis_x->orth_coor, axis_y[ 0 ]->length );
  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_coor = axis_x->orth_axis_coor[ i ];
    axis_y[ i ]->orth_coor_is_min = CoorNear( axis_y[ i ]->orth_coor, 0 );
    axis_y[ i ]->orth_coor_is_max = CoorNear( axis_y[ i ]->orth_coor, axis_x->length );
  }

  axis_x->cat_coor = axis_x->orth_coor;
  axis_x->cat_coor_is_min = axis_x->orth_coor_is_min;
  axis_x->cat_coor_is_max = axis_x->orth_coor_is_max;
  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->cat_coor = axis_y[ i ]->orth_coor;
    axis_y[ i ]->cat_coor_is_min = axis_y[ i ]->orth_coor_is_min;
    axis_y[ i ]->cat_coor_is_max = axis_y[ i ]->orth_coor_is_max;
  }

  if ( chart_box ) {
    if ( !axis_x->orth_coor_is_min && !axis_x->orth_coor_is_max ) {
      if ( axis_x->style == AxisStyle::Edge ) axis_x->style = AxisStyle::Line;
    }
    for ( auto a : axis_y ) {
      if ( !a->orth_coor_is_min && !a->orth_coor_is_max ) {
        if ( a->style == AxisStyle::Edge ) a->style = AxisStyle::Line;
      }
    }
  }

  if ( axis_x->category_axis && !axis_x->grid_set ) {
    axis_x->major_grid_enable = false;
    axis_x->minor_grid_enable = false;
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

void Main::SeriesPrepare(
  std::vector< LegendBox >* lb_list
)
{
  bar_tot = 0;
  lol_tot = 0;

  uint32_t bar_tmp[ 2 ] = { 0, 0 };
  for ( auto series : series_list ) {
    series->chart_area.min.x = 0;
    series->chart_area.max.x = chart_w;
    series->chart_area.min.y = 0;
    series->chart_area.max.y = chart_h;
    series->bar_one_width = bar_one_width;
    series->bar_all_width = bar_all_width;
    series->axis_x = axis_x;
    series->axis_y = axis_y[ series->axis_y_n ];
    series->lb_list = lb_list;
    series->tag_db = tag_db;

    if ( series->type == SeriesType::Lollipop ) {
      lol_tot++;
    }

    if ( series->type == SeriesType::Bar ) {
      bar_tmp[ series->axis_y_n ]++;
    }

    if ( series->type == SeriesType::StackedBar ) {
      if ( bar_tmp[ series->axis_y_n ] == 0 ) bar_tmp[ series->axis_y_n ]++;
    }

    if ( !series->tag_text_color.IsDefined() ) {
      series->tag_text_color.Set( &text_color );
    }

    if ( !series->tag_fill_color.IsDefined() ) {
      if ( series->line_color.IsClear() ) {
        if ( series->fill_color.IsClear() ) {
          series->tag_fill_color.Clear();
        } else {
          series->tag_fill_color.Set( &series->fill_color );
        }
      } else {
        series->tag_fill_color.Set( &series->line_color );
      }
      series->tag_fill_color.SetTransparency( 0.5 );
    }

    if ( !series->tag_line_color.IsDefined() ) {
      if ( series->line_color.IsClear() ) {
        if ( series->fill_color.IsClear() ) {
          series->tag_line_color.Clear();
        } else {
          series->tag_line_color.Set( &series->fill_color );
        }
      } else {
        series->tag_line_color.Set( &series->line_color );
      }
      series->tag_line_color.SetTransparency( 0.0 );
    }

    series->DetermineVisualProperties();
  }
  bar_tot = bar_tmp[ 0 ] + bar_tmp[ 1 ];

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Main::BuildSeries(
  SVG::Group* below_axes_g,
  SVG::Group* above_axes_g,
  SVG::Group* tag_g
)
{
  std::vector< double > sa_ofs_pos[ 2 ];
  std::vector< double > sa_ofs_neg[ 2 ];
  std::vector< Point > sa_pts_pos[ 2 ];
  std::vector< Point > sa_pts_neg[ 2 ];
  bool sa_first[ 2 ] = { true, true };

  std::vector< double > bar_ofs_pos[ 2 ];
  std::vector< double > bar_ofs_neg[ 2 ];
  uint32_t bar_num[ 2 ] = { 0, 0 };
  uint32_t bar_cur = 0;
  bool bar_first[ 2 ] = { true, true };

  uint32_t lol_num = 0;

  Group* stacked_area_fill_g = below_axes_g->AddNewGroup();
  Group* stacked_area_line_g = below_axes_g->AddNewGroup();
  Group* bar_area_g          = below_axes_g->AddNewGroup();
  Group* lollipop_stem_g     = below_axes_g->AddNewGroup();

  for ( auto series : series_list ) {
    int y_n = series->axis_y_n;
    if ( series->type == SeriesType::StackedArea ) {
      if ( sa_first[ y_n ] ) {
        sa_ofs_pos[ y_n ].assign( category_list.size(), series->base );
        sa_ofs_neg[ y_n ].assign( category_list.size(), series->base );
      }
      sa_first[ y_n ] = false;
      series->Build(
        stacked_area_line_g, stacked_area_fill_g, above_axes_g, tag_g,
        0, 1,
        &sa_ofs_pos[ y_n ], &sa_ofs_neg[ y_n ],
        &sa_pts_pos[ y_n ], &sa_pts_neg[ y_n ]
      );
    }
    if ( series->type == SeriesType::Area ) {
      std::vector< double > ofs_pos( category_list.size(), series->base );
      std::vector< double > ofs_neg( category_list.size(), series->base );
      std::vector< Point > pts_pos;
      std::vector< Point > pts_neg;
      series->Build(
        bar_area_g, bar_area_g, above_axes_g, tag_g,
        0, 1,
        &ofs_pos, &ofs_neg,
        &pts_pos, &pts_neg
      );
    }
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar
    ) {
      if ( bar_first[ y_n ] || series->type == SeriesType::Bar ) {
        bar_ofs_pos[ y_n ].assign( category_list.size(), series->base );
        bar_ofs_neg[ y_n ].assign( category_list.size(), series->base );
      }
      if ( series->type == SeriesType::Bar || bar_first[ y_n ] ) {
        if ( !bar_first[ 0 ] || !bar_first[ 1 ] ) {
          bar_cur++;
          bar_num[ y_n ] = bar_cur;
        }
      }
      series->Build(
        bar_area_g, nullptr, nullptr, tag_g,
        bar_num[ y_n ], bar_tot,
        &bar_ofs_pos[ y_n ], &bar_ofs_neg[ y_n ]
      );
      bar_first[ y_n ] = false;
    }
    if ( series->type == SeriesType::Lollipop ) {
      series->Build(
        lollipop_stem_g, nullptr, above_axes_g, tag_g,
        lol_num, lol_tot
      );
      lol_num++;
    }
    if (
      series->type == SeriesType::XY ||
      series->type == SeriesType::Line ||
      series->type == SeriesType::Scatter ||
      series->type == SeriesType::Point
    ) {
      series->Build(
        above_axes_g, nullptr, above_axes_g, tag_g,
        0, 1
      );
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Main::AddTitle(
  SVG::Group* chart_g,
  std::vector< SVG::Object* >& avoid_objects
)
{
  if ( title.empty() && sub_title.empty() && sub_sub_title.empty() ) return;

  U space_x = 40;
  U space_y = 10;
  BoundaryBox bb;
  std::vector< SVG::Object* > title_objs;

  Group* text_g = chart_g->AddNewGroup();

  U x = chart_w / 2;
  AnchorX a = AnchorX::Mid;
  if ( title_pos == Pos::Left ) {
    x = 0;
    a = AnchorX::Min;
  }
  if ( title_pos == Pos::Right ) {
    x = chart_w;
    a = AnchorX::Max;
  }
  U y = chart_h + space_y;
  if ( !sub_sub_title.empty() ) {
    Object* obj = label_db->Create( text_g, sub_sub_title, 14 );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + 3;
  }
  if ( !sub_title.empty() ) {
    Object* obj = label_db->Create( text_g, sub_title, 20 );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + 3;
  }
  if ( !title.empty() ) {
    Object* obj = label_db->Create( text_g, title, 36 );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y;
  }
  MoveObjs( Dir::Up, title_objs, avoid_objects, space_x, space_y );
  y = 0;
  for ( auto obj : title_objs ) {
    y = std::max( y, obj->GetBB().max.y );
  }
  y = text_g->GetBB().max.y - y;
  if ( y > 0 ) {
    for ( auto obj : title_objs ) {
      obj->Move( 0, y );
    }
  }

  if ( title_inside ) {
    bb = text_g->GetBB();
    U mx = box_spacing;
    U my = box_spacing;
    text_g->Add(
      new Rect(
        bb.min.x - mx, bb.min.y - my,
        bb.max.x + mx, bb.max.y + my,
        box_spacing
      )
    );
    text_g->Last()->Attr()->LineColor()->Set( &axis_color );
    text_g->Last()->Attr()->SetLineWidth( 1 );
    if ( !LegendColor()->IsClear() ) {
      text_g->Last()->Attr()->FillColor()->Set( LegendColor() );
    }
    text_g->FrontToBack();

    if ( title_pos == Pos::Left ) {
      text_g->MoveTo( AnchorX::Min, AnchorY::Max, mx, chart_h - my );
    } else
    if ( title_pos == Pos::Right ) {
      text_g->MoveTo( AnchorX::Max, AnchorY::Max, chart_w - mx, chart_h - my );
    } else
    {
      text_g->MoveTo( AnchorX::Mid, AnchorY::Max, chart_w / 2, chart_h - my );
    }

    avoid_objects.push_back( text_g );
  }

  return;
}

//------------------------------------------------------------------------------

void Main::AddFootnotes(
  SVG::Group* chart_g
)
{
  U dx = 16;
  U dy = 16;

  BoundaryBox bb = chart_g->GetBB();
  if ( footnote_line ) {
    dy = dy / 2;
    chart_g->Add( new Line(
      bb.min.x + dx, bb.min.y - dy, bb.max.x - dx, bb.min.y - dy
    ) );
    chart_g->Last()->Attr()->LineColor()->Set( &text_color );
    chart_g->Last()->Attr()->SetLineWidth( 1 );
  }

  for ( const auto& footnote : footnotes ) {
    if ( footnote.txt.empty() ) continue;

    bb = chart_g->GetBB();
    U x = bb.min.x + dx;
    U y = bb.min.y - dy;
    AnchorX a = AnchorX::Min;
    label_db->Create( chart_g, footnote.txt, 14 );
    if ( footnote.pos == Pos::Center ) {
      x = chart_w / 2;
      a = AnchorX::Mid;
    }
    if ( footnote.pos == Pos::Right ) {
      x = bb.max.x - dx;
      a = AnchorX::Max;
    }
    chart_g->Last()->MoveTo( a, AnchorY::Max, x, y );

    dy = 2;
  }

  return;
}

//------------------------------------------------------------------------------

void Main::AddChartMargin(
  SVG::Group* chart_g
)
{
  BoundaryBox bb = chart_g->GetBB();

  U delta = 0;
  for ( auto series : series_list ) {
    if (
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar
    ) {
      delta = std::max( +delta, series->line_width / 2 );
    }
    if ( series->marker_show ) {
      delta = std::max( +delta, -series->marker_out.x1 );
      delta = std::max( +delta, -series->marker_out.y1 );
      delta = std::max( +delta, +series->marker_out.x2 );
      delta = std::max( +delta, +series->marker_out.y2 );
    }
  }

  bb.min.x = std::min( +bb.min.x, -delta );
  bb.max.x = std::max( +bb.max.x, chart_w + delta );
  bb.min.y = std::min( +bb.min.y, -delta );
  bb.max.y = std::max( +bb.max.y, chart_h + delta );

  bb.min.x -= margin + frame_width;
  bb.max.x += margin + frame_width;
  bb.min.y -= margin + frame_width;
  bb.max.y += margin + frame_width;

  chart_g->Add( new Rect( bb.min, bb.max ) );
  chart_g->Last()->Attr()->FillColor()->Clear();
  chart_g->Last()->Attr()->LineColor()->Clear();
  chart_g->Last()->Attr()->SetLineWidth( 0 );

  bb.min.x += frame_width / 2;
  bb.max.x -= frame_width / 2;
  bb.min.y += frame_width / 2;
  bb.max.y -= frame_width / 2;

  chart_g->Add( new Rect( bb.min, bb.max ) );
  chart_g->Last()->Attr()->SetLineWidth( frame_width );
  if ( frame_width > 0 ) {
    chart_g->Last()->Attr()->LineColor()->Set( &frame_color );
  } else {
    chart_g->Last()->Attr()->LineColor()->Clear();
  }
  chart_g->FrontToBack();
}

//------------------------------------------------------------------------------

Canvas* Main::Build( void )
{
  Canvas* canvas = new Canvas();

  Group* chart_g = canvas->TopGroup()->AddNewGroup();
  chart_g->Attr()->TextFont()->SetFamily(
    "DejaVu Sans Mono,Consolas,Menlo,Courier New"
  );
  chart_g->Attr()->TextFont()
    ->SetWidthFactor( width_adj )
    ->SetHeightFactor( height_adj )
    ->SetBaselineFactor( baseline_adj );
  chart_g->Attr()->FillColor()->Set( &background_color );
  chart_g->Attr()->TextColor()->Set( &text_color );
  chart_g->Attr()->LineColor()->Clear();
  chart_g->Add( new Rect( 0, 0, chart_w, chart_h ) );
  if ( !chart_area_color.IsClear() ) {
    chart_g->Last()->Attr()->FillColor()->Set( &chart_area_color );
  }

  Group* grid_minor_g          = chart_g->AddNewGroup();
  Group* grid_major_g          = chart_g->AddNewGroup();
  Group* grid_zero_g           = chart_g->AddNewGroup();
  Group* label_bg_g            = chart_g->AddNewGroup();
  Group* chartbox_below_axes_g = chart_g->AddNewGroup();
  Group* axes_line_g           = chart_g->AddNewGroup();
  Group* chartbox_above_axes_g = chart_g->AddNewGroup();
  Group* axes_num_g            = chart_g->AddNewGroup();
  Group* axes_label_g          = chart_g->AddNewGroup();
  Group* tag_g                 = chart_g->AddNewGroup();
  Group* legend_g              = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( &axis_color );
  axes_line_g->Attr()->SetLineCap( LineCap::Square );
  axes_line_g->Attr()->FillColor()->Clear();

  chartbox_below_axes_g->Attr()->FillColor()->Clear();
  chartbox_above_axes_g->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->TextFont()->SetSize( 14 );
  axes_num_g->Attr()->LineColor()->Clear();

  // This group only has numbers so optimize baseline to ensure vertical
  // centering within boundary box.
  tag_g->Attr()->TextFont()
    ->SetWidthFactor( 1.0 )
    ->SetHeightFactor( 0.80 )
    ->SetBaselineFactor( 0.30 );

  legend_g->Attr()->TextFont()->SetSize( 14 );

  axis_x->length      = (axis_x->angle == 0) ? chart_w : chart_h;
  axis_x->orth_length = (axis_x->angle == 0) ? chart_h : chart_w;
  axis_x->chart_box   = chart_box;
  for ( auto a : axis_y ) {
    a->length      = (a->angle == 0) ? chart_w : chart_h;
    a->orth_length = (a->angle == 0) ? chart_h : chart_w;
    a->chart_box   = chart_box;
  }

  std::vector< LegendBox > lb_list;

  SeriesPrepare( &lb_list );
  AxisPrepare( tag_g );

  std::vector< SVG::Object* > avoid_objects;

  for ( uint32_t phase : {0, 1} ) {
    axis_x->Build(
      category_list,
      phase,
      avoid_objects,
      grid_minor_g, grid_major_g, grid_zero_g,
      axes_line_g, axes_num_g, axes_label_g
    );
    for ( int i : { 1, 0 } ) {
      std::vector< std::string > empty;
      axis_y[ i ]->Build(
        empty,
        phase,
        avoid_objects,
        grid_minor_g, grid_major_g, grid_zero_g,
        axes_line_g, axes_num_g, axes_label_g
      );
    }
  }

  if ( chart_box ) {
    axes_line_g->Add( new Rect( 0, 0, chart_w, chart_h ) );
  }

  axis_x->BuildLabel( avoid_objects, axes_label_g );
  for ( auto a : axis_y ) {
    a->BuildLabel( avoid_objects, axes_label_g );
  }

  if ( title_inside ) {
    AddTitle( chart_g, avoid_objects );
  }

  CalcLegendBoxes( legend_g, lb_list, avoid_objects );

  BuildSeries( chartbox_below_axes_g, chartbox_above_axes_g, tag_g );

  PlaceLegends( avoid_objects, lb_list, legend_g );

  if ( !title_inside ) {
    AddTitle( chart_g, avoid_objects );
  }

/*
  {
    for ( auto obj : avoid_objects ) {
      if ( obj->Empty() ) continue;
      BoundaryBox bb = obj->GetBB();
      bb.min.x -= 0.1;
      bb.min.y -= 0.1;
      bb.max.x += 0.1;
      bb.max.y += 0.1;
      chart_g->Add( new Rect( bb.min, bb.max ) );
      chart_g->Last()->Attr()->FillColor()->Clear();
      chart_g->Last()->Attr()->SetLineWidth( 4 );
      chart_g->Last()->Attr()->LineColor()->Set( ColorName::blue );
      chart_g->Last()->Attr()->LineColor()->SetTransparency( 0.5 );
    }
  }
*/

  AddFootnotes( chart_g );

  // Add background for text objects in the Label data base.
  {
    bool partial_ok = true;
    if ( chart_area_color.IsClear() ) {
      label_bg_g->Attr()->FillColor()->Set( &background_color );
    } else {
      label_bg_g->Attr()->FillColor()->Set( &chart_area_color );
      partial_ok = false;
    }
    BoundaryBox area;
    area.min.x = 0; area.max.x = chart_w;
    area.min.y = 0; area.max.y = chart_h;
    label_db->AddBackground( label_bg_g, area, partial_ok );
  }

  AddChartMargin( chart_g );

  return canvas;
}

///////////////////////////////////////////////////////////////////////////////
