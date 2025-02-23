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

#include <chart_tag.h>
#include <chart_axis.h>
#include <chart_series.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Tag::Tag( void )
{
  tag.valid = false;
}

Tag::~Tag( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Tag::RecordTag( const SVG::BoundaryBox& bb )
{
  recorded_tags.push_back( bb );
}

bool Tag::Collision( const SVG::BoundaryBox& bb )
{
  // Search backwards since most recently added tag is most likely to collide.
  for ( auto it = recorded_tags.crbegin(); it != recorded_tags.crend(); ++it ) {
    if (
      bb.max.x > it->min.x && bb.min.x < it->max.x &&
      bb.max.y > it->min.y && bb.min.y < it->max.y
    )
      return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Tag::BuildTag(
  Series* series, SVG::Group* tag_g, const Datum& datum, SVG::U& r
)
{
  Group* g = tag_g->AddNewGroup();

  if (
    series->type == SeriesType::XY ||
    series->type == SeriesType::Scatter
  ) {
    std::string s;
    s.reserve(
      0
      + 1
      + datum.tag_x.size() + series->axis_x->number_unit.size()
      + 1
      + datum.tag_y.size() + series->axis_y->number_unit.size()
      + 1
    );
    s += '(';
    s += datum.tag_x;
    s += series->axis_x->number_unit;
    s += ',';
    s += datum.tag_y;
    s += series->axis_y->number_unit;
    s += ')';
    g->Add( new Text( s ) );
  } else {
    std::string s{ datum.tag_y };
    s += series->axis_y->number_unit;
    g->Add( new Text( s ) );
  }

  BoundaryBox bb = g->Last()->GetBB();
  r = (bb.max.y - bb.min.y) / 3;

  U d = r * 0.75;
  if ( series->tag_box ) {
    g->Add(
      new Rect( bb.min.x - d, bb.min.y - d, bb.max.x + d, bb.max.y + d, r )
    );
    g->FrontToBack();
  }

  return g;
}

//------------------------------------------------------------------------------

SVG::U Tag::GetBeyond( Series* series, SVG::Group* tag_g )
{
  bool bar_type =
    series->type == SeriesType::Bar ||
    series->type == SeriesType::StackedBar ||
    series->type == SeriesType::Lollipop;

  Pos tag_pos = series->tag_pos;
  if ( tag_pos != Pos::Base && tag_pos != Pos::End && tag_pos != Pos::Center ) {
    tag_pos = Pos::Beyond;
  }

  if ( !bar_type || tag_pos != Pos::Beyond ) return 0;

  std::string s( series->max_tag_y_size, '0' );
  Datum datum;
  datum.tag_y = s;
  U r;
  Group* g = tag_g->AddNewGroup();
  series->ApplyTagStyle( g );
  BuildTag( series, g, datum, r );
  BoundaryBox bb = g->GetBB();
  tag_g->DeleteFront();

  U beyond =
    ( (series->axis_x->angle == 0)
      ? (bb.max.y - bb.min.y)
      : (bb.max.x - bb.min.x)
    ) + tag_spacing;
  if ( series->type == SeriesType::Lollipop ) {
    beyond +=
      (series->axis_x->angle == 0)
      ? series->tag_dist_y
      : series->tag_dist_x;
  }

  return beyond + min_base_dist;
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Tag::AddLineTag( void )
{
  int dir_bst = tag.dir_bst;
  int dir_ops = (tag.dir_bst + 4) % 8;

  U r;
  Group* g = BuildTag( tag.series, tag.tag_g, tag.datum, r );

  BoundaryBox bb;

  auto place = [&]( int dir, bool check_tag_collosion )
  {
    AnchorX ax =
      (dir < 0 || dir == 2 || dir == 6) ? AnchorX::Mid :
      (dir > 2 && dir < 6) ? AnchorX::Max : AnchorX::Min;
    AnchorY ay =
      (dir < 0 || dir == 0 || dir == 4) ? AnchorY::Mid :
      (dir > 4) ? AnchorY::Max : AnchorY::Min;
    U x = tag.p.x;
    U y = tag.p.y;

    U dx = tag.series->tag_dist_x + tag_spacing;
    U dy = tag.series->tag_dist_y + tag_spacing;
    if ( dir % 2 ) {
      dx = dy = dx * dy / std::sqrt( dx * dx + dy * dy );
    }
    if ( tag.series->tag_box ) {
      dx -= (dir % 2 == 1) ? (r * 0.3) : 0.0;
      dy -= (dir % 2 == 1) ? (r * 0.3) : 0.0;
    }

    if ( ax == AnchorX::Min ) x += dx;
    if ( ax == AnchorX::Max ) x -= dx;
    if ( ay == AnchorY::Min ) y += dy;
    if ( ay == AnchorY::Max ) y -= dy;
    g->MoveTo( ax, ay, x, y );

    bb = g->GetBB();
    bool ok =
      bb.min.x > tag.series->chart_area.min.x &&
      bb.max.x < tag.series->chart_area.max.x &&
      bb.min.y > tag.series->chart_area.min.y &&
      bb.max.y < tag.series->chart_area.max.y;
    if ( check_tag_collosion && ok ) {
      ok = !Collision( bb );
    }
    return ok;
  };

  U cx = (tag.series->chart_area.min.x + tag.series->chart_area.max.x) / 2;
  U cy = (tag.series->chart_area.min.y + tag.series->chart_area.max.y) / 2;

  bool pos_ok1[ 8 ] = { true, true, true, true, true, true, true, true };
  bool pos_ok2[ 8 ] = { true, true, true, true, true, true, true, true };
  int  dir_cur;
  int  dir_inc;
  bool pos_auto = false;
  switch ( tag.series->tag_pos ) {
    case Pos::Left:
      dir_cur = 4;
      dir_inc = (tag.p.y < cy) ? -1 : +1;
      break;
    case Pos::Right:
      dir_cur = 0;
      dir_inc = (tag.p.y < cy) ? +1 : -1;
      break;
    case Pos::Bottom:
      dir_cur = 6;
      dir_inc = (tag.p.x < cx) ? +1 : -1;
      break;
    case Pos::Center:
      if ( place( -1, false ) ) goto Placed;
    case Pos::Top:
      dir_cur = 2;
      dir_inc = (tag.p.x < cx) ? -1 : +1;
      break;
    default:
      pos_auto = true;
      dir_cur = dir_bst;
      if ( tag.dir_prv >= 0 || tag.dir_nxt >= 0 ) {
        int d1 = (tag.dir_prv >= 0) ? tag.dir_prv : tag.dir_nxt;
        int d2 = (tag.dir_nxt >= 0) ? tag.dir_nxt : tag.dir_prv;
        int dir = (d1 + d2) / 2;
        if ( d1 - d2 > 4 || d2 - d1 > 4 ) dir = (dir + 4) % 8;
        if ( (d1 + d1) % 2 && DirCmp( dir, dir_bst, +3 ) ) dir++;
        dir_cur = (dir + 4) % 8;
        if (  d1 == (d2 + 4) % 8 ) {
          if ( tag.p.x < cx ) {
            if ( DirCmp( dir_cur, 3, +2 ) ) dir_cur = (dir_cur + 4) % 8;
          } else {
            if ( DirCmp( dir_cur, 1, -2 ) ) dir_cur = (dir_cur + 4) % 8;
          }
        }
      }
      if ( tag.dir_prv >= 0 ) {
        pos_ok1[ tag.dir_prv ] = false;
        pos_ok2[ tag.dir_prv ] = false;
        pos_ok2[ (tag.dir_prv + 1) % 8 ] = false;
        pos_ok2[ (tag.dir_prv + 7) % 8 ] = false;
      }
      if ( tag.dir_nxt >= 0 ) {
        pos_ok1[ tag.dir_nxt ] = false;
        pos_ok2[ tag.dir_nxt ] = false;
        pos_ok2[ (tag.dir_nxt + 1) % 8 ] = false;
        pos_ok2[ (tag.dir_nxt + 7) % 8 ] = false;
      }
      if ( pos_ok2[ dir_bst ] ) {
        dir_cur = dir_bst;
      } else
      if ( pos_ok2[ (dir_ops + 7) % 8 ] && pos_ok2[ (dir_ops + 1) % 8 ] ) {
        dir_cur = dir_ops;
      }
      if ( tag.p.x < cx ) {
        dir_inc = DirCmp( dir_cur, dir_bst - 1, -3 ) ? +1 : -1;
      } else {
        dir_inc = DirCmp( dir_cur, dir_bst + 1, +3 ) ? -1 : +1;
      }
      break;
  }

  if ( pos_auto ) {
    for ( bool check_tag_collosion : { true, false } ) {
      for ( int i = 0; i < 8; i++ ) {
        if ( pos_ok2[ dir_cur ] ) {
          if ( place( dir_cur, check_tag_collosion ) ) goto Placed;
        }
        dir_cur = (dir_cur + dir_inc + 256) % 8;
        dir_inc = ((dir_inc < 0) ? +1 : -1) - dir_inc;
      }
      dir_cur = (dir_cur + 4) % 8;
      for ( int i = 0; i < 8; i++ ) {
        if ( pos_ok1[ dir_cur ] ) {
          if ( place( dir_cur, check_tag_collosion ) ) goto Placed;
        }
        dir_cur = (dir_cur + dir_inc + 256) % 8;
        dir_inc = ((dir_inc < 0) ? +1 : -1) - dir_inc;
      }
      dir_cur = (dir_cur + 4) % 8;
      for ( int i = 0; i < 8; i++ ) {
        if ( place( dir_cur, check_tag_collosion ) ) goto Placed;
        dir_cur = (dir_cur + dir_inc + 256) % 8;
        dir_inc = ((dir_inc < 0) ? +1 : -1) - dir_inc;
      }
      dir_cur = (dir_cur + 4) % 8;
    }
  } else {
    for ( int i = 0; i < 8; i++ ) {
      if ( place( dir_cur, false ) ) goto Placed;
      dir_cur = (dir_cur + dir_inc + 256) % 8;
      dir_inc = ((dir_inc < 0) ? +1 : -1) - dir_inc;
    }
    dir_cur = (dir_cur + 4) % 8;
  }

  // Give up, just place at center.
  place( -1, false );

  Placed:
  RecordTag( bb );
  tag.series->UpdateLegendBoxes(
    Point( bb.min.x, bb.min.y ), Point( bb.max.x, bb.max.y )
  );
  tag.series->UpdateLegendBoxes(
    Point( bb.min.x, bb.max.y ), Point( bb.max.x, bb.min.y )
  );
  return g;
}

//------------------------------------------------------------------------------

void Tag::LineTag(
  Series* series, SVG::Group* tag_g,
  Point p, const Datum& datum, bool datum_valid,
  bool connected, Pos direction
)
{
  tag.dir_nxt = Direction( p.x - tag.p.x, p.y - tag.p.y );
  tag.dir_nxt = (tag.valid && connected) ? tag.dir_nxt : -1;
  EndLineTag();
  tag.valid       = true;
  tag.series      = series;
  tag.tag_g       = tag_g;
  tag.p           = p;
  tag.datum       = datum;
  tag.datum_valid = datum_valid;
  switch ( direction ) {
    case Pos::Left   : tag.dir_bst = 4; break;
    case Pos::Right  : tag.dir_bst = 0; break;
    case Pos::Bottom : tag.dir_bst = 6; break;
    default          : tag.dir_bst = 2; break;
  }
  tag.dir_prv = (tag.dir_nxt < 0 ) ? -1 : ((tag.dir_nxt + 4 ) % 8);
  tag.dir_nxt = -1;

  return;
}

void Tag::EndLineTag( void )
{
  if ( tag.valid && tag.datum_valid ) {
    AddLineTag();
  }
  tag.valid = false;
  return;
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Tag::AddBarTag(
  Series* series, SVG::Group* tag_g,
  SVG::Point p1, SVG::Point p2, const Datum& datum,
  Pos direction
)
{
  U r;
  Group* g = BuildTag( series, tag_g, datum, r );

  // Default anchor point.
  AnchorX dax = AnchorX::Mid;
  AnchorY day = AnchorY::Mid;
  switch ( direction ) {
    case Pos::Right : dax = AnchorX::Min; break;
    case Pos::Left  : dax = AnchorX::Max; break;
    case Pos::Top   : day = AnchorY::Min; break;
    case Pos::Bottom: day = AnchorY::Max; break;
    default: break;
  }

  U tag_dist =
    (direction == Pos::Left || direction == Pos::Right)
    ? series->tag_dist_x
    : series->tag_dist_y;

  U base_dist = std::max( 2 * tag_dist, +min_base_dist );
  U end_dist = 2 * tag_dist;
  U beyond_dist = 0;
  if ( series->type == SeriesType::Lollipop ) {
    base_dist = min_base_dist;
    end_dist = tag_dist;
    beyond_dist = tag_dist;
  }

  U spc_x = tag_spacing;
  U spc_y = tag_spacing;

  BoundaryBox bb;

  auto place = [&]( Pos pos )
  {
    U x = p2.x;
    U y = p2.y;
    AnchorX ax = dax;
    AnchorY ay = day;
    if ( pos == Pos::Center ) {
      x = (p1.x + p2.x) / 2;
      y = (p1.y + p2.y) / 2;
      ax = AnchorX::Mid;
      ay = AnchorY::Mid;
    } else {
      if ( pos == Pos::End ) {
        if ( dax != AnchorX::Mid ) {
          ax = (dax == AnchorX::Max) ? AnchorX::Min : AnchorX::Max;
        }
        if ( day != AnchorY::Mid ) {
          ay = (day == AnchorY::Max) ? AnchorY::Min : AnchorY::Max;
        }
      } else
      if ( pos == Pos::Base ) {
        x = p1.x;
        y = p1.y;
      }
      if ( direction == Pos::Right ) {
        if ( pos == Pos::Base   ) x += spc_x + base_dist;
        if ( pos == Pos::End    ) x -= spc_x + end_dist;
        if ( pos == Pos::Beyond ) x += spc_x + beyond_dist;
      }
      if ( direction == Pos::Left ) {
        if ( pos == Pos::Base   ) x -= spc_x + base_dist;
        if ( pos == Pos::End    ) x += spc_x + end_dist;
        if ( pos == Pos::Beyond ) x -= spc_x + beyond_dist;
      }
      if ( direction == Pos::Top ) {
        if ( pos == Pos::Base   ) y += spc_y + base_dist;
        if ( pos == Pos::End    ) y -= spc_y + end_dist;
        if ( pos == Pos::Beyond ) y += spc_y + beyond_dist;
      }
      if ( direction == Pos::Bottom ) {
        if ( pos == Pos::Base   ) y -= spc_y + base_dist;
        if ( pos == Pos::End    ) y += spc_y + end_dist;
        if ( pos == Pos::Beyond ) y -= spc_y + beyond_dist;
      }
    }
    g->MoveTo( ax, ay, x, y );
    bb = g->GetBB();
    return
      ( direction == Pos::Top || direction == Pos::Bottom ||
        ( bb.min.x > series->chart_area.min.x &&
          bb.max.x < series->chart_area.max.x
        )
      ) &&
      ( direction == Pos::Right || direction == Pos::Left ||
        ( bb.min.y > series->chart_area.min.y &&
          bb.max.y < series->chart_area.max.y
        )
      ) &&
      ( pos == Pos::Base ||
        ( !(direction == Pos::Right  && bb.min.x < (p1.x + spc_x + base_dist)) &&
          !(direction == Pos::Left   && bb.max.x > (p1.x - spc_x - base_dist)) &&
          !(direction == Pos::Top    && bb.min.y < (p1.y + spc_y + base_dist)) &&
          !(direction == Pos::Bottom && bb.max.y > (p1.y - spc_y - base_dist))
        )
      );
  };

  Pos tag_pos = series->tag_pos;
  if ( tag_pos != Pos::Base && tag_pos != Pos::End && tag_pos != Pos::Center ) {
    tag_pos = Pos::Beyond;
  }

  if ( place( tag_pos ) ) goto Placed;
  if ( tag_pos == Pos::Beyond ) {
    if ( place( Pos::End ) ) goto Placed;
  }
  if ( tag_pos != Pos::Base ) {
    if ( place( Pos::Base ) ) goto Placed;
  }

  Placed:
  RecordTag( bb );
  series->UpdateLegendBoxes(
    Point( bb.min.x, bb.min.y ), Point( bb.max.x, bb.max.y )
  );
  series->UpdateLegendBoxes(
    Point( bb.min.x, bb.max.y ), Point( bb.max.x, bb.min.y )
  );
  return g;
}

//------------------------------------------------------------------------------

void Tag::BarTag(
  Series* series, SVG::Group* tag_g,
  SVG::Point p1, SVG::Point p2, const Datum& datum,
  Pos direction
)
{
  AddBarTag( series, tag_g, p1, p2, datum, direction );
  return;
}

////////////////////////////////////////////////////////////////////////////////

int Tag::Direction( double x, double y )
{
  const double q = 2.4142;      // tan( 90deg - 45deg/2 )

  double ax = std::abs( x );
  double ay = std::abs( y );

  int dir = 1;
  dir = dir >> (ax > ay * q);
  dir = dir << (ay > ax * q);

  if ( x < 0 ) dir = 4 - dir;
  if ( y < 0 ) dir = 8 - dir;

  return (dir & 0x7) - 2 * (x == 0 && y == 0);
}

bool Tag::DirCmp( int dir1, int dir2, int ofs )
{
  int msk = ofs >> (sizeof( int ) * 8 - 1);
  int fwd = dir1 - dir2;
  int bck = dir2 - dir1;
  int num = (fwd & ~msk) | (bck & msk);
  int abs = (ofs ^ msk) - msk;
  return (num & 7) <= abs;
}

////////////////////////////////////////////////////////////////////////////////
