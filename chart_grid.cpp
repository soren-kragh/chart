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

#include <chart_grid.h>

#include <algorithm>
#include <numeric>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Grid::Grid( void )
{
}

Grid::~Grid( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Grid::Init( void )
{
  for ( auto& elem : element_list ) {
    if ( elem.grid_x1 > elem.grid_x2 ) std::swap( elem.grid_x1, elem.grid_x2 );
    if ( elem.grid_y1 > elem.grid_y2 ) std::swap( elem.grid_y1, elem.grid_y2 );
    max_x = std::max( max_x, elem.grid_x2 );
    max_y = std::max( max_y, elem.grid_y2 );
  }
  cell_t space;
  cell_list_x.resize( max_x + 1, space );
  cell_list_y.resize( max_y + 1, space );
}

////////////////////////////////////////////////////////////////////////////////

uint32_t Grid::Solve1( std::vector< cell_t >& cell_list )
{
  bool is_x = &cell_list == &cell_list_x;

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

      U c = (cell_list[ g1 ].e1.coor + cell_list[ g2 ].e2.coor) / 2;
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Min)
        : (elem.anchor_y == SVG::AnchorY::Min)
      ) {
        c = cell_list[ g1 ].e1.coor + ar;
      }
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Max)
        : (elem.anchor_y == SVG::AnchorY::Max)
      ) {
        c = cell_list[ g2 ].e2.coor - ar;
      }

      cell_list[ g1 ].e1.pad =
        std::max(
          +cell_list[ g1 ].e1.pad,
          cell_list[ g1 ].e1.coor - (c - ar - (a1 - f1))
        );
      cell_list[ g2 ].e2.pad =
        std::max(
          +cell_list[ g2 ].e2.pad,
          (c + ar + (f2 - a2)) - cell_list[ g2 ].e2.coor
        );
    }
  };

  for ( auto& cell : cell_list ) {
    cell.e1.coor    = 0;
    cell.e1.pad     = 0;
    cell.e1.pad_use = 0;
    cell.e2.coor    = 0;
    cell.e2.pad     = 0;
    cell.e2.pad_use = 0;
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
      for ( auto& cell : cell_list ) {
        cell.e1.pad_use = true;
        cell.e2.pad_use = true;
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

        cell_list[ g2 ].e2.coor =
          std::max(
            +cell_list[ g2 ].e2.coor,
            cell_list[ g1 ].e1.coor + (a2 - a1)
          );

        U coor = cell_list[ g2 ].e2.coor;
        for ( uint32_t g = g2 + 1; g < cell_list.size(); g++ ) {
          cell_list[ g ].e1.coor = std::max( cell_list[ g ].e1.coor, coor );
          coor = cell_list[ g ].e1.coor;
          cell_list[ g ].e2.coor = std::max( cell_list[ g ].e2.coor, coor );
          coor = cell_list[ g ].e2.coor;
        }
      }

      for ( auto& cell : cell_list ) {
        cell.e1.pad = 0;
        cell.e2.pad = 0;
      }
      update_pad();

      U coor = -num_hi;
      for ( auto& cell : cell_list ) {
        U aw = cell.e2.coor - cell.e1.coor;
        cell.e1.coor =
          std::max(
            +cell.e1.coor, coor + (cell.e1.pad_use ? +cell.e1.pad : 0)
          );
        cell.e2.coor = cell.e1.coor + aw;
        coor = cell.e2.coor + (cell.e2.pad_use ? +cell.e2.pad : 0);
      }
    }

/*
    printf( "Trial %1d initial placement:\n", cur_trial );
    RenumberCoor( cell_list );
    DisplayCoor( cell_list );
*/

    uint32_t max_iter = 100000;
    uint32_t cur_iter = 0;

    while ( !solved && cur_iter < max_iter ) {
      cur_iter++;
      tot_iter++;

      for ( auto& cell : cell_list ) {
        cell.e1.pad = 0;
        cell.e2.pad = 0;
        cell.e1.adj = (cell.e2.coor - cell.e1.coor) / 2;
        cell.e2.adj = (cell.e1.coor - cell.e2.coor) / 2;
      }

      update_pad();

      for ( auto& elem : element_list ) {
        U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
        U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
        uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
        uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;

        U aw = a2 - a1;
        U sw = cell_list[ g2 ].e2.coor - cell_list[ g1 ].e1.coor;

        cell_list[ g1 ].e1.adj =
          std::min( +cell_list[ g1 ].e1.adj, (sw - aw) / 2 );
        cell_list[ g2 ].e2.adj =
          std::max( +cell_list[ g2 ].e2.adj, (aw - sw) / 2 );
      }

      {
        edge_t* e1 = nullptr;
        edge_t* e2 = nullptr;
        for ( auto& cell : cell_list ) {
          e1 = &cell.e1;
          if ( e2 ) {
            U overlap =
              (e2->coor + (e2->pad_use ? +e2->pad : 0)) -
              (e1->coor - (e1->pad_use ? +e1->pad : 0));
            e2->adj -= overlap / 2;
            e1->adj += overlap / 2;
          }
          e2 = &cell.e2;
        }
      }

      U acu_adj = 0;
      for ( auto& cell : cell_list ) {
        cell.e1.coor += cell.e1.adj * 0.9;
        cell.e2.coor += cell.e2.adj * 0.9;
        acu_adj += std::abs( cell.e1.adj );
        acu_adj += std::abs( cell.e2.adj );
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
        for ( auto& cell : cell_list ) {
          e1 = &cell.e1;
          if ( e2 ) {
            U overlap = (e2->coor + e2->pad) - (e1->coor - e1->pad);
            if ( overlap > 4 * converge_limit ) {
              e2->pad_use = true;
              e1->pad_use = true;
              solved = false;
            }
          }
          e2 = &cell.e2;
        }
        break;
      }

    }

