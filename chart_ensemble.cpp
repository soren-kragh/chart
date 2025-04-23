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
  for ( auto& chart : chart_list ) {
    delete chart.chart;
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
  chart_t chart;
  chart.chart = new Main( top_g->AddNewGroup() );

  // Note that the Y grid coordinates are in normal bottom to top "mathematical"
  // direction, whereas rows goes top to bottom. The InitGrid() will reorient
  // the Y grid coordinates to match these notations.
  chart.grid_x1 = grid_col1;
  chart.grid_y1 = grid_row1;
  chart.grid_x2 = grid_col2;
  chart.grid_y2 = grid_row2;

  chart.anchor_x = SVG::AnchorX::Mid;
  if ( align_hor != Chart::Pos::Auto ) {
    if ( align_hor == Chart::Pos::Left   ) chart.anchor_x = SVG::AnchorX::Min;
    if ( align_hor == Chart::Pos::Right  ) chart.anchor_x = SVG::AnchorX::Max;
    chart.anchor_x_defined = true;
  }

  chart.anchor_y = SVG::AnchorY::Mid;
  if ( align_ver != Chart::Pos::Auto ) {
    if ( align_ver == Chart::Pos::Bottom ) chart.anchor_y = SVG::AnchorY::Min;
    if ( align_ver == Chart::Pos::Top    ) chart.anchor_y = SVG::AnchorY::Max;
    chart.anchor_y_defined = true;
  }

  chart_list.push_back( chart );

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::InitGrid( void )
{
  for ( auto& chart : chart_list ) {
    grid_max_x = std::max( grid_max_x, chart.grid_x2 );
    grid_max_y = std::max( grid_max_y, chart.grid_y2 );
  }
  space_t space;
  space_list_x.resize( grid_max_x + 1, space );
  space_list_y.resize( grid_max_y + 1, space );
  for ( auto& chart : chart_list ) {
    chart.full_bb = chart.chart->GetGroup()->GetBB();
    chart.area_bb.Update( 0, 0 );
    chart.area_bb.Update( chart.chart->chart_w, chart.chart->chart_h );

    // Convert row location to Y grid coordinates.
    std::swap( chart.grid_y1, chart.grid_y2 );
    chart.grid_y1 = grid_max_y - chart.grid_y1;
    chart.grid_y2 = grid_max_y - chart.grid_y2;

    if ( !chart.anchor_x_defined ) {
      if ( chart.grid_x1 == 0 && chart.grid_x2 < grid_max_x ) {
        chart.anchor_x = SVG::AnchorX::Min;
      }
      if ( chart.grid_x1 > 0 && chart.grid_x2 == grid_max_x ) {
        chart.anchor_x = SVG::AnchorX::Max;
      }
    }

    if ( !chart.anchor_y_defined ) {
      if ( chart.grid_y1 == 0 && chart.grid_y2 < grid_max_y ) {
        chart.anchor_y = SVG::AnchorY::Min;
      }
      if ( chart.grid_y1 > 0 && chart.grid_y2 == grid_max_y ) {
        chart.anchor_y = SVG::AnchorY::Max;
      }
      chart.anchor_y_defined = true;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SolveGridSpace( std::vector< space_t >& space_list )
{
  bool is_x = &space_list == &space_list_x;

  // Create chart sorting based on grid position.
  std::vector< size_t > sorted_indices( chart_list.size() );
  std::iota( sorted_indices.begin(), sorted_indices.end(), 0 );
  std::sort(
    sorted_indices.begin(), sorted_indices.end(),
    [&]( size_t a_index, size_t b_index ) {
      const auto& a = chart_list[ a_index ];
      const auto& b = chart_list[ b_index ];
      uint32_t a1 = is_x ? a.grid_x1 : a.grid_y1;
      uint32_t a2 = is_x ? a.grid_x2 : a.grid_y2;
      uint32_t b1 = is_x ? b.grid_x1 : b.grid_y1;
      uint32_t b2 = is_x ? b.grid_x2 : b.grid_y2;
      return (a1 != b1) ? (a1 < b1) : (a2 < b2);
    }
  );

  auto update_pad = [&]( void ) {
    for ( auto& chart : chart_list ) {
      U f1 = is_x ? chart.full_bb.min.x : chart.full_bb.min.y;
      U f2 = is_x ? chart.full_bb.max.x : chart.full_bb.max.y;
      U a1 = is_x ? chart.area_bb.min.x : chart.area_bb.min.y;
      U a2 = is_x ? chart.area_bb.max.x : chart.area_bb.max.y;
      uint32_t g1 = is_x ? chart.grid_x1 : chart.grid_y1;
      uint32_t g2 = is_x ? chart.grid_x2 : chart.grid_y2;

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
        auto& chart = chart_list[ i ];

        U a1 = is_x ? chart.area_bb.min.x : chart.area_bb.min.y;
        U a2 = is_x ? chart.area_bb.max.x : chart.area_bb.max.y;
        uint32_t g1 = is_x ? chart.grid_x1 : chart.grid_y1;
        uint32_t g2 = is_x ? chart.grid_x2 : chart.grid_y2;

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

      for ( auto& chart : chart_list ) {
        U a1 = is_x ? chart.area_bb.min.x : chart.area_bb.min.y;
        U a2 = is_x ? chart.area_bb.max.x : chart.area_bb.max.y;
        uint32_t g1 = is_x ? chart.grid_x1 : chart.grid_y1;
        uint32_t g2 = is_x ? chart.grid_x2 : chart.grid_y2;

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
  for ( auto& chart : chart_list ) {
    chart.chart->Build();
  }
  ComputeGrid();
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
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 0; chart.grid_y1 = 0;
    chart.full_bb.Update( 600, 0 ); chart.grid_x2 = 1; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.min.x -= 100;
    chart.full_bb.max.x += 100;
    chart_list.push_back( chart );
  }
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 0; chart.grid_y1 = 0;
    chart.full_bb.Update( 100, 0 ); chart.grid_x2 = 0; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.min.x -= 50;
    chart.full_bb.max.x += 50;
    chart_list.push_back( chart );
  }
  {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 1; chart.grid_y1 = 0;
    chart.full_bb.Update( 100, 0 ); chart.grid_x2 = 1; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.min.x -= 50;
    chart.full_bb.max.x += 50;
    chart_list.push_back( chart );
  }
  for ( int i = 0; i < 3; i++ ) {
    chart_t chart;
    chart.full_bb.Update(   0, 0 ); chart.grid_x1 = 2; chart.grid_y1 = 0;
    chart.full_bb.Update( 100, 0 ); chart.grid_x2 = 2; chart.grid_y2 = 0;
    chart.area_bb.Update( chart.full_bb );
    chart.full_bb.min.x -= 50;
    chart.full_bb.max.x += 50;
    chart_list.push_back( chart );
  }


  InitGrid();

  SolveGridSpace( space_list_x );

  return;
}

////////////////////////////////////////////////////////////////////////////////
