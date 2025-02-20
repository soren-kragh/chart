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

  struct Entry {
    SVG::Object* link;
    SVG::U leading_space;
    SVG::U trailing_space;
  };

  struct Container {
    SVG::Object* link;
    SVG::BoundaryBox bb;
  };

  std::unordered_map< SVG::Object*, Entry > entries;
  std::unordered_map< SVG::Group*, Container > containers;

  // Create the given label, which might be multi-line text. Return the created
  // container, which is a group of text objects (one per line). If append is
  // true (implies add_to_db), then this text is instead appended to the
  // previously created container given by g.
  SVG::Group* Create(
    SVG::Group* g, const std::string& txt,
    SVG::U size = 0, bool add_to_db = false, bool append = false
  );

  // If the text objects inside the container has been moved, then this must be
  // called. This must be done before the container itself is moved.
  void Update( SVG::Group* container );

  // Delete the label container from the data base.
  void Delete( SVG::Group* container );

  // Add a background rectangle for all label objects in the data base provided
  // the background rectangle fits fully (or partially) inside the given area.
  void AddBackground(
    SVG::Group* bg_g, const SVG::BoundaryBox& area, bool partial_ok
  );

};

}