/*
    printf( "Trial %1d final placement:\n", cur_trial );
    RenumberCoor( cell_list );
    DisplayCoor( cell_list );
*/
  }

  return tot_iter;
}

////////////////////////////////////////////////////////////////////////////////

uint32_t Grid::Solve2( std::vector< cell_t >& cell_list )
{
  bool is_x = &cell_list == &cell_list_x;

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
      uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
      uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;
      U f1 = is_x ? elem.full_bb.min.x : elem.full_bb.min.y;
      U f2 = is_x ? elem.full_bb.max.x : elem.full_bb.max.y;
      U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
      U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;

      U ar = (a2 - a1) / 2;

      U c = (cell_list[ g1 ].e1.coor + cell_list[ g2 ].e2.coor) / 2;
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Min)
        : (elem.anchor_y == SVG::AnchorY::Min)
      ) {
        c = cell_list[ g1 ].e1.coor + ar;
      }
      if (
        is_x
        ? (elem.anchor_x == SVG::AnchorX::Max)
        : (elem.anchor_y == SVG::AnchorY::Max)
      ) {
        c = cell_list[ g2 ].e2.coor - ar;
      }

      cell_list[ g1 ].e1.pad =
        std::max(
          +cell_list[ g1 ].e1.pad,
          cell_list[ g1 ].e1.coor - (c - ar - (a1 - f1))
        );
      cell_list[ g2 ].e2.pad =
        std::max(
          +cell_list[ g2 ].e2.pad,
          (c + ar + (f2 - a2)) - cell_list[ g2 ].e2.coor
        );
    }
  };

  for ( auto& elem : element_list ) {
    uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
    uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;
    if ( g1 == g2 ) {
      U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
      U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
      cell_list[ g2 ].e2.coor = std::max( +cell_list[ g2 ].e2.coor, a2 - a1 );
    } else {
      cell_list[ g1 ].e1.constrained = true;
      cell_list[ g2 ].e2.constrained = true;
    }
  }

