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

#include <chart_label.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Label::Label( void )
{
}

Label::~Label( void )
{
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Label::AddText(
  SVG::Group* g, bool add_to_db,
  const std::string& txt, SVG::U size, SVG::Object* prv
)
{
  g = g->AddNewGroup();
  if ( size > 0 ) {
    g->Attr()->TextFont()->SetSize( size );
  }
  Attributes attr;
  g->Attr()->Collect( attr );
  U h = attr.TextFont()->GetHeight();
  U w = attr.TextFont()->GetWidth();

  U y = 0;
  LabelEntry e{ 0, 0, prv };
  std::string s;
  bool non_space_seen = false;
  auto it = txt.cbegin();
  while ( it != txt.cend() ) {
    auto c = *(it++);
    if ( c == '\n' || it == txt.cend() ) {
      g->Add( new Text( 0, y, s ) );
      y -= h;
      if ( add_to_db ) obj_map[ g->Last() ] = e;
      e.leading_space = 0;
      e.trailing_space = 0;
      e.prv = g->Last();
      non_space_seen = false;
      s.clear();
    } else {
      if ( c == ' ' ) {
        if ( non_space_seen ) {
          e.trailing_space += w;
        } else {
          e.leading_space += w;
        }
      } else {
        non_space_seen = true;
        e.trailing_space = 0;
      }
      s += c;
    }
  }

  // If leading_space is negative, it indicates that the entry is the group
  // of text objects.
  e.leading_space = -1;
  if ( add_to_db ) obj_map[ g ] = e;
  return g;
}

////////////////////////////////////////////////////////////////////////////////

void Label::DelText( SVG::Group* obj )
{
  SVG::Object* prv = obj;
  while ( prv ) {
    auto it = obj_map.find( prv );
    prv = it->second.prv;
    obj_map.erase( it );
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

void Label::AddBackground( SVG::Group* bg_g, const SVG::BoundaryBox& area )
{
  for ( const auto& [obj, e] : obj_map ) {
    if ( e.leading_space < 0 ) continue;
    BoundaryBox bb = obj->GetBB();
    U r = (bb.max.y - bb.min.y) / 3;
    bb.min.x += e.leading_space;
    bb.max.x -= e.trailing_space;
    bb.min.x -= r/2;
    bb.max.x += r/2;
    bb.max.y += r/3;
    if (
      bb.min.x >= area.min.x && bb.max.x <= area.max.x &&
      bb.min.y >= area.min.y && bb.max.y <= area.max.y
    ) {
      bg_g->Add( new Rect( bb.min, bb.max, r ) );
    }
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
