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

#include <chart_series.h>
#include <chart_axis.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Series::Series( SeriesType type )
{
  axis_y_n = 0;
  base = 0;

  color_list.emplace_back(); color_list.back().Set( ColorName::royalblue     );
  color_list.emplace_back(); color_list.back().Set( ColorName::tomato        );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkseagreen  );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkturquoise );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkmagenta   );
  color_list.emplace_back(); color_list.back().Set( ColorName::deepskyblue   );
  color_list.emplace_back(); color_list.back().Set( ColorName::orange        );
  color_list.emplace_back(); color_list.back().Set( ColorName::brown         );
  color_list.emplace_back(); color_list.back().Set( ColorName::gold          );
  color_list.emplace_back(); color_list.back().Set( ColorName::slategrey     );

  this->type = type;
  SetName( "" );
  SetAxisY( 0 );

  SetLineWidth(
    (type == SeriesType::Area || type == SeriesType::StackedArea) ? 0 : 1
  );
  line_color.Set( ColorName::black );
  SetLineDash( 0 );

  fill_color.Clear();

  SetMarkerSize( 0 );
  SetMarkerShape( MarkerShape::Circle );

  marker_show = false;
  marker_show_out = false;
  marker_show_int = false;
  has_line = false;
  has_fill = false;
}

Series::~Series( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Series::SetName( const std::string name )
{
  this->name = name;
}

void Series::SetAxisY( int axis_y_n )
{
  this->axis_y_n = axis_y_n;
}

void Series::SetBase( double base )
{
  this->base = base;
}

void Series::SetStyle( int style )
{
  line_color.Set( &color_list[ style % color_list.size() ] );
  fill_color.Set( &line_color );
  style = style / color_list.size();
  style = style % 8;
  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    if ( type == SeriesType::Area || type == SeriesType::StackedArea ) {
      fill_color.SetTransparency( 0.5 );
    } else {
      fill_color.Lighten( 0.5 );
    }
    SetLineWidth( 1 );
    SetLineDash( 0 );
  } else {
    fill_color.Lighten( 0.5 );
    if ( style == 0 ) {
      SetLineWidth( 4 );
      SetLineDash( 0 );
    }
    if ( style == 1 ) {
      SetLineWidth( 4 );
      SetLineDash( 5, 3 );
    }
    if ( style == 2 ) {
      SetLineWidth( 4 );
      SetLineDash( 10, 6 );
    }
    if ( style == 3 ) {
      SetLineWidth( 4 );
      SetLineDash( 20, 12 );
    }
    if ( style == 4 ) {
      SetLineWidth( 2 );
      SetLineDash( 0 );
    }
    if ( style == 5 ) {
      SetLineWidth( 2 );
      SetLineDash( 5, 3 );
    }
    if ( style == 6 ) {
      SetLineWidth( 2 );
      SetLineDash( 10, 6 );
    }
    if ( style == 7 ) {
      SetLineWidth( 2 );
      SetLineDash( 20, 12 );
    }
  }
  SetMarkerSize(
    ( type == SeriesType::Scatter ||
      type == SeriesType::Point ||
      type == SeriesType::Lollipop
    )
    ? 12
    : 0
  );
}

//------------------------------------------------------------------------------

void Series::SetLineWidth( SVG::U width )
{
  if ( width > 0 ) {
    line_width = width;
  } else {
    line_width = 0;
  }
}

void Series::SetLineDash( SVG::U dash )
{
  line_dash = dash;
  line_hole = dash;
}

void Series::SetLineDash( SVG::U dash, SVG::U hole )
{
  line_dash = dash;
  line_hole = hole;
}

//------------------------------------------------------------------------------

void Series::SetMarkerSize( SVG::U size )
{
  marker_size = std::max( 0.0, +size );
}

void Series::SetMarkerShape( MarkerShape shape )
{
  marker_shape = shape;
}

//------------------------------------------------------------------------------

void Series::ApplyFillStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  obj->Attr()->FillColor()->Set( &fill_color );
}

void Series::ApplyLineStyle( SVG::Object* obj )
{
  obj->Attr()->SetLineWidth( line_width );
  if ( line_width > 0 ) {
    if ( line_dash > 0 ) {
      obj->Attr()->SetLineDash( line_dash, line_hole );
    }
    if ( marker_show ) {
      obj->Attr()->SetLineJoin( LineJoin::Round );
    }
    obj->Attr()->LineColor()->Set( &line_color );
  } else {
    obj->Attr()->LineColor()->Clear();
  }
  obj->Attr()->FillColor()->Clear();
}

