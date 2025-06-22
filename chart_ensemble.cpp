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
  html_db = new HTML( this );
  legend_obj = new Legend( this );

  foreground_color.Set( ColorName::black );
  background_color.Set( ColorName::white );
  border_color.Set( ColorName::black );
}

Ensemble::~Ensemble( void )
{
  for ( auto& elem : grid.element_list ) {
    delete elem.chart;
  }
  delete legend_obj;
  delete html_db;
  delete canvas;
}

////////////////////////////////////////////////////////////////////////////////

bool Ensemble::NewChart(
  uint32_t grid_row1, uint32_t grid_col1,
  uint32_t grid_row2, uint32_t grid_col2,
  Pos align_hor,
  Pos align_ver
)
{
  for ( auto& elem : grid.element_list ) {
    if (
      !(grid_col1 < elem.grid_x1 && grid_col2 < elem.grid_x1) &&
      !(grid_col1 > elem.grid_x2 && grid_col2 > elem.grid_x2) &&
      !(grid_row1 < elem.grid_y1 && grid_row2 < elem.grid_y1) &&
      !(grid_row1 > elem.grid_y2 && grid_row2 > elem.grid_y2)
    )
      return false;
  }

  Grid::element_t elem;
  elem.chart = new Main( this, top_g->AddNewGroup() );
  html_db->NewChart( elem.chart );

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

  grid.element_list.push_back( elem );
  last_chart = elem.chart;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SetLetterSpacing(
  float width_adj, float height_adj, float baseline_adj
)
{
  this->width_adj    = width_adj;
  this->height_adj   = height_adj;
  this->baseline_adj = baseline_adj;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SetTitle( const std::string& txt )
{
  title = txt;
}

void Ensemble::SetSubTitle( const std::string& txt )
{
  sub_title = txt;
}

void Ensemble::SetSubSubTitle( const std::string& txt )
{
  sub_sub_title = txt;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SetLegendHeading( const std::string& txt )
{
  legend_obj->heading = txt;
}

void Ensemble::SetLegendFrame( bool enable )
{
  legend_frame = enable;
  legend_frame_specified = true;
}

void Ensemble::SetLegendPos( Pos pos )
{
  // Shared legend grid elements are identified by chart == nullptr; start
  // by removing if already specified earlier.
  grid.element_list.erase(
    std::remove_if(
      grid.element_list.begin(), grid.element_list.end(),
      []( const Grid::element_t& elem ) { return elem.chart == nullptr; }
    ),
    grid.element_list.end()
  );

  legend_obj->pos = pos;
  legend_obj->grid_coor_specified = false;
}

bool Ensemble::SetLegendPos(
  uint32_t grid_row1, uint32_t grid_col1,
  uint32_t grid_row2, uint32_t grid_col2,
  Chart::Pos align_hor, Chart::Pos align_ver
)
{
  SetLegendPos( Pos::Auto );

  for ( auto& elem : grid.element_list ) {
    if (
      !(grid_col1 < elem.grid_x1 && grid_col2 < elem.grid_x1) &&
      !(grid_col1 > elem.grid_x2 && grid_col2 > elem.grid_x2) &&
      !(grid_row1 < elem.grid_y1 && grid_row2 < elem.grid_y1) &&
      !(grid_row1 > elem.grid_y2 && grid_row2 > elem.grid_y2)
    )
      return false;
  }

  Grid::element_t elem;
  elem.full_bb.Update( 0, 0 );
  elem.area_bb.Update( 0, 0 );

  // Note that the Y grid coordinates are in normal bottom to top "mathematical"
  // direction, whereas rows goes top to bottom. The InitGrid() will reorient
  // the Y grid coordinates to match these notations.
  elem.grid_x1 = grid_col1;
  elem.grid_y1 = grid_row1;
  elem.grid_x2 = grid_col2;
  elem.grid_y2 = grid_row2;
  elem.anchor_x_defined = true;
  elem.anchor_y_defined = true;
  elem.anchor_x = AnchorX::Mid;
  elem.anchor_y = AnchorY::Mid;
  if ( align_hor == Pos::Left   ) elem.anchor_x = AnchorX::Min;
  if ( align_hor == Pos::Right  ) elem.anchor_x = AnchorX::Max;
  if ( align_ver == Pos::Bottom ) elem.anchor_y = AnchorY::Min;
  if ( align_ver == Pos::Top    ) elem.anchor_y = AnchorY::Max;

  grid.element_list.push_back( elem );

  legend_obj->grid_coor_specified = true;

  return true;
}

void Ensemble::SetLegendSize( float size )
{
  legend_obj->size = size;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::AddFootnote(std::string& txt)
{
  footnotes.emplace_back( footnote_t{ txt, Pos::Left } );
}

void Ensemble::SetFootnotePos( Pos pos )
{
  if ( !footnotes.empty() ) {
    footnotes.back().pos = pos;
  }
}

void Ensemble::SetFootnoteLine( bool footnote_line )
{
  this->footnote_line = footnote_line;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::InitGrid( void )
{
  grid.Init( std::max( 0.0, +grid_padding ), area_padding );

  for ( auto& elem : grid.element_list ) {
    // Convert row location to Y grid coordinates.
    std::swap( elem.grid_y1, elem.grid_y2 );
    elem.grid_y1 = grid.max_y - elem.grid_y1;
    elem.grid_y2 = grid.max_y - elem.grid_y2;

    if ( !elem.anchor_x_defined ) {
      if ( elem.grid_x1 == 0 && elem.grid_x2 < grid.max_x ) {
        elem.anchor_x = SVG::AnchorX::Min;
      }
      if ( elem.grid_x1 > 0 && elem.grid_x2 == grid.max_x ) {
        elem.anchor_x = SVG::AnchorX::Max;
      }
    }

    if ( !elem.anchor_y_defined ) {
      if ( elem.grid_y1 == 0 && elem.grid_y2 < grid.max_y ) {
        elem.anchor_y = SVG::AnchorY::Min;
      }
      if ( elem.grid_y1 > 0 && elem.grid_y2 == grid.max_y ) {
        elem.anchor_y = SVG::AnchorY::Max;
      }
      elem.anchor_y_defined = true;
    }

    if ( elem.chart ) {
      elem.area_bb.Update( 0, 0 );
      elem.area_bb.Update( elem.chart->chart_w, elem.chart->chart_h );
      if ( grid_padding < 0 ) {
        elem.full_bb = elem.area_bb;
      } else {
        elem.full_bb = elem.chart->GetGroup()->GetBB();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::SolveGrid( void )
{
  grid.Solve( grid.cell_list_x );
  grid.Solve( grid.cell_list_y );
}

////////////////////////////////////////////////////////////////////////////////

SVG::BoundaryBox Ensemble::TopBB( void )
{
  BoundaryBox bb = top_g->GetBB();
  for ( auto& elem : grid.element_list ) {
    if ( elem.chart ) {
      bb.Update(
        elem.area_bb.min.x + elem.chart->g_dx - max_area_pad,
        elem.area_bb.min.y + elem.chart->g_dy - max_area_pad
      );
      bb.Update(
        elem.area_bb.max.x + elem.chart->g_dx + max_area_pad,
        elem.area_bb.max.y + elem.chart->g_dy + max_area_pad
      );
    }
  }
  return bb;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::BuildLegends( void )
{
  if ( legend_obj->Cnt() == 0 ) return;

  BoundaryBox build_bb;
  BoundaryBox moved_bb;

  Group* legend_g = top_g->AddNewGroup();
  legend_g->Attr()->TextFont()->SetSize( 14 * legend_obj->size );

  bool framed =
    legend_frame_specified ? legend_frame : !legend_obj->heading.empty();

  Legend::LegendDims legend_dims;
  legend_obj->CalcLegendDims( framed, legend_g, legend_dims );

  U padding = 2 * std::max( grid_padding, area_padding );
  U in_grid_mx = std::max( 0.0, legend_obj->MarginX( framed ) - padding );
  U in_grid_my = std::max( 0.0, legend_obj->MarginY( framed ) - padding );

  if ( legend_obj->grid_coor_specified ) {
    Grid::element_t* elem = nullptr;
    for ( auto& e : grid.element_list ) {
      if ( !e.chart ) elem = &e;
    }

    BoundaryBox avail_bb;
    U avail_w;
    U avail_h;
    auto UpdateAvail = [&]( void )
    {
      avail_bb.Reset();
      avail_bb.Update(
        grid.cell_list_x[ elem->grid_x1 ].e1.coor,
        grid.cell_list_y[ elem->grid_y1 ].e1.coor
      );
      avail_bb.Update(
        grid.cell_list_x[ elem->grid_x2 ].e2.coor,
        grid.cell_list_y[ elem->grid_y2 ].e2.coor
      );
      avail_w = avail_bb.max.x - avail_bb.min.x;
      avail_h = avail_bb.max.y - avail_bb.min.y;
    };

    uint32_t nx = 1;

    U min_w;
    U min_h;
    U legend_w;
    U legend_h;

    legend_obj->GetDims( min_w, legend_h, legend_dims, framed, 1 );
    legend_obj->GetDims( legend_w, min_h, legend_dims, framed, legend_obj->Cnt() );

    auto update = [&]( void )
    {
      legend_obj->GetDims( legend_w, legend_h, legend_dims, framed, nx );
      elem->full_bb.Reset();
      elem->full_bb.Update( 0, 0 );
      elem->full_bb.Update( legend_w + 2 * in_grid_mx, legend_h + 2 * in_grid_my );
      elem->area_bb = elem->full_bb;
      SolveGrid();
      UpdateAvail();
    };

    UpdateAvail();

    bool no_space_x = avail_w < 1;
    bool no_space_y = avail_h < 1;

    if ( no_space_x && no_space_y ) {
      legend_obj->GetBestFit(
        legend_dims, nx, framed, 1.0, 1.0, num_hi, num_hi
      );
      update();
    } else
    if ( no_space_x ) {
      legend_obj->GetBestFit(
        legend_dims, nx, framed, 0, avail_h * 1.5, min_w, avail_h
      );
      update();
    } else
    if ( no_space_y ) {
      legend_obj->GetBestFit(
        legend_dims, nx, framed, avail_w * 1.5, 0, avail_w, min_h
      );
      update();
    } else
    {
      legend_obj->GetBestFit( legend_dims, nx, framed, avail_w, avail_h );
      update();
    }

    legend_obj->BuildLegends(
      framed, ForegroundColor(), LegendColor(),
      legend_g->AddNewGroup(), nx
    );
    Object* legend = legend_g->Last();
    build_bb = legend->GetBB();
    U x = (avail_bb.min.x + avail_bb.max.x) / 2;
    U y = (avail_bb.min.y + avail_bb.max.y) / 2;
    if ( elem->anchor_x == AnchorX::Min ) x = avail_bb.min.x;
    if ( elem->anchor_x == AnchorX::Max ) x = avail_bb.max.x;
    if ( elem->anchor_y == AnchorY::Min ) y = avail_bb.min.y;
    if ( elem->anchor_y == AnchorY::Max ) y = avail_bb.max.y;
    legend->MoveTo( elem->anchor_x, elem->anchor_y, x, y );
    moved_bb = legend->GetBB();
  }

  if ( !build_bb.Defined() && legend_obj->pos == Pos::Auto ) {
    std::vector< Grid::hole_t > holes;

    grid.GetHoles( holes );

    std::sort(
      holes.begin(), holes.end(),
      [&]( Grid::hole_t a, Grid::hole_t b ) {
        bool a_top = a.y2 == grid.max_y;
        bool a_bot = a.y1 == 0;
        bool a_side = a.x1 == 0 || a.x2 == grid.max_x;
        bool a_corner = a_side && (a_top || a_bot);
        bool a_rim = a_side || a_top || a_bot;

        bool b_top = b.y2 == grid.max_y;
        bool b_bot = b.y1 == 0;
        bool b_side = b.x1 == 0 || b.x2 == grid.max_x;
        bool b_corner = b_side && (b_top || b_bot);
        bool b_rim = b_side || b_top || b_bot;

        if ( a_rim != b_rim ) return a_rim;
        if ( a_corner != b_corner ) return a_corner;
        if ( a_bot != b_bot ) return a_bot;
        if ( a_top != b_top ) return a_top;

        double a_size = (a.bb.max.x - a.bb.min.x) * (a.bb.max.y - a.bb.min.y);
        double b_size = (b.bb.max.x - b.bb.min.x) * (b.bb.max.y - b.bb.min.y);

        return a_size > b_size;
      }
    );

/*
    {
      uint32_t n = 1;
      for ( auto& h : holes ) {
        top_g->Add( new Rect( h.bb.min, h.bb.max ) );
        top_g->Last()->Attr()->SetLineWidth( 2 );
        top_g->Last()->Attr()->FillColor()->Clear();
        top_g->Last()->Attr()->LineColor()->Set( ColorName::red );
        std::ostringstream oss;
        oss << n;
        top_g->Add( new Text( oss.str() ) );
        top_g->Last()->Attr()->TextFont()->SetSize( 20 );
        top_g->Last()->Attr()->TextColor()->Set( ColorName::black );
        top_g->Last()->MoveTo(
          AnchorX::Mid, AnchorY::Mid,
          (h.bb.min.x + h.bb.max.x) / 2,
          (h.bb.min.y + h.bb.max.y) / 2
        );
        ++n;
      }
    }
*/

    for ( auto& hole : holes ) {
      U avail_w = hole.bb.max.x - hole.bb.min.x - 2 * in_grid_mx;
      U avail_h = hole.bb.max.y - hole.bb.min.y - 2 * in_grid_my;
      uint32_t nx;
      bool fits =
        avail_w > 0 && avail_h > 0 &&
        legend_obj->GetBestFit( legend_dims, nx, framed, avail_w, avail_h );
      if ( fits ) {
        legend_obj->BuildLegends(
          framed, ForegroundColor(), LegendColor(),
          legend_g->AddNewGroup(), nx
        );
        Object* legend = legend_g->Last();
        build_bb = legend->GetBB();
        legend->MoveTo(
          AnchorX::Mid, AnchorY::Mid,
          (hole.bb.min.x + hole.bb.max.x) / 2,
          (hole.bb.min.y + hole.bb.max.y) / 2
        );
        moved_bb = legend->GetBB();
        break;
      }
    }
  }

  if ( !build_bb.Defined() ) {
    BoundaryBox all_bb = top_g->GetBB();

    if ( legend_obj->pos == Pos::Left || legend_obj->pos == Pos::Right ) {

      U mx = legend_obj->MarginX( framed );
      U x = all_bb.min.x - mx;
      U y = all_bb.max.y;
      AnchorX anchor_x = AnchorX::Max;
      if ( legend_obj->pos == Pos::Right ) {
        x = all_bb.max.x + mx;
        anchor_x = AnchorX::Min;
      }
      U avail_h = all_bb.max.y - all_bb.min.y;
      uint32_t nx;
      legend_obj->GetBestFit( legend_dims, nx, framed, 0, avail_h );
      legend_obj->BuildLegends(
        framed, ForegroundColor(), LegendColor(),
        legend_g->AddNewGroup(), nx
      );
      Object* legend = legend_g->Last();
      build_bb = legend->GetBB();
      legend->MoveTo( anchor_x, AnchorY::Max, x, y );
      moved_bb = legend->GetBB();

    } else {

      U my = legend_obj->MarginY( framed );
      U x = (all_bb.min.x + all_bb.max.x) / 2;
      U y = all_bb.min.y - my;
      AnchorY anchor_y = AnchorY::Max;
      if ( legend_obj->pos == Pos::Top ) {
        y = all_bb.max.y + my;
        anchor_y = AnchorY::Min;
      }
      U avail_w = all_bb.max.x - all_bb.min.x;
      uint32_t nx;
      legend_obj->GetBestFit( legend_dims, nx, framed, avail_w, 0 );
      legend_obj->BuildLegends(
        framed, ForegroundColor(), LegendColor(),
        legend_g->AddNewGroup(), nx
      );
      Object* legend = legend_g->Last();
      build_bb = legend->GetBB();
      legend->MoveTo( AnchorX::Mid, anchor_y, x, y );
      moved_bb = legend->GetBB();

    }
  }

  for ( auto series : legend_obj->series_list ) {
    html_db->MoveLegend(
      series,
      moved_bb.min.x - build_bb.min.x,
      moved_bb.min.y - build_bb.min.y
    );
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::BuildTitle( void )
{
  U dx = 0;
  U dy = 16;
  U spacing = 4 * title_size;

  BoundaryBox bb = TopBB();

  U line_y = bb.max.y + dy / 2;

  U x = (bb.min.x + bb.max.x) / 2;
  AnchorX a = AnchorX::Mid;
  if ( title_pos == Pos::Left ) {
    x = bb.min.x + dx;
    a = AnchorX::Min;
  }
  if ( title_pos == Pos::Right ) {
    x = bb.max.x - dx;
    a = AnchorX::Max;
  }

  U y = bb.max.y + dy;
  if ( !sub_sub_title.empty() ) {
    Object* obj =
      Label::CreateLabel( top_g, sub_sub_title, 14 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !sub_title.empty() ) {
    Object* obj = Label::CreateLabel( top_g, sub_title, 20 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !title.empty() ) {
    Object* obj = Label::CreateLabel( top_g, title, 36 * title_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
  }

  if ( title_line ) {
    bb = TopBB();
    top_g->Add( new Line( bb.min.x + dx, line_y, bb.max.x - dx, line_y ) );
    top_g->Last()->Attr()->LineColor()->Set( ForegroundColor() );
    top_g->Last()->Attr()->SetLineWidth( 1 );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::BuildFootnotes( void )
{
  U dx = 0;
  U dy = 16;
  U spacing = 2 * footnote_size;

  BoundaryBox bb = TopBB();

  if ( footnote_line ) {
    dy = dy / 2;
    top_g->Add( new Line(
      bb.min.x + dx, bb.min.y - dy, bb.max.x - dx, bb.min.y - dy
    ) );
    top_g->Last()->Attr()->LineColor()->Set( ForegroundColor() );
    top_g->Last()->Attr()->SetLineWidth( 1 );
  }

  for ( const auto& footnote : footnotes ) {
    if ( footnote.txt.empty() ) continue;

    bb = top_g->GetBB();
    U x = bb.min.x + dx;
    U y = bb.min.y - dy;
    AnchorX a = AnchorX::Min;
    Label::CreateLabel( top_g, footnote.txt, 14 * footnote_size );
    top_g->Last()->Attr()->TextColor()->Set( ForegroundColor() );
    if ( footnote.pos == Pos::Center ) {
      x = (bb.min.x + bb.max.x) / 2;
      a = AnchorX::Mid;
    }
    if ( footnote.pos == Pos::Right ) {
      x = bb.max.x - dx;
      a = AnchorX::Max;
    }
    top_g->Last()->MoveTo( a, AnchorY::Max, x, y );

    dy = spacing;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::BuildBackground( void )
{
  BoundaryBox top_bb = TopBB();

  {
    BoundaryBox bb{ top_bb };

    U delta = padding + border_width + margin;
    bb.min.x -= delta;
    bb.max.x += delta;
    bb.min.y -= delta;
    bb.max.y += delta;

    if ( enable_html ) {
      for ( auto& elem : grid.element_list ) {
        if ( elem.chart ) {
          bb.Update(
            elem.area_bb.min.x + elem.chart->g_dx,
            elem.area_bb.min.y + elem.chart->g_dy
          );
          bb.Update(
            elem.area_bb.max.x + elem.chart->g_dx,
            elem.area_bb.max.y + elem.chart->g_dy
          );
        }
      }
    }

    top_g->Add( new Rect( bb.min, bb.max ) );
    top_g->Last()->Attr()->FillColor()->Clear();
    top_g->Last()->Attr()->LineColor()->Clear();
    top_g->Last()->Attr()->SetLineWidth( 0 );
    top_g->FrontToBack();
  }

  {
    BoundaryBox bb{ top_bb };

    bb.min.x -= padding + border_width / 2;
    bb.max.x += padding + border_width / 2;
    bb.min.y -= padding + border_width / 2;
    bb.max.y += padding + border_width / 2;

    top_g->Add( new Rect( bb.min, bb.max ) );
    top_g->Last()->Attr()->SetLineWidth( border_width );
    if ( border_width > 0 ) {
      top_g->Last()->Attr()->LineColor()->Set( BorderColor() );
    } else {
      top_g->Last()->Attr()->LineColor()->Clear();
    }
    top_g->FrontToBack();
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::MoveCharts( void )
{
  for ( auto& elem : grid.element_list ) {
    if ( elem.chart ) {
      U gx1 = grid.cell_list_x[ elem.grid_x1 ].e1.coor;
      U gx2 = grid.cell_list_x[ elem.grid_x2 ].e2.coor;
      U gy1 = grid.cell_list_y[ elem.grid_y1 ].e1.coor;
      U gy2 = grid.cell_list_y[ elem.grid_y2 ].e2.coor;

      U mx = (gx1 + gx2) / 2 - (elem.area_bb.min.x + elem.area_bb.max.x) / 2;
      U my = (gy1 + gy2) / 2 - (elem.area_bb.min.y + elem.area_bb.max.y) / 2;

      if ( elem.anchor_x == SVG::AnchorX::Min ) mx = gx1 - elem.area_bb.min.x;
      if ( elem.anchor_x == SVG::AnchorX::Max ) mx = gx2 - elem.area_bb.max.x;

      if ( elem.anchor_y == SVG::AnchorY::Min ) my = gy1 - elem.area_bb.min.y;
      if ( elem.anchor_y == SVG::AnchorY::Max ) my = gy2 - elem.area_bb.max.y;

      elem.chart->Move( mx, my );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string Ensemble::Build( void )
{
  if ( Empty() ) {
    NewChart( 0, 0, 0, 0 );
  }

  top_g->Attr()->TextFont()->SetFamily(
    "monospace"
  );
  top_g->Attr()->TextFont()
    ->SetWidthFactor( width_adj )
    ->SetHeightFactor( height_adj )
    ->SetBaselineFactor( baseline_adj );
  top_g->Attr()->SetTextMathDigits( true );

  top_g->Attr()->TextColor()->Set( ForegroundColor() );
  top_g->Attr()->LineColor()->Set( ForegroundColor() );
  top_g->Attr()->FillColor()->Set( BackgroundColor() );

  max_area_pad = 0;
  for ( auto& elem : grid.element_list ) {
    if ( elem.chart ) {
      elem.chart->Build();
      U area_pad = elem.chart->GetAreaPadding();
      max_area_pad = std::max( max_area_pad, area_pad );
    }
  }

  if ( legend_obj->Cnt() == 0 ) {
    SetLegendPos( Pos::Auto );
  }

  InitGrid();
  SolveGrid();

  if ( legend_obj->grid_coor_specified ) {
    BuildLegends();
    MoveCharts();
  } else {
    MoveCharts();
    BuildLegends();
  }

  BuildTitle();
  BuildFootnotes();

  BuildBackground();

/*
  {
    BoundaryBox bb = canvas->TopGroup()->GetBB();

    Group* g = canvas->TopGroup()->AddNewGroup();
    g->Attr()->LineColor()->Set( ColorName::orange );
    g->Attr()->SetLineWidth( 2 )->LineColor()->SetOpacity( 0.5 );
    g->Attr()->FillColor()->Set( ColorName::yellow );
    g->Attr()->FillColor()->SetOpacity( 0.1 );

    for ( auto& s : grid.cell_list_x ) {
      g->Add( new Rect( s.e1.coor, bb.min.y, s.e2.coor, bb.max.y ) );
      g->Last()->Attr()->SetLineWidth( 0 );
      g->Add( new Line( s.e1.coor, bb.min.y, s.e1.coor, bb.max.y ) );
      g->Add( new Line( s.e2.coor, bb.min.y, s.e2.coor, bb.max.y ) );
    }

    for ( auto& s : grid.cell_list_y ) {
      g->Add( new Rect( bb.min.x, s.e1.coor, bb.max.x, s.e2.coor ) );
      g->Last()->Attr()->SetLineWidth( 0 );
      g->Add( new Line( bb.min.x, s.e1.coor, bb.max.x, s.e1.coor ) );
      g->Add( new Line( bb.min.x, s.e2.coor, bb.max.x, s.e2.coor ) );
    }
  }
*/

  std::ostringstream oss;
  if ( enable_html ) {
    oss << html_db->GenHTML( canvas );
  } else {
    oss << canvas->GenSVG();
  }
  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
