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
  axis_x.SetUnitPos( Auto );
  axis_y.SetUnitPos( Auto );
  SetLegendPos( Auto );
}

Main::~Main( void )
{
  for ( auto series : series_list ) {
    delete series;
  }
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

void Main::SetTitle( std::string txt )
{
  title = txt;
}

void Main::SetSubTitle( std::string txt )
{
  sub_title = txt;
}

void Main::SetSubSubTitle( std::string txt )
{
  sub_sub_title = txt;
}

void Main::SetFootnote( std::string txt )
{
  footnote = txt;
}

void Main::SetLegendPos( Pos pos )
{
  legend_pos = pos;
}

Series* Main::AddSeries( std::string name )
{
  Series* series = new Series( name );
  series->SetStyle( series_list.size() );
  series_list.push_back( series );
  return series;
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

void Main::CalcLegendSize( Group* g, U& ch, U& tw, U& th )
{
  uint32_t max_chars = 1;
  uint32_t max_lines = 1;
  for ( Series* series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    uint32_t cur_chars = 0;
    uint32_t cur_lines = 1;
    for ( char c : series->name ) {
      if ( c == '\n' ) {
        cur_lines++;
        if ( max_chars < cur_chars ) max_chars = cur_chars;
        cur_chars = 0;
      } else {
        cur_chars++;
      }
    }
    if ( max_chars < cur_chars ) max_chars = cur_chars;
    if ( max_lines < cur_lines ) max_lines = cur_lines;
  }
  std::string s;
  while ( s.length() < max_chars ) s += '-';
  g->Add( new Text( 0, 0, s ) );
  BoundaryBox bb = g->Last()->GetBB();
  g->DeleteFront();
  ch = bb.max.y - bb.min.y;
  tw = bb.max.x - bb.min.x;
  th = ch * max_lines;
}

//-----------------------------------------------------------------------------

// Determine potential placement of series legends in chart interior.
void Main::CalcLegendBoxes(
  Group* g, std::vector< LegendBox >& lb_list,
  const std::vector< SVG::Object* >& axes_objects
)
{
  U ch;
  U tw;
  U th;
  CalcLegendSize( g, ch, tw, th );
  uint32_t lc = LegendCnt();

  auto add_lbs = [&]( AnchorX anchor_x, AnchorY anchor_y, U dx = 0, U dy = 0 )
  {
    uint32_t nx = (anchor_x == MidX) ? lc :  1;
    uint32_t ny = (anchor_x == MidX) ?  1 : lc;
    while ( nx > 0 && ny > 0 ) {
      BoundaryBox bb;
      g->Add(
        new Rect(
          0, 0,
          nx * (tw + 2*legend_bx) + (nx - 1) * legend_sx,
          ny * (th + 2*legend_by) + (ny - 1) * legend_sy
        )
      );
      U x = legend_sx + dx;
      U y = legend_sy + dy;
      if ( anchor_x == MidX ) x = chart_w / 2;
      if ( anchor_x == MaxX ) x = chart_w - legend_sx - dx;
      if ( anchor_y == MidY ) y = chart_h / 2;
      if ( anchor_y == MaxY ) y = chart_h - legend_sy - dy;
      g->Last()->MoveTo( anchor_x, anchor_y, x, y );
      if ( !Collides( g->Last(), axes_objects, legend_sx, legend_sy, bb ) ) {
        LegendBox lb;
        lb.bb = g->Last()->GetBB();
        if (
          lb.bb.min.x > 0 && lb.bb.max.x < chart_w &&
          lb.bb.min.y > 0 && lb.bb.max.y < chart_h
        ) {
          lb.bb.min.x -= legend_sx; lb.bb.max.x += legend_sx;
          lb.bb.min.y -= legend_sy; lb.bb.max.y += legend_sy;
          lb.nx = nx;
          lb.sx = nx * ny - lc;
          lb_list.push_back( lb );
        }
      }
      g->DeleteFront();
      if ( anchor_x == MidX ) {
        if ( ny == lc ) break;
        ny++;
        nx = (lc + ny - 1) / ny;
      } else {
        if ( nx == lc ) break;
        nx++;
        ny = (lc + nx - 1) / nx;
      }
    }
  };

  for ( int i = 0; i < 4; i++ ) {
    U dx = 0;
    U dy = 0;
    if ( (i >> 0) & 1 ) dx = axis_x.tick_major_len;
    if ( (i >> 1) & 1 ) dy = axis_y.tick_major_len;
    add_lbs( MaxX, MaxY, dx, dy );
    add_lbs( MaxX, MinY, dx, dy );
    add_lbs( MinX, MaxY, dx, dy );
    add_lbs( MinX, MinY, dx, dy );
    add_lbs( MidX, MinY, dx, dy );
    add_lbs( MidX, MaxY, dx, dy );
  }
  add_lbs( MaxX, MidY );
  add_lbs( MinX, MidY );
}

//-----------------------------------------------------------------------------

void Main::BuildLegend( Group* g, int nx )
{
  g->Attr()->SetTextAnchor( MinX, MaxY );
  U ch;
  U tw;
  U th;
  CalcLegendSize( g, ch, tw, th );
  int n = 0;
  for ( Series* series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    U px = (n % nx) * +(tw + 2*legend_bx + legend_sx);
    U py = (n / nx) * -(th + 2*legend_by + legend_sy);
    U d = series->width / 2 + 2;
    g->Add(
      new Rect(
        px - d, py + d,
        px + tw + 2*legend_bx + d, py - th - 2*legend_by - d,
        ch/2 + d
      )
    );
    g->Add(
      new Rect( px, py, px + tw + 2*legend_bx, py - th - 2*legend_by, ch/2 )
    );
    series->ApplyStyle( g->Last() );
    px += legend_bx;
    py -= legend_by;
    std::string s;
    for ( char c : series->name ) {
      if ( c == '\n' ) {
        g->Add( new Text( px, py, s ) );
        py -= ch;
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

///////////////////////////////////////////////////////////////////////////////

void Main::AutoRange( void )
{
  double min_x = axis_x.log_scale ? 10 : 0;
  double max_x = axis_x.log_scale ? 10 : 0;
  double min_y = axis_y.log_scale ? 10 : 0;
  double max_y = axis_y.log_scale ? 10 : 0;
  bool first = true;
  for ( Series* series : series_list ) {
    for ( auto& datum : series->datum_list ) {
      if ( !axis_x.Valid( datum.x ) ) continue;
      if ( !axis_y.Valid( datum.y ) ) continue;
      if ( first || min_x > datum.x ) min_x = datum.x;
      if ( first || max_x < datum.x ) max_x = datum.x;
      if ( first || min_y > datum.y ) min_y = datum.y;
      if ( first || max_y < datum.y ) max_y = datum.y;
      first = false;
    }
  }

  axis_x.LegalizeMinMax( min_x, max_x );
  axis_y.LegalizeMinMax( min_y, max_y );

  return;
}

///////////////////////////////////////////////////////////////////////////////

Canvas* Main::Build( void )
{
  axis_x.length = chart_w;
  axis_y.length = chart_h;

  AutoRange();

  Canvas* canvas = new Canvas();

  Group* chart_g = canvas->TopGroup()->AddNewGroup();
  chart_g->Attr()->TextFont()->SetFamily( "DejaVu Sans Mono,Consolas,Menlo,Courier New" );
  chart_g->Attr()->FillColor()->Set( White );
  chart_g->Attr()->LineColor()->Clear();

  Group* grid_minor_g = chart_g->AddNewGroup();
  Group* grid_major_g = chart_g->AddNewGroup();
  Group* grid_zero_g  = chart_g->AddNewGroup();

  grid_minor_g->Attr()
    ->SetLineWidth( 0.5 )
    ->SetLineDash( 1, 3 )
    ->LineColor()->Set( Black );
  grid_major_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 4, 3 )
    ->LineColor()->Set( Black );
  grid_zero_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 8, 6 )
    ->LineColor()->Set( Black );

  Group* axes_line_g  = chart_g->AddNewGroup();
  Group* chartbox_g   = chart_g->AddNewGroup();
  Group* axes_num_g   = chart_g->AddNewGroup();
  Group* axes_label_g = chart_g->AddNewGroup();
  Group* legend_g     = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( Black );

  chartbox_g->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->TextFont()->SetSize( 14 );
  axes_num_g->Attr()->LineColor()->Clear();

  legend_g->Attr()->TextFont()->SetSize( 14 );

  std::vector< SVG::Object* > axes_objects;

  for ( uint32_t phase : {0, 1} ) {
    axis_x.Build(
      phase,
      axis_y, axes_objects,
      grid_minor_g, grid_major_g, grid_zero_g,
      axes_line_g, axes_num_g, axes_label_g
    );
    axis_y.Build(
      phase,
      axis_x, axes_objects,
      grid_minor_g, grid_major_g, grid_zero_g,
      axes_line_g, axes_num_g, axes_label_g
    );
  }

  // Do title.
  {
    U space_x = 50;
    U space_y = 10;
    U by = chart_h + space_y;
    BoundaryBox bb;
    Object* main = nullptr;
    Object* sub1 = nullptr;
    Object* sub2 = nullptr;
    if ( sub_sub_title != "" ) {
      sub2 = MultiLineText( chart_g, sub_sub_title, 14 );
    }
    if ( sub_title != "" ) {
      sub1 = MultiLineText( chart_g, sub_title, 20 );
    }
    if ( title != "" ) {
      main = MultiLineText( chart_g, title, 36 );
    }
    bool done = false;
    while ( true ) {
      U y = by;
      if ( sub2 != nullptr ) {
        sub2->MoveTo( MidX, MinY, chart_w/2, y );
        y = sub2->GetBB().max.y + 3;
      }
      if ( sub1 != nullptr ) {
        sub1->MoveTo( MidX, MinY, chart_w/2, y );
        y = sub1->GetBB().max.y + 3;
      }
      if ( main != nullptr ) {
        main->MoveTo( MidX, MinY, chart_w/2, y );
        y = main->GetBB().max.y;
      }
      if ( done ) break;
      U ny = by;
      if ( Chart::Collides( sub2, axes_objects, space_x, space_y, bb ) ) {
        ny += bb.max.y + space_y - sub2->GetBB().min.y;
      } else
      if ( Chart::Collides( sub1, axes_objects, space_x, space_y, bb ) ) {
        ny += bb.max.y + space_y - sub1->GetBB().min.y;
      } else
      if ( Chart::Collides( main, axes_objects, space_x, space_y, bb ) ) {
        ny += bb.max.y + space_y - main->GetBB().min.y;
      }
      if ( ny > by ) {
        by = ny;
        continue;
      }
      bb = chart_g->GetBB();
      if ( y < bb.max.y ) {
        by += bb.max.y - y;
      }
      done = true;
    }
  }

  std::vector< LegendBox > lb_list;
  CalcLegendBoxes( legend_g, lb_list, axes_objects );

  for ( Series* series : series_list ) {
    Group* series_g = chartbox_g->AddNewGroup();
    series->Build( series_g, axis_x, axis_y, lb_list );
  }

  // Find best legend placement.
  if ( LegendCnt() > 0 ) {
    Pos legend_pos = this->legend_pos;
    if ( legend_pos == Auto ) {
      LegendBox best_lb;
      bool best_lb_defined = false;
      for ( LegendBox& lb : lb_list ) {
        if ( !best_lb_defined ) {
          best_lb = lb;
          best_lb_defined = true;
        }
        if (
          lb.collisions < best_lb.collisions ||
          (lb.collisions == best_lb.collisions && lb.sx < best_lb.sx)
        ) {
          best_lb = lb;
        }
      }
      if ( best_lb_defined ) {
        BuildLegend( legend_g->AddNewGroup(), best_lb.nx );
        legend_g->Last()->MoveTo(
          MidX, MidY,
          (best_lb.bb.min.x + best_lb.bb.max.x) / 2,
          (best_lb.bb.min.y + best_lb.bb.max.y) / 2
        );
      } else {
        legend_pos = Bottom;
      }
    }
    if ( legend_pos != Auto ) {
      U ch;
      U tw;
      U th;
      CalcLegendSize( legend_g, ch, tw, th );
      BoundaryBox bb = chart_g->GetBB();
      if ( legend_pos == Left || legend_pos == Right ) {
        U avail_h = chart_h;
        uint32_t nx = 1;
        while ( 1 ) {
          uint32_t ny = (LegendCnt() + nx - 1) / nx;
          U need_h = ny * (th + 2*legend_by) + (ny + 1) * legend_sy;
          if ( need_h > avail_h && ny > 1 ) {
            nx++;
            continue;
          }
          break;
        }
        BuildLegend( legend_g->AddNewGroup(), nx );
        U y = chart_h / 2;
        if ( legend_pos == Left ) {
          U x = 0 - legend_sx;
          while ( true ) {
            legend_g->Last()->MoveTo( MaxX, MidY, x, y );
            Collides(
              legend_g->Last(), axes_objects, legend_sx, legend_sy, bb
            );
            if ( bb.min.x - legend_sx < x ) {
              x = bb.min.x - legend_sx;
            } else {
              break;
            }
          }
        } else {
          U x = chart_w + legend_sx;
          while ( true ) {
            legend_g->Last()->MoveTo( MinX, MidY, x, y );
            Collides(
              legend_g->Last(), axes_objects, legend_sx, legend_sy, bb
            );
            if ( bb.max.x + legend_sx > x ) {
              x = bb.max.x + legend_sx;
            } else {
              break;
            }
          }
        }
      } else {
        U avail_w = chart_w;
        uint32_t nx = LegendCnt();
        uint32_t ny = 1;
        while ( 1 ) {
          nx = (LegendCnt() + ny - 1) / ny;
          U need_w = nx * (tw + 2*legend_bx) + (nx - 1) * legend_sx;
          if ( need_w > avail_w && nx > 1 ) {
            ny++;
            continue;
          }
          break;
        }
        BuildLegend( legend_g->AddNewGroup(), nx );
        U x = chart_w / 2;
        legend_g->Last()->MoveTo( MidX, MaxY, x, bb.min.y - legend_sy );
      }
    }
  }

  if ( footnote != "" ) {
    BoundaryBox bb = chart_g->GetBB();
    U x = bb.min.x + 15;
    U y = bb.min.y - 15;
    MultiLineText( chart_g, footnote, 14 );
    chart_g->Last()->MoveTo( MinX, MaxY, x, y );
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
