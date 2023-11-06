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

#include <chart_common.h>

using namespace SVG;

///////////////////////////////////////////////////////////////////////////////

SVG::Object* Chart::Label(
  SVG::Group* g, const std::string txt, SVG::U size
)
{
  g = g->AddNewGroup();
  if ( size > 0 ) {
    g->Attr()->TextFont()->SetSize( size );
  }
  U y = 0;
  BoundaryBox bb;
  std::string s;
  for ( char c : txt ) {
    if ( c == '\n' ) {
      g->Add( new Text( 0, y, s ) );
      bb = g->Last()->GetBB();
      y -= bb.max.y - bb.min.y;
      s = "";
    } else {
      s += c;
    }
  }
  if ( s != "" ) {
    g->Add( new Text( 0, y, s ) );
  }
  bb = g->Last()->GetBB();
  U r = (bb.max.y - bb.min.y) / 3;
  bb = g->GetBB();
  g->Add( new Rect( bb.min.x - r/2, bb.min.y, bb.max.x + r/2, bb.max.y, r ) );
  g->LastToBack();

  return g;
}

///////////////////////////////////////////////////////////////////////////////

bool Chart::Collides(
  SVG::Object* obj, const std::vector< SVG::Object* >& objects,
  SVG::U margin_x, SVG::U margin_y,
  SVG::BoundaryBox& bb
)
{
  if ( obj == NULL || obj->Empty() ) return false;
  bb = BoundaryBox();
  bool collision = false;
  for ( Object* object : objects ) {
    if ( object->Empty() ) continue;
    if ( SVG::Collides( obj, object, margin_x, margin_y ) ) {
      BoundaryBox cb = object->GetBB();
      bb.Update( cb.min );
      bb.Update( cb.max );
      collision = true;
    }
  }
  return collision;
}

bool Chart::Collides(
  SVG::Object* obj, const std::vector< SVG::Object* >& objects,
  SVG::U margin_x, SVG::U margin_y
)
{
  BoundaryBox bb;
  return Chart::Collides( obj, objects, margin_x, margin_y, bb );
}

///////////////////////////////////////////////////////////////////////////////
