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

#include <chart_ensemble.h>
#include <chart_main.h>

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Main::Main( Ensemble* ensemble, SVG::Group* svg_g )
{
  label_db    = new Label();
  legend_obj  = new Legend( ensemble );
  tag_db      = new Tag();
  axis_x      = new Axis( true , label_db );
  axis_y[ 0 ] = new Axis( false, label_db );
  axis_y[ 1 ] = new Axis( false, label_db );

  this->ensemble = ensemble;
  this->svg_g = svg_g;
  chart_area_color.Clear();
  frame_color.Undef();
  title_pos_x  = Pos::Center;
  title_pos_y  = Pos::Top;
  title_inside = false;
  title_size   = 1.0;
  title_frame            = false;
  title_frame_specified  = false;
  legend_frame           = false;
  legend_frame_specified = false;
}

Main::~Main( void )
{
  for ( auto series : series_list ) {
    delete series;
  }
  delete axis_x;
  delete axis_y[ 0 ];
  delete axis_y[ 1 ];
  delete label_db;
  delete legend_obj;
  delete tag_db;
}

///////////////////////////////////////////////////////////////////////////////

void Main::Move( SVG::U dx, SVG::U dy )
{
  svg_g->Move( dx, dy );
  g_dx = dx;
  g_dy = dy;
}

///////////////////////////////////////////////////////////////////////////////

void Main::SetChartArea( SVG::U width, SVG::U height )
{
  chart_w = std::max( U( 10 ), width );
  chart_h = std::max( U( 10 ), height );
}

