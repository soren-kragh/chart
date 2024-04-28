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

Series::Series( std::string name )
{
  this->name = name;

  SetType( SeriesType::XY );
  SetAxisY( 0 );
  SetWidth( 1 );
  SetDash( 0 );
  marker_size = -1;     // Negative means auto.
  SetMarkerShape( MarkerShape::Circle );

  color_list.emplace_back(); color_list.back().Set( ColorName::Blue, 0.1 );
  color_list.emplace_back(); color_list.back().Set( ColorName::Red );
  color_list.emplace_back(); color_list.back().Set( ColorName::Green, 0.3 );
  color_list.emplace_back(); color_list.back().Set( ColorName::Cyan, 0, 0.2 );
  color_list.emplace_back(); color_list.back().Set( ColorName::Purple, 0.1 );
  color_list.emplace_back(); color_list.back().Set( ColorName::Blue, 0.6 );
  color_list.emplace_back(); color_list.back().Set( ColorName::Orange );
  color_list.emplace_back(); color_list.back().Set( ColorName::Brown, 0, 0.3 );
}

Series::~Series( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Series::SetType( SeriesType type )
{
  this->type = type;
}

void Series::SetAxisY( int axis_y_n )
{
  this->axis_y_n = axis_y_n;
}

void Series::SetStyle( int style )
{
  if ( style < 64 ) {
    color.Set( &color_list[ style % color_list.size() ] );
    style = style / color_list.size();
  } else {
    color.Set( ColorName::Black );
  }
  style = style % 8;
  if ( style == 0 ) {
    SetWidth( 4 );
    SetDash( 0 );
  }
  if ( style == 1 ) {
    SetWidth( 4 );
    SetDash( 5, 3 );
  }
  if ( style == 2 ) {
    SetWidth( 4 );
    SetDash( 10, 6 );
  }
  if ( style == 3 ) {
    SetWidth( 4 );
    SetDash( 20, 12 );
  }
  if ( style == 4 ) {
    SetWidth( 2 );
    SetDash( 0 );
  }
  if ( style == 5 ) {
    SetWidth( 2 );
    SetDash( 5, 3 );
  }
  if ( style == 6 ) {
    SetWidth( 2 );
    SetDash( 10, 6 );
  }
  if ( style == 7 ) {
    SetWidth( 2 );
    SetDash( 20, 12 );
  }
}

void Series::SetWidth( SVG::U width )
{
  this->width = width;
}

void Series::SetDash( SVG::U dash )
{
  this->dash = dash;
  this->hole = 0;
}

void Series::SetDash( SVG::U dash, SVG::U hole )
{
  this->dash = dash;
  this->hole = hole;
}

void Series::SetMarkerSize( SVG::U marker_size )
{
  this->marker_size = marker_size;
}

void Series::SetMarkerShape( MarkerShape marker_shape )
{
  this->marker_shape = marker_shape;
}

void Series::ApplyStyle( SVG::Object* obj )
{
  obj->Attr()->SetLineWidth( width );
  if ( dash > 0 ) {
    obj->Attr()->SetLineDash( dash, (hole > 0) ? hole : dash );
  }
  obj->Attr()->LineColor()->Set( &color );
}

////////////////////////////////////////////////////////////////////////////////

void Series::Add( double x, double y )
{
  datum_list.emplace_back( x, y );
}

////////////////////////////////////////////////////////////////////////////////

// Returns:
//   0 : No intersection.
//   1 : One intersection; c1 is the point.
//   2 : Two intersections; c1 and c2 are the points.
int Series::ClipLine(
  SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
  BoundaryBox& box
)
{
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
  if ( p1.y < box.min.y && p2.y >= box.min.y ) {
    bot_x = intersect_y( box.min.y, p1, p2 );
    bot_v = bot_x > (box.min.x - e2) && bot_x < (box.max.x + e2);
  }
  if ( p1.y <= box.max.y && p2.y > box.max.y ) {
    top_x = intersect_y( box.max.y, p1, p2 );
    top_v = top_x > (box.min.x - e2) && top_x < (box.max.x + e2);
  }

  // Detect left and right clippings.
  if ( p1.x > p2.x ) std::swap( p1, p2 );
  if ( p1.x < box.min.x && p2.x >= box.min.x ) {
    lft_y = intersect_x( box.min.x, p1, p2 );
    lft_v = lft_y > (box.min.y - e2) && lft_y < (box.max.y + e2);
  }
  if ( p1.x < box.max.x && p2.x >= box.max.x ) {
    rgt_y = intersect_x( box.max.x, p1, p2 );
    rgt_v = rgt_y > (box.min.y - e2) && rgt_y < (box.max.y + e2);
  }

  // The four potential clip points.
  Point bot_c{ bot_x, box.min.y };
  Point top_c{ top_x, box.max.y };
  Point lft_c{ box.min.x, lft_y };
  Point rgt_c{ box.max.x, rgt_y };

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
    bool p1_inside =
      p1.x >= lb.bb.min.x && p1.x <= lb.bb.max.x &&
      p1.y >= lb.bb.min.y && p1.y <= lb.bb.max.y;
    bool p2_inside =
      p2.x >= lb.bb.min.x && p2.x <= lb.bb.max.x &&
      p2.y >= lb.bb.min.y && p2.y <= lb.bb.max.y;
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
    lb.collision_weight += std::sqrt( dx*dx + dy*dy ) + 1;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Series::ComputeMarker( SVG::U rim )
{
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

  marker_show = false;
  marker_diameter = 0;
  if ( marker_size < 0 ) {
    if ( type == SeriesType::Scatter ) marker_diameter = 3 * width;
  } else {
    marker_diameter = marker_size;
  }
  marker_radius = marker_diameter / 2;
  marker_show =
    (type == SeriesType::XY)
    ? (marker_diameter > width)
    : (marker_diameter > 0);
  marker_hollow = marker_diameter >= 4 * width;

  compute( marker_int, -width );
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

void Series::Build(
  SVG::Group* g,
  Axis* x_axis,
  Axis* y_axis,
  std::vector< LegendBox >& lb_list
)
{
  ComputeMarker();

  ApplyStyle( g );
  Group* lg = nullptr;
  Group* pg = nullptr;
  Group* fg = nullptr;
  if ( marker_show ) {
    if ( type == SeriesType::XY ) {
      lg = g->AddNewGroup();
      lg->Attr()->SetLineJoin( LineJoin::Round );
    }
    pg = g->AddNewGroup();
    pg->Attr()->SetLineDash( 0 );
    pg->Attr()->LineColor()->Clear();
    pg->Attr()->FillColor()->Set( &color );
    if ( marker_hollow ) {
      fg = g->AddNewGroup();
      fg->Attr()->SetLineDash( 0 );
      fg->Attr()->LineColor()->Clear();
      SVG::Color color_light{ color };
      color_light.Lighten( 0.5 );
      fg->Attr()->FillColor()->Set( &color_light );
    }
  } else {
    lg = g;
  }

  // Define clip-box.
  SVG::BoundaryBox box;
  if ( x_axis->angle == 0 ) {
    box.Update( x_axis->length, 0 );
    box.Update( 0, y_axis->length );
  } else {
    box.Update( 0, x_axis->length );
    box.Update( y_axis->length, 0 );
  }

  // Used for extra margin in comparisons to account for precision issues. This
  // may cause an unintended extra clip-detection near the corners, but the
  // points will in that case be very near each other and will thus be detected
  // by near(). Use different epsilons for near-detection and clip-detection in
  // order to ensure that any spurious clip-detection will be caught by the
  // more inclusive near-detection.
  e1 = std::max( box.max.x - box.min.x, box.max.y - box.min.y ) * epsilon;
  e2 = e1 * 0.1;

  // Enlarge the clip-box a little bit to ensure that points lying exactly at
  // the boundary are not excluded due to precision issues.
  box.min.x -= e1;
  box.min.y -= e1;
  box.max.x += e1;
  box.max.y += e1;

  Poly* poly = nullptr;
  bool adding_segments = false;

  Point prv;
  auto add_point = [&]( Point p, bool clipped = false )
  {
    if ( type == SeriesType::XY ) {
      if ( !adding_segments ) lg->Add( poly = new Poly() );
      poly->Add( p );
      if ( adding_segments ) {
        UpdateLegendBoxes( lb_list, prv, p );
      }
    } else {
      UpdateLegendBoxes( lb_list, p, p );
    }
    if ( !clipped && marker_show ) {
      BuildMarker( pg, marker_out, p );
      if ( marker_hollow ) {
        BuildMarker( fg, marker_int, p );
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
  for ( Datum& datum : datum_list ) {
    old = cur;
    if ( x_axis->angle == 0 ) {
      cur.x = x_axis->Coor( datum.x );
      cur.y = y_axis->Coor( datum.y );
    } else {
      cur.y = x_axis->Coor( datum.x );
      cur.x = y_axis->Coor( datum.y );
    }
    bool valid = x_axis->Valid( datum.x ) && y_axis->Valid( datum.y );
    bool inside =
      cur.x >= box.min.x && cur.x <= box.max.x &&
      cur.y >= box.min.y && cur.y <= box.max.y;
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
        int n = ClipLine( c1, c2, old, cur, box );
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

////////////////////////////////////////////////////////////////////////////////
