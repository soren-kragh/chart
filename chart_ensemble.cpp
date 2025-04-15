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

Ensemble::PartBB Ensemble::Part::GetPartBB( void )
{
  if ( chart != nullptr ) {
    return chart_bb;
  }

  std::vector< PartBB > outlines;

  for ( auto part : parts ) {
    outlines.emplace_back( part->GetPartBB() );
  }

/*
  BoundaryBox max_fbb;
  BoundaryBox max_abb;
  U max_fwx = 0;
  U max_fwy = 0;
  U max_awx = 0;
  U max_awy = 0;

  for ( auto part : parts ) {
    BoundaryBox fbb;
    BoundaryBox abb;
    GetBB( full, fbb, abb );
    max_fwx = std::max( +max_fwx, fbb.max.x - fbb.min.x );
    max_fwy = std::max( +max_fwy, fbb.max.y - fbb.min.y );
    max_awx = std::max( +max_awx, abb.max.x - abb.min.x );
    max_awy = std::max( +max_awy, abb.max.y - abb.min.y );
    max_fbb.Update( fbb );
    max_abb.Update( abb );
  }

  for ( auto part : parts ) {
    BoundaryBox fbb;
    BoundaryBox abb;
    GetBB( full, fbb, abb );
    U dx = 0;
    U dy = 0;
    if ( vertical ) {
    } else {
      if ( anchor_y == AnchorY::Mid ) {
        if ( full ) {
          dy = 
        } else {
        }
      }
    }
    fbb.min.x += dx; fbb.min.y += dy;
    fbb.max.x += dx; fbb.max.y += dy;
    abb.min.x += dx; abb.min.y += dy;
    abb.max.x += dx; abb.max.y += dy;
    agr_full_bb.Update( fbb );
    agr_area_bb.Update( abb );
  }
*/

  return chart_bb;
}

//------------------------------------------------------------------------------

void Ensemble::Part::Arrange( void )
{

  return;
}

////////////////////////////////////////////////////////////////////////////////