void Main::SetChartBox( bool chart_box )
{
  this->chart_box = chart_box;
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

void Main::SetTitlePos( Pos pos_x, Pos pos_y )
{
  if ( pos_x != Pos::Left && pos_x != Pos::Right ) pos_x = Pos::Center;
  if ( pos_y != Pos::Bottom ) pos_y = Pos::Top;
  this->title_pos_x = pos_x;
  this->title_pos_y = pos_y;
}

void Main::SetTitleInside( bool inside )
{
  this->title_inside = inside;
}

void Main::SetTitleFrame( bool enable )
{
  title_frame = enable;
  title_frame_specified = true;
}

void Main::SetLegendHeading( const std::string& txt )
{
  legend_obj->heading = txt;
}

void Main::SetLegendFrame( bool enable )
{
  legend_frame = enable;
  legend_frame_specified = true;
}

void Main::SetLegendPos( Pos pos )
{
  legend_obj->pos = pos;
}

void Main::SetLegendSize( float size )
{
  legend_obj->size = size;
}

void Main::SetBarWidth( float one_width, float all_width )
{
  bar_one_width = one_width;
  bar_all_width = all_width;
}

void Main::SetLayeredBarWidth( float width )
{
  bar_layered_width = width;
}

void Main::SetBarMargin( float margin )
{
  bar_margin = margin;
}

Series* Main::AddSeries( SeriesType type )
{
  Series* series = new Series( this, type );
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

// Determine potential placement of series legends in chart interior.
void Main::CalcLegendBoxes(
  Group* g, std::vector< LegendBox >& lb_list,
  const std::vector< SVG::Object* >& avoid_objects
)
{
  Legend::LegendDims legend_dims;
  legend_obj->CalcLegendDims(
    legend_frame_specified ? legend_frame : true,
    g, legend_dims
  );
  uint32_t lc = legend_obj->Cnt();

  bool framed = legend_frame_specified ? legend_frame : true;

  auto add_lbs = [&](
    AnchorX anchor_x, AnchorY anchor_y, bool can_move = true
  )
  {
    uint32_t nx = (anchor_x == AnchorX::Mid) ? lc :  1;
    uint32_t ny = (anchor_x == AnchorX::Mid) ?  1 : lc;
    while ( nx > 0 && ny > 0 ) {
      {
        U w;
        U h;
        legend_obj->GetDims( w, h, legend_dims, framed, nx );
        w += 2 * box_spacing;
        h += 2 * box_spacing;
        g->Add( new Rect( 0, 0, w, h ) );
      }
      Object* obj = g->Last();
      U x = 0;
      U y = 0;
      if ( anchor_x == AnchorX::Mid ) x = chart_w / 2;
      if ( anchor_x == AnchorX::Max ) x = chart_w;
      if ( anchor_y == AnchorY::Mid ) y = chart_h / 2;
      if ( anchor_y == AnchorY::Max ) y = chart_h;
      obj->MoveTo( anchor_x, anchor_y, x, y );

      if ( can_move ) {
        bool done = false;
        while ( !done ) {
          done = true;
          BoundaryBox obj_bb = obj->GetBB();
          for ( auto ao : avoid_objects ) {
            if ( !SVG::Collides( obj, ao ) ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            U dx =
              (anchor_x == AnchorX::Min)
              ? (ao_bb.max.x - obj_bb.min.x)
              : (ao_bb.min.x - obj_bb.max.x);
            U dy =
              (anchor_y == AnchorY::Min)
              ? (ao_bb.max.y - obj_bb.min.y)
              : (ao_bb.min.y - obj_bb.max.y);
            if ( anchor_x == AnchorX::Mid ) dx = 0;
            if ( anchor_y == AnchorY::Mid ) dy = 0;
            if ( dx != 0 && std::abs( dx ) < std::abs( dy ) ) {
              dy = 0;
            } else {
              dx = 0;
            }
            obj->Move( dx, dy );
            if ( std::abs( dx ) > epsilon && std::abs( dy ) > epsilon ) {
              done = false;
              break;
            }
          }
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
          lb.anchor_x = anchor_x;
          lb.anchor_y = anchor_y;
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

  if ( dual_y ) {
    if ( axis_x->angle == 0 ) {
      add_lbs( AnchorX::Mid, ay1 );
      add_lbs( AnchorX::Mid, ay2 );
      add_lbs( ax1, ay1 );
      add_lbs( ax2, ay1 );
      add_lbs( ax1, ay2 );
      add_lbs( ax2, ay2 );
      add_lbs( ax1, AnchorY::Mid );
      add_lbs( ax2, AnchorY::Mid );
    } else {
      add_lbs( ax1, ay1 );
      add_lbs( ax1, AnchorY::Mid );
      add_lbs( ax1, ay2 );
      add_lbs( AnchorX::Mid, ay1 );
      add_lbs( AnchorX::Mid, ay2 );
      add_lbs( ax2, ay1 );
      add_lbs( ax2, ay2 );
      add_lbs( ax2, AnchorY::Mid );
    }
  } else {
    add_lbs( ax1, ay1 );
    add_lbs( AnchorX::Mid, ay1 );
    add_lbs( ax1, AnchorY::Mid );
    add_lbs( ax1, ay2 );
    add_lbs( AnchorX::Mid, ay2 );
    add_lbs( ax2, ay1 );
    add_lbs( ax2, ay2 );
    add_lbs( ax2, AnchorY::Mid );
  }

/*
  for ( auto lb : lb_list ) {
    g->Add( new Rect( lb.bb.min, lb.bb.max ) );
    g->Last()->Attr()->SetLineWidth( 2 );
    g->Last()->Attr()->FillColor()->Clear();
    g->Last()->Attr()->LineColor()->Set( ColorName::orange );
  }
*/
}

//-----------------------------------------------------------------------------

void Main::PlaceLegends(
  std::vector< SVG::Object* >& avoid_objects,
  const std::vector< LegendBox >& lb_list,
  Group* legend_g
)
{
  if ( legend_obj->Cnt() == 0 ) return;

  BoundaryBox build_bb;
  BoundaryBox moved_bb;

  if ( legend_obj->pos == Pos::Auto ) {
    AnchorX title_anchor_x = AnchorX::Mid;
    if ( title_pos_x == Pos::Left ) title_anchor_x = AnchorX::Min;
    if ( title_pos_x == Pos::Right ) title_anchor_x = AnchorX::Max;

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
            ( lb.weight2 == best_lb.weight2 &&
              ( lb.sp < best_lb.sp ||
                ( lb.sp == best_lb.sp &&
                  title_inside && title_anchor_x != AnchorX::Mid &&
                  lb.anchor_x == title_anchor_x &&
                  best_lb.anchor_x != title_anchor_x
                )
              )
            )
          )
        )
      ) {
        best_lb = lb;
      }
    }
    if ( best_lb_defined ) {
      legend_obj->BuildLegends(
        legend_frame_specified ? legend_frame : true,
        AxisColor(), FrameColor(),
        legend_g->AddNewGroup(), best_lb.nx
      );
      build_bb = legend_g->Last()->GetBB();
      legend_g->Last()->MoveTo(
        AnchorX::Mid, AnchorY::Mid,
        (best_lb.bb.min.x + best_lb.bb.max.x) / 2,
        (best_lb.bb.min.y + best_lb.bb.max.y) / 2
      );
      moved_bb = legend_g->Last()->GetBB();
      ensemble->html_db->MoveLegends(
        this,
        moved_bb.min.x - build_bb.min.x,
        moved_bb.min.y - build_bb.min.y
      );
      return;
    } else {
      legend_obj->pos = Pos::Bottom;
    }
  }

  bool framed =
    legend_frame_specified ? legend_frame : !legend_obj->heading.empty();

  Legend::LegendDims legend_dims;
  legend_obj->CalcLegendDims( framed, legend_g, legend_dims );

  if ( legend_obj->pos == Pos::Left || legend_obj->pos == Pos::Right ) {

    U mx = legend_obj->MarginX( framed );
    U my = legend_obj->MarginY( framed );

    uint32_t nx;
    legend_obj->GetBestFit( legend_dims, nx, framed, 0, chart_h );
    legend_obj->BuildLegends(
      framed, AxisColor(), FrameColor(),
      legend_g->AddNewGroup(), nx
    );
    Object* legend = legend_g->Last();
    build_bb = legend->GetBB();

    U x = 0 - mx;
    Dir dir = Dir::Left;
    AnchorX anchor_x = AnchorX::Max;
    if ( legend_obj->pos == Pos::Right ) {
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
        ( (legend_obj->pos == Pos::Right)
          ? (bb.min.x + epsilon < best_x)
          : (bb.min.x - epsilon > best_x)
        )
      ) {
        best_anchor_y = anchor_y;
        best_x = bb.min.x;
        best_y = y;
        best_found = true;
      }
    }
    legend->MoveTo( anchor_x, best_anchor_y, x, best_y );
    MoveObj( dir, legend, avoid_objects, mx, my );
    moved_bb = legend->GetBB();
    ensemble->html_db->MoveLegends(
      this,
      moved_bb.min.x - build_bb.min.x,
      moved_bb.min.y - build_bb.min.y
    );
    avoid_objects.push_back( legend );

  } else {

    U mx = 40;
    U my = legend_obj->MarginY( framed );

    uint32_t nx;
    legend_obj->GetBestFit( legend_dims, nx, framed, chart_w, 0 );
    legend_obj->BuildLegends(
      framed, AxisColor(), FrameColor(),
      legend_g->AddNewGroup(), nx
    );
    Object* legend = legend_g->Last();
    build_bb = legend->GetBB();

    U y = 0 - my;
    Dir dir = Dir::Down;
    AnchorY anchor_y = AnchorY::Max;
    if ( legend_obj->pos == Pos::Top ) {
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
        ( (legend_obj->pos == Pos::Top)
          ? (bb.min.y + epsilon < best_y)
          : (bb.min.y - epsilon > best_y)
        )
      ) {
        best_anchor_x = anchor_x;
        best_x = x;
        best_y = bb.min.y;
        best_found = true;
      }
    }
    legend->MoveTo( best_anchor_x, anchor_y, best_x, y );
    MoveObj( dir, legend, avoid_objects, mx, my );
    moved_bb = legend->GetBB();
    ensemble->html_db->MoveLegends(
      this,
      moved_bb.min.x - build_bb.min.x,
      moved_bb.min.y - build_bb.min.y
    );
    avoid_objects.push_back( legend );

  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

int Main::CatStrideEmpty( void )
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
  for ( auto a : { axis_x, axis_y[ 0 ], axis_y[ 1 ] } ) {
    if ( !a->GridColor()->IsDefined() ) {
      a->GridColor()->Set( ensemble->ForegroundColor() );
    }
  }

  axis_x->length      = (axis_x->angle == 0) ? chart_w : chart_h;
  axis_x->orth_length = (axis_x->angle == 0) ? chart_h : chart_w;
  axis_x->chart_box   = chart_box;
  for ( auto a : axis_y ) {
    a->length      = (a->angle == 0) ? chart_w : chart_h;
    a->orth_length = (a->angle == 0) ? chart_h : chart_w;
    a->chart_box   = chart_box;
  }

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
        series->type == SeriesType::LayeredBar ||
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
    axis_x->min -= bar_margin;
    axis_x->max += bar_margin;
    axis_x->orth_axis_cross = axis_x->min;
    axis_x->reverse = axis_x->reverse ^ (axis_x->angle != 0);
    axis_x->cat_stride_empty = CatStrideEmpty();
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
    bool init_ofs[ 2 ][ 2 ] = { { true, true }, { true, true } };
    for ( auto series : series_list ) {
      int type_n = (series->type == SeriesType::StackedArea) ? 1 : 0;
      int axis_n = series->axis_y_n;
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::LayeredBar
      ) {
        init_ofs[ type_n ][ 0 ] = true;
        init_ofs[ type_n ][ 1 ] = true;
      }
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar ||
        series->type == SeriesType::LayeredBar ||
        series->type == SeriesType::StackedArea
      ) {
        if ( init_ofs[ type_n ][ axis_n ] ) {
          ofs_pos[ type_n ][ axis_n ].assign( category_list.size(), series->base );
          ofs_neg[ type_n ][ axis_n ].assign( category_list.size(), series->base );
        }
        init_ofs[ type_n ][ axis_n ] = false;
      }
      series->DetermineMinMax(
        ofs_pos[ type_n ][ axis_n ],
        ofs_neg[ type_n ][ axis_n ]
      );
      if ( series->type == SeriesType::LayeredBar ) {
        init_ofs[ type_n ][ 0 ] = true;
        init_ofs[ type_n ][ 1 ] = true;
      }
      if (
        series->type == SeriesType::Bar ||
        series->type == SeriesType::StackedBar
      ) {
        init_ofs[ type_n ][ 1 - axis_n ] = true;
      }
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
      series->axis_y = axis_y[ 0 ];
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
          series->type == SeriesType::LayeredBar ||
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
          axis_y[ i ]->orth_axis_cross_is_base = true;
        } else {
          axis_x->pos = Pos::Auto;
        }
      } else {
        for ( int i = 0; i < 2; i++ ) {
          if ( base_def[ i ] == 1 ) {
            axis_y[ i ]->orth_axis_cross = base[ i ];
            axis_y[ i ]->orth_axis_cross_is_base = true;
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
    if ( axis_x->style == AxisStyle::Edge ) {
      axis_x->style = AxisStyle::Line;
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
  Color tag_bg_color;
  if ( !ChartAreaColor()->IsClear() ) {
    tag_bg_color.Set( ChartAreaColor() );
  } else {
    tag_bg_color.Set( ensemble->BackgroundColor() );
  }
  if ( tag_bg_color.IsClear() ) tag_bg_color.Set( ColorName::white );

  bool bar_next_can_stack = false;
  bool bar_next_can_layer = false;
  int bar_prev_y_n = 0;
  Series* bottom_layer_series = nullptr;

  bar_tot = 0;
  lol_tot = 0;

  uint32_t bar_layer_cur = 0;

  uint32_t series_id = 0;
  for ( auto series : series_list ) {
    series->id = series_id++;

    series->chart_area.min.x = 0;
    series->chart_area.max.x = chart_w;
    series->chart_area.min.y = 0;
    series->chart_area.max.y = chart_h;
    series->axis_x = axis_x;
    series->axis_y = axis_y[ series->axis_y_n ];
    series->lb_list = lb_list;
    series->tag_db = tag_db;
    if ( ensemble->enable_html ) {
      if ( !series->name.empty() || series->anonymous_snap ) {
        series->html_db = ensemble->html_db;
      }
    }

    if ( series->type == SeriesType::Lollipop ) {
      lol_tot++;
    }

    series->bar_layer_num = 0;
    series->bar_layer_tot = 1;
    if ( series->type == SeriesType::Bar ) {
      bar_tot++;
      bar_next_can_stack = true;
      bar_next_can_layer = true;
      bar_prev_y_n = series->axis_y_n;
      bar_layer_cur = 0;
      bottom_layer_series = series;
    }
    if ( series->type == SeriesType::StackedBar ) {
      if ( !bar_next_can_stack || series->axis_y_n != bar_prev_y_n ) bar_tot++;
      bar_next_can_stack = true;
      bar_next_can_layer = false;
      bar_prev_y_n = series->axis_y_n;
    }
    if ( series->type == SeriesType::LayeredBar ) {
      if ( !bar_next_can_layer || series->axis_y_n != bar_prev_y_n ) {
        bar_tot++;
        bar_layer_cur = 0;
        bottom_layer_series = series;
      } else {
        bar_layer_cur++;
        bottom_layer_series->bar_layer_tot = bar_layer_cur + 1;
      }
      bar_next_can_stack = false;
      bar_next_can_layer = true;
      bar_prev_y_n = series->axis_y_n;
      series->bar_layer_num = bar_layer_cur;
    }

    if ( !series->tag_text_color.IsDefined() ) {
      series->tag_text_color.Set( TextColor() );
    }

    if ( !series->tag_fill_color.IsDefined() ) {
      Color c{ &series->line_color };
      if ( c.IsClear() ) {
        c.Set( &series->fill_color );
      }
      if ( c.IsClear() ) {
        series->tag_fill_color.Set( &tag_bg_color );
      } else {
        c.SetTransparency( 0.0 );
        series->tag_fill_color.Set( &tag_bg_color, &c, 0.2 );
      }
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

    if ( !series->name.empty() ) {
      if ( series->global_legend ) {
        ensemble->legend_obj->Add( series );
      } else {
        legend_obj->Add( series );
      }
    }
  }

  for ( auto series : series_list ) {
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::LayeredBar
    ) {
      if ( series->bar_layer_num == 0 ) {
        bottom_layer_series = series;
      } else
      if ( series->type == SeriesType::LayeredBar ) {
        series->bar_layer_tot = bottom_layer_series->bar_layer_tot;
      }
    }
  }

  html.all_inline = bar_tot <= 1 && lol_tot <= 1;

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

  bool bar_next_can_stack = false;
  bool bar_next_can_layer = false;
  int bar_prev_y_n = 0;
  std::vector< double > bar_ofs_pos;
  std::vector< double > bar_ofs_neg;
  uint32_t bar_cur = 0;
  bool bar_first = true;
  bool bar_init = true;

  uint32_t lol_num = 0;

  Group* stacked_area_fill_g = below_axes_g->AddNewGroup();
  Group* stacked_area_line_g = below_axes_g->AddNewGroup();
  Group* bar_area_g          = below_axes_g->AddNewGroup();
  Group* bar_line_g          = below_axes_g->AddNewGroup();
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
        stacked_area_line_g, stacked_area_line_g, stacked_area_fill_g,
        above_axes_g, tag_g,
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
        bar_area_g, bar_area_g, bar_area_g, above_axes_g, tag_g,
        0, 1,
        &ofs_pos, &ofs_neg,
        &pts_pos, &pts_neg
      );
    }
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::LayeredBar
    ) {
      if ( series->type == SeriesType::Bar ) {
        if ( !bar_first ) bar_cur++;
        bar_next_can_stack = true;
        bar_next_can_layer = true;
        bar_init = true;
      }
      if ( series->type == SeriesType::StackedBar ) {
        if ( !bar_next_can_stack || series->axis_y_n != bar_prev_y_n ) {
          if ( !bar_first ) bar_cur++;
          bar_init = true;
        }
        bar_next_can_stack = true;
        bar_next_can_layer = false;
      }
      if ( series->type == SeriesType::LayeredBar ) {
        if ( !bar_next_can_layer || series->axis_y_n != bar_prev_y_n ) {
          if ( !bar_first ) bar_cur++;
        }
        bar_next_can_stack = false;
        bar_next_can_layer = true;
        bar_init = true;
      }
      if ( bar_init ) {
        bar_ofs_pos.assign( category_list.size(), series->base );
        bar_ofs_neg.assign( category_list.size(), series->base );
        bar_init = false;
      }
      series->Build(
        bar_area_g, bar_line_g, nullptr, nullptr, tag_g,
        bar_cur, bar_tot,
        &bar_ofs_pos, &bar_ofs_neg
      );
      bar_prev_y_n = series->axis_y_n;
      bar_first = false;
    }
    if ( series->type == SeriesType::Lollipop ) {
      series->Build(
        lollipop_stem_g, lollipop_stem_g, nullptr, above_axes_g, tag_g,
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
        above_axes_g, above_axes_g, nullptr, above_axes_g, tag_g,
        0, 1
      );
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Main::BuildTitle(
  std::vector< SVG::Object* >& avoid_objects
)
{
  if ( title.empty() && sub_title.empty() && sub_sub_title.empty() ) return;

  U spacing = 4 * title_size;

  bool framed = title_frame_specified ? title_frame : title_inside;

  U space_x = 5 * box_spacing;
  U space_y = 1 * box_spacing;
  BoundaryBox bb;
  std::vector< SVG::Object* > title_objs;

  Group* text_g = svg_g->AddNewGroup();

  U x = chart_w / 2;
  AnchorX a = AnchorX::Mid;
  if ( title_pos_x == Pos::Left ) {
    x = 0;
    a = AnchorX::Min;
  }
  if ( title_pos_x == Pos::Right ) {
    x = chart_w;
    a = AnchorX::Max;
  }
  U y = chart_h + space_y;
  if ( !sub_sub_title.empty() ) {
    Object* obj = Label::CreateLabel( text_g, sub_sub_title, 14 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !sub_title.empty() ) {
    Object* obj = Label::CreateLabel( text_g, sub_title, 20 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !title.empty() ) {
    Object* obj = Label::CreateLabel( text_g, title, 36 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    title_objs.push_back( obj );
    bb = obj->GetBB();
  }
  MoveObjs( Dir::Up, title_objs, avoid_objects, space_x, space_y );

  if ( framed ) {
    bb = text_g->GetBB();
    text_g->Add(
      new Rect(
        bb.min.x - box_spacing, bb.min.y - box_spacing,
        bb.max.x + box_spacing, bb.max.y + box_spacing,
        box_spacing
      )
    );
    text_g->Last()->Attr()->LineColor()->Set( AxisColor() );
    text_g->Last()->Attr()->SetLineWidth( 1 );
    if ( FrameColor()->IsDefined() ) {
      text_g->Last()->Attr()->FillColor()->Set( FrameColor() );
    }
    text_g->FrontToBack();
    y = chart_h + space_y;
    switch ( title_pos_x ) {
      case Pos::Left  : text_g->MoveTo( a, AnchorY::Min, 0        , y ); break;
      case Pos::Right : text_g->MoveTo( a, AnchorY::Min, chart_w  , y ); break;
      default         : text_g->MoveTo( a, AnchorY::Min, chart_w/2, y );
    }
    MoveObj( Dir::Up, text_g, avoid_objects, box_spacing, box_spacing );
  }

  y = 0;
  for ( auto obj : avoid_objects ) {
    if ( !obj->Empty() ) {
      y = std::max( y, obj->GetBB().max.y );
    }
  }
  y = y - text_g->GetBB().max.y;
  if ( y > 0 ) {
    text_g->Move( 0, y );
  }

  if ( title_inside ) {
    U mx = box_spacing;
    U my = box_spacing;
    U px = chart_w / 2;
    U py = chart_h - my;
    AnchorX ax = AnchorX::Mid;
    AnchorY ay = AnchorY::Max;
    if ( title_pos_x == Pos::Left ) {
      px = mx;
      ax = AnchorX::Min;
    }
    if ( title_pos_x == Pos::Right ) {
      px = chart_w - mx;
      ax = AnchorX::Max;
    }
    if ( title_pos_y == Pos::Bottom ) {
      py = my;
      ay = AnchorY::Min;
    }
    text_g->MoveTo( ax, ay, px, py );

    for ( int pass = 0; pass < 2; pass++ ) {
      if ( ax != AnchorX::Mid ) {
        U old_x = coor_hi;
        while ( true ) {
          bb = text_g->GetBB();
          if ( bb.min.x == old_x ) break;
          old_x = bb.min.x;
          U dx = 0;
          for ( auto ao : avoid_objects ) {
            if ( !SVG::Collides( text_g, ao, mx, 0 ) ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            if ( ax == AnchorX::Min && ao_bb.max.x < (chart_w * 1 / 4) ) {
              dx = ao_bb.max.x - bb.min.x + mx;
              break;
            }
            if ( ax == AnchorX::Max && ao_bb.min.x > (chart_w * 3 / 4) ) {
              dx = ao_bb.min.x - bb.max.x - mx;
              break;
            }
          }
          if ( dx == 0 ) break;
          text_g->Move( dx, 0 );
        }
      }
      bb = text_g->GetBB();
      if ( bb.min.x < mx || bb.max.x > chart_w - mx ) {
        text_g->MoveTo( AnchorX::Mid, ay, chart_w / 2, py );
      }
      {
        U old_y = coor_hi;
        while ( true ) {
          bb = text_g->GetBB();
          if ( bb.min.y == old_y ) break;
          old_y = bb.min.y;
          U dy = 0;
          for ( auto ao : avoid_objects ) {
            if ( !SVG::Collides( text_g, ao, 0, my ) ) continue;
            BoundaryBox ao_bb = ao->GetBB();
            if ( ay == AnchorY::Min && ao_bb.max.y < (chart_h * 1 / 4) ) {
              dy = ao_bb.max.y - bb.min.y + my;
              break;
            }
            if ( ay == AnchorY::Max && ao_bb.min.y > (chart_h * 3 / 4) ) {
              dy = ao_bb.min.y - bb.max.y - my;
              break;
            }
          }
          if ( dy == 0 ) break;
          text_g->Move( 0, dy );
        }
      }
    }

    avoid_objects.push_back( text_g );
  }

  return;
}

//------------------------------------------------------------------------------

SVG::U Main::GetAreaPadding( void )
{
  U delta = 0;
  for ( auto series : series_list ) {
    if (
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::LayeredBar
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

  return delta;
}

//------------------------------------------------------------------------------

void Main::PrepareHTML( void )
{
  if ( axis_x->angle == 0 ) {

    if ( axis_x->category_axis ) {
      ensemble->html_db->DefAxisX(
        this, axis_x->cat_coor_is_max ? 0 : 1, axis_x,
        axis_x->reverse ? axis_x->max : axis_x->min,
        axis_x->reverse ? axis_x->min : axis_x->max,
        NumberFormat::Fixed, false, false, true
      );
    } else {
      ensemble->html_db->DefAxisX(
        this, axis_x->orth_coor_is_max ? 0 : 1, axis_x,
        axis_x->reverse ? axis_x->max : axis_x->min,
        axis_x->reverse ? axis_x->min : axis_x->max,
        axis_x->number_format, axis_x->number_sign, axis_x->log_scale
      );
    }

    for ( auto a : axis_y ) {
      if ( a->show ) {
        ensemble->html_db->DefAxisY(
          this, a->orth_coor_is_max ? 1 : 0, a,
          a->reverse ? a->min : a->max,
          a->reverse ? a->max : a->min,
          a->number_format, a->number_sign, a->log_scale
        );
      }
    }

  } else {

    if ( axis_x->category_axis ) {
      ensemble->html_db->DefAxisY(
        this, axis_x->cat_coor_is_max ? 1 : 0, axis_x,
        axis_x->reverse ? axis_x->min : axis_x->max,
        axis_x->reverse ? axis_x->max : axis_x->min,
        NumberFormat::Fixed, false, false, true
      );
    } else {
      ensemble->html_db->DefAxisY(
        this, axis_x->orth_coor_is_max ? 1 : 0, axis_x,
        axis_x->reverse ? axis_x->min : axis_x->max,
        axis_x->reverse ? axis_x->max : axis_x->min,
        axis_x->number_format, axis_x->number_sign, axis_x->log_scale
      );
    }

    for ( auto a : axis_y ) {
      if ( a->show ) {
        ensemble->html_db->DefAxisX(
          this, a->orth_coor_is_max ? 0 : 1, a,
          a->reverse ? a->max : a->min,
          a->reverse ? a->min : a->max,
          a->number_format, a->number_sign, a->log_scale
        );
      }
    }

    html.axis_swap = true;

  }

  return;
}

//------------------------------------------------------------------------------

void Main::Build( void )
{
  if ( !AxisColor()->IsDefined() ) {
    AxisColor()->Set( ensemble->ForegroundColor() );
  }
  if ( !TextColor()->IsDefined() ) {
    TextColor()->Set( ensemble->ForegroundColor() );
  }

  svg_g->Attr()->TextColor()->Set( TextColor() );
  svg_g->Attr()->LineColor()->Clear();
  svg_g->Add( new Rect( 0, 0, chart_w, chart_h ) );
  if ( ChartAreaColor()->IsClear() ) {
    svg_g->Last()->Attr()->FillColor()->Clear();
  } else {
    svg_g->Last()->Attr()->FillColor()->Set( ChartAreaColor() );
  }

  Group* grid_minor_g          = svg_g->AddNewGroup();
  Group* grid_major_g          = svg_g->AddNewGroup();
  Group* grid_zero_g           = svg_g->AddNewGroup();
  Group* label_bg_g            = svg_g->AddNewGroup();
  Group* chartbox_below_axes_g = svg_g->AddNewGroup();
  Group* axes_line_g           = svg_g->AddNewGroup();
  Group* chartbox_above_axes_g = svg_g->AddNewGroup();
  Group* axes_num_g            = svg_g->AddNewGroup();
  Group* axes_label_g          = svg_g->AddNewGroup();
  Group* tag_g                 = svg_g->AddNewGroup();
  Group* legend_g              = svg_g->AddNewGroup();

  axes_line_g->Attr()->SetLineWidth( 2 )->LineColor()->Set( AxisColor() );
  axes_line_g->Attr()->SetLineCap( LineCap::Square );
  axes_line_g->Attr()->FillColor()->Set( AxisColor() );

  chartbox_below_axes_g->Attr()->FillColor()->Clear();
  chartbox_above_axes_g->Attr()->FillColor()->Clear();

  axes_num_g->Attr()->LineColor()->Clear();

  // This group only has numbers so optimize baseline to ensure vertical
  // centering within boundary box.
  tag_g->Attr()->TextFont()
    ->SetWidthFactor( 1.0 )
    ->SetHeightFactor( 0.80 )
    ->SetBaselineFactor( 0.30 );

  legend_g->Attr()->TextFont()->SetSize( 14 * legend_obj->size );

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
    axes_line_g->Last()->Attr()->FillColor()->Clear();
  }

  axis_x->BuildLabel( avoid_objects, axes_label_g );
  for ( auto a : axis_y ) {
    a->BuildLabel( avoid_objects, axes_label_g );
  }

  if ( title_inside ) {
    BuildTitle( avoid_objects );
  }

  CalcLegendBoxes( legend_g, lb_list, avoid_objects );

  BuildSeries( chartbox_below_axes_g, chartbox_above_axes_g, tag_g );

  PlaceLegends( avoid_objects, lb_list, legend_g );

  if ( !title_inside ) {
    BuildTitle( avoid_objects );
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
      svg_g->Add( new Rect( bb.min, bb.max ) );
      svg_g->Last()->Attr()->FillColor()->Clear();
      svg_g->Last()->Attr()->SetLineWidth( 4 );
      svg_g->Last()->Attr()->LineColor()->Set( ColorName::blue );
      svg_g->Last()->Attr()->LineColor()->SetTransparency( 0.5 );
    }
  }
*/

  // Add background for text objects in the Label data base.
  {
    bool partial_ok = true;
    if ( ChartAreaColor()->IsClear() ) {
      label_bg_g->Attr()->FillColor()->Set( ensemble->BackgroundColor() );
    } else {
      label_bg_g->Attr()->FillColor()->Set( ChartAreaColor() );
      partial_ok = false;
    }
    BoundaryBox area;
    area.min.x = 0; area.max.x = chart_w;
    area.min.y = 0; area.max.y = chart_h;
    label_db->AddBackground( label_bg_g, area, partial_ok );
  }

  if ( ensemble->enable_html ) {
    PrepareHTML();
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////
