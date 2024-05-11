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

void Main::CalcLegendSize( Group* g, LegendDims& legend_dims )
{
  uint32_t max_chars = 1;
  uint32_t max_lines = 1;
  U mh = 0;
  Series::MarkerDims mo;
  mo.x1 = 0; mo.y1 = 0; mo.x2 = 0; mo.y2 = 0;
  for ( Series* series : series_list ) {
    series->ComputeMarker();
    if ( series->marker_show ) {
      mo.x1 = std::min( mo.x1, series->marker_out.x1 );
      mo.y1 = std::min( mo.y1, series->marker_out.y1 );
      mo.x2 = std::max( mo.x2, series->marker_out.x2 );
      mo.y2 = std::max( mo.y2, series->marker_out.y2 );
      U h = mo.y2 - mo.y1;
      if ( series->type == SeriesType::XY ) h += 4 + 2 * legend_by;
      mh = std::max( mh, h );
    }
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
  legend_dims.ch = bb.max.y - bb.min.y;
  legend_dims.ti = mo.x2;
  legend_dims.w  = (bb.max.x - bb.min.x) + 2 * legend_bx + 2 * legend_dims.ti;
  legend_dims.h  = std::max( +mh, legend_dims.ch * max_lines + 2 * legend_by );
  legend_dims.gx = legend_gx - mo.x1;
  legend_dims.gy = legend_gy;
  legend_dims.mx = -mo.x1;
}

//-----------------------------------------------------------------------------

// Determine potential placement of series legends in chart interior.
void Main::CalcLegendBoxes(
  Group* g, std::vector< LegendBox >& lb_list,
  const std::vector< SVG::Object* >& axis_objects
)
{
  LegendDims legend_dims;
  CalcLegendSize( g, legend_dims );
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
          nx * legend_dims.w + (nx - 1) * legend_dims.gx + 2 * legend_sx +
          2 * legend_dims.mx,
          ny * legend_dims.h + (ny - 1) * legend_dims.gy + 2 * legend_sy
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
  if ( axis_y[ 0 ]->at_orth_max ) std::swap( ax1, ax2 );
  if ( axis_x->at_orth_max ) std::swap( ay1, ay2 );

  for ( bool can_move : { false, true } ) {
    if ( !dual_y ) add_lbs( ax1, ay1, can_move, can_move );
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
    g->Last()->Attr()->LineColor()->Set( ColorName::Black );
  }
*/
}

//-----------------------------------------------------------------------------

void Main::BuildLegend( Group* g, int nx )
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
  LegendDims legend_dims;
  CalcLegendSize( g, legend_dims );
  int ny = (LegendCnt() + nx - 1) / nx;

  Point r1{
    -legend_sx / 2 - legend_dims.mx, +legend_sy / 2
  };
  Point r2{
    legend_dims.mx +
    +(nx * legend_dims.w + (nx - 1) * legend_dims.gx + legend_sx / 2),
    -(ny * legend_dims.h + (ny - 1) * legend_dims.gy + legend_sy / 2)
  };
  g->Add( new Rect( r1, r2, 4 ) );
  g->Last()->Attr()->LineColor()->Set( ColorName::Black, 0.5 );
  g->Last()->Attr()->SetLineWidth( 1 );

  int n = 0;
  for ( Series* series : series_list ) {
    if ( series->name.length() == 0 ) continue;
    U px = (n % nx) * +(legend_dims.w + legend_dims.gx);
    U py = (n / nx) * -(legend_dims.h + legend_dims.gy);
    Point marker_p{ px, (py + py - legend_dims.h) / 2 };
    marker_p.y -= (series->marker_out.y1 + series->marker_out.y2) / 2;
    if ( series->type == SeriesType::XY ) {
      g->Add(
        new Rect(
          px, py,
          px + legend_dims.w,
          py - legend_dims.h,
          legend_dims.ch/2
        )
      );
      series->ApplyStyle( g->Last() );
    }
    if ( series->marker_show ) {
      series->BuildMarker( g, series->marker_out, marker_p );
      series->ApplyStyle( g->Last() );
      g->Last()->Attr()->SetLineDash( 0 );
      g->Last()->Attr()->LineColor()->Clear();
      g->Last()->Attr()->FillColor()->Set( &series->color );
      if ( series->marker_hollow ) {
        series->BuildMarker( g, series->marker_int, marker_p );
        g->Last()->Attr()->SetLineDash( 0 );
        g->Last()->Attr()->LineColor()->Clear();
        SVG::Color color_light{ series->color };
        color_light.Lighten( 0.5 );
        g->Last()->Attr()->FillColor()->Set( &color_light );
      }
    }
    int lines = 1;
    for ( char c : series->name ) if ( c == '\n' ) lines++;
    px += legend_bx + legend_dims.ti;
    py -= legend_by + (legend_dims.h - 2 * legend_by - lines * legend_dims.ch) / 2;
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
        lb.collision_weight < best_lb.collision_weight ||
        (lb.collision_weight == best_lb.collision_weight && lb.sp < best_lb.sp)
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
  CalcLegendSize( legend_g, legend_dims );

  if ( legend_pos == Pos::Left || legend_pos == Pos::Right ) {

    U avail_h = chart_h;
    uint32_t nx = 1;
    while ( 1 ) {
      uint32_t ny = (LegendCnt() + nx - 1) / nx;
      U need_h = ny * legend_dims.h + (ny - 1) * legend_dims.gy + 2 * legend_sy;
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
      U need_w = nx * legend_dims.w + (nx - 1) * legend_dims.gx + 2 * legend_sx;
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
    axis_x->orth_style[ i ] = axis_y[ dual_y ? i : 0 ]->style;
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
  if ( axis_x->pos == Pos::Bottom ) {
    axis_y[ 0 ]->orth_axis_cross = axis_y[ 0 ]->min;
  }
  if ( axis_x->pos == Pos::Top ) {
    axis_y[ 0 ]->orth_axis_cross = axis_y[ 0 ]->max;
  }
  if ( axis_y[ 0 ]->pos == Pos::Left ) {
    axis_x->orth_axis_cross = axis_x->min;
  }
  if ( axis_y[ 0 ]->pos == Pos::Right ) {
    axis_x->orth_axis_cross = axis_x->max;
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
    Axis* a = dual_y ? axis_y[ i ] : axis_y[ 0 ];
    axis_x->orth_length_ext[ i ] =
      a->length + ((a->style == AxisStyle::Arrow) ? +a->overhang : 0);
  }

  for ( int i : { 0, 1 } ) {
    axis_y[ i ]->orth_style[ 0 ] = axis_x->style;
    axis_y[ i ]->orth_style[ 1 ] = axis_x->style;
    axis_x->orth_style[ i ] = axis_y[ dual_y ? i : 0 ]->style;
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

  Group* axes_line_g  = chart_g->AddNewGroup();
  Group* chartbox_g   = chart_g->AddNewGroup();
  Group* axes_num_g   = chart_g->AddNewGroup();
  Group* axes_label_g = chart_g->AddNewGroup();
  Group* legend_g     = chart_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( ColorName::Black );
  axes_line_g->Attr()->SetLineCap( LineCap::Square );

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
    for ( int i : { 1, 0 } ) {
      axis_y[ i ]->Build(
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
      chart_g->Last()->Attr()->LineColor()->Set( ColorName::Blue );
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