void Series::ApplyMarkStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  if ( line_width > 0 ) {
    obj->Attr()->FillColor()->Set( &line_color );
    if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
      obj->Attr()->FillColor()->SetOpacity( 1.0 );
    }
  } else {
    obj->Attr()->FillColor()->Clear();
  }
}

void Series::ApplyHoleStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  obj->Attr()->FillColor()->Set( &fill_color );
  if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
    obj->Attr()->FillColor()->SetOpacity( 1.0 );
  }
}

////////////////////////////////////////////////////////////////////////////////

void Series::Add( double x, double y )
{
  datum_list.emplace_back( x, y );
}

////////////////////////////////////////////////////////////////////////////////

bool Series::Inside(
  const SVG::Point p, const SVG::BoundaryBox& clip_box
)
{
  return
    p.x >= clip_box.min.x && p.x <= clip_box.max.x &&
    p.y >= clip_box.min.y && p.y <= clip_box.max.y;
}

////////////////////////////////////////////////////////////////////////////////

// Returns:
//   0 : No intersection.
//   1 : One intersection; c1 is the point.
//   2 : Two intersections; c1 and c2 are the points.
int Series::ClipLine(
  SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
  const SVG::BoundaryBox& clip_box
)
{
  // Record original p1.
  Point o1 = p1;

  auto intersect_x = []( U x, Point p1, Point p2 )
  {
    U dx = p1.x - p2.x;
    U dy = p1.y - p2.y;
    U cp = p1.x * p2.y - p1.y * p2.x;
    U y = (dy * x + cp) / dx;
    return y;
  };
  auto intersect_y = []( U y, Point p1, Point p2 )
  {
    U dx = p1.x - p2.x;
    U dy = p1.y - p2.y;
    U cp = p1.x * p2.y - p1.y * p2.x;
    U x = (dx * y - cp) / dy;
    return x;
  };
  auto near = [&]( Point p1, Point p2 )
  {
    return
      std::abs( p1.x - p2.x ) < e1 &&
      std::abs( p1.y - p2.y ) < e1;
  };

  // The clip coordinate for the four sides and a flag telling if it is valid.
  U bot_x; bool bot_v = false;
  U top_x; bool top_v = false;
  U lft_y; bool lft_v = false;
  U rgt_y; bool rgt_v = false;

  // Detect bottom and top clippings.
  if ( p1.y > p2.y ) std::swap( p1, p2 );
  if ( p1.y < clip_box.min.y && p2.y >= clip_box.min.y ) {
    bot_x = intersect_y( clip_box.min.y, p1, p2 );
    bot_v = bot_x > (clip_box.min.x - e2) && bot_x < (clip_box.max.x + e2);
  }
  if ( p1.y <= clip_box.max.y && p2.y > clip_box.max.y ) {
    top_x = intersect_y( clip_box.max.y, p1, p2 );
    top_v = top_x > (clip_box.min.x - e2) && top_x < (clip_box.max.x + e2);
  }

  // Detect left and right clippings.
  if ( p1.x > p2.x ) std::swap( p1, p2 );
  if ( p1.x < clip_box.min.x && p2.x >= clip_box.min.x ) {
    lft_y = intersect_x( clip_box.min.x, p1, p2 );
    lft_v = lft_y > (clip_box.min.y - e2) && lft_y < (clip_box.max.y + e2);
  }
  if ( p1.x <= clip_box.max.x && p2.x > clip_box.max.x ) {
    rgt_y = intersect_x( clip_box.max.x, p1, p2 );
    rgt_v = rgt_y > (clip_box.min.y - e2) && rgt_y < (clip_box.max.y + e2);
  }

  // The four potential clip points.
  Point bot_c{ bot_x, clip_box.min.y };
  Point top_c{ top_x, clip_box.max.y };
  Point lft_c{ clip_box.min.x, lft_y };
  Point rgt_c{ clip_box.max.x, rgt_y };

  // Prune very close clip-detections in the corners.
  if ( bot_v && lft_v && near( bot_c, lft_c ) ) lft_v = false;
  if ( bot_v && rgt_v && near( bot_c, rgt_c ) ) rgt_v = false;
  if ( top_v && lft_v && near( top_c, lft_c ) ) lft_v = false;
  if ( top_v && rgt_v && near( top_c, rgt_c ) ) rgt_v = false;

  // Deliver result.
  Point* c = &c1;
  int n = 0;
  if ( bot_v ) { *c = bot_c; c = &c2; n++; }
  if ( top_v ) { *c = top_c; c = &c2; n++; }
  if ( lft_v ) { *c = lft_c; c = &c2; n++; }
  if ( rgt_v ) { *c = rgt_c; c = &c2; n++; }

  // When we have two clip points, we must make sure that the order of points on
  // line is p1:c1:c2:p2, where p1 and p2 are the original arguments.
  if ( n == 2 ) {
    p1 = o1;    // Restore original argument.
    double dx1 = c1.x - p1.x;
    double dy1 = c1.y - p1.y;
    double dx2 = c2.x - p1.x;
    double dy2 = c2.y - p1.y;
    if ( dx1*dx1 + dy1*dy1 > dx2*dx2 + dy2*dy2 ) {
      std::swap( c1, c2 );
    }
  }

  return n;
}