/*
  {
    uint32_t seed = std::time( 0 ) * getpid();
    SVG_DBG( "coor seed = " << seed );
    std::srand( seed );
    for ( auto& cell : cell_list ) {
      cell.e1.coor = std::rand();
      cell.e2.coor = std::rand();
    }
  }
*/

  bool solved        = false;
  uint32_t phase     = 0;
  uint32_t tot_iter  = 0;
  uint32_t max_trial = 5;
  uint32_t cur_trial = 0;

  while ( cur_trial < max_trial ) {
    if ( solved ) {
      if ( phase > 0 ) break;
      phase++;
      solved = false;
      cur_trial = 0;
      cell_list.front().e1.locked = true;
      cell_list.back().e2.locked = true;
    }
    cur_trial++;

    if ( cur_trial == max_trial ) {
      // At last trial, just include all padding in the solver such that they
      // can have a chance to take effect before we bail out.
      for ( auto& cell : cell_list ) {
        cell.e1.pad_use = true;
        cell.e2.pad_use = true;
      }
    }

    // Initial placement (not really needed, but gives much faster convergence
    // in some cases).
    for ( size_t i = 0; i < element_list.size(); i++ ) {
      bool moved = false;

      for ( auto i : sorted_indices ) {
        auto& elem = element_list[ i ];

        uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
        uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;
        U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
        U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;

        U d = cell_list[ g1 ].e1.coor + (a2 - a1) - cell_list[ g2 ].e2.coor;
        if ( d > epsilon ) {
          cell_list[ g2 ].e1.coor += d;
          cell_list[ g2 ].e2.coor += d;
          moved = true;
        }
      }

      for ( auto& cell : cell_list ) {
        cell.e1.pad = 0;
        cell.e2.pad = 0;
      }
      update_pad();

      cell_t* prv_cell = nullptr;
      cell_t* cur_cell = nullptr;
      for ( auto& cell : cell_list ) {
        cur_cell = &cell;
        if ( prv_cell ) {
          U d =
            (prv_cell->e2.coor + (prv_cell->e2.pad_use ? +prv_cell->e2.pad : 0)) -
            (cur_cell->e1.coor - (cur_cell->e1.pad_use ? +cur_cell->e1.pad : 0));
          if ( d > epsilon ) {
            cur_cell->e1.coor += d;
            cur_cell->e2.coor += d;
            moved = true;
          }
        }
        prv_cell = cur_cell;
      }

      if ( !moved ) break;
    }

/*
    printf( "Trial %1d initial placement:\n", cur_trial );
    RenumberCoor( cell_list );
    DisplayCoor( cell_list );
*/

    uint32_t max_iter = 100000;
    uint32_t cur_iter = 0;

    while ( !solved && cur_iter < max_iter ) {
      cur_iter++;
      tot_iter++;

      for ( auto& cell : cell_list ) {
        cell.e1.pad   = 0;
        cell.e2.pad   = 0;
        cell.e1.adj   = (cell.e2.coor - cell.e1.coor) / 2;
        cell.e2.adj   = (cell.e1.coor - cell.e2.coor) / 2;
        cell.e1.slack = +num_hi;
        cell.e2.slack = -num_hi;
      }
      update_pad();

      for ( auto& elem : element_list ) {
        uint32_t g1 = is_x ? elem.grid_x1 : elem.grid_y1;
        uint32_t g2 = is_x ? elem.grid_x2 : elem.grid_y2;
        U a1 = is_x ? elem.area_bb.min.x : elem.area_bb.min.y;
        U a2 = is_x ? elem.area_bb.max.x : elem.area_bb.max.y;
        U aw = a2 - a1;
        U sw = cell_list[ g2 ].e2.coor - cell_list[ g1 ].e1.coor;
        U d = (sw - aw) / 2;
        if ( g2 > g1 ) {
          cell_list[ g1 ].e1.slack = std::min( +cell_list[ g1 ].e1.slack, +d );
          cell_list[ g2 ].e2.slack = std::max( +cell_list[ g2 ].e2.slack, -d );
        } else {
          cell_list[ g1 ].e1.adj = std::min( +cell_list[ g1 ].e1.adj, +d );
          cell_list[ g2 ].e2.adj = std::max( +cell_list[ g2 ].e2.adj, -d );
        }
      }

      {
        cell_t* prv_cell = nullptr;
        cell_t* cur_cell = nullptr;
        for ( auto& cell : cell_list ) {
          cur_cell = &cell;
          if ( cur_cell->e1.slack < 0 ) {
            cur_cell->e1.adj += cur_cell->e1.slack;
            cur_cell->e2.adj += cur_cell->e1.slack;
          }
          if ( cur_cell->e2.slack > 0 ) {
            cur_cell->e1.adj += cur_cell->e2.slack;
            cur_cell->e2.adj += cur_cell->e2.slack;
          }
          if ( prv_cell ) {
            U adj =
              (cur_cell->e1.coor - (cur_cell->e1.pad_use ? +cur_cell->e1.pad : 0)) -
              (prv_cell->e2.coor + (prv_cell->e2.pad_use ? +prv_cell->e2.pad : 0));
            adj = adj / 2;
            if ( adj < 0 ) {
              prv_cell->e1.adj += adj;
              prv_cell->e2.adj += adj;
              cur_cell->e1.adj -= adj;
              cur_cell->e2.adj -= adj;
            } else {
              bool p_ok = (phase == 0) || !prv_cell->e2.constrained || prv_cell->e1.constrained;
              bool c_ok = (phase == 0) || !cur_cell->e1.constrained || cur_cell->e2.constrained;
              if ( prv_cell->e1.slack > 0 && p_ok ) {
                U a = std::min( +adj, +prv_cell->e1.slack );
                prv_cell->e1.adj += a;
                prv_cell->e2.adj += a;
              }
              if ( cur_cell->e2.slack < 0 && c_ok ) {
                U a = std::min( +adj, -cur_cell->e2.slack );
                cur_cell->e1.adj -= a;
                cur_cell->e2.adj -= a;
              }
            }
          }
          prv_cell = cur_cell;
        }
      }

      U acu_adj = 0;
      U max_adj = 0;
      for ( auto& cell : cell_list ) {
        if ( cell.e1.locked ) cell.e1.adj = 0;
        if ( cell.e2.locked ) cell.e2.adj = 0;
        cell.e1.coor += cell.e1.adj * 0.3;
        cell.e2.coor += cell.e2.adj * 0.3;
        acu_adj += std::abs( cell.e1.adj );
        acu_adj += std::abs( cell.e2.adj );
        max_adj = std::max( +max_adj, std::abs( cell.e1.adj ) );
        max_adj = std::max( +max_adj, std::abs( cell.e2.adj ) );
      }

      U converge_limit = 1e-4;

/*
      printf(
        "-    %4d    %10d    %16.10f    %12.10f\n",
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
        for ( auto& cell : cell_list ) {
          e1 = &cell.e1;
          if ( e2 ) {
            U overlap = (e2->coor + e2->pad) - (e1->coor - e1->pad);
            if ( overlap > 4 * converge_limit ) {
              e2->pad_use = true;
              e1->pad_use = true;
              solved = false;
            }
          }
          e2 = &cell.e2;
        }
        break;
      }

    }

/*
    printf( "Trial %1d final placement:\n", cur_trial );
    RenumberCoor( cell_list );
    DisplayCoor( cell_list );
*/
  }

  return tot_iter;
}

