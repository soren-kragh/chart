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

#include <chart_series.h>
#include <chart_axis.h>

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Series::Series( std::string name )
{
  this->name = name;
  SetWidth( 1 );
  SetDash( 0 );

  color_list.emplace_back(); color_list.back().Set( Blue, 0.1 );
  color_list.emplace_back(); color_list.back().Set( Red );
  color_list.emplace_back(); color_list.back().Set( Green, 0.5 );
  color_list.emplace_back(); color_list.back().Set( Cyan, 0, 0.2 );
  color_list.emplace_back(); color_list.back().Set( Purple );
  color_list.emplace_back(); color_list.back().Set( Blue, 0.6 );
  color_list.emplace_back(); color_list.back().Set( Orange );
  color_list.emplace_back(); color_list.back().Set( Brown, 0, 0.3 );
}

Series::~Series( void )
{
}

///////////////////////////////////////////////////////////////////////////////

void Series::SetStyle( int style )
{
  color.Set( &color_list[ style % color_list.size() ] );
  style = style / color_list.size();
  style = style % 8;
  if ( style == 0 ) {
    SetWidth( 4 );
    SetDash( 0 );
  }
  if ( style == 1 ) {
    SetWidth( 4 );
    SetDash( 3, 8 );
  }
  if ( style == 2 ) {
    SetWidth( 4 );
    SetDash( 10, 8 );
  }
  if ( style == 3 ) {
    SetWidth( 4 );
    SetDash( 20, 8 );
  }
  if ( style == 4 ) {
    SetWidth( 2 );
    SetDash( 0 );
  }
  if ( style == 5 ) {
    SetWidth( 2 );
    SetDash( 3, 8 );
  }
  if ( style == 6 ) {
    SetWidth( 2 );
    SetDash( 10, 8 );
  }
  if ( style == 7 ) {
    SetWidth( 2 );
    SetDash( 20, 8 );
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

///////////////////////////////////////////////////////////////////////////////

void Series::Add( double x, double y )
{
  datum_list.emplace_back( x, y );
}

///////////////////////////////////////////////////////////////////////////////

void Series::UpdateLegendBoxes(
  std::vector< LegendBox >& lb_list,
  U x1, U y1,
  U x2, U y2
)
{
  U vx = x2 - x1;
  U vy = y2 - y1;
  for ( LegendBox& lb : lb_list ) {
    if ( x1 < lb.bb.min.x && x2 < lb.bb.min.x ) continue;
    if ( x1 > lb.bb.max.x && x2 > lb.bb.max.x ) continue;
    if ( y1 < lb.bb.min.y && y2 < lb.bb.min.y ) continue;
    if ( y1 > lb.bb.max.y && y2 > lb.bb.max.y ) continue;
    int c = 0;
    if ( vx * (y2 - lb.bb.min.y) - vy * (x2 - lb.bb.min.x) < 0 ) c++;
    if ( vx * (y2 - lb.bb.max.y) - vy * (x2 - lb.bb.min.x) < 0 ) c++;
    if ( vx * (y2 - lb.bb.min.y) - vy * (x2 - lb.bb.max.x) < 0 ) c++;
    if ( vx * (y2 - lb.bb.max.y) - vy * (x2 - lb.bb.max.x) < 0 ) c++;
    if ( (c > 0 && c < 4) || (vx == 0 && vy == 0) ) lb.collisions++;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Series::ApplyStyle( SVG::Object* obj )
{
  obj->Attr()->SetLineWidth( width );
  if ( dash > 0 ) {
    obj->Attr()->SetLineDash( dash, (hole > 0) ? hole : dash );
  }
  obj->Attr()->LineColor()->Set( &color );
}

// To account to rounding errors.
static double e = 0;

bool LineIntersectsLine(
  U& ix, U& iy,
  U x1, U y1, U x2, U y2,
  U x3, U y3, U x4, U y4
)
{
  double dx12 = x1 - x2;
  double dy12 = y1 - y2;
  double dx34 = x3 - x4;
  double dy34 = y3 - y4;
  double d = dx12 * dy34 - dy12 * dx34;
  if ( d == 0 ) return false;
  double p = x1 * y2 - y1 * x2;
  double q = x3 * y4 - y3 * x4;
  double x = (dx34 * p - dx12 * q) / d;
  double y = (dy34 * p - dy12 * q) / d;
  // Be generous in the detection (via e) in order not to miss an intersection
  // due to rounding errors.
  bool i = true;
  i = i && ((x1 < x2) ? (x > x1-e && x < x2+e) : (x > x2-e && x < x1+e));
  i = i && ((y1 < y2) ? (y > y1-e && y < y2+e) : (y > y2-e && y < y1+e));
  i = i && ((x3 < x4) ? (x > x3-e && x < x4+e) : (x > x4-e && x < x3+e));
  i = i && ((y3 < y4) ? (y > y3-e && y < y4+e) : (y > y4-e && y < y3+e));
  if ( i ) {
    ix = x;
    iy = y;
  }
  return i;
}

// Returns:
//   0 : No intersection.
//   1 : One intersection; (ix1,iy1) is the point.
//   2 : Two intersections; (ix1,iy1) and (ix2,iy2) are the points.
int LineIntersectsBox(
  U& ix1, U& iy1,
  U& ix2, U& iy2,
  U lx1, U ly1, U lx2, U ly2,
  U bx1, U by1, U bx2, U by2
)
{
  int n = 0;
  U* ix = &ix1;
  U* iy = &iy1;
  // Create corner gaps to avoid getting multiple intersections in the corners.
  // We use 3*e because LineIntersectsLine is generous by e in its checks;
  // this way we end up with an effective corner gap of at least e.
  double m = 3*e;
  if ( LineIntersectsLine( *ix, *iy, lx1, ly1, lx2, ly2, bx1, by1+m, bx1, by2-m ) ) {
    if ( 2 == ++n ) return n;
    ix = &ix2; iy = &iy2;
  }
  if ( LineIntersectsLine( *ix, *iy, lx1, ly1, lx2, ly2, bx2, by1+m, bx2, by2-m ) ) {
    if ( 2 == ++n ) return n;
    ix = &ix2; iy = &iy2;
  }
  if ( LineIntersectsLine( *ix, *iy, lx1, ly1, lx2, ly2, bx1+m, by1, bx2-m, by1 ) ) {
    if ( 2 == ++n ) return n;
    ix = &ix2; iy = &iy2;
  }
  if ( LineIntersectsLine( *ix, *iy, lx1, ly1, lx2, ly2, bx1+m, by2, bx2-m, by2 ) ) {
    if ( 2 == ++n ) return n;
    ix = &ix2; iy = &iy2;
  }
  return n;
}

void Series::Build(
  SVG::Group* g,
  Axis& x_axis,
  Axis& y_axis,
  std::vector< LegendBox >& lb_list
)
{
  ApplyStyle( g );
  U w = x_axis.length;
  U h = y_axis.length;
  e = std::max( w, h ) / 1.0e5;
  U ix1 = 0;
  U iy1 = 0;
  U ix2 = 0;
  U iy2 = 0;
  Poly* p = NULL;
  U ox = 0;
  U oy = 0;
  bool first = true;
  for ( Datum& datum : datum_list ) {
    U x = x_axis.Coor( datum.x );
    U y = y_axis.Coor( datum.y );
    bool valid =
      (!x_axis.log_scale || datum.x > 0) &&
      (!y_axis.log_scale || datum.y > 0);
    bool inside = (x >= 0 && x <= w && y >= 0 && y <= h);
    if ( !valid ) {
      first = true;
      p = NULL;
      continue;
    }
    if ( first ) {
      if ( inside ) {
        g->Add( p = new Poly() );
        p->Add( x, y );
        if ( datum_list.size() == 1 ) {
          UpdateLegendBoxes( lb_list, x, y, x, y );
          p->Add( x, y );
        }
      }
      first = false;
    } else {
      UpdateLegendBoxes( lb_list, ox, oy, x, y );
      if ( p != NULL && inside ) {
        // Common case when we stay inside the chart area.
        p->Add( x, y );
      } else {
        // Handle clipping in and out of the chart area.
        int n =
          LineIntersectsBox(
            ix1, iy1, ix2, iy2,
            ox, oy, x, y,
            0, 0, w, h
          );
        if ( p == NULL ) {
          // We were outside.
          if ( inside ) {
            // We went from outside to now inside.
            g->Add( p = new Poly() );
            if ( n == 2 ) {
              // When going from being outside to now being inside the area we
              // should only get one intersection, remove the one closest to
              // (x,y).
              double dx1 = ix1 - x;
              double dy1 = iy1 - y;
              double dx2 = ix2 - x;
              double dy2 = iy2 - y;
              if ( dx1*dx1 + dy1*dy1 < dx2*dx2 + dy2*dy2 ) {
                ix1 = ix2;
                iy1 = iy2;
              }
            }
            if ( n > 0 ) {
              p->Add( ix1, iy1 );
            }
            p->Add( x, y );
          } else
          if ( n == 2 ) {
            // We are still outside, but the line segment passes through the
            // chart area.
            g->Add( p = new Poly() );
            p->Add( ix1, iy1 );
            p->Add( ix2, iy2 );
            p = NULL;
          }
        } else {
          // We went from inside to now outside.
          if ( n == 2 ) {
            // When going from being inside to now being outside the area we
            // should only get one intersection, remove the one closest to
            // (ox,oy).
            double dx1 = ix1 - ox;
            double dy1 = iy1 - oy;
            double dx2 = ix2 - ox;
            double dy2 = iy2 - oy;
            if ( dx1*dx1 + dy1*dy1 < dx2*dx2 + dy2*dy2 ) {
              ix1 = ix2;
              iy1 = iy2;
            }
          } else
          if ( n == 0 ) {
            // We now moved outside the chart area, but we did not get an
            // intersection. This means that the line slipped out through one
            // of the tiny coner gaps deliberately created in LineIntersectsBox
            // (the m variable). Detect which corner.
            ix1 = (x > w/2) ? w : U( 0 );
            iy1 = (y > h/2) ? h : U( 0 );
          }
          p->Add( ix1, iy1 );
          p = NULL;
        }
      }
    }
    ox = x;
    oy = y;
  }
}

///////////////////////////////////////////////////////////////////////////////
