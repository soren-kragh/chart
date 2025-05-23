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

void Ensemble::SetHeading( const std::string& txt )
{
  heading = txt;
}

void Ensemble::SetSubHeading( const std::string& txt )
{
  sub_heading = txt;
}

void Ensemble::SetSubSubHeading( const std::string& txt )
{
  sub_sub_heading = txt;
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
  legend_obj->pos = pos;
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
  grid.Init();

  for ( auto& elem : grid.element_list ) {
    elem.area_bb.Update( 0, 0 );
    elem.area_bb.Update( elem.chart->chart_w, elem.chart->chart_h );

    if ( grid_padding < 0 ) {
      elem.full_bb = elem.area_bb;
    } else {
      elem.full_bb = elem.chart->GetGroup()->GetBB();
      elem.full_bb.min.x -= grid_padding;
      elem.full_bb.max.x += grid_padding;
      elem.full_bb.min.y -= grid_padding;
      elem.full_bb.max.y += grid_padding;
    }

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
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::ComputeGrid( void )
{
  InitGrid();
  grid.Solve2( grid.cell_list_x );
  grid.Solve2( grid.cell_list_y );
}

////////////////////////////////////////////////////////////////////////////////

SVG::BoundaryBox Ensemble::TopBB( void )
{
  BoundaryBox bb = top_g->GetBB();
  for ( auto& elem : grid.element_list ) {
    bb.Update(
      elem.area_bb.min.x + elem.chart->g_dx - max_area_pad,
      elem.area_bb.min.y + elem.chart->g_dy - max_area_pad
    );
    bb.Update(
      elem.area_bb.max.x + elem.chart->g_dx + max_area_pad,
      elem.area_bb.max.y + elem.chart->g_dy + max_area_pad
    );
  }
  return bb;
}

////////////////////////////////////////////////////////////////////////////////

void Ensemble::BuildLegends( void )
{
  if ( legend_obj->Cnt() == 0 ) return;

  BoundaryBox build_bb;
  BoundaryBox moved_bb;
  BoundaryBox all_bb = top_g->GetBB();

  Group* legend_g = top_g->AddNewGroup();
  legend_g->Attr()->TextFont()->SetSize( 14 * legend_obj->size );

  bool framed =
    legend_frame_specified ? legend_frame : !legend_obj->heading.empty();

  Legend::LegendDims legend_dims;
  legend_obj->CalcLegendDims( framed, legend_g, legend_dims );

  if ( legend_obj->pos == Pos::Left || legend_obj->pos == Pos::Right ) {

    U mx = framed ? +box_spacing : 20;
    U x = all_bb.min.x - mx;
    U y = all_bb.max.y;
    AnchorX anchor_x = AnchorX::Max;
    if ( legend_obj->pos == Pos::Right ) {
      x = all_bb.max.x + mx;
      anchor_x = AnchorX::Min;
    }
    U avail_h = all_bb.max.y - all_bb.min.y;
    uint32_t nx = 1;
    while ( 1 ) {
      uint32_t ny = (legend_obj->Cnt() + nx - 1) / nx;
      U need_h = ny * legend_dims.sy + (ny - 1) * legend_dims.dy;
      if ( need_h > avail_h && ny > 1 ) {
        nx++;
        continue;
      }
      break;
    }
    legend_obj->BuildLegends(
      framed, ForegroundColor(), LegendColor(),
      legend_g->AddNewGroup(), nx
    );
    Object* legend = legend_g->Last();
    build_bb = legend->GetBB();
    legend->MoveTo( anchor_x, AnchorY::Max, x, y );
    moved_bb = legend->GetBB();

  } else {

    U my = box_spacing;
    U x = (all_bb.min.x + all_bb.max.x) / 2;
    U y = all_bb.min.y - my;
    AnchorY anchor_y = AnchorY::Max;
    if ( legend_obj->pos == Pos::Top ) {
      y = all_bb.max.y + my;
      anchor_y = AnchorY::Min;
    }
    U avail_w = all_bb.max.x - all_bb.min.x;
    uint32_t nx = legend_obj->Cnt();
    uint32_t ny = 1;
    while ( 1 ) {
      nx = (legend_obj->Cnt() + ny - 1) / ny;
      U need_w = nx * legend_dims.sx + (nx - 1) * legend_dims.dx;
      if ( need_w > avail_w && nx > 1 ) {
        ny++;
        continue;
      }
      break;
    }
    legend_obj->BuildLegends(
      framed, ForegroundColor(), LegendColor(),
      legend_g->AddNewGroup(), nx
    );
    Object* legend = legend_g->Last();
    build_bb = legend->GetBB();
    legend->MoveTo( AnchorX::Mid, anchor_y, x, y );
    moved_bb = legend->GetBB();

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

void Ensemble::BuildHeading( void )
{
  U dx = 0;
  U dy = 16;
  U spacing = 4 * heading_size;

  BoundaryBox bb = TopBB();

  U line_y = bb.max.y + dy / 2;

  U x = (bb.min.x + bb.max.x) / 2;
  AnchorX a = AnchorX::Mid;
  if ( heading_pos == Pos::Left ) {
    x = bb.min.x + dx;
    a = AnchorX::Min;
  }
  if ( heading_pos == Pos::Right ) {
    x = bb.max.x - dx;
    a = AnchorX::Max;
  }

  U y = bb.max.y + dy;
  if ( !sub_sub_heading.empty() ) {
    Object* obj =
      Label::CreateLabel( top_g, sub_sub_heading, 14 * heading_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !sub_heading.empty() ) {
    Object* obj = Label::CreateLabel( top_g, sub_heading, 20 * heading_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
    y += bb.max.y - bb.min.y + spacing;
  }
  if ( !heading.empty() ) {
    Object* obj = Label::CreateLabel( top_g, heading, 36 * heading_size );
    obj->MoveTo( a, AnchorY::Min, x, y );
    bb = obj->GetBB();
  }

  if ( heading_line ) {
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
  U spacing = 2 * heading_size;

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
        bb.Update(
          elem.area_bb.min.x + elem.chart->g_dx - snap_point_radius,
          elem.area_bb.min.y + elem.chart->g_dy - snap_point_radius
        );
        bb.Update(
          elem.area_bb.max.x + elem.chart->g_dx + snap_point_radius,
          elem.area_bb.max.y + elem.chart->g_dy + snap_point_radius
        );
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

std::string Ensemble::Build( void )
{
  if ( Empty() ) {
    NewChart( 0, 0, 0, 0 );
  }

  top_g->Attr()->TextFont()->SetFamily(
    "Noto Mono,Lucida Console,Courier New,monospace"
  );
  top_g->Attr()->TextFont()
    ->SetWidthFactor( width_adj )
    ->SetHeightFactor( height_adj )
    ->SetBaselineFactor( baseline_adj );

  top_g->Attr()->TextColor()->Set( ForegroundColor() );
  top_g->Attr()->LineColor()->Set( ForegroundColor() );
  top_g->Attr()->FillColor()->Set( BackgroundColor() );

  max_area_pad = 0;
  for ( auto& elem : grid.element_list ) {
    elem.chart->Build();
    U area_pad = elem.chart->GetAreaPadding();
    max_area_pad = std::max( max_area_pad, area_pad );
  }

  ComputeGrid();

  for ( auto& elem : grid.element_list ) {
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

  BuildLegends();

  BuildHeading();
  BuildFootnotes();

  BuildBackground();

/*
  {
    BoundaryBox bb = canvas->TopGroup()->GetBB();

    Group* g = canvas->TopGroup()->AddNewGroup();
    g->Attr()->LineColor()->Set( ColorName::black );
    g->Attr()->SetLineWidth( 0.5 );
    g->Attr()->FillColor()->Set( ColorName::yellow );
    g->Attr()->FillColor()->SetOpacity( 0.2 );

    for ( auto& s : grid.cell_list_x ) {
      g->Add( new Rect( s.e1.coor, bb.min.y, s.e2.coor, bb.max.y ) );
    }

    for ( auto& s : grid.cell_list_y ) {
      g->Add( new Rect( bb.min.x, s.e1.coor, bb.max.x, s.e2.coor ) );
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
