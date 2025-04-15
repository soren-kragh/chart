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

#include <chart_common.h>
#include <chart_main.h>

namespace Chart {

class Ensemble
{

public:

  Ensemble( void );
  ~Ensemble( void );

private:

  struct Part {
    ~Part( void );

    // Associated with leaf parts (identified by chart != nullptr).
    Main* chart = nullptr;
    SVG::U trans_x{ 0.0 };
    SVG::U trans_y{ 0.0 };

    // Associated with non-leaf parts (identified by chart == nullptr).
    std::deque< Part* > parts;
    bool vertical = false;
    bool expand = false;

    // Associated with all parts.
    SVG::BoundaryBox full_bb;
    SVG::BoundaryBox area_bb;
    SVG::AnchorX anchor_x = SVG::AnchorX::Mid;
    SVG::AnchorY anchor_y = SVG::AnchorY::Mid;
    bool anchor_full = false;

    void DetermineBB( void );
    void Arrange( void );
  };

  Part* top_part = nullptr;

};

}
