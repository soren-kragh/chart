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

  Part* max_x_part = nullptr;
  Part* max_y_part = nullptr;

  for ( auto part : parts ) {
    part->DetermineBB();
    if ( max_x_part == nullptr ) {
      max_x_part = part;
      max_y_part = part;
      continue;
    }
    U wx =
      part->anchor_full
      ? (part->full_bb.max.x - part->full_bb.min.x)
      : (part->area_bb.max.x - part->area_bb.min.x);
    U wy =
      part->anchor_full
      ? (part->full_bb.max.y - part->full_bb.min.y)
      : (part->area_bb.max.y - part->area_bb.min.y);
    U mx =
      max_x_part->anchor_full
      ? (max_x_part->full_bb.max.x - max_x_part->full_bb.min.x)
      : (max_x_part->area_bb.max.x - max_x_part->area_bb.min.x);
    U my =
      max_y_part->anchor_full
      ? (max_y_part->full_bb.max.y - max_y_part->full_bb.min.y)
      : (max_y_part->area_bb.max.y - max_y_part->area_bb.min.y);
    if ( wx > mx ) max_x_part = part;
    if ( wy > my ) max_y_part = part;
  }

  full_bb.Reset();
  area_bb.Reset();



  return;
}

//------------------------------------------------------------------------------

void Ensemble::Part::Arrange( void )
{
  return;
}

////////////////////////////////////////////////////////////////////////////////
