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

#include <unordered_set>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Series::Series( Main* main, SeriesType type )
{
  this->main = main;
  id = 0;

  axis_x = nullptr;
  axis_y = nullptr;
  axis_y_n = 0;
  base = 0;
  tag_db = nullptr;
  tag_enable = false;
  tag_pos = Pos::Auto;
  tag_size = 1.0;
  tag_box = false;

  html_db = nullptr;

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

void Series::SetName( const std::string& name )
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
      SetLineDash( 4, 2 );
    }
    if ( style == 2 ) {
      SetLineWidth( 4 );
      SetLineDash( 8, 2 );
    }
    if ( style == 3 ) {
      SetLineWidth( 4 );
      SetLineDash( 16, 2 );
    }
    if ( style == 4 ) {
      SetLineWidth( 2 );
      SetLineDash( 0 );
    }
    if ( style == 5 ) {
      SetLineWidth( 2 );
      SetLineDash( 4, 2 );
    }
    if ( style == 6 ) {
      SetLineWidth( 2 );
      SetLineDash( 8, 2 );
    }
    if ( style == 7 ) {
      SetLineWidth( 2 );
      SetLineDash( 16, 2 );
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
  if ( line_hole == 0 ) line_dash = 0;
  if ( line_dash == 0 ) line_hole = 0;
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
  if (
    marker_shape == MarkerShape::Cross ||
    marker_shape == MarkerShape::LineX ||
    marker_shape == MarkerShape::LineY
  ) {
    ApplyLineStyle( obj );
    obj->Attr()->SetLineDash( 0 );
  } else {
    if ( line_width > 0 ) {
      obj->Attr()->FillColor()->Set( &line_color );
      if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
        obj->Attr()->FillColor()->SetOpacity( 1.0 );
      }
    } else {
      obj->Attr()->FillColor()->Clear();
    }
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

void Series::ApplyTagStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Set( &tag_line_color );
  obj->Attr()->FillColor()->Set( &tag_fill_color );
  obj->Attr()->TextColor()->Set( &tag_text_color );
  obj->Attr()->SetLineWidth( 1 );
  obj->Attr()->TextFont()->SetSize( 12 * tag_size );
  obj->Attr()->TextFont()->SetBold();
}

////////////////////////////////////////////////////////////////////////////////

