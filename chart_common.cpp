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

void Chart::TextBG(
  SVG::Group* g, const SVG::BoundaryBox& bb, SVG::U h
)
{
  U r = h / 3;
  g->Add(
    new Rect(
      bb.min.x - r/2, bb.min.y, bb.max.x + r/2, bb.max.y + r/3, r
    )
  );
  g->FrontToBack();
}

// Return newly created group object or nullptr if new_g is false.
SVG::Group* Chart::MultiLineText(
  SVG::Group* g,
  bool new_g,
  const std::string txt,
  SVG::U size,
  bool bg,              // Add background.
  bool bg_truncate,     // Truncate leading/trailing white-space.
  bool bg_per_line      // Per line background.
)
{
  if ( new_g ) {
    g = g->AddNewGroup();
    if ( size > 0 ) {
      g->Attr()->TextFont()->SetSize( size );
    }
  }
  Attributes attr;
  g->Attr()->Collect( attr );
  U h = attr.TextFont()->GetHeight();
  U w = attr.TextFont()->GetWidth();

  BoundaryBox bb_all;
  BoundaryBox bb_trc;;
  BoundaryBox bb;
  auto draw_bg = [&]( void )
  {
    TextBG( g, bb, h );
  };

  U y = 0;
  std::string s;
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
      g->Add( new Text( 0, y, s ) );
      if ( size > 0 && !new_g ) {
        g->Last()->Attr()->TextFont()->SetSize( size );
      }
      {
        bb = g->Last()->GetBB();
        if ( bg && bg_per_line && !bg_truncate ) draw_bg();
        bb_all.Update( bb.min );
        bb_all.Update( bb.max );
        bb.min.x += w * x0;
        bb.max.x -= w * (x2 - x1 );
        if ( bg && bg_per_line && bg_truncate ) draw_bg();
        bb_trc.Update( bb.min );
        bb_trc.Update( bb.max );
      }
      y -= h;
      s = "";
    }
  }
  if ( bg ) {
    bb = bb_all;
    draw_bg();
    if ( bg_truncate || bg_per_line ) {
      g->Last()->Attr()->FillColor()->Clear();
    }
    if ( bg_truncate && !bg_per_line ) {
      bb = bb_trc;
      draw_bg();
    }
  }

  return new_g ? g : nullptr;
}

SVG::Group* Chart::MultiLineText(
  SVG::Group* g, const std::string txt, SVG::U size
)
{
  return MultiLineText( g, true, txt, size, false, false, false );
}

SVG::Group* Chart::Label(
  SVG::Group* g, const std::string txt, SVG::U size
)
{
  return MultiLineText( g, true, txt, size, true, true, true );
}

///////////////////////////////////////////////////////////////////////////////

Object* Chart::Collides(
  SVG::Object* obj, const std::vector< SVG::Object* >& objects,
  SVG::U margin_x, SVG::U margin_y
)
{
  if ( obj == nullptr || obj->Empty() ) return nullptr;
  for ( auto object : objects ) {
    if ( object->Empty() ) continue;
    if ( SVG::Collides( obj, object, margin_x, margin_y ) ) {
      return object;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

// Move objects so as to avoid collisions with other objects.
void Chart::MoveObjs(
  Dir dir,
  const std::vector< SVG::Object* >& move_objs,
  const std::vector< SVG::Object* >& avoid_objs,
  SVG::U margin_x, SVG::U margin_y
)
{
  do {
    U dx = 0;
    U dy = 0;
    for ( auto obj : move_objs ) {
      Object* col =
        Collides( obj, avoid_objs, margin_x - epsilon, margin_y - epsilon );
      if ( col != nullptr ) {
        BoundaryBox col_bb = col->GetBB();
        BoundaryBox obj_bb = obj->GetBB();
        switch ( dir ) {
          case Dir::Right : dx = col_bb.max.x - obj_bb.min.x + margin_x; break;
          case Dir::Left  : dx = col_bb.min.x - obj_bb.max.x - margin_x; break;
          case Dir::Up    : dy = col_bb.max.y - obj_bb.min.y + margin_y; break;
          case Dir::Down  : dy = col_bb.min.y - obj_bb.max.y - margin_y; break;
        }
        break;
      }
    }
    if ( dx != 0 || dy != 0  ) {
      for ( auto obj : move_objs ) {
        obj->Move( dx, dy );
      }
      continue;
    }
  } while ( false );
}

void Chart::MoveObj(
  Dir dir,
  SVG::Object* obj,
  const std::vector< SVG::Object* >& avoid_objs,
  SVG::U margin_x, SVG::U margin_y
)
{
  std::vector< SVG::Object* > move_objs;
  move_objs.push_back( obj );
  MoveObjs( dir, move_objs, avoid_objs, margin_x, margin_y );
}

///////////////////////////////////////////////////////////////////////////////
