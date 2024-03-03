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
  axis_x = new Axis( 0 );
  axis_y[ 0 ] = new Axis( 90 );
  axis_y[ 1 ] = new Axis( 90 );
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

void Main::SetBW( bool bw )
{
  this->bw = bw;
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
  int style = series_list.size() % 64;
  if ( bw ) style = (style % 8) + 64;
  series->SetStyle( style );
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
  g->Add( new Text( s ) );
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
  const std::vector< SVG::Object* >& axis_objects
)
{
  U ch;
  U tw;
  U th;
  CalcLegendSize( g, ch, tw, th );
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
          nx * (tw + 2*legend_bx) + (nx - 1) * legend_sx,
          ny * (th + 2*legend_by) + (ny - 1) * legend_sy
        )
      );
      Object* obj = g->Last();
      U x = legend_sx;
      U y = legend_sy;
      if ( anchor_x == AnchorX::Mid ) x = chart_w / 2;
      if ( anchor_x == AnchorX::Max ) x = chart_w - legend_sx;
      if ( anchor_y == AnchorY::Mid ) y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h - legend_sy;
      obj->MoveTo( anchor_x, anchor_y, x, y );

      if ( can_move_y && anchor_y != AnchorY::Mid ) {
        while ( true ) {
          BoundaryBox obj_bb = obj->GetBB();
          U dy = 0;
          for ( auto ao : axis_objects ) {
            if (
              !SVG::Collides(
                obj, ao, legend_sx - epsilon, legend_sy - epsilon
              )
            ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            if ( anchor_y == AnchorY::Min ) {
              if ( ao_bb.max.y < (chart_h * 1 / 4) ) {
                dy = ao_bb.max.y - obj_bb.min.y + legend_sy;
                break;
              }
            } else {
              if ( ao_bb.min.y > (chart_h * 3 / 4) ) {
                dy = ao_bb.min.y - obj_bb.max.y - legend_sy;
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
            if (
              SVG::Collides(
                obj, ao, legend_sx - epsilon, legend_sy - epsilon
              )
            ) {
              BoundaryBox ao_bb = ao->GetBB();
              if ( anchor_x == AnchorX::Min ) {
                if ( ao_bb.max.x < (chart_w * 1 / 4) ) {
                  dx = ao_bb.max.x - obj_bb.min.x + legend_sx;
                  break;
                }
              } else {
                if ( ao_bb.min.x > (chart_w * 3 / 4) ) {
                  dx = ao_bb.min.x - obj_bb.max.x - legend_sx;
                  break;
                }
              }
            }
          }
          if ( dx == 0 ) break;
          obj->Move( dx, 0 );
        }
      }

      if ( !Collides( obj, axis_objects, legend_sx, legend_sy ) ) {
        LegendBox lb;
        lb.bb = obj->GetBB();
        if (
          ( anchor_x != AnchorX::Mid ||
            (lb.bb.max.x - lb.bb.min.x) > (lb.bb.max.y - lb.bb.min.y)
          ) &&
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
      if ( anchor_x == AnchorX::Mid ) {
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

  bool dual_y = axis_y[ 0 ]->show && axis_y[ 1 ]->show;

  for ( bool can_move : { false, true } ) {
    if ( dual_y ) {
      add_lbs( AnchorX::Mid, AnchorY::Max, can_move, can_move );
      add_lbs( AnchorX::Mid, AnchorY::Min, can_move, can_move );
    }
    add_lbs( AnchorX::Max, AnchorY::Max, can_move, can_move );
    add_lbs( AnchorX::Max, AnchorY::Min, can_move, can_move );
    add_lbs( AnchorX::Min, AnchorY::Max, can_move, can_move );
    add_lbs( AnchorX::Min, AnchorY::Min, can_move, can_move );
    if ( !dual_y ) {
      add_lbs( AnchorX::Mid, AnchorY::Max, can_move, can_move );
      add_lbs( AnchorX::Mid, AnchorY::Min, can_move, can_move );
    }
    if ( can_move ) {
      add_lbs( AnchorX::Mid, AnchorY::Mid, false, false );
      if ( !dual_y ) {
        add_lbs( AnchorX::Max, AnchorY::Mid, true, false );
        add_lbs( AnchorX::Min, AnchorY::Mid, true, false );
      }
    }
  }

/*
  for ( auto lb : lb_list ) {
    g->Add( new Rect( lb.bb.min, lb.bb.max ) );
    g->Last()->Attr()->SetLineWidth( 0.5 );
    g->Last()->Attr()->FillColor()->Clear();
    g->Last()->Attr()->LineColor()->Set( ColorName::Black );
  }
*/
}

//-----------------------------------------------------------------------------

void Main::BuildLegend( Group* g, int nx )
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
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

//-----------------------------------------------------------------------------

void Main::PlaceLegend(
  const std::vector< SVG::Object* >& axis_objects,
  const std::vector< LegendBox >& lb_list,
  Group* legend_g
)
{
  if ( LegendCnt() == 0 ) return;

  Pos legend_pos = this->legend_pos;

  if ( legend_pos == Pos::Auto ) {
    LegendBox best_lb;
    bool best_lb_defined = false;
    for ( const LegendBox& lb : lb_list ) {
      if ( !best_lb_defined ) {
        best_lb = lb;
        best_lb_defined = true;
      }
      if (
        lb.collision_weight < best_lb.collision_weight ||
        (lb.collision_weight == best_lb.collision_weight && lb.sx < best_lb.sx)
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

  U ch;
  U tw;
  U th;
  CalcLegendSize( legend_g, ch, tw, th );

  if ( legend_pos == Pos::Left || legend_pos == Pos::Right ) {

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
    Object* legend = legend_g->Last();

    U x = 0 - legend_sx;
    Dir dir = Dir::Left;
    AnchorX anchor_x = AnchorX::Max;
    if ( legend_pos == Pos::Right ) {
      x = chart_w + legend_sx;
      dir = Dir::Right;
      anchor_x = AnchorX::Min;
    }
    AnchorY best_anchor_y{ AnchorY::Mid };
    U best_x{ 0 };
    U best_y{ 0 };
    for ( auto anchor_y : { AnchorY::Max, AnchorY::Mid, AnchorY::Min } ) {
      U y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h - legend_sy;
      if ( anchor_y == AnchorY::Min ) y = legend_sy;
      legend->MoveTo( anchor_x, anchor_y, x, y );
      MoveObj( dir, legend, axis_objects, legend_sx, legend_sy );
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
    MoveObj( dir, legend, axis_objects, legend_sx, legend_sy );

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
    Object* legend = legend_g->Last();

    U x = chart_w / 2;
    U y = 0 - legend_sy;
    legend->MoveTo( AnchorX::Mid, AnchorY::Max, x, y );
    MoveObj( Dir::Down, legend, axis_objects, legend_sx, legend_sy );

  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Main::AxisPrepare( void )
{
  axis_x->data_def = false;
  axis_x->data_min = axis_x->log_scale ? 10 : 0;
  axis_x->data_max = axis_x->log_scale ? 10 : 0;
  for ( auto a : axis_y ) {
    a->data_def = false;
    a->data_min = a->log_scale ? 10 : 0;
    a->data_max = a->log_scale ? 10 : 0;
  }
  for ( Series* series : series_list ) {
    Axis* ax = axis_x;
    Axis* ay = axis_y[ series->axis_y_n ];
    for ( auto& datum : series->datum_list ) {
      if ( !ax->Valid( datum.x ) ) continue;
      if ( !ay->Valid( datum.y ) ) continue;
      if ( !ax->data_def || ax->data_min > datum.x ) ax->data_min = datum.x;
      if ( !ax->data_def || ax->data_max < datum.x ) ax->data_max = datum.x;
      if ( !ay->data_def || ay->data_min > datum.y ) ay->data_min = datum.y;
      if ( !ay->data_def || ay->data_max < datum.y ) ay->data_max = datum.y;
      ax->data_def = true;
      ay->data_def = true;
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

  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
    axis_x->orth_style[ i ] = axis_y[ i ]->style;
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

  for ( auto a : axis_y ) {
    a->orth_length_ext[ 0 ] =
    a->orth_length_ext[ 1 ] =
      axis_x->length +
      ((axis_x->style == AxisStyle::Arrow) ? +axis_x->overhang : 0);
  }
  for ( int i : { 0, 1 } ) {
    axis_x->orth_length_ext[ i ] =
      axis_y[ i ]->length +
      ((axis_y[ i ]->style == AxisStyle::Arrow) ? +axis_y[ i ]->overhang : 0);
  }

  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
    axis_x->orth_style[ i ] = axis_y[ i ]->style;
  }

  axis_x->at_orth_min =
    axis_y[ 0 ]->CoorNear(
      axis_y[ 0 ]->orth_axis_coor[ 0 ], 0
    );
  axis_x->at_orth_max =
    axis_y[ 0 ]->CoorNear(
      axis_y[ 0 ]->orth_axis_coor[ 0 ], axis_y[ 0 ]->length
    );
  axis_x->at_orth_coor = axis_y[ 0 ]->orth_axis_coor[ 0 ];
  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->at_orth_min =
      axis_x->CoorNear( axis_x->orth_axis_coor[ i ], 0 );
    axis_y[ i ]->at_orth_max =
      axis_x->CoorNear( axis_x->orth_axis_coor[ i ], axis_x->length );
    axis_y[ i ]->at_orth_coor = axis_x->orth_axis_coor[ i ];
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
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

Canvas* Main::Build( void )
{
  Canvas* canvas = new Canvas();

  Group* chart_g = canvas->TopGroup()->AddNewGroup();
  chart_g->Attr()->TextFont()->SetFamily(
    "DejaVu Sans Mono,Consolas,Menlo,Courier New"
  );
  chart_g->Attr()->FillColor()->Set( ColorName::White );
  chart_g->Attr()->LineColor()->Clear();
  chart_g->Add( new Rect( 0, 0, chart_w, chart_h ) );

  Group* grid_minor_g = chart_g->AddNewGroup();
  Group* grid_major_g = chart_g->AddNewGroup();
  Group* grid_zero_g  = chart_g->AddNewGroup();

  grid_minor_g->Attr()
    ->SetLineWidth( 0.5 )
    ->SetLineDash( 1, 3 )
    ->LineColor()->Set( ColorName::Black );
  grid_major_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 4, 3 )
    ->LineColor()->Set( ColorName::Black );
  grid_zero_g->Attr()
    ->SetLineWidth( 1.0 )
    ->SetLineDash( 8, 6 )
    ->LineColor()->Set( ColorName::Black );

  Group* axes_line_g  = chart_g->AddNewGroup();
  Group* chartbox_g   = chart_g->AddNewGroup();
  Group* axes_num_g   = chart_g->AddNewGroup();
  Group* axes_label_g = chart_g->AddNewGroup();
  Group* legend_g     = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( ColorName::Black );

  chartbox_g->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->TextFont()->SetSize( 14 );
  axes_num_g->Attr()->LineColor()->Clear();

  legend_g->Attr()->TextFont()->SetSize( 14 );

  std::vector< SVG::Object* > axis_objects;

  axis_x->length = chart_w;
  axis_x->orth_length = chart_h;
  for ( auto a : axis_y ) {
    a->length = chart_h;
    a->orth_length = chart_w;
  }

  AxisPrepare();

  for ( uint32_t phase : {0, 1} ) {
    axis_x->Build(
      phase,
      axis_objects,
      grid_minor_g, grid_major_g, grid_zero_g,
      axes_line_g, axes_num_g, axes_label_g
    );
    for ( auto a : axis_y ) {
      a->Build(
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
    U space_x = 50;
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
  }

/*
  {
    for ( auto obj : axis_objects ) {
      if ( obj->Empty() ) continue;
      BoundaryBox bb = obj->GetBB();
      chart_g->Add( new Rect( bb.min, bb.max ) );
      chart_g->Last()->Attr()->FillColor()->Clear();
      chart_g->Last()->Attr()->SetLineWidth( 4 );
      chart_g->Last()->Attr()->LineColor()->Set( ColorName::Orange );
    }
  }
*/

  std::vector< LegendBox > lb_list;
  CalcLegendBoxes( legend_g, lb_list, axis_objects );

  for ( Series* series : series_list ) {
    Group* series_g = chartbox_g->AddNewGroup();
    series->Build( series_g, axis_x, axis_y[ series->axis_y_n ], lb_list );
  }

  PlaceLegend( axis_objects, lb_list, legend_g );

  if ( footnote != "" ) {
    BoundaryBox bb = chart_g->GetBB();
    U x = bb.min.x + 15;
    U y = bb.min.y - 15;
    MultiLineText( chart_g, footnote, 14 );
    chart_g->Last()->MoveTo( AnchorX::Min, AnchorY::Max, x, y );
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
