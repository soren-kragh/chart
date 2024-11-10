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

  SetMarkerSize(
    ( type == SeriesType::Scatter ||
      type == SeriesType::Point ||
      type == SeriesType::Lollipop
    )
    ? 12
    : 0
  );
  SetMarkerShape( MarkerShape::Circle );
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
  style = style / color_list.size();
  style = style % 8;
  if ( type == SeriesType::Area || type == SeriesType::StackedArea ) {
    SetLineWidth( 0 );
    SetLineDash( 0 );
  } else {
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
  if ( size > 0 ) {
    marker_size = size;
  } else {
    marker_size = 0;
  }
}

void Series::SetMarkerShape( MarkerShape shape )
{
  marker_shape = shape;
}

//------------------------------------------------------------------------------

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
  obj->Attr()->FillColor()->Set( &line_color );
}

void Series::ApplyFillStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  if ( fill_color.IsDefined() ) {
    obj->Attr()->FillColor()->Set( &fill_color );
  } else {
    obj->Attr()->FillColor()->Set( &line_color );
    obj->Attr()->FillColor()->Lighten( 0.5 );
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
  const BoundaryBox& clip_box
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

////////////////////////////////////////////////////////////////////////////////

void Series::UpdateLegendBoxes(
  std::vector< LegendBox >& lb_list, Point p1, Point p2
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
    if ( p1_inside ) lb.weight1 += 1;
    if ( p2_inside ) lb.weight1 += 1;
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

void Series::ComputeMarker( SVG::U rim )
{
  marker_show = false;
  marker_diameter = 0;

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

  auto compute = [&]( MarkerDims& m, U delta )
  {
    switch ( marker_shape ) {
      case MarkerShape::Square :
        m.x1 = -1.0000 * (0.9 * marker_radius + delta);
        m.x2 = +1.0000 * (0.9 * marker_radius + delta);
        m.y1 = -1.0000 * (0.9 * marker_radius + delta);
        m.y2 = +1.0000 * (0.9 * marker_radius + delta);
        break;
      case MarkerShape::Triangle :
        m.x1 = -1.7320 * (0.7 * marker_radius + delta);
        m.x2 = +1.7320 * (0.7 * marker_radius + delta);
        m.y1 = -2.0000 * (0.7 * marker_radius + delta);
        m.y2 = +1.0000 * (0.7 * marker_radius + delta);
        break;
      case MarkerShape::Diamond :
        m.x1 = -1.4142 * (0.9 * marker_radius + delta);
        m.x2 = +1.4142 * (0.9 * marker_radius + delta);
        m.y1 = -1.4142 * (0.9 * marker_radius + delta);
        m.y2 = +1.4142 * (0.9 * marker_radius + delta);
        break;
      default :
        m.x1 = -1.0000 * (1.0 * marker_radius + delta);
        m.x2 = +1.0000 * (1.0 * marker_radius + delta);
        m.y1 = -1.0000 * (1.0 * marker_radius + delta);
        m.y2 = +1.0000 * (1.0 * marker_radius + delta);
        break;
    }
    return;
  };

  U line_w = 0;
  if ( !line_color.IsClear() ) {
    line_w = std::max( line_w, line_width );
  }

  marker_diameter = marker_size;
  marker_radius = marker_diameter / 2;
  marker_show =
    marker_size > 0 &&
    ( (type == SeriesType::Scatter || type == SeriesType::Point)
      ? (marker_diameter > 0)
      : (marker_diameter > line_w)
    );
  marker_hollow = marker_diameter >= 3 * line_w;

  compute( marker_int, -line_w );
  compute( marker_out, 0 );
  compute( marker_rim, rim );

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

void Series::BuildArea(
  const SVG::BoundaryBox& clip_box,
  Group* line_g,
  Group* mark_g,
  Group* fill_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  U clamp_coor = y_axis->Coor( base );
  auto clamp = [&]( Point& p )
  {
    if ( x_axis->angle == 0 ) {
      p.y = clamp_coor;
    } else {
      p.x = clamp_coor;
    }
  };

  bool has_area = !fill_g->Attr()->FillColor()->IsClear();
  bool has_line = line_width > 0;

  Poly* area_obj = nullptr;
  Poly* line_obj = nullptr;
  Point prv;
  auto add_point = [&]( Point p, bool clipped, bool part_of_line )
  {
    if ( has_area ) {
      if ( area_obj ) {
        UpdateLegendBoxes( lb_list, prv, p );
      } else {
        fill_g->Add( area_obj = new Poly() );
      }
      area_obj->Add( p );
    }
    if ( has_line && part_of_line ) {
      if ( line_obj ) {
        line_obj->Add( p );
        if ( clipped ) line_obj = nullptr;
      } else {
        line_g->Add( line_obj = new Poly() );
        line_obj->Add( p );
      }
      if ( !clipped && marker_show ) {
        BuildMarker( mark_g, marker_out, p );
        if ( marker_hollow ) {
          BuildMarker( fill_g, marker_int, p );
        }
      }
    } else {
      line_obj = nullptr;
    }
    prv = p;
  };
  auto end_point = [&]( void )
  {
    area_obj = nullptr;
    line_obj = nullptr;
  };

  double sum = 0;
  for ( const Datum& datum : datum_list ) {
    sum += datum.y - base;
  }
  bool first = true;
  Point cur;
  Point old;
  bool cur_valid;
  bool cur_inside;
  bool old_inside;
  size_t n = datum_list.size();
  for ( const Datum& datum : datum_list ) {
    bool rtz = (--n == 0);
    old = cur;
    old_inside = cur_inside;
    size_t i = datum.x;
    double y = datum.y;
    y -= base;
    if ( y < 0 || (y == 0 && sum < 0) ) {
      y += ofs_neg->at( i );
      ofs_neg->at( i ) = y;
    } else {
      y += ofs_pos->at( i );
      ofs_pos->at( i ) = y;
    }

    cur.x = x_axis->Coor( datum.x );
    cur.y = y_axis->Coor( y );
    if ( x_axis->angle != 0 ) std::swap( cur.x, cur.y );
    cur_inside = Inside( cur, clip_box );
    cur_valid = y_axis->Valid( y );

    bool old_part_of_line = true;
    bool cur_part_of_line = true;

    if ( first ) {
      if ( !cur_valid ) continue;
      old = cur;
      clamp( old );
      old_inside = Inside( old, clip_box );
      if ( old_inside ) {
        add_point( old, false, false );
      }
      old_part_of_line = false;
      first = false;
    }

    while ( true ) {
      if ( cur_valid ) {
        if ( old_inside && cur_inside ) {
          // Common case when we stay inside the chart area.
          add_point( cur, false, cur_part_of_line );
        } else {
          // Handle clipping in and out of the chart area.
          Point c1, c2;
          int n = ClipLine( c1, c2, old, cur, clip_box );
          if ( old_inside ) {
            // We went from inside to now outside.
            if ( n == 1 ) add_point( c1, true, old_part_of_line );
          } else
          if ( cur_inside ) {
            // We went from outside to now inside.
            if ( n == 1 ) add_point( c1, true, old_part_of_line );
            add_point( cur, false, cur_part_of_line );
          } else {
            if ( n == 2 ) {
              // We are still outside, but the line segment passes through the
              // chart area.
              add_point( c1, true, old_part_of_line && cur_part_of_line );
              add_point( c2, true, old_part_of_line && cur_part_of_line );
            }
          }
        }
      } else {
        cur = old;
        cur_inside = old_inside;
        cur_valid = true;
        rtz = true;
      }
      if ( rtz ) {
        old = cur;
        old_inside = cur_inside;
        clamp( cur );
        cur_inside = Inside( cur, clip_box );
        rtz = false;
        old_part_of_line = false;
        cur_part_of_line = false;
        continue;
      }
      break;
    }

    if ( !cur_part_of_line ) {
      end_point();
      first = true;
    }

  }

  end_point();

  return;
}

//------------------------------------------------------------------------------

void Series::BuildBar(
  const SVG::BoundaryBox& clip_box,
  Group* line_g,
  Group* mark_g,
  Group* fill_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  double tx = (bar_tot == 1) ? 1.0 : bar_cluster_width;
  double wx = tx / bar_tot;
  double cx = (1 - tx + wx) / 2 + bar_num * wx;
  cx = cx - 0.5;
  wx = wx * bar_width;
  Point p1;
  Point p2;

  bool has_area = !fill_g->Attr()->FillColor()->IsClear();
  U db = std::min( 1.0, line_width / 2 );

  for ( const Datum& datum : datum_list ) {
    size_t i = datum.x;
    double x = datum.x + cx;
    bool valid = x_axis->Valid( x ) && y_axis->Valid( datum.y );
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
        BuildMarker( mark_g, marker_out, p2 );
        if ( marker_hollow ) {
          BuildMarker( fill_g, marker_int, p2 );
        }
      }
      UpdateLegendBoxes( lb_list, p1, p2 );
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
        if ( has_area ) {
          Point c1{ p1 };
          Point c2{ p2 };
          if ( !cut_lft ) c1.x += db;
          if ( !cut_rgt ) c2.x -= db;
          if ( !cut_bot ) c1.y += db;
          if ( !cut_bot ) c2.y -= db;
          fill_g->Add( new Rect( c1, c2 ) );
        }
        if ( line_width > 0 ) {
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
        mark_g->Add( new Rect( p1, p2 ) );
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
  Group* fill_g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list
)
{
  Poly* poly = nullptr;
  bool adding_segments = false;

  bool has_line =
    line_width > 0 &&
    (type == SeriesType::XY || type == SeriesType::Line);

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
      UpdateLegendBoxes( lb_list, p, p );
    }
    if ( !clipped && marker_show ) {
      BuildMarker( mark_g, marker_out, p );
      if ( marker_hollow ) {
        BuildMarker( fill_g, marker_int, p );
      }
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
      end_point();
      first = true;
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
  SVG::Group* g1,
  SVG::Group* g2,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  ComputeMarker();

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

  Group* line_g = nullptr;
  Group* mark_g = nullptr;
  Group* fill_g = nullptr;

  if (
    type == SeriesType::XY ||
    type == SeriesType::Scatter ||
    type == SeriesType::Line ||
    type == SeriesType::Point ||
    type == SeriesType::Lollipop
  ) {
    if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
      line_g = g1->AddNewGroup();
      ApplyLineStyle( line_g );
    }
    if ( marker_show ) {
      mark_g = g1->AddNewGroup();
      ApplyMarkStyle( mark_g );
      if ( marker_hollow ) {
        fill_g = g1->AddNewGroup();
        ApplyFillStyle( fill_g );
      }
    }
  }

  if (
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    line_g = g1->AddNewGroup();
    ApplyLineStyle( line_g );
    mark_g = g1->AddNewGroup();
    ApplyMarkStyle( mark_g );
    Group* fill_g = g2->AddNewGroup();
    ApplyFillStyle( fill_g );
    BuildArea(
      clip_box,
      line_g, mark_g, fill_g,
      x_axis, y_axis,
      lb_list,
      bar_num, bar_tot,
      ofs_pos, ofs_neg
    );
  }

  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar
  ) {
    fill_g = g1->AddNewGroup();
    ApplyFillStyle( fill_g );
    mark_g = g1->AddNewGroup();
    ApplyMarkStyle( mark_g );
    line_g = g1->AddNewGroup();
    ApplyLineStyle( line_g );
  }

  if (
    type == SeriesType::Lollipop ||
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar
  ) {
    BuildBar(
      clip_box,
      line_g, mark_g, fill_g,
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
      line_g, mark_g, fill_g,
      x_axis, y_axis,
      lb_list
    );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