void Series::PrunePoly( std::vector< Point >& points, bool no_html )
{
  using PI = std::vector< Point >::const_iterator;

  if ( points.size() > 2 && prune_dist >= 0.001 ) {

    size_t idx = 0;

    // p1 and p2 are the start and end points of the collection, which is all
    // points from p1 to p2 both inclusive.
    PI p1;
    PI p2;

    // e1 and e2 are the start/end points of the line making up collection. All
    // points in the collection are spaced less than prune_dist from the line
    // from e1 to e2. Having e1/e2 enables us to prune points even when there is
    // a lot of zigzagging back and forth along (or almost along as dictated by
    // prune_dist) the e1/e2 line, as would be the case for example with noisy
    // sensor data etc.
    PI e1;
    PI e2;

    // d1/d2 is the distance of furthest point in the collection to the
    // left/right from the e1-to-e2 line.
    U d1;
    U d2;

    // Returns the distance from p to the line going from e1 to e2. The sign of
    // the returned distance indicates if p lies to the left (positive) or the
    // right (negative) of the line.
    auto dist2line = []( PI e1, PI e2, PI p )
    {
      double dx = e2->x - e1->x;
      double dy = e2->y - e1->y;
      double px = p->x - e1->x;
      double py = p->y - e1->y;

      double cross = dx * py - dy * px;

      return cross / std::sqrt( dx * dx + dy * dy );
    };

    // Returns true if p was integrated into the p1 to p2 collection thereby
    // causing the previous point (p2) to be pruned.
    auto prune = [&]( PI p )
    {
      auto new_e1 = e1;
      auto new_e2 = e2;

      double vex = e2->x - e1->x;
      double vey = e2->y - e1->y;

      bool vex_tiny = std::abs( vex ) < epsilon;
      bool vey_tiny = std::abs( vey ) < epsilon;

      if ( vex_tiny && vey_tiny ) {
        new_e2 = p;
      } else {
        double dot1 = (p->x - e1->x) * vex + (p->y - e1->y) * vey;
        double dot2 = (p->x - e2->x) * vex + (p->y - e2->y) * vey;
        double d;
        if ( dot1 < 0 || dot2 > 0 ) {
          // p is before/after the current e1 to e2 line, so extend e1 or e2.
          if ( dot2 > 0 ) {
            // Extend e2.
            d = dist2line( e1, p, e2 );
          } else {
            // Swap e1/e2 direction and extend new e2 (previous e1).
            d = dist2line( e2, p, e1 );
            std::swap( d1, d2 );
            new_e1 = e2;
          }
          new_e2 = p;
          // Do not accept pruning that causes vertical/horizontal lines to
          // become slightly skewed, as this is a much more visible artifact:
          if ( (vex_tiny || vey_tiny) && std::abs( d ) > epsilon ) return false;
          // We use the distance form the old e2 to the new extended e1/e2 line
          // and update d1/d2 accordingly. This is not mathematically correct,
          // ideally all points from p1 to p2 should be reexamined. But this
          // heuristic is judged to be a good enough to avoid O(n^2) complexity.
          if ( d > 0 ) {
            d1 = d1 + d;
            d2 = std::max( 0.0, d2 - d );
          } else {
            d1 = std::max( 0.0, d1 + d );
            d2 = d2 - d;
          }
        } else {
          d = dist2line( e1, e2, p );
          if ( d > 0 ) {
            d1 = std::max( +d1, +d );
          } else {
            d2 = std::max( +d2, -d );
          }
        }
        if ( d1 > prune_dist || d2 > prune_dist ) return false;
      }

      e1 = new_e1;
      e2 = new_e2;
      p2 = p;
      return true;
    };

    PI p = points.cbegin();
    p1 = e1 = p++;
    p2 = e2 = p++;
    d1 = d2 = 0;

    while ( p != points.cend() ) {
      if ( !prune( p ) ) {
        points[ idx++ ] = *p1;
        if ( e1 != p1 ) points[ idx++ ] = *e1;
        if ( e2 != p2 ) points[ idx++ ] = *e2;
        p1 = e1 = p2;
        p2 = e2 = p;
        d1 = d2 = 0;
      }
      ++p;
    }

    points[ idx++ ] = *p1;
    if ( e1 != p1 ) points[ idx++ ] = *e1;
    if ( e2 != p2 ) points[ idx++ ] = *e2;
    points[ idx++ ] = *p2;

    points.resize( idx );
  }

  if ( !no_html && html_db ) {
    for ( const auto& p : points ) html_db->DontPruneSnapPoint( p );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::PrunePoints( std::vector< Point >& points )
{
  if ( points.size() > 1 && prune_dist >= 0.001 ) {

    std::unordered_set<
      SVG::Point, HTML::PointHash, HTML::PointEqual
    > mandatory;

    // Make sure extremes are included; for Scatter plot this does not make
    // sense as the points are totally random.
    if ( type != SeriesType::Scatter ) {
      auto pts = points;
      PrunePoly( pts, true );
      for ( auto& p : pts ) {
        mandatory.insert( p );
      }
    }

    std::unordered_set< uint64_t > existing;
    double f = 1.0 / prune_dist;
    size_t idx = 0;
    for ( const auto& p : points ) {
      uint64_t key =
        (static_cast< uint64_t >( p.y * f ) << 32) |
        (static_cast< uint64_t >( p.x * f ) <<  0);
      if ( existing.insert( key ).second || mandatory.count( p ) > 0 ) {
        points[ idx++ ] = p;
      }
    }
    points.resize( idx );
  }

  if ( html_db ) {
    for ( const auto& p : points ) html_db->DontPruneSnapPoint( p );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::Add( double x, double y )
{
  datum_list.emplace_back( x, y );
}

void Series::Add(
  double x, double y,
  const std::string_view tag_x,
  const std::string_view tag_y
)
{
  datum_list.emplace_back( x, y, tag_x, tag_y );
}

////////////////////////////////////////////////////////////////////////////////

bool Series::Inside( const SVG::Point p, const SVG::BoundaryBox& bb )
{
  return
    p.x >= bb.min.x && p.x <= bb.max.x &&
    p.y >= bb.min.y && p.y <= bb.max.y;
}

////////////////////////////////////////////////////////////////////////////////

// Returns:
//   0 : No intersection.
//   1 : One intersection; c1 is the point.
//   2 : Two intersections; c1 and c2 are the points.
int Series::ClipLine(
  SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
  const SVG::BoundaryBox& bb
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
  U bot_x{ 0 }; bool bot_v = false;
  U top_x{ 0 }; bool top_v = false;
  U lft_y{ 0 }; bool lft_v = false;
  U rgt_y{ 0 }; bool rgt_v = false;

  // Detect bottom and top clippings.
  if ( p1.y > p2.y ) std::swap( p1, p2 );
  if ( p1.y < bb.min.y && p2.y >= bb.min.y ) {
    bot_x = intersect_y( bb.min.y, p1, p2 );
    bot_v = bot_x > (bb.min.x - e2) && bot_x < (bb.max.x + e2);
  }
  if ( p1.y <= bb.max.y && p2.y > bb.max.y ) {
    top_x = intersect_y( bb.max.y, p1, p2 );
    top_v = top_x > (bb.min.x - e2) && top_x < (bb.max.x + e2);
  }

  // Detect left and right clippings.
  if ( p1.x > p2.x ) std::swap( p1, p2 );
  if ( p1.x < bb.min.x && p2.x >= bb.min.x ) {
    lft_y = intersect_x( bb.min.x, p1, p2 );
    lft_v = lft_y > (bb.min.y - e2) && lft_y < (bb.max.y + e2);
  }
  if ( p1.x <= bb.max.x && p2.x > bb.max.x ) {
    rgt_y = intersect_x( bb.max.x, p1, p2 );
    rgt_v = rgt_y > (bb.min.y - e2) && rgt_y < (bb.max.y + e2);
  }

  // The four potential clip points.
  Point bot_c{ bot_x, bb.min.y };
  Point top_c{ top_x, bb.max.y };
  Point lft_c{ bb.min.x, lft_y };
  Point rgt_c{ bb.max.x, rgt_y };

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

SVG::Point Series::MoveInside( SVG::Point p )
{
  if ( p.x < chart_area.min.x ) p.x = chart_area.min.x;
  if ( p.x > chart_area.max.x ) p.x = chart_area.max.x;
  if ( p.y < chart_area.min.y ) p.y = chart_area.min.y;
  if ( p.y > chart_area.max.y ) p.y = chart_area.max.y;
  return p;
}

////////////////////////////////////////////////////////////////////////////////

void Series::UpdateLegendBoxes(
  Point p1, Point p2,
  bool p1_inc, bool p2_inc
)
{
  Point c1;
  Point c2;
  for ( LegendBox& lb : *lb_list ) {
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
  line_color_shown = false;
  fill_color_shown = false;

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
    line_color_shown = has_line;
  }

  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    has_fill = !fill_color.IsClear();
    fill_color_shown = has_fill;
  }

  // Minimal tag distance from center of data point.
  tag_dist_x = tag_dist_y = has_line ? (line_width / 2) : 0;

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
      case MarkerShape::Cross :
        m.x1 = -1.0000 * (0.9 * radius + delta);
        m.x2 = +1.0000 * (0.9 * radius + delta);
        m.y1 = -1.0000 * (0.9 * radius + delta);
        m.y2 = +1.0000 * (0.9 * radius + delta);
        break;
      case MarkerShape::Triangle :
        m.x1 = -1.7320 * (0.7 * radius + delta);
        m.x2 = +1.7320 * (0.7 * radius + delta);
        m.y1 = -1.0000 * (0.7 * radius + delta);
        m.y2 = +2.0000 * (0.7 * radius + delta);
        break;
      case MarkerShape::InvTriangle :
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
      case MarkerShape::LineX :
        m.x1 = (axis_x->angle == 0) ? -radius : 0.0;
        m.x2 = (axis_x->angle == 0) ? +radius : 0.0;
        m.y1 = (axis_x->angle == 0) ? 0.0 : -radius;
        m.y2 = (axis_x->angle == 0) ? 0.0 : +radius;
        break;
      case MarkerShape::LineY :
        m.x1 = (axis_x->angle != 0) ? -radius : 0.0;
        m.x2 = (axis_x->angle != 0) ? +radius : 0.0;
        m.y1 = (axis_x->angle != 0) ? 0.0 : -radius;
        m.y2 = (axis_x->angle != 0) ? 0.0 : +radius;
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
    if (
      marker_shape == MarkerShape::Cross ||
      marker_shape == MarkerShape::LineX ||
      marker_shape == MarkerShape::LineY
    ) {
      marker_show_int = false;
    } else {
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
  }
  marker_show = marker_show_out || marker_show_int;
  if ( !marker_show_out || !marker_show_int ) lw = 0;

  if ( marker_show_int ) fill_color_shown = true;
  if ( marker_show_out ) line_color_shown = true;

  compute( marker_int, -lw );
  compute( marker_out, 0 );

  tag_dist_x =
    std::max(
      marker_show ? ((marker_out.x2 - marker_out.x1) / 2) : 0, +tag_dist_x
    );
  tag_dist_y =
    std::max(
      marker_show ? ((marker_out.y2 - marker_out.y1) / 2) : 0, +tag_dist_y
    );

  return;
}

////////////////////////////////////////////////////////////////////////////////

bool Series::SameLegend( Series* s1, Series* s2 )
{
  bool same =
    s1->type             == s2->type &&
    s1->name             == s2->name &&
    s1->marker_show      == s2->marker_show &&
    s1->marker_show_out  == s2->marker_show_out &&
    s1->marker_show_int  == s2->marker_show_int &&
    s1->has_line         == s2->has_line &&
    s1->has_fill         == s2->has_fill &&
    s1->line_color_shown == s2->line_color_shown &&
    s1->fill_color_shown == s2->fill_color_shown;

  if ( same && s1->line_color_shown ) {
    same = s1->line_color == s2->line_color;
  }
  if ( same && s1->fill_color_shown ) {
    same = s1->fill_color == s2->fill_color;
  }

  if ( same && s1->has_line ) {
    same =
      s1->legend_outline == s2->legend_outline &&
      s1->line_width     == s2->line_width &&
      s1->line_dash      == s2->line_dash &&
      s1->line_hole      == s2->line_hole;
  }

  if ( same && s1->marker_show ) {
    same =
      s1->marker_size  == s2->marker_size &&
      s1->marker_shape == s2->marker_shape;
  }

  return same;
}

////////////////////////////////////////////////////////////////////////////////

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
          { p.x, p.y + m.y2,
            p.x + m.x2, p.y + m.y1,
            p.x + m.x1, p.y + m.y1
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    case MarkerShape::InvTriangle :
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
    case MarkerShape::Cross :
      g = g->AddNewGroup();
      g->Add(
        new Line(
          p.x + m.x1, p.y + m.y1,
          p.x + m.x2, p.y + m.y2
        )
      );
      g->Add(
        new Line(
          p.x + m.x2, p.y + m.y1,
          p.x + m.x1, p.y + m.y2
        )
      );
      break;
    case MarkerShape::LineX :
    case MarkerShape::LineY :
      g->Add(
        new Line(
          p.x + m.x1, p.y + m.y1,
          p.x + m.x2, p.y + m.y2
        )
      );
      break;
    default :
      break;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

int Series::GetStackDir( void )
{
  if ( type != SeriesType::StackedArea ) {
    return 0;
  }
  double sum = 0;
  for ( const Datum& datum : datum_list ) {
    if ( axis_y->Valid( datum.y ) ) sum += datum.y - base;
  }
  return (sum < 0) ? -1 : 1;
}

////////////////////////////////////////////////////////////////////////////////

void Series::DetermineMinMax(
  std::vector< double >& ofs_pos,
  std::vector< double >& ofs_neg
)
{
  bool stackable =
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::StackedArea;

  def_x = false;
  min_x = axis_x->log_scale ? 10 : 0;
  max_x = axis_x->log_scale ? 10 : 0;

  def_y = false;
  min_y = axis_y->log_scale ? 10 : 0;
  max_y = axis_y->log_scale ? 10 : 0;
  min_y_is_base = false;
  max_y_is_base = false;

  max_tag_x_size = 0;
  max_tag_y_size = 0;

  int stack_dir = GetStackDir();
  if (
    stackable ||
    type == SeriesType::Lollipop ||
    type == SeriesType::Area
  ) {
    double y = base;
    if ( axis_y->Valid( y ) ) {
      if ( !def_y || min_y > y ) {
        min_y = y;
        min_y_is_base = true;
      }
      if ( !def_y || max_y < y ) {
        max_y = y;
        max_y_is_base = true;
      }
      def_y = true;
    }
  }

  for ( auto& datum : datum_list ) {
    double x = datum.x;
    double y = datum.y;
    if ( !axis_x->Valid( x ) ) continue;
    if ( !axis_y->Valid( y ) ) continue;
    if ( stackable ) {
      size_t i = x;
      y -= base;
      if ( stack_dir < 0 || (stack_dir == 0 && y < 0) ) {
        y += ofs_neg.at( i );
        ofs_neg[ i ] = y;
      } else {
        y += ofs_pos.at( i );
        ofs_pos[ i ] = y;
      }
      if ( !axis_y->Valid( y ) ) continue;
    }
    max_tag_x_size = std::max( max_tag_x_size, datum.tag_x.size() );
    max_tag_y_size = std::max( max_tag_y_size, datum.tag_y.size() );
    if ( !def_x || min_x > x ) min_x = x;
    if ( !def_x || max_x < x ) max_x = x;
    if ( !def_y || min_y > y ) {
      min_y = y;
      min_y_is_base = false;
    }
    if ( !def_y || max_y < y ) {
      max_y = y;
      max_y_is_base = false;
    }
    def_x = true;
    def_y = true;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::BuildArea(
  Group* fill_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg,
  std::vector< SVG::Point >* pts_pos,
  std::vector< SVG::Point >* pts_neg
)
{
  std::vector< Point > fill_points;
  std::vector< Point > line_points;
  std::vector< Point > mark_points;

  int stack_dir = GetStackDir();

  Pos tag_direction;
  bool reverse = axis_y->reverse ^ (stack_dir < 0);
  if ( axis_x->angle == 0 ) {
    tag_direction = reverse ? Pos::Bottom : Pos::Top;
  } else {
    tag_direction = reverse ? Pos::Left : Pos::Right;
  }

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

  // Replace leading/trailing skipped data points in the series with invalid
  // number.
  for ( auto it = datum_list.begin(); it != datum_list.end(); ++it ) {
    if ( axis_y->Valid( it->y ) ) break;
    it->y = num_invalid;
  }
  for ( auto it = datum_list.rbegin(); it != datum_list.rend(); ++it ) {
    if ( axis_y->Valid( it->y ) ) break;
    it->y = num_invalid;
  }

  bool first_in_stack = (stack_dir < 0) ? pts_neg->empty() : pts_pos->empty();

  // Initialize the fill polygon with the points from the top of the previous
  // polygon, which are contained in pts_pos/pts_neg.
  if ( has_fill ) {
    if ( stack_dir < 0 ) {
      for ( auto it = pts_neg->rbegin(); it != pts_neg->rend(); ++it ) {
        fill_points.push_back( *it );
      }
    } else {
      for ( auto it = pts_pos->rbegin(); it != pts_pos->rend(); ++it ) {
        fill_points.push_back( *it );
      }
    }
  }
  if ( stack_dir < 0 ) {
    pts_neg->clear();
  } else {
    pts_pos->clear();
  }

  auto commit_line = [&]( void )
  {
    if ( !line_points.empty() ) {
      PrunePoly( line_points );
      auto it = line_points.cbegin();
      uint64_t max_poly = 1024;
      uint64_t d = (line_points.size() + max_poly - 1) / max_poly;
      uint64_t n = 0;
      for ( uint64_t i = 1; i <= d; ++i ) {
        uint64_t m = line_points.size() * i / d;
        Poly* poly = new Poly();
        line_g->Add( poly );
        while ( n < m ) {
          poly->Add( *(it++) );
          ++n;
        }
      }
      line_points.clear();
    }
  };

  Point ap_prv_p;
  size_t ap_line_cnt = 0;
  auto add_point =
  [&]( Point p, const Datum& datum, bool is_datum, bool on_line )
  {
    if ( on_line ) {
      UpdateLegendBoxes(
        (ap_line_cnt == 0) ? p : ap_prv_p, p, false, is_datum
      );
    }
    if ( stack_dir < 0 ) {
      pts_neg->push_back( p );
    } else {
      pts_pos->push_back( p );
    }
    if ( has_fill ) {
      fill_points.push_back( p );
    }
    if ( has_line && on_line ) {
      line_points.push_back( p );
    }
    if ( is_datum ) {
      if ( marker_show ) mark_points.push_back( p );
      if ( html_db ) {
        html_db->AddSnapPoint( this, p, datum.x, datum.tag_y );
      }
    }
    if ( tag_enable ) {
      tag_db->LineTag(
        this, tag_g, p, datum, is_datum,
        has_line && on_line && ap_line_cnt > 0, tag_direction
      );
    }
    if ( on_line && (is_datum || ap_line_cnt == 0) ) {
      ap_line_cnt++;
    } else {
      commit_line();
      ap_line_cnt = 0;
    }
    ap_prv_p = p;
    return;
  };

  Point dp_prv_p;
  bool dp_prv_on_line = false;
  bool dp_prv_inside = false;
  bool dp_first = true;
  auto do_point = [&]( Point p, const Datum& datum, bool on_line = true )
  {
    if ( axis_x->angle != 0 ) std::swap( p.x, p.y );
    bool inside = Inside( p );
    if ( dp_first ) {
      if ( inside ) {
        add_point( p, datum, on_line, on_line );
      }
    } else {
      if ( dp_prv_inside && inside ) {
        // Common case when we stay inside the chart area.
        add_point( p, datum, on_line, on_line );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, dp_prv_p, p );
        if ( dp_prv_inside ) {
          // We went from inside to now outside.
          if ( n == 1 ) add_point( c1, datum, false, on_line && dp_prv_on_line );
        } else
        if ( inside ) {
          // We went from outside to now inside.
          if ( n == 1 ) add_point( c1, datum, false, on_line && dp_prv_on_line );
          add_point( p, datum, on_line, on_line );
        } else
        if ( n == 2 ) {
          // We are still outside, but the line segment passes through the
          // chart area.
          add_point( c1, datum, false, on_line && dp_prv_on_line );
          add_point( c2, datum, false, on_line && dp_prv_on_line );
        }
      }
    }
    if ( !inside ) {
      add_point( MoveInside( p ), datum, false, false );
    }
    dp_prv_p = p;
    dp_prv_on_line = on_line;
    dp_prv_inside = inside;
    dp_first = false;
    return;
  };

  if ( !datum_list.empty() ) {
    Datum dummy_datum;
    Point beg_p{
      axis_x->Coor( 0 ),
      axis_y->Coor( (stack_dir < 0) ? ofs_neg->front() : ofs_pos->front() )
    };
    Point end_p{
      axis_x->Coor( ofs_pos->size() - 1 ),
      axis_y->Coor( (stack_dir < 0) ? ofs_neg->back() : ofs_pos->back() )
    };
    if ( first_in_stack ) do_point( beg_p, dummy_datum, false );
    double prv_base = 0;
    bool prv_valid = false;
    bool first = true;
    for ( const Datum& datum : datum_list ) {
      size_t i = datum.x;
      double y = datum.y;
      if ( axis_y->Skip( datum.y ) ) {
        continue;
      }
      bool valid = axis_y->Valid( y );
      y -= base;
      if ( !first && prv_valid && !valid ) {
        Point p{ axis_x->Coor( datum.x - 1 ), axis_y->Coor( base ) };
        do_point( p, datum, false );
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
        Point p{ axis_x->Coor( datum.x ), axis_y->Coor( base ) };
        do_point( p, datum, false );
      }
      Point p{ axis_x->Coor( datum.x ), axis_y->Coor( y ) };
      do_point( p, datum, valid );
      prv_valid = valid;
      first = false;
    }
    if ( first_in_stack ) do_point( end_p, dummy_datum, false );
  }

  if ( !fill_points.empty() ) {
    PrunePoly( fill_points );
    Poly* poly = new Poly();
    fill_g->Add( poly );
    for ( auto& p : fill_points ) {
      poly->Add( p );
    }
    poly->Close();
  }

  commit_line();

  if ( !mark_points.empty() ) {
    PrunePoints( mark_points );
    for ( auto& p : mark_points ) {
      if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
      if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Series::BuildBar(
  Group* fill_g,
  Group* tbar_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  Pos zero_direction = Pos::Auto;
  {
    bool has_pos_bar = false;
    bool has_neg_bar = false;
    for ( const Datum& datum : datum_list ) {
      if ( datum.y - base > 0 ) has_pos_bar = true;
      if ( datum.y - base < 0 ) has_neg_bar = true;
    }
    if ( axis_x->angle == 0 ) {
      if ( axis_y->reverse ) {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Bottom : Pos::Top;
      } else {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Top : Pos::Bottom;
      }
    } else {
      if ( axis_y->reverse ) {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Left : Pos::Right;
      } else {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Right : Pos::Left;
      }
    }
  }

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

  for ( const Datum& datum : datum_list ) {
    size_t i = datum.x;
    double x = datum.x + cx;
    bool valid = axis_y->Valid( datum.y );
    if ( !valid ) continue;

    U q = axis_x->Coor( x );
    p1.x = p2.x = q;
    if ( type == SeriesType::Lollipop ) {
      p1.y = axis_y->Coor( base );
      p2.y = axis_y->Coor( datum.y );
    } else {
      double y = datum.y - base;
      if ( y < 0 ) {
        p1.y = axis_y->Coor( ofs_neg->at( i ) );
        ofs_neg->at( i ) += y;
        p2.y = axis_y->Coor( ofs_neg->at( i ) );
      } else {
        p1.y = axis_y->Coor( ofs_pos->at( i ) );
        ofs_pos->at( i ) += y;
        p2.y = axis_y->Coor( ofs_pos->at( i ) );
      }
    }
    if ( axis_x->angle != 0 ) {
      std::swap( p1.x, p1.y );
      std::swap( p2.x, p2.y );
    }

    bool p1_inside = Inside( p1 );
    bool p2_inside = Inside( p2 );
    if ( !p1_inside || !p2_inside ) {
      Point c1, c2;
      int n = ClipLine( c1, c2, p1, p2 );
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
      if ( axis_x->angle == 0 ) {
        p1.x = p2.x = q;
      } else {
        p1.y = p2.y = q;
      }
    }

    if ( html_db && p2_inside ) {
      html_db->AddSnapPoint( this, p2, datum.x, datum.tag_y );
      html_db->DontPruneSnapPoint( p2 );
    }

    if ( tag_enable ) {
      Pos direction = zero_direction;
      if ( p2.x > p1.x ) direction = Pos::Right;
      if ( p2.x < p1.x ) direction = Pos::Left;
      if ( p2.y > p1.y ) direction = Pos::Top;
      if ( p2.y < p1.y ) direction = Pos::Bottom;
      tag_db->BarTag( this, tag_g, p1, p2, datum, direction );
    }

    if ( type == SeriesType::Lollipop ) {
      line_g->Add( new Line( p1, p2 ) );
      if ( p2_inside && marker_show ) {
        if ( marker_show_out ) BuildMarker( mark_g, marker_out, p2 );
        if ( marker_show_int ) BuildMarker( hole_g, marker_int, p2 );
      }
      UpdateLegendBoxes( p1, p2, false, true );
    }

    if (
      datum.y != base &&
      (type == SeriesType::Bar || type == SeriesType::StackedBar)
    ) {
      U w = std::abs( axis_x->Coor( wx / 2 ) - axis_x->Coor( 0 ) );
      bool cut_bot = false;
      bool cut_top = false;
      bool cut_lft = false;
      bool cut_rgt = false;
      if ( axis_x->angle == 0 ) {
        p1.x -= w;
        p2.x += w;
        if ( p1.y < p2.y ) {
          if ( !p1_inside ) cut_bot = true;
          if ( !p2_inside ) cut_top = true;
          cut_bot = true;
        }
        if ( p1.y > p2.y ) {
          if ( !p1_inside ) cut_top = true;
          if ( !p2_inside ) cut_bot = true;
          cut_top = true;
        }
      } else {
        p1.y -= w;
        p2.y += w;
        if ( p1.x < p2.x ) {
          if ( !p1_inside ) cut_lft = true;
          if ( !p2_inside ) cut_rgt = true;
          cut_lft = true;
        }
        if ( p1.x > p2.x ) {
          if ( !p1_inside ) cut_rgt = true;
          if ( !p2_inside ) cut_lft = true;
          cut_rgt = true;
        }
      }
      if ( p1.x > p2.x ) std::swap( p1.x, p2.x );
      if ( p1.y > p2.y ) std::swap( p1.y, p2.y );
      UpdateLegendBoxes( Point( p1.x, p1.y ), Point( p1.x, p2.y ) );
      UpdateLegendBoxes( Point( p1.x, p1.y ), Point( p2.x, p1.y ) );
      UpdateLegendBoxes( Point( p2.x, p1.y ), Point( p2.x, p2.y ) );
      UpdateLegendBoxes( Point( p1.x, p2.y ), Point( p2.x, p2.y ) );
      bool has_interior =
        p2.x - p1.x > line_width + 2 &&
        p2.y - p1.y > line_width + 2;
      if ( has_interior ) {
        if ( has_fill ) {
          Point c1{ p1 };
          Point c2{ p2 };
          if ( has_line ) {
            U d = std::min( 0.25, line_width / 2 );
            U q = (line_dash > 0) ? +d : (line_width / 2);
            c1.x += cut_lft ? -d : +q;
            c2.x -= cut_rgt ? -d : +q;
            c1.y += cut_bot ? -d : +q;
            c2.y -= cut_top ? -d : +q;
          }
          fill_g->Add( new Rect( c1, c2 ) );
        }
        if ( has_line ) {
          U d = line_width / 2;
          U q = std::min( 0.25, +d );
          if ( cut_bot && cut_top ) {
            line_g->Add( new Line( p1.x + d, p1.y - q, p1.x + d, p2.y + q ) );
            line_g->Add( new Line( p2.x - d, p1.y - q, p2.x - d, p2.y + q ) );
          } else
          if ( cut_lft && cut_rgt ) {
            line_g->Add( new Line( p1.x - q, p1.y + d, p2.x + q, p1.y + d ) );
            line_g->Add( new Line( p1.x - q, p2.y - d, p2.x + q, p2.y - d ) );
          } else
          if ( cut_bot ) {
            line_g->Add( new Poly(
              { p1.x + d, p1.y - q, p1.x + d, p2.y - d,
                p2.x - d, p2.y - d, p2.x - d, p1.y - q
              }
            ) );
          } else
          if ( cut_top ) {
            line_g->Add( new Poly(
              { p1.x + d, p2.y + q, p1.x + d, p1.y + d,
                p2.x - d, p1.y + d, p2.x - d, p2.y + q
              }
            ) );
          } else
          if ( cut_lft ) {
            line_g->Add( new Poly(
              { p1.x - q, p1.y + d, p2.x - d, p1.y + d,
                p2.x - d, p2.y - d, p1.x - q, p2.y - d
              }
            ) );
          } else
          if ( cut_rgt ) {
            line_g->Add( new Poly(
              { p2.x + q, p1.y + d, p1.x + d, p1.y + d,
                p1.x + d, p2.y - d, p2.x + q, p2.y - d
              }
            ) );
          } else {
            line_g->Add( new Rect( p1.x + d, p1.y + d, p2.x - d, p2.y - d ) );
          }
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
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g
)
{
  std::vector< Point > line_points;
  std::vector< Point > mark_points;

  bool adding_segments = false;

  Pos tag_direction;
  if ( axis_x->angle == 0 ) {
    tag_direction = axis_y->reverse ? Pos::Bottom : Pos::Top;
  } else {
    tag_direction = axis_y->reverse ? Pos::Left : Pos::Right;
  }

  Point prv;
  auto add_point =
    [&]( Point p, const Datum& datum, bool clipped = false )
  {
    if ( has_line ) {
      line_points.push_back( p );
      if ( adding_segments ) {
        UpdateLegendBoxes( prv, p );
      }
    } else {
      UpdateLegendBoxes( p, p, true, false );
    }
    if ( !clipped ) {
      if ( marker_show ) mark_points.push_back( p );
      if ( html_db ) {
        if ( axis_x->category_axis ) {
          html_db->AddSnapPoint( this, p, datum.x, datum.tag_y );
        } else {
          html_db->AddSnapPoint( this, p, datum.tag_x, datum.tag_y );
        }
      }
    }
    if ( tag_enable ) {
      tag_db->LineTag(
        this, tag_g, p, datum, !clipped,
        adding_segments && has_line, tag_direction
      );
    }
    prv = p;
    adding_segments = true;
  };
  auto end_point = [&]( void )
  {
    if ( !line_points.empty() ) {
      PrunePoly( line_points );
      auto it = line_points.cbegin();
      uint64_t max_poly = 1024;
      uint64_t d = (line_points.size() + max_poly - 1) / max_poly;
      uint64_t n = 0;
      for ( uint64_t i = 1; i <= d; ++i ) {
        uint64_t m = line_points.size() * i / d;
        Poly* poly = new Poly();
        line_g->Add( poly );
        while ( n < m ) {
          poly->Add( *(it++) );
          ++n;
        }
      }
      line_points.clear();
    }
    if ( !mark_points.empty() ) {
      PrunePoints( mark_points );
      for ( auto& p : mark_points ) {
        if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
        if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
      }
      mark_points.clear();
    }
    adding_segments = false;
    tag_db->EndLineTag();
  };

  bool first = true;
  Point cur;
  Point old;
  for ( const Datum& datum : datum_list ) {
    old = cur;
    if ( axis_x->angle == 0 ) {
      cur.x = axis_x->Coor( datum.x );
      cur.y = axis_y->Coor( datum.y );
    } else {
      cur.y = axis_x->Coor( datum.x );
      cur.x = axis_y->Coor( datum.y );
    }
    bool valid = axis_x->Valid( datum.x ) && axis_y->Valid( datum.y );
    bool inside = Inside( cur );
    if ( !valid ) {
      if (
        axis_x->Skip( datum.x ) ||
        (axis_x->Valid( datum.x ) && axis_y->Skip( datum.y ))
      ) {
        cur = old;
      } else {
        end_point();
        first = true;
      }
    } else
    if ( first ) {
      if ( inside ) {
        add_point( cur, datum );
      }
      first = false;
    } else {
      if ( adding_segments && inside ) {
        // Common case when we stay inside the chart area.
        add_point( cur, datum );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, old, cur );
        if ( !adding_segments ) {
          // We were outside.
          if ( inside ) {
            // We went from outside to now inside.
            if ( n == 1 ) add_point( c1, datum, true );
            add_point( cur, datum );
          } else {
            if ( n == 2 ) {
              // We are still outside, but the line segment passes through the
              // chart area.
              add_point( c1, datum, true );
              add_point( c2, datum, true );
              end_point();
            }
          }
        } else {
          // We went from inside to now outside.
          if ( n == 1 ) add_point( c1, datum, true );
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
  SVG::Group* line_g,
  SVG::Group* area_fill_g,
  SVG::Group* marker_g,
  SVG::Group* tag_g,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg,
  std::vector< SVG::Point >* pts_pos,
  std::vector< SVG::Point >* pts_neg
)
{
  // Used for extra margin in comparisons to account for precision issues. This
  // may cause an unintended extra clip-detection near the corners, but the
  // points will in that case be very near each other and will thus be detected
  // by near(). Use different epsilons for near-detection and clip-detection in
  // order to ensure that any spurious clip-detection will be caught by the
  // more inclusive near-detection.
  e1 =
    std::max(
      chart_area.max.x - chart_area.min.x,
      chart_area.max.y - chart_area.min.y
    ) * epsilon;
  e2 = e1 * 0.1;

  // Enlarge the chart area a little bit to ensure that points lying exactly at
  // the boundary are not excluded due to precision issues.
  chart_area.min.x -= e1;
  chart_area.min.y -= e1;
  chart_area.max.x += e1;
  chart_area.max.y += e1;

  Group* fill_g = nullptr;
  Group* tbar_g = nullptr;
  Group* mark_g = nullptr;
  Group* hole_g = nullptr;

  if ( type == SeriesType::Area || type == SeriesType::StackedArea ) {
    fill_g = area_fill_g->AddNewGroup();
  } else {
    fill_g = main_g->AddNewGroup();
  }
  ApplyFillStyle( fill_g );

  // Tiny bars.
  if ( has_line ) {
    tbar_g = line_g->AddNewGroup();
    tbar_g->Attr()->LineColor()->Clear();
    tbar_g->Attr()->FillColor()->Set( &line_color );
  } else {
    tbar_g = main_g->AddNewGroup();
    ApplyFillStyle( tbar_g );
  }

  line_g = line_g->AddNewGroup();
  ApplyLineStyle( line_g );
  if ( type == SeriesType::Bar || type == SeriesType::StackedBar ) {
    line_g->ParrentGroup()->FrontToBack();
  }

  if ( marker_g != nullptr ) {
    mark_g = marker_g->AddNewGroup();
    ApplyMarkStyle( mark_g );
    hole_g = marker_g->AddNewGroup();
    ApplyHoleStyle( hole_g );
  }

  tag_g = tag_g->AddNewGroup();
  ApplyTagStyle( tag_g );

  if (
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    BuildArea(
      fill_g, line_g, mark_g, hole_g, tag_g,
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
      fill_g, tbar_g, line_g, mark_g, hole_g, tag_g,
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
      line_g, mark_g, hole_g, tag_g
    );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