SVG::Point Series::MoveInside(
  SVG::Point p, const BoundaryBox& clip_box
)
{
  if ( p.x < clip_box.min.x ) p.x = clip_box.min.x;
  if ( p.x > clip_box.max.x ) p.x = clip_box.max.x;
  if ( p.y < clip_box.min.y ) p.y = clip_box.min.y;
  if ( p.y > clip_box.max.y ) p.y = clip_box.max.y;
  return p;
}

////////////////////////////////////////////////////////////////////////////////

void Series::UpdateLegendBoxes(
  std::vector< LegendBox >& lb_list,
  Point p1, Point p2,
  bool p1_inc, bool p2_inc
)
{
  Point c1;
  Point c2;
  for ( LegendBox& lb : lb_list ) {
    if ( p1.x < lb.bb.min.x && p2.x < lb.bb.min.x ) continue;
    if ( p1.x > lb.bb.max.x && p2.x > lb.bb.max.x ) continue;
    if ( p1.y < lb.bb.min.y && p2.y < lb.bb.min.y ) continue;
    if ( p1.y > lb.bb.max.y && p2.y > lb.bb.max.y ) continue;
    bool p1_inside = Inside( p1, lb.bb );
    bool p2_inside = Inside( p2, lb.bb );
    if ( p1_inside && p1_inc ) lb.weight1 += 1;
    if ( p2_inside && p2_inc ) lb.weight1 += 1;
    if ( p1_inside && p2_inside ) {
      c1 = p1;
      c2 = p2;
    } else {
      int c = ClipLine( c1, c2, p1, p2, lb.bb );
      if ( p1_inside || p2_inside ) {
        if ( c != 1 ) continue;
        c2 = p1_inside ? p1 : p2;
      } else {
        if ( c != 2 ) continue;
      }
    }
    double dx = c1.x - c2.x;
    double dy = c1.y - c2.y;
    lb.weight2 += std::sqrt( dx*dx + dy*dy );
  }
}

////////////////////////////////////////////////////////////////////////////////

