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

#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Ensemble::Ensemble( void )
{
}

Ensemble::~Ensemble( void )
{
}

////////////////////////////////////////////////////////////////////////////////

Ensemble::Part::~Part( void )
{
  delete chart;
  for ( auto part : parts ) {
    delete part;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::Part::DetermineBB( void )
{
  if ( chart != nullptr ) {
    return;
  }

  Part* mx_part = nullptr;
  Part* my_part = nullptr;

  for ( auto part : parts ) {
    part->DetermineBB();
    if ( mx_part == nullptr ) {
      mx_part = part;
      my_part = part;
    } else {
      U wx = part->area_bb.max.x - part->area_bb.min.x;
      U wy = part->area_bb.max.y - part->area_bb.min.y;
      U mx = mx_part->area_bb.max.x - mx_part->area_bb.min.x;
      U my = my_part->area_bb.max.y - my_part->area_bb.min.y;
      if ( wx > mx ) mx_part = part;
      if ( wy > my ) my_part = part;
    }
  }

  BoundaryBox mx_bb{ mx_part->area_bb };
  BoundaryBox my_bb{ my_part->area_bb };

  full_bb.Reset();
  area_bb.Reset();

  U x = 0;
  U y = 0;

  for ( auto part : parts ) {
    BoundaryBox fbb{ part->full_bb };
    BoundaryBox abb{ part->area_bb };
    U dy = y - fbb.min.y;
    U dx = x - fbb.min.x;
    if ( vertical ) {
      dx = (mx_bb.max.x + mx_bb.min.x)/2 - (abb.max.x + abb.min.x)/2;
      if ( part->anchor_x == AnchorX::Min ) {
        dx = mx_bb.min.x - abb.min.x;
      }
      if ( part->anchor_x == AnchorX::Max ) {
        dx = mx_bb.max.x - abb.max.x;
      }
    } else {
      dy = (my_bb.max.y + my_bb.min.y)/2 - (abb.max.y + abb.min.y)/2;
      if ( part->anchor_y == AnchorY::Min ) {
        dy = my_bb.min.y - abb.min.y;
      }
      if ( part->anchor_y == AnchorY::Max ) {
        dy = my_bb.max.y - abb.max.y;
      }
    }
    fbb.min.x += dx; fbb.max.x += dx;
    fbb.min.y += dy; fbb.max.y += dy;
    abb.min.x += dx; abb.max.x += dx;
    abb.min.y += dy; abb.max.y += dy;
    full_bb.Update( fbb );
    area_bb.Update( abb );
    if ( vertical ) {
      y += fbb.max.y - fbb.min.y;
    } else {
      x += fbb.max.x - fbb.min.x;
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Ensemble::Part::Arrange( void )
{
  return;
}

////////////////////////////////////////////////////////////////////////////////