////////////////////////////////////////////////////////////////////////////////

void Grid::DisplayCoor( std::vector< cell_t >& cell_list )
{
  uint32_t n = 0;
  for ( auto& cell : cell_list ) {
    n++;
    if ( n > 8 ) {
      printf( "..." );
      break;
    }
    printf( "| %8.3f : %8.3f |", +cell.e1.coor, +cell.e2.coor );
  }
  printf( "\n" );
}

void Grid::DisplayAdj( std::vector< cell_t >& cell_list )
{
  uint32_t n = 0;
  for ( auto& cell : cell_list ) {
    n++;
    if ( n > 8 ) {
      printf( "..." );
      break;
    }
    printf( "· %8.3f : %8.3f ·", +cell.e1.adj, +cell.e2.adj );
  }
  printf( "\n" );
}

void Grid::DisplaySlack( std::vector< cell_t >& cell_list )
{
  uint32_t n = 0;
  for ( auto& cell : cell_list ) {
    n++;
    if ( n > 8 ) {
      printf( "..." );
      break;
    }
    printf( "~ %8.3f : %8.3f ~", +cell.e1.slack, +cell.e2.slack );
  }
  printf( "\n" );
}

void Grid::RenumberCoor( std::vector< cell_t >& cell_list )
{
  bool first = true;
  U ofs = 0;
  for ( auto& cell : cell_list ) {
    if ( first ) {
      ofs = cell.e1.coor;
    }
    cell.e1.coor -= ofs;
    cell.e2.coor -= ofs;
    first = false;
  }
}

void Grid::Test( void )
{
  auto AddElem = [&](
    U w, uint32_t x1, uint32_t x2, U x1_pad = 0, U x2_pad = 0
  ) {
    element_t elem;
    elem.full_bb.Update( 0, 0 ); elem.grid_x1 = x1; elem.grid_y1 = 0;
    elem.full_bb.Update( w, 0 ); elem.grid_x2 = x2; elem.grid_y2 = 0;
    elem.area_bb.Update( elem.full_bb );
    elem.full_bb.min.x -= x1_pad;
    elem.full_bb.max.x += x2_pad;
    element_list.push_back( elem );
    SVG_DBG( x1 << ":" << x2 );
  };

/*
  for ( int i = 0; i < 100; i++ ) {
    AddElem( 10, i, i );
    if ( i > 0 ) AddElem( 30, i - 1, i );
    if ( i > 1 ) AddElem( 60, i - 2, i );
    if ( i > 2 ) AddElem( 300, i - 3, i );
  }
  AddElem( 20*100 - 10, 0, 100 - 1 );
*/
/*
  {
    uint32_t seed = std::time( 0 ) * getpid();
    SVG_DBG( "grid seed = " << seed );
    std::srand( seed );
    uint32_t gw = 6;
    for ( uint32_t i = 0; i < 6; i++ ) {
      AddElem(
        10 * (1 + std::abs( rand() ) % 10),
        std::abs( rand() ) % gw,
        std::abs( rand() ) % gw
      );
    }
  }
*/

  AddElem( 100, 0, 0, 20, 20 );
  AddElem( 100, 1, 1, 20, 20 );
  AddElem( 100, 2, 2 );
  AddElem( 100, 3, 3, 20, 20 );
  AddElem( 100, 0, 1 );
  AddElem( 300, 2, 3 );
  AddElem( 300, 0, 2 );


  Init();

  uint32_t tot_iter = Solve2( cell_list_x );
  SVG_DBG( "tot_iter = " << tot_iter );
  if ( tot_iter > 100000 ) exit( 1 );

  if ( cell_list_x.back().e2.coor > 10000 ) exit( 1 );

  return;
}

////////////////////////////////////////////////////////////////////////////////
