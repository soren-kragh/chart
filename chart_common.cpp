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
  U r = 0;
  BoundaryBox bb;
  std::string s;
  uint32_t min_x0 = 0;  // Minimal leading spaces.
  uint32_t max_x1 = 0;  // Maximum line length excluding trailing spaces.
  uint32_t max_x2 = 0;  // Maximum line length including trailing spaces.
  bool first_line = true;
  for ( uint32_t n = 0; n < txt.length(); n++ ) {
    char c = txt[ n ];
    if ( c != '\n' ) s += c;
    if ( c == '\n' || n == txt.length() - 1 ) {
      uint32_t x0 = 0;
      uint32_t x1 = 0;
      uint32_t x2 = 0;
      for ( const char c : s ) {
        x2++;
        if ( c == ' ' ) {
          if ( x1 == 0 ) x0 = x2;
        } else {
          x1 = x2;
        }
      }
      if ( first_line || x0 < min_x0 ) min_x0 = x0;
      if ( first_line || x1 > max_x1 ) max_x1 = x1;
      if ( first_line || x2 > max_x2 ) max_x2 = x2;
      first_line = false;
      g->Add( new Text( 0, y, s ) );
      bb = g->Last()->GetBB();
      y -= bb.max.y - bb.min.y;
      r = (bb.max.y - bb.min.y) / 3;
      s = "";
    }
  }
  bb = g->GetBB();
  g->Add( new Rect( bb.min.x - r/2, bb.min.y, bb.max.x + r/2, bb.max.y, r ) );
  g->FrontToBack();
  if ( min_x0 > 0 || max_x1 < max_x2 ) {
    g->Last()->Attr()->FillColor()->Clear();
    if ( min_x0 < max_x1 ) {
      U lx = bb.min.x + (bb.max.x - bb.min.x) * min_x0 / max_x2;
      U rx = bb.min.x + (bb.max.x - bb.min.x) * max_x1 / max_x2;
      g->Add( new Rect( lx - r/2, bb.min.y, rx + r/2, bb.max.y, r ) );
      g->FrontToBack();
    }
  }

  return g;
}

///////////////////////////////////////////////////////////////////////////////

bool Chart::Collides(
  SVG::Object* obj, const std::vector< SVG::Object* >& objects,
  SVG::U margin_x, SVG::U margin_y,
  SVG::BoundaryBox& bb
)
{
  if ( obj == nullptr || obj->Empty() ) return false;
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
