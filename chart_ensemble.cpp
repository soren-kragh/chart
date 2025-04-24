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

#include <algorithm>
#include <numeric>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Ensemble::Ensemble( void )
{
  canvas = new Canvas();
  top_g = canvas->TopGroup()->AddNewGroup();
}

Ensemble::~Ensemble( void )
{
  for ( auto& elem : element_list ) {
    delete elem.chart;
  }
  delete canvas;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::NewChart(
  uint32_t grid_row1, uint32_t grid_col1,
  uint32_t grid_row2, uint32_t grid_col2,
  Pos align_hor,
  Pos align_ver
)
{
  element_t elem;
  elem.chart = new Main( top_g->AddNewGroup() );

  // Note that the Y grid coordinates are in normal bottom to top "mathematical"
  // direction, whereas rows goes top to bottom. The InitGrid() will reorient
  // the Y grid coordinates to match these notations.
  elem.grid_x1 = grid_col1;
  elem.grid_y1 = grid_row1;
  elem.grid_x2 = grid_col2;
  elem.grid_y2 = grid_row2;

  elem.anchor_x = SVG::AnchorX::Mid;
  if ( align_hor != Chart::Pos::Auto ) {
    if ( align_hor == Chart::Pos::Left   ) elem.anchor_x = SVG::AnchorX::Min;
    if ( align_hor == Chart::Pos::Right  ) elem.anchor_x = SVG::AnchorX::Max;
    elem.anchor_x_defined = true;
  }

  elem.anchor_y = SVG::AnchorY::Mid;
  if ( align_ver != Chart::Pos::Auto ) {
    if ( align_ver == Chart::Pos::Bottom ) elem.anchor_y = SVG::AnchorY::Min;
    if ( align_ver == Chart::Pos::Top    ) elem.anchor_y = SVG::AnchorY::Max;
    elem.anchor_y_defined = true;
  }

  element_list.push_back( elem );

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::InitGrid( void )
{
  for ( auto& elem : element_list ) {
    grid_max_x = std::max( grid_max_x, elem.grid_x2 );
    grid_max_y = std::max( grid_max_y, elem.grid_y2 );
  }
  space_t space;
  space_list_x.resize( grid_max_x + 1, space );
  space_list_y.resize( grid_max_y + 1, space );
  for ( auto& elem : element_list ) {
    elem.full_bb = elem.chart->GetGroup()->GetBB();
    elem.area_bb.Update( 0, 0 );
    elem.area_bb.Update( elem.chart->chart_w, elem.chart->chart_h );

    // Convert row location to Y grid coordinates.
    std::swap( elem.grid_y1, elem.grid_y2 );
    elem.grid_y1 = grid_max_y - elem.grid_y1;
    elem.grid_y2 = grid_max_y - elem.grid_y2;

    if ( !elem.anchor_x_defined ) {
      if ( elem.grid_x1 == 0 && elem.grid_x2 < grid_max_x ) {
        elem.anchor_x = SVG::AnchorX::Min;
      }
      if ( elem.grid_x1 > 0 && elem.grid_x2 == grid_max_x ) {
        elem.anchor_x = SVG::AnchorX::Max;
      }
    }

    if ( !elem.anchor_y_defined ) {
      if ( elem.grid_y1 == 0 && elem.grid_y2 < grid_max_y ) {
        elem.anchor_y = SVG::AnchorY::Min;
      }
      if ( elem.grid_y1 > 0 && elem.grid_y2 == grid_max_y ) {
        elem.anchor_y = SVG::AnchorY::Max;
      }
      elem.anchor_y_defined = true;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SolveGridSpace( std::vector< space_t >& space_list )
{
  bool is_x = &space_list == &space_list_x;

  // Create element sorting based on grid position.
  std::vector< size_t > sorted_indices( element_list.size() );
  std::iota( sorted_indices.begin(), sorted_indices.end(), 0 );
  std::sort(
    sorted_indices.begin(), sorted_indices.end(),
    [&]( size_t a_index, size_t b_index ) {
      const auto& a = element_list[ a_index ];
      const auto& b = element_list[ b_index ];
      uint32_t a1 = is_x ? a.grid_x1 : a.grid_y1;
      uint32_t a2 = is_x ? a.grid_x2 : a.grid_y2;
      uint32_t b1 = is_x ? b.grid_x1 : b.grid_y1;
      uint32_t b2 = is_x ? b.grid_x2 : b.grid_y2;
      return (a1 != b1) ? (a1 < b1) : (a2 < b2);
    }
  );

  auto update_pad = [&]( void ) {
    for ( auto& elem : element_list ) {
      U f1 = is_x ? elem.full_bb.min.x : elem.full_bb.min.y;
      U f2 = is_x ? elem.full_bb.max.x : elem.full_bb.max.y;
      U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
      U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
      uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
      uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;

      U ar = (a2 - a1) / 2;

      U c = (space_list[ g1 ].e1.coor + space_list[ g2 ].e2.coor) / 2;
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Min)
        : (elem.anchor_y == SVG::AnchorY::Min)
      ) {
        c = space_list[ g1 ].e1.coor + ar;
      }
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Max)
        : (elem.anchor_y == SVG::AnchorY::Max)
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
  };

  for ( auto& s : space_list ) {
    s.e1.coor    = 0;
    s.e1.pad     = 0;
    s.e1.pad_use = 0;
    s.e2.coor    = 0;
    s.e2.pad     = 0;
    s.e2.pad_use = 0;
  }

  bool solved = false;

  uint32_t tot_iter = 0;

  uint32_t max_trial = 5;
  uint32_t cur_trial = 0;

  while ( !solved && cur_trial < max_trial ) {
    cur_trial++;

    if ( cur_trial == max_trial ) {
      // At last trial, just include all padding in the solver such that they
      // can have a chance to take some effect before we bail out.
      for ( auto& s : space_list ) {
        s.e1.pad_use = true;
        s.e2.pad_use = true;
      }
    }

    // Initial placement (not really needed, but gives much faster convergence
    // in some cases).
    {
      for ( auto i : sorted_indices ) {
        auto& elem = element_list[ i ];

        U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
        U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
        uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
        uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;

        space_list[ g2 ].e2.coor =
          std::max(
            +space_list[ g2 ].e2.coor,
            space_list[ g1 ].e1.coor + (a2 - a1)
          );

        U coor = space_list[ g2 ].e2.coor;
        for ( uint32_t g = g2 + 1; g < space_list.size(); g++ ) {
          space_list[ g ].e1.coor = std::max( space_list[ g ].e1.coor, coor );
          coor = space_list[ g ].e1.coor;
          space_list[ g ].e2.coor = std::max( space_list[ g ].e2.coor, coor );
          coor = space_list[ g ].e2.coor;
        }
      }

      for ( auto& s : space_list ) {
        s.e1.pad = 0;
        s.e2.pad = 0;
      }
      update_pad();

      U coor = -num_hi;
      for ( auto& s : space_list ) {
        U aw = s.e2.coor - s.e1.coor;
        s.e1.coor =
          std::max( +s.e1.coor, coor + (s.e1.pad_use ? +s.e1.pad : 0) );
        s.e2.coor = s.e1.coor + aw;
        coor = s.e2.coor + (s.e2.pad_use ? +s.e2.pad : 0);
      }
    }

/*
    printf( "Trial %1d initial placement:\n", cur_trial );
    RenumberGridSpace( space_list );
    DisplayGridSpace( space_list );
*/

    uint32_t max_iter = 100000;
    uint32_t cur_iter = 0;

    while ( !solved && cur_iter < max_iter ) {
      cur_iter++;
      tot_iter++;

      for ( auto& s : space_list ) {
        s.e1.pad = 0;
        s.e2.pad = 0;
        s.e1.adj = (s.e2.coor - s.e1.coor) / 2;
        s.e2.adj = (s.e1.coor - s.e2.coor) / 2;
      }

      update_pad();

      for ( auto& elem : element_list ) {
        U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
        U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
        uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
        uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;

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
        s.e1.coor += s.e1.adj * 0.9;
        s.e2.coor += s.e2.adj * 0.9;
        acu_adj += std::abs( s.e1.adj );
        acu_adj += std::abs( s.e2.adj );
      }

      U converge_limit = 1e-3;

/*
      printf(
        "-    %4d    %10d    %12.6f    %12.10f\n",
        cur_trial, tot_iter, +acu_adj, +converge_limit
      );
*/

      // To get alignment of the core chart areas, we initially do not take
      // padding into account. Therefore, when we have converged, check if the
      // padding collides and if so we have to take padding into consideration
      // and iterate more.
      solved = acu_adj < converge_limit;
      if ( solved || cur_iter == max_iter ) {
        edge_t* e1 = nullptr;
        edge_t* e2 = nullptr;
        for ( auto& s : space_list ) {
          e1 = &s.e1;
          if ( e2 ) {
            U overlap = (e2->coor + e2->pad) - (e1->coor - e1->pad);
            if ( overlap > 4 * converge_limit ) {
              e2->pad_use = true;
              e1->pad_use = true;
              solved = false;
            }
          }
          e2 = &s.e2;
        }
        break;
      }

    }

/*
    printf( "Trial %1d final placement:\n", cur_trial );
    RenumberGridSpace( space_list );
    DisplayGridSpace( space_list );
*/
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::ComputeGrid( void )
{
  InitGrid();
  SolveGridSpace( space_list_x );
  SolveGridSpace( space_list_y );
}

////////////////////////////////////////////////////////////////////////////////

std::string Ensemble::Build( void )
{
  if ( Empty() ) {
    NewChart( 0, 0, 0, 0 );
  }
  for ( auto& elem : element_list ) {
    elem.chart->Build();
  }

  ComputeGrid();

  for ( auto& elem : element_list ) {
    U gx1 = space_list_x[ elem.grid_x1 ].e1.coor;
    U gx2 = space_list_x[ elem.grid_x2 ].e2.coor;
    U gy1 = space_list_y[ elem.grid_y1 ].e1.coor;
    U gy2 = space_list_y[ elem.grid_y2 ].e2.coor;

    U mx = (gx1 + gx2) / 2 - (elem.area_bb.min.x + elem.area_bb.max.x) / 2;
    U my = (gy1 + gy2) / 2 - (elem.area_bb.min.y + elem.area_bb.max.y) / 2;

    if ( elem.anchor_x == SVG::AnchorX::Min ) mx = gx1 - elem.area_bb.min.x;
    if ( elem.anchor_x == SVG::AnchorX::Max ) mx = gx2 - elem.area_bb.max.x;

    if ( elem.anchor_y == SVG::AnchorY::Min ) my = gy1 - elem.area_bb.min.y;
    if ( elem.anchor_y == SVG::AnchorY::Max ) my = gy2 - elem.area_bb.max.y;

    elem.chart->GetGroup()->Move( mx, my );
  }

  std::ostringstream oss;
  oss << canvas->GenSVG();
  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::DisplayGridSpace( std::vector< space_t >& space_list )
{
  uint32_t n = 0;
  for ( auto& s : space_list ) {
    n++;
    if ( n > 8 ) {
      printf( "..." );
      break;
    }
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
    element_t elem;
    elem.full_bb.Update(   0, 0 ); elem.grid_x1 = 0; elem.grid_y1 = 0;
    elem.full_bb.Update( 600, 0 ); elem.grid_x2 = 1; elem.grid_y2 = 0;
    elem.area_bb.Update( elem.full_bb );
    elem.full_bb.min.x -= 100;
    elem.full_bb.max.x += 100;
    element_list.push_back( elem );
  }
  {
    element_t elem;
    elem.full_bb.Update(   0, 0 ); elem.grid_x1 = 0; elem.grid_y1 = 0;
    elem.full_bb.Update( 100, 0 ); elem.grid_x2 = 0; elem.grid_y2 = 0;
    elem.area_bb.Update( elem.full_bb );
    elem.full_bb.min.x -= 50;
    elem.full_bb.max.x += 50;
    element_list.push_back( elem );
  }
  {
    element_t elem;
    elem.full_bb.Update(   0, 0 ); elem.grid_x1 = 1; elem.grid_y1 = 0;
    elem.full_bb.Update( 100, 0 ); elem.grid_x2 = 1; elem.grid_y2 = 0;
    elem.area_bb.Update( elem.full_bb );
    elem.full_bb.min.x -= 50;
    elem.full_bb.max.x += 50;
    element_list.push_back( elem );
  }
  for ( int i = 0; i < 3; i++ ) {
    element_t elem;
    elem.full_bb.Update(   0, 0 ); elem.grid_x1 = 2; elem.grid_y1 = 0;
    elem.full_bb.Update( 100, 0 ); elem.grid_x2 = 2; elem.grid_y2 = 0;
    elem.area_bb.Update( elem.full_bb );
    elem.full_bb.min.x -= 50;
    elem.full_bb.max.x += 50;
    element_list.push_back( elem );
  }


  InitGrid();

  SolveGridSpace( space_list_x );

  return;
}

////////////////////////////////////////////////////////////////////////////////