void Series::DetermineVisualProperties( void )
{
  marker_show = false;
  marker_show_out = false;
  marker_show_int = false;
  has_line = false;
  has_fill = false;

  if (
    type == SeriesType::XY ||
    type == SeriesType::Line ||
    type == SeriesType::Lollipop ||
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    has_line = line_width > 0 && !line_color.IsClear();
  }

  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    has_fill = !fill_color.IsClear();
  }

  if (
    type != SeriesType::XY &&
    type != SeriesType::Scatter &&
    type != SeriesType::Line &&
    type != SeriesType::Point &&
    type != SeriesType::Lollipop &&
    type != SeriesType::Area &&
    type != SeriesType::StackedArea
  ) {
    return;
  }

  U radius = marker_size / 2;

  auto compute = [&]( MarkerDims& m, U delta )
  {
    switch ( marker_shape ) {
      case MarkerShape::Square :
        m.x1 = -1.0000 * (0.9 * radius + delta);
        m.x2 = +1.0000 * (0.9 * radius + delta);
        m.y1 = -1.0000 * (0.9 * radius + delta);
        m.y2 = +1.0000 * (0.9 * radius + delta);
        break;
      case MarkerShape::Triangle :
        m.x1 = -1.7320 * (0.7 * radius + delta);
        m.x2 = +1.7320 * (0.7 * radius + delta);
        m.y1 = -2.0000 * (0.7 * radius + delta);
        m.y2 = +1.0000 * (0.7 * radius + delta);
        break;
      case MarkerShape::Diamond :
        m.x1 = -1.4142 * (0.9 * radius + delta);
        m.x2 = +1.4142 * (0.9 * radius + delta);
        m.y1 = -1.4142 * (0.9 * radius + delta);
        m.y2 = +1.4142 * (0.9 * radius + delta);
        break;
      default :
        m.x1 = -1.0000 * (1.0 * radius + delta);
        m.x2 = +1.0000 * (1.0 * radius + delta);
        m.y1 = -1.0000 * (1.0 * radius + delta);
        m.y2 = +1.0000 * (1.0 * radius + delta);
        break;
    }
    return;
  };

  U lw = line_width;

  if ( radius > 0 ) {
    marker_show_out = !line_color.IsClear() && line_width > 0;
    marker_show_int = !fill_color.IsClear();
    if ( 2 * radius < 3 * line_width ) {
      if ( has_line && 2 * radius < line_width ) {
        lw = line_width / 2 - radius;
        radius = line_width / 2;
        marker_show_out = marker_show_out && lw > 0;
      } else {
        marker_show_int = marker_show_int && !marker_show_out;
      }
    }
  }
  marker_show = marker_show_out || marker_show_int;
  if ( !marker_show_out || !marker_show_int ) lw = 0;

  compute( marker_int, -lw );
  compute( marker_out, 0 );

  return;
}

//------------------------------------------------------------------------------

