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

#pragma once

#include <unordered_map>
#include <chart_common.h>

namespace Chart {

class Label
{
  friend class Main;
  friend class Axis;

public:

  Label( void );
  ~Label( void );

private:

  struct LabelEntry {
    SVG::U leading_space;
    SVG::U trailing_space;
    SVG::Object* prv;
  };

  std::unordered_map< SVG::Object*, LabelEntry > obj_map;

  // Add the given text, which might be multi-line text. Return the created
  // object, which is a group of text objects (one per line). If prv is given,
  // then this text is associated with that text object.
  SVG::Group* AddText(
    SVG::Group* g, bool add_to_db,
    const std::string& txt, SVG::U size = 0, SVG::Object* prv = nullptr
  );

  // Delete the text object from the data base; obj is a group returned from
  // AddText().
  void DelText( SVG::Group* obj );

  // Add a background rectangle for all text objects in the data base provided
  // the background rectangle fits fully inside the given area.
  void AddBackground( SVG::Group* bg_g, const SVG::BoundaryBox& area );

};

}
