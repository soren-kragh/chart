//
//  Copyright (C) 2023, S. Kragh
//
//  This file is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License for more details.
//  <http://www.gnu.org/licenses/>.
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
  chart_w = width;
  chart_h = height;
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
  double min_x = 0;
  double max_x = 0;
  double min_y = 0;
  double max_y = 0;
  bool first = true;
  for ( Series* series : series_list ) {
    for ( auto& datum : series->datum_list ) {
      if ( first || min_x > datum.x ) min_x = datum.x;
      if ( first || max_x < datum.x ) max_x = datum.x;
      if ( first || min_y > datum.y ) min_y = datum.y;
      if ( first || max_y < datum.y ) max_y = datum.y;
      first = false;
    }
  }
  if ( first || min_x == max_x ) {
    min_x -= 1;
    max_x += 1;
  }
  if ( first || min_y == max_y ) {
    min_y -= 1;
    max_y += 1;
  }
  bool auto_x = false;
  bool auto_y = false;
  if ( axis_x.min >= axis_x.max ) {
    auto_x = true;
    axis_x.min = min_x;
    axis_x.max = max_x;
  }
  if ( axis_y.min >= axis_y.max ) {
    auto_y = true;
    axis_y.min = min_y;
    axis_y.max = max_y;
    if ( min_y > 0 && min_y / (max_y - min_y) < 0.35 ) {
      axis_y.min = 0;
    }
    if ( max_y < 0 && max_y / (min_y - max_y) < 0.35 ) {
      axis_y.max = 0;
    }
  }
  axis_x.AutoTick();
  axis_y.AutoTick();
  if ( auto_x ) {
    if ( axis_x.major > 0 ) {
      if ( axis_x.min != 0 ) {
        axis_x.min = std::floor(axis_x.min / axis_x.major) * axis_x.major;
      }
      if ( axis_x.max != 0 ) {
        axis_x.max = std::ceil(axis_x.max / axis_x.major) * axis_x.major;
      }
    }
    axis_x.orth_axis_cross = axis_x.min;
  }
  if ( auto_y ) {
    if ( axis_y.major > 0 ) {
      if ( axis_y.min != 0 ) {
        axis_y.min = std::floor(axis_y.min / axis_y.major - 0.4) * axis_y.major;
      }
      if ( axis_y.max != 0 ) {
        axis_y.max = std::ceil(axis_y.max / axis_y.major + 0.4) * axis_y.major;
      }
    }
    if ( axis_y.max < 0 ) {
      axis_y.orth_axis_cross = axis_y.max;
    } else {
      axis_y.orth_axis_cross = axis_y.min;
    }
    if ( axis_y.min*axis_y.max < 0 ) {
      axis_y.orth_axis_cross = 0;
    }
  }
  if ( axis_x.orth_axis_cross < axis_x.min ) axis_x.orth_axis_cross = axis_x.min;
  if ( axis_x.orth_axis_cross > axis_x.max ) axis_x.orth_axis_cross = axis_x.max;
  if ( axis_y.orth_axis_cross < axis_y.min ) axis_y.orth_axis_cross = axis_y.min;
  if ( axis_y.orth_axis_cross > axis_y.max ) axis_y.orth_axis_cross = axis_y.max;
}

///////////////////////////////////////////////////////////////////////////////

Canvas* Main::Build( void )
{
  AutoRange();

  Canvas* canvas = new Canvas();

  Group* chart_g = canvas->TopGroup()->AddNewGroup();
  chart_g->Attr()->TextFont()->SetFamily( "DejaVu Sans Mono,Courier New" );
  chart_g->Attr()->FillColor()->Set( White );
  chart_g->Attr()->LineColor()->Clear();

  Group* grid_minor_g = chart_g->AddNewGroup();
  Group* grid_major_g = chart_g->AddNewGroup();
  Group* grid_zero_g  = chart_g->AddNewGroup();

  grid_minor_g->Attr()
    ->SetLineWidth( 0.5 )
    ->SetLineDash( 2, 3 )
    ->LineColor()->Set( Black, 0.7 );
  grid_major_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 4 )
    ->LineColor()->Set( Black, 0.6 );
  grid_zero_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 4 )
    ->LineColor()->Set( Blue, 0.5 );

  Group* axes_line_g  = chart_g->AddNewGroup();
  Group* chartbox_g   = chart_g->AddNewGroup();
  Group* axes_num_g   = chart_g->AddNewGroup();
  Group* axes_label_g = chart_g->AddNewGroup();
  Group* legend_g     = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( Black );

  chartbox_g->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->TextFont()->SetSize( 12 );
  axes_num_g->Attr()->TextFont()->SetBold();
  axes_num_g->Attr()->LineColor()->Clear();

  legend_g->Attr()->TextFont()->SetSize( 14 );

  std::vector< SVG::Object* > axes_objects;

  axis_x.length = chart_w;
  axis_y.length = chart_h;

  axis_x.Build(
    0, axis_y, axes_objects,
    grid_minor_g, grid_major_g, grid_zero_g,
    axes_line_g, axes_num_g, axes_label_g
  );
  axis_y.Build(
    90, axis_x, axes_objects,
    grid_minor_g, grid_major_g, grid_zero_g,
    axes_line_g, axes_num_g, axes_label_g
  );

  // Do title.
  {
    U space_x = 50;
    U space_y = 5;
    U by = chart_h + space_y;
    BoundaryBox bb;
    Object* main = NULL;
    Object* sub1 = NULL;
    Object* sub2 = NULL;
    if ( sub_sub_title != "" ) {
      sub2 = Label( chart_g, sub_sub_title, 14 );
    }
    if ( sub_title != "" ) {
      sub1 = Label( chart_g, sub_title, 20 );
    }
    if ( title != "" ) {
      main = Label( chart_g, title, 36 );
    }
    bool done = false;
    while ( true ) {
      U y = by;
      if ( sub2 != NULL ) {
        sub2->MoveTo( MidX, MinY, chart_w/2, y );
        y = sub2->GetBB().max.y;
      }
      if ( sub1 != NULL ) {
        sub1->MoveTo( MidX, MinY, chart_w/2, y );
        y = sub1->GetBB().max.y;
      }
      if ( main != NULL ) {
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
    U x = 0;
    U y = chart_g->GetBB().min.y - 5;
    Label( chart_g, footnote, 12 );
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
  chart_g->LastToBack();

  return canvas;
}

///////////////////////////////////////////////////////////////////////////////