void Series::BuildMarker( Group* g, const MarkerDims& m, SVG::Point p )
{
  Poly* poly;

  switch ( marker_shape ) {
    case MarkerShape::Circle :
      g->Add( new Circle( p, m.x2 ) );
      break;
    case MarkerShape::Square :
      g->Add( new Rect(
        p.x + m.x1, p.y + m.y1,
        p.x + m.x2, p.y + m.y2
      ) );
      break;
    case MarkerShape::Triangle :
      poly =
        new Poly(
          { p.x, p.y + m.y1,
            p.x + m.x2, p.y + m.y2,
            p.x + m.x1, p.y + m.y2
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    case MarkerShape::Diamond :
      poly =
        new Poly(
          { p.x + m.x2, p.y,
            p.x, p.y + m.y2,
            p.x + m.x1, p.y,
            p.x, p.y + m.y1
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    default :
      break;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

int Series::GetStackDir( Axis* y_axis )
{
  if ( type != SeriesType::StackedArea ) {
    return 0;
  }
  double sum = 0;
  for ( const Datum& datum : datum_list ) {
    if ( y_axis->Valid( datum.y ) ) sum += datum.y - base;
  }
  return (sum < 0) ? -1 : 1;
}

////////////////////////////////////////////////////////////////////////////////

void Series::BuildArea(
  const SVG::BoundaryBox& clip_box,
  Group* fill_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg,
  std::vector< SVG::Point >* pts_pos,
  std::vector< SVG::Point >* pts_neg
)
{
  int stack_dir = GetStackDir( y_axis );

  // Normalize number of elements in datum_list by inserting invalid values
  // before and after the defined values as needed.
  {
    if ( !datum_list.empty() ) {
      size_t n = datum_list[ 0 ].x;
      if ( n > 0 ) {
        datum_list.resize( datum_list.size() + n );
        std::move_backward(
          datum_list.begin(),
          datum_list.begin() + datum_list.size() - n,
          datum_list.end()
        );
        for ( size_t i = 0; i < n; i++ ) {
          datum_list[ i ] = Datum( i, num_invalid );
        }
      }
    }
    size_t n = ofs_pos->size();
    for ( size_t i = datum_list.size(); i < n; i++ ) {
      datum_list.emplace_back( i, num_invalid );
    }
  }

  Poly* fill_obj = nullptr;
  Poly* line_obj = nullptr;

  bool first_in_stack = (stack_dir < 0) ? pts_neg->empty() : pts_pos->empty();

  // Initialize the fill polygon with the points from the top of the previous
  // polygon, which are contained in pts_pos/pts_neg.
  if ( has_fill ) {
    fill_g->Add( fill_obj = new Poly() );
    if ( stack_dir < 0 ) {
      for ( auto it = pts_neg->rbegin(); it != pts_neg->rend(); ++it ) {
        fill_obj->Add( *it );
      }
    } else {
      for ( auto it = pts_pos->rbegin(); it != pts_pos->rend(); ++it ) {
        fill_obj->Add( *it );
      }
    }
  }
  if ( stack_dir < 0 ) {
    pts_neg->clear();
  } else {
    pts_pos->clear();
  }

  Point ap_prv_p;
  size_t ap_line_cnt = 0;
  auto add_point = [&]( Point p, bool is_datum, bool on_line )
  {
    if ( on_line ) {
      UpdateLegendBoxes(
        lb_list, (ap_line_cnt == 0) ? p : ap_prv_p, p, false, is_datum
      );
    }
    if ( stack_dir < 0 ) {
      pts_neg->push_back( p );
    } else {
      pts_pos->push_back( p );
    }
    if ( has_fill ) {
      fill_obj->Add( p );
    }
    if ( has_line && on_line ) {
      if ( ap_line_cnt == 0 ) line_g->Add( line_obj = new Poly() );
      line_obj->Add( p );
    }
    if ( is_datum && marker_show ) {
      if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
      if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
    }
    if ( on_line && (is_datum || ap_line_cnt == 0) ) {
      ap_line_cnt++;
    } else {
      ap_line_cnt = 0;
    }
    ap_prv_p = p;
    return;
  };

  Point dp_prv_p;
  bool dp_prv_on_line = false;
  bool dp_prv_inside = false;
  bool dp_first = true;
  auto do_point = [&]( Point p, bool on_line = true )
  {
    if ( x_axis->angle != 0 ) std::swap( p.x, p.y );
    bool inside = Inside( p, clip_box );
    if ( dp_first ) {
      if ( inside ) {
        add_point( p, on_line, on_line );
      }
    } else {
      if ( dp_prv_inside && inside ) {
        // Common case when we stay inside the chart area.
        add_point( p, on_line, on_line );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, dp_prv_p, p, clip_box );
        if ( dp_prv_inside ) {
          // We went from inside to now outside.
          if ( n == 1 ) add_point( c1, false, on_line && dp_prv_on_line );
        } else
        if ( inside ) {
          // We went from outside to now inside.
          if ( n == 1 ) add_point( c1, false, on_line && dp_prv_on_line );
          add_point( p, on_line, on_line );
        } else
        if ( n == 2 ) {
          // We are still outside, but the line segment passes through the
          // chart area.
          add_point( c1, false, on_line && dp_prv_on_line );
          add_point( c2, false, on_line && dp_prv_on_line );
        }
      }
    }
    if ( !inside ) {
      add_point( MoveInside( p, clip_box ), false, false );
    }
    dp_prv_p = p;
    dp_prv_on_line = on_line;
    dp_prv_inside = inside;
    dp_first = false;
    return;
  };

  if ( !datum_list.empty() ) {
    Point beg_p{
      x_axis->Coor( 0 ),
      y_axis->Coor( (stack_dir < 0) ? ofs_neg->front() : ofs_pos->front() )
    };
    Point end_p{
      x_axis->Coor( ofs_pos->size() - 1 ),
      y_axis->Coor( (stack_dir < 0) ? ofs_neg->back() : ofs_pos->back() )
    };
    if ( first_in_stack ) do_point( beg_p, false );
    double prv_base = 0;
    bool prv_valid = false;
    bool first = true;
    for ( const Datum& datum : datum_list ) {
      size_t i = datum.x;
      double y = datum.y;
      if ( y_axis->Skip( datum.y ) ) {
        continue;
      }
      bool valid = y_axis->Valid( y );
      y -= base;
      if ( !first && prv_valid && !valid ) {
        Point p{ x_axis->Coor( datum.x - 1 ), y_axis->Coor( prv_base ) };
        do_point( p, false );
      }
      if ( !valid ) y = 0;
      if ( stack_dir < 0 ) {
        prv_base = ofs_neg->at( i );
        y += prv_base;
        ofs_neg->at( i ) = y;
      } else {
        prv_base = ofs_pos->at( i );
        y += prv_base;
        ofs_pos->at( i ) = y;
      }
      if ( !first && !prv_valid && valid ) {
        Point p{ x_axis->Coor( datum.x ), y_axis->Coor( prv_base ) };
        do_point( p, false );
      }
      Point p{ x_axis->Coor( datum.x ), y_axis->Coor( y ) };
      do_point( p, valid );
      prv_valid = valid;
      first = false;
    }
    if ( first_in_stack ) do_point( end_p, false );
  }

  return;
}

//------------------------------------------------------------------------------

void Series::BuildBar(
  const SVG::BoundaryBox& clip_box,
  Group* fill_g,
  Group* tbar_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  double wx;    // Width of bar.
  double cx;    // Center of bar.
  {
    double sa = 1.0 - bar_all_width;
    double so = 1.0 - bar_one_width;
    so = bar_all_width * so / (bar_tot - so);
    if ( sa < so ) {
      so = (1 - bar_one_width) / bar_tot;
      sa = so;
    }
    wx = (1.0 - sa + so) / bar_tot - so;
    cx = sa/2 - 0.5 + wx/2 + bar_num * (wx + so);
  }

  Point p1;
  Point p2;

  U db = std::min( 1.0, line_width / 2 );

  for ( const Datum& datum : datum_list ) {
    size_t i = datum.x;
    double x = datum.x + cx;
    bool valid = y_axis->Valid( datum.y );
    if ( !valid || datum.y == base ) continue;

    p1.x = x_axis->Coor( x );
    p2.x = p1.x;
    if ( type == SeriesType::Lollipop ) {
      p1.y = y_axis->Coor( base );
      p2.y = y_axis->Coor( datum.y );
    } else {
      double y = datum.y - base;
      if ( y < 0 ) {
        p1.y = y_axis->Coor( ofs_neg->at( i ) );
        ofs_neg->at( i ) += y;
        p2.y = y_axis->Coor( ofs_neg->at( i ) );
      } else {
        p1.y = y_axis->Coor( ofs_pos->at( i ) );
        ofs_pos->at( i ) += y;
        p2.y = y_axis->Coor( ofs_pos->at( i ) );
      }
    }
    if ( x_axis->angle != 0 ) {
      std::swap( p1.x, p1.y );
      std::swap( p2.x, p2.y );
    }

    bool p1_inside = Inside( p1, clip_box );
    bool p2_inside = Inside( p2, clip_box );
    if ( !p1_inside || !p2_inside ) {
      Point c1, c2;
      int n = ClipLine( c1, c2, p1, p2, clip_box );
      if ( p1_inside ) {
        if ( n != 1 ) continue;
        p2 = c1;
      } else
      if ( p2_inside ) {
        if ( n != 1 ) continue;
        p1 = c1;
      } else
      {
        if ( n != 2 ) continue;
        p1 = c1;
        p2 = c2;
      }
    }

    if ( type == SeriesType::Lollipop ) {
      line_g->Add( new Line( p1, p2 ) );
      if ( p2_inside && marker_show ) {
        if ( marker_show_out ) BuildMarker( mark_g, marker_out, p2 );
        if ( marker_show_int ) BuildMarker( hole_g, marker_int, p2 );
      }
      UpdateLegendBoxes( lb_list, p1, p2, false, true );
    }

    if ( type == SeriesType::Bar || type == SeriesType::StackedBar ) {
      U d = std::abs( x_axis->Coor( wx / 2 ) - x_axis->Coor( 0 ) );
      bool cut_bot = false;
      bool cut_top = false;
      bool cut_lft = false;
      bool cut_rgt = false;
      if ( x_axis->angle == 0 ) {
        p1.x -= d;
        p2.x += d;
        if ( p1.y < p2.y ) {
          if ( !p1_inside ) cut_bot = true;
          if ( !p2_inside ) cut_top = true;
        }
        if ( p1.y > p2.y ) {
          if ( !p1_inside ) cut_top = true;
          if ( !p2_inside ) cut_bot = true;
        }
      } else {
        p1.y -= d;
        p2.y += d;
        if ( p1.x < p2.x ) {
          if ( !p1_inside ) cut_lft = true;
          if ( !p2_inside ) cut_rgt = true;
        }
        if ( p1.x > p2.x ) {
          if ( !p1_inside ) cut_rgt = true;
          if ( !p2_inside ) cut_lft = true;
        }

      }
      if ( p1.x > p2.x ) std::swap( p1.x, p2.x );
      if ( p1.y > p2.y ) std::swap( p1.y, p2.y );
      UpdateLegendBoxes( lb_list, Point( p1.x, p1.y ), Point( p1.x, p2.y ) );
      UpdateLegendBoxes( lb_list, Point( p1.x, p1.y ), Point( p2.x, p1.y ) );
      UpdateLegendBoxes( lb_list, Point( p2.x, p1.y ), Point( p2.x, p2.y ) );
      UpdateLegendBoxes( lb_list, Point( p1.x, p2.y ), Point( p2.x, p2.y ) );
      bool has_interior =
        p2.x - p1.x > line_width + 2 &&
        p2.y - p1.y > line_width + 2;
      if ( has_interior ) {
        if ( has_fill ) {
          Point c1{ p1 };
          Point c2{ p2 };
          if ( !cut_lft ) c1.x += db;
          if ( !cut_rgt ) c2.x -= db;
          if ( !cut_bot ) c1.y += db;
          if ( !cut_bot ) c2.y -= db;
          fill_g->Add( new Rect( c1, c2 ) );
        }
        if ( has_line ) {
          U d = line_width / 2;
          if ( cut_bot && cut_top ) {
            line_g->Add( new Line( p1.x + d, p1.y, p1.x + d, p2.y ) );
            line_g->Add( new Line( p2.x - d, p1.y, p2.x - d, p2.y ) );
            continue;
          }
          if ( cut_lft && cut_rgt ) {
            line_g->Add( new Line( p1.x, p1.y + d, p2.x, p1.y + d ) );
            line_g->Add( new Line( p1.x, p2.y - d, p2.x, p2.y - d ) );
            continue;
          }
          if ( cut_bot ) {
            line_g->Add( new Poly(
              { p1.x + d, p1.y, p1.x + d, p2.y - d,
                p2.x - d, p2.y - d, p2.x - d, p1.y
              }
            ) );
            continue;
          }
          if ( cut_top ) {
            line_g->Add( new Poly(
              { p1.x + d, p2.y, p1.x + d, p1.y + d,
                p2.x - d, p1.y + d, p2.x - d, p2.y
              }
            ) );
            continue;
          }
          if ( cut_lft ) {
            line_g->Add( new Poly(
              { p1.x, p1.y + d, p2.x - d, p1.y + d,
                p2.x - d, p2.y - d, p1.x, p2.y - d
              }
            ) );
            continue;
          }
          if ( cut_rgt ) {
            line_g->Add( new Poly(
              { p2.x, p1.y + d, p1.x + d, p1.y + d,
                p1.x + d, p2.y - d, p2.x, p2.y - d
              }
            ) );
            continue;
          }
          line_g->Add( new Rect( p1.x + d, p1.y + d, p2.x - d, p2.y - d ) );
        }
      } else {
        tbar_g->Add( new Rect( p1, p2 ) );
      }
    }

  }

  return;
}

//------------------------------------------------------------------------------

void Series::BuildLine(
  const SVG::BoundaryBox& clip_box,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list
)
{
  Poly* poly = nullptr;
  bool adding_segments = false;

  Point prv;
  auto add_point = [&]( Point p, bool clipped = false )
  {
    if ( has_line ) {
      if ( !adding_segments ) line_g->Add( poly = new Poly() );
      poly->Add( p );
      if ( adding_segments ) {
        UpdateLegendBoxes( lb_list, prv, p );
      }
    } else {
      UpdateLegendBoxes( lb_list, p, p, true, false );
    }
    if ( !clipped && marker_show ) {
      if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
      if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
    }
    prv = p;
    adding_segments = true;
  };
  auto end_point = [&]( void )
  {
    poly = nullptr;
    adding_segments = false;
  };

  bool first = true;
  Point cur;
  Point old;
  for ( const Datum& datum : datum_list ) {
    old = cur;
    if ( x_axis->angle == 0 ) {
      cur.x = x_axis->Coor( datum.x );
      cur.y = y_axis->Coor( datum.y );
    } else {
      cur.y = x_axis->Coor( datum.x );
      cur.x = y_axis->Coor( datum.y );
    }
    bool valid = x_axis->Valid( datum.x ) && y_axis->Valid( datum.y );
    bool inside = Inside( cur, clip_box );
    if ( !valid ) {
      if (
        x_axis->Skip( datum.x ) ||
        (x_axis->Valid( datum.x ) && y_axis->Skip( datum.y ))
      ) {
        cur = old;
      } else {
        end_point();
        first = true;
      }
    } else
    if ( first ) {
      if ( inside ) {
        add_point( cur );
      }
      first = false;
    } else {
      if ( adding_segments && inside ) {
        // Common case when we stay inside the chart area.
        add_point( cur );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, old, cur, clip_box );
        if ( !adding_segments ) {
          // We were outside.
          if ( inside ) {
            // We went from outside to now inside.
            if ( n == 1 ) add_point( c1, true );
            add_point( cur );
          } else {
            if ( n == 2 ) {
              // We are still outside, but the line segment passes through the
              // chart area.
              add_point( c1, true );
              add_point( c2, true );
              end_point();
            }
          }
        } else {
          // We went from inside to now outside.
          if ( n == 1 ) add_point( c1, true );
          end_point();
        }
      }
    }
  }
  end_point();
}

//------------------------------------------------------------------------------

void Series::Build(
  SVG::Group* main_g,
  SVG::Group* stacked_area_fill_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg,
  std::vector< SVG::Point >* pts_pos,
  std::vector< SVG::Point >* pts_neg
)
{
  // Define clip-box.
  SVG::BoundaryBox clip_box;
  if ( x_axis->angle == 0 ) {
    clip_box.Update( x_axis->length, 0 );
    clip_box.Update( 0, y_axis->length );
  } else {
    clip_box.Update( 0, x_axis->length );
    clip_box.Update( y_axis->length, 0 );
  }

  // Used for extra margin in comparisons to account for precision issues. This
  // may cause an unintended extra clip-detection near the corners, but the
  // points will in that case be very near each other and will thus be detected
  // by near(). Use different epsilons for near-detection and clip-detection in
  // order to ensure that any spurious clip-detection will be caught by the
  // more inclusive near-detection.
  e1 =
    std::max(
      clip_box.max.x - clip_box.min.x,
      clip_box.max.y - clip_box.min.y
    ) * epsilon;
  e2 = e1 * 0.1;

  // Enlarge the clip-box a little bit to ensure that points lying exactly at
  // the boundary are not excluded due to precision issues.
  clip_box.min.x -= e1;
  clip_box.min.y -= e1;
  clip_box.max.x += e1;
  clip_box.max.y += e1;

  Group* fill_g = nullptr;
  Group* tbar_g = nullptr;
  Group* line_g = nullptr;
  Group* mark_g = nullptr;
  Group* hole_g = nullptr;

  if ( type == SeriesType::StackedArea ) {
    fill_g = stacked_area_fill_g->AddNewGroup();
  } else {
    fill_g = main_g->AddNewGroup();
  }
  ApplyFillStyle( fill_g );

  tbar_g = main_g->AddNewGroup();
  if ( !line_color.IsClear() && line_width > 0 ) {
    tbar_g->Attr()->LineColor()->Clear();
    tbar_g->Attr()->FillColor()->Set( &line_color );
  } else {
    ApplyFillStyle( tbar_g );
  }

  line_g = main_g->AddNewGroup();
  ApplyLineStyle( line_g );

  mark_g = main_g->AddNewGroup();
  ApplyMarkStyle( mark_g );

  hole_g = main_g->AddNewGroup();
  ApplyHoleStyle( hole_g );

  if (
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    BuildArea(
      clip_box,
      fill_g, line_g, mark_g, hole_g,
      x_axis, y_axis,
      lb_list,
      bar_num, bar_tot,
      ofs_pos, ofs_neg,
      pts_pos, pts_neg
    );
  }

  if (
    type == SeriesType::Lollipop ||
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar
  ) {
    BuildBar(
      clip_box,
      fill_g, tbar_g, line_g, mark_g, hole_g,
      x_axis, y_axis,
      lb_list,
      bar_num, bar_tot,
      ofs_pos, ofs_neg
    );
  }

  if (
    type == SeriesType::XY ||
    type == SeriesType::Scatter ||
    type == SeriesType::Line ||
    type == SeriesType::Point
  ) {
    BuildLine(
      clip_box,
      line_g, mark_g, hole_g,
      x_axis, y_axis,
      lb_list
    );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
