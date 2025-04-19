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

void Ensemble::InitGrid( void )
{
  uint32_t max_x = 0;
  uint32_t max_y = 0;
  for ( auto& chart : chart_list ) {
    max_x = std::max( max_x, chart.grid_x2 );
    max_y = std::max( max_y, chart.grid_y2 );
  }
  space_t space;
  space_list_x.resize( max_x + 1, space );
  space_list_y.resize( max_y + 1, space );
}

////////////////////////////////////////////////////////////////////////////////

uint32_t Ensemble::SolveGridSpace( std::vector< space_t >& space_list )
{
  bool is_x = &space_list == &space_list_x;

  uint32_t max_iter = 1000000;
  uint32_t cur_iter = 0;

  while ( cur_iter < max_iter ) {
    cur_iter++;

    for ( auto& s : space_list ) {
      s.e1.pad = 0;
      s.e2.pad = 0;
      s.e1.adj = (s.e2.coor - s.e1.coor) / 2;
      s.e2.adj = (s.e1.coor - s.e2.coor) / 2;
    }

    // Update pad values.
    for ( auto& chart : chart_list ) {
      U f1 = is_x ? chart.full_bb.min.x : chart.full_bb.min.y;
      U f2 = is_x ? chart.full_bb.max.x : chart.full_bb.max.y;
      U a1 = is_x ? chart.area_bb.min.x : chart.area_bb.min.y;
      U a2 = is_x ? chart.area_bb.max.x : chart.area_bb.max.y;
      U g1 = is_x ? chart.grid_x1 : chart.grid_y1;
      U g2 = is_x ? chart.grid_x2 : chart.grid_y2;

      U ar = (a2 - a1) / 2;

      U c = (space_list[ g1 ].e1.coor + space_list[ g2 ].e2.coor) / 2;
      if (
        is_x
        ? (chart.anchor_x == SVG::AnchorX::Min)
        : (chart.anchor_y == SVG::AnchorY::Min)
      ) {
        c = space_list[ g1 ].e1.coor + ar;
      }
      if (
        is_x
        ? (chart.anchor_x == SVG::AnchorX::Max)
        : (chart.anchor_y == SVG::AnchorY::Max)
      ) {
        c = space_list[ g2 ].e2.coor - ar;
      }

      space_list[ g1 ].e1.pad =
        std::max(
          +space_list[ g1 ].e1.pad,
          space_list[ g1 ].e1.coor - (c - ar - (a1 - f1))
        );
      space_list[ g2 ].e2.pad =
        std::max(
          +space_list[ g2 ].e2.pad,
          (c + ar + (f2 - a2)) - space_list[ g2 ].e2.coor
        );
    }

    for ( auto& chart : chart_list ) {
      U a1 = is_x ? chart.area_bb.min.x : chart.area_bb.min.y;
      U a2 = is_x ? chart.area_bb.max.x : chart.area_bb.max.y;
      U g1 = is_x ? chart.grid_x1 : chart.grid_y1;
      U g2 = is_x ? chart.grid_x2 : chart.grid_y2;

      U aw = a2 - a1;
      U sw = space_list[ g2 ].e2.coor - space_list[ g1 ].e1.coor;

      space_list[ g1 ].e1.adj =
        std::min( +space_list[ g1 ].e1.adj, (sw - aw) / 2 );
      space_list[ g2 ].e2.adj =
        std::max( +space_list[ g2 ].e2.adj, (aw - sw) / 2 );
    }

    {
      edge_t* e1 = nullptr;
      edge_t* e2 = nullptr;
      for ( auto& s : space_list ) {
        e1 = &s.e1;
        if ( e2 ) {
          U overlap =
            (e2->coor + (e2->pad_use ? +e2->pad : 0)) -
            (e1->coor - (e1->pad_use ? +e1->pad : 0));
          e2->adj -= overlap / 2;
          e1->adj += overlap / 2;
        }
        e2 = &s.e2;
      }
    }

    U acu_adj = 0;
    for ( auto& s : space_list ) {
      s.e1.coor += s.e1.adj * 0.75;
      s.e2.coor += s.e2.adj * 0.75;
      acu_adj += std::abs( s.e1.adj );
      acu_adj += std::abs( s.e2.adj );
    }

    // Make the convergence limit dependent on how long we have iterated.
    double min_limit = 1e-5;
    double max_limit = 1e+0;
    U converge_limit =
      min_limit +
      (1.0 * cur_iter * cur_iter * (max_limit - min_limit)) /
      (1.0 * max_iter * max_iter);

    printf( "-    %12.6f    %12.10f\n", +acu_adj, +converge_limit );

    // To get alignment of the core chart areas, we initially do not take
    // padding into account. Therefore, when we have converged, check if the
    // padding collides and if so we have to take padding into consideration and
    // iterate more.
    if ( acu_adj < converge_limit ) {
      bool collisions = false;
      edge_t* e1 = nullptr;
      edge_t* e2 = nullptr;
      for ( auto& s : space_list ) {
        e1 = &s.e1;
        if ( e2 ) {
          U overlap = (e2->coor + e2->pad) - (e1->coor - e1->pad);
          if ( overlap > 4 * converge_limit ) {
            e2->pad_use = true;
            e1->pad_use = true;
            collisions  = true;
          }
        }
        e2 = &s.e2;
      }
      if ( !collisions ) break; // Solution found!
    }

    // If we have iterated a really long time, include all padding in the solver
    // such that they can have a chance to take some effect before we bail out.
    if ( cur_iter == max_iter / 2 ) {
      for ( auto& s : space_list ) {
        s.e1.pad_use = true;
        s.e2.pad_use = true;
      }
    }

  }

  return cur_iter;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::DisplayGridSpace( std::vector< space_t >& space_list )
{
  for ( auto& s : space_list ) {
    printf( "| %8.3f : %8.3f |", +s.e1.coor, +s.e2.coor );
  }
  printf( "\n" );
}

void Ensemble::RenumberGridSpace( std::vector< space_t >& space_list )
{
  bool first = true;
  U ofs = 0;
  for ( auto& s : space_list ) {
    if ( first ) {
      ofs = s.e1.coor;
    }
    s.e1.coor -= ofs;
    s.e2.coor -= ofs;
    first = false;
  }
}

void Ensemble::Test( void )
{
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 0; chart.grid_y1 = 0;
    chart.full_bb.Update( 100, 0 ); chart.grid_x2 = 0; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.max.x += 0;
    chart_list.push_back( chart );
  }
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 1; chart.grid_y1 = 0;
    chart.full_bb.Update( 200, 0 ); chart.grid_x2 = 1; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.min.x -= 400;
    chart_list.push_back( chart );
  }
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 2; chart.grid_y1 = 0;
    chart.full_bb.Update( 100, 0 ); chart.grid_x2 = 2; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart_list.push_back( chart );
  }
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 0; chart.grid_y1 = 0;
    chart.full_bb.Update( 1000, 0 ); chart.grid_x2 = 2; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart_list.push_back( chart );
  }

  InitGrid();

  uint32_t iter = SolveGridSpace( space_list_x );
  RenumberGridSpace( space_list_x );
  DisplayGridSpace( space_list_x );
  printf( "%8d iterations\n", iter );

  return;
}

////////////////////////////////////////////////////////////////////////////////
