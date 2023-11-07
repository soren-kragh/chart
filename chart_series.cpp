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

void Series::Build(
  SVG::Group* g,
  const Axis& x_axis,
  const Axis& y_axis,
  std::vector< LegendBox >& lb_list
)
{
  Poly* p = new Poly();
  g->Add( p );
  U ox = 0;
  U oy = 0;
  bool first = true;
  for ( Datum& datum : datum_list ) {
    U x = (datum.x - x_axis.min) * x_axis.length / (x_axis.max - x_axis.min);
    U y = (datum.y - y_axis.min) * y_axis.length / (y_axis.max - y_axis.min);
    p->Add( x, y );
    if ( !first ) {
      UpdateLegendBoxes( lb_list, ox, oy, x, y );
    }
    ox = x;
    oy = y;
    first = false;
  }
  if ( datum_list.size() == 1 ) {
    g->DeleteFront();
    g->Add( new Circle( ox, oy, 8 ) );
    UpdateLegendBoxes( lb_list, ox, oy, ox, oy );
  }
  ApplyStyle( g->Last() );
}

///////////////////////////////////////////////////////////////////////////////
