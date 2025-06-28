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
#include <chart_html.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

void HTML::NewChart( Main* main )
{
  main_list.push_back( main );
}

//------------------------------------------------------------------------------

void HTML::DefAxisX(
  Main* main, int n, Axis* axis, double val1, double val2,
  NumberFormat number_format,
  bool number_sign, bool logarithmic, bool is_cat
)
{
  main->html.x_axis[ n ] = {
    .axis          = axis,
    .is_cat        = is_cat,
    .number_format = number_format,
    .number_sign   = number_sign,
    .logarithmic   = logarithmic,
    .val1          = val1,
    .val2          = val2
  };
}

void HTML::DefAxisY(
  Main* main, int n, Axis* axis, double val1, double val2,
  NumberFormat number_format,
  bool number_sign, bool logarithmic, bool is_cat
)
{
  main->html.y_axis[ n ] = {
    .axis          = axis,
    .is_cat        = is_cat,
    .number_format = number_format,
    .number_sign   = number_sign,
    .logarithmic   = logarithmic,
    .val1          = val1,
    .val2          = val2
  };
}

//------------------------------------------------------------------------------

void HTML::LegendPos( Series* series, const SVG::BoundaryBox& bb )
{
  series_legend_map[ series ] = bb;
}

void HTML::MoveLegend( Series* series, SVG::U dx, SVG::U dy )
{
  for ( Series* s = series; s != nullptr; s = s->same_legend_series ) {
    auto it = series_legend_map.find( s );
    if ( it != series_legend_map.end() ) {
      it->second.min.x += dx;
      it->second.min.y += dy;
      it->second.max.x += dx;
      it->second.max.y += dy;
    }
  }
}

void HTML::MoveLegends( Main* main, SVG::U dx, SVG::U dy )
{
  for ( auto series : main->legend_obj->series_list ) {
    MoveLegend( series, dx, dy );
  }
}

//------------------------------------------------------------------------------

void HTML::AddSnapPoint(
  Series* series,
  SVG::Point p, std::string_view tag_x, std::string_view tag_y
)
{
  series->main->html.snap_points.push_back(
    { series->id, 0, p, tag_x, tag_y }
  );
  series->has_snap = true;
}

void HTML::AddSnapPoint(
  Series* series,
  SVG::Point p, uint32_t cat_idx, std::string_view tag_y
)
{
  series->main->html.snap_points.push_back(
    { series->id, cat_idx, p, "", tag_y }
  );
  series->has_snap = true;
}

void HTML::DontPruneSnapPoint( SVG::Point p )
{
  dont_prune_set.insert( p );
}

////////////////////////////////////////////////////////////////////////////////

std::string quoteJS( std::string_view s ) {
  std::ostringstream oss;
  oss << '"';
  for ( char c : s ) {
    if ( static_cast<unsigned char>( c ) < ' ' ) {
        oss << ' ';
    } else if ( c == '"' ) {
        oss << "\\\"";
    } else if ( c == '\\' ) {
        oss << "\\\\";
    } else {
        oss << c;
    }
  }
  oss << '"';
  return oss.str();
}

//------------------------------------------------------------------------------

void HTML::GenChartData( Main* main, std::ostringstream& oss )
{
  BoundaryBox chart_bb = main->GetGroup()->GetBB();

  BoundaryBox area_bb;
  // Standard SVG coordinates (Y direction down) given here.
  area_bb.min.x = main->g_dx;
  area_bb.min.y = -(main->g_dy + main->chart_h);
  area_bb.max.x = main->g_dx + main->chart_w;
  area_bb.max.y = -(main->g_dy);

  Color bg_color;
  if ( !main->ChartAreaColor()->IsClear() ) {
    bg_color.Set( main->ChartAreaColor() );
  } else {
    bg_color.Set( main->ensemble->BackgroundColor() );
  }
  if ( bg_color.IsClear() ) bg_color.Set( ColorName::white );

  oss << "{\n";

  // Never hide the mouse cursor as it causes stuttering for large SVGs.
  bool hide_mouse_cursor = false;
  {
    Color crosshairLineColor{ main->AxisColor() };
    MakeColorVisible( &crosshairLineColor, &bg_color );

    Color crosshairFillColor{ &bg_color };

    Color axisBoxLineColor{ &crosshairLineColor };

    Color axisBoxFillColor;
    axisBoxFillColor.Set( &crosshairFillColor, &crosshairLineColor, 0.2 );

    Color highlightColor{ "gold" };
    MakeColorVisible( &crosshairLineColor, &bg_color );

    if ( !crosshairLineColor.IsDefined() ) crosshairLineColor.Clear();
    if ( !crosshairFillColor.IsDefined() ) crosshairFillColor.Clear();
    if ( !axisBoxLineColor.IsDefined() ) axisBoxLineColor.Clear();
    if ( !axisBoxFillColor.IsDefined() ) axisBoxFillColor.Clear();

    oss
      << "backgroundColor : " << bg_color.SVG() << ",\n"
      << "crosshairLineColor : " << crosshairLineColor.SVG() << ",\n"
      << "crosshairFillColor : " << crosshairFillColor.SVG() << ",\n"
      << "axisBoxLineColor : " << axisBoxLineColor.SVG() << ",\n"
      << "axisBoxFillColor : " << axisBoxFillColor.SVG() << ",\n"
      << "highlightColor : " << highlightColor.SVG() << ",\n";

    if ( crosshairLineColor.IsClear() ) hide_mouse_cursor = false;
  }

  oss  << "axisFontSize : 14,\n";
  oss  << "infoFontSize : 14,\n";

  oss << "area:{";
  oss << "x1:" << area_bb.min.x.SVG( false ) << ',';
  oss << "y1:" << area_bb.min.y.SVG( false ) << ',';
  oss << "x2:" << area_bb.max.x.SVG( false ) << ',';
  oss << "y2:" << area_bb.max.y.SVG( false ) << "},\n";

  // Convert to standard SVG coordinates.
  std::swap( chart_bb.min.y, chart_bb.max.y );
  chart_bb.min.y = -chart_bb.min.y;
  chart_bb.max.y = -chart_bb.max.y;

  oss << "chart:{";
  oss << "x1:" << chart_bb.min.x.SVG( false ) << ',';
  oss << "y1:" << chart_bb.min.y.SVG( false ) << ',';
  oss << "x2:" << chart_bb.max.x.SVG( false ) << ',';
  oss << "y2:" << chart_bb.max.y.SVG( false ) << "},\n";

  oss << "axisX : [";
  for ( auto a : main->html.x_axis ) {
    oss << "{";
    oss << "show:" << (a.axis != nullptr) << ',';
    if ( a.axis ) {
      oss << "areaVal1:" << a.val1 << ',';
      oss << "areaVal2:" << a.val2 << ',';
      oss << "isX:" << (a.axis == main->axis_x) << ',';
      oss << "isCategory:" << a.is_cat << ',';
      oss << "logarithmic:" << a.logarithmic << ',';
      oss << "showSign:" << a.number_sign << ',';
      if ( a.number_format == NumberFormat::Magnitude ) {
        oss << "format:\"Engineering\",";
      } else
      if ( a.number_format == NumberFormat::Scientific ) {
        oss << "format:\"Scientific\",";
      } else {
        oss << "format:\"Fixed\",";
      }
    }
    oss << "},";
  }
  oss << "],\n";

  oss << "axisY : [";
  for ( auto a : main->html.y_axis ) {
    oss << "{";
    oss << "show:" << (a.axis != nullptr) << ',';
    if ( a.axis ) {
      oss << "areaVal1:" << a.val1 << ',';
      oss << "areaVal2:" << a.val2 << ',';
      oss << "isX:" << (a.axis == main->axis_x) << ',';
      oss << "isCategory:" << a.is_cat << ',';
      oss << "logarithmic:" << a.logarithmic << ',';
      oss << "showSign:" << a.number_sign << ',';
      if ( a.number_format == NumberFormat::Magnitude ) {
        oss << "format:\"Engineering\",";
      } else
      if ( a.number_format == NumberFormat::Scientific ) {
        oss << "format:\"Scientific\",";
      } else {
        oss << "format:\"Fixed\",";
      }
    }
    oss << "},";
  }
  oss << "],\n";

  oss << "axisSwap : " << main->html.axis_swap << ",\n";
  oss << "hideMouseCursor : " << hide_mouse_cursor << ",\n";
  oss << "inLine : " << main->html.all_inline << ",\n";

  for ( auto s1 : main->series_list ) {
    if ( !s1->has_snap ) continue;
    for ( auto s2 : main->series_list ) {
      if ( !s2->has_snap || s1 == s2 ) continue;
      if (
        s1->line_color_shown && s2->line_color_shown &&
        Color::Diff( s1->LineColor(), s2->LineColor() ) < 0.1
      )
        s1->line_color_same_cnt++;
      if (
        s1->fill_color_shown && s2->fill_color_shown &&
        Color::Diff( s1->FillColor(), s2->FillColor() ) < 0.1
      )
        s1->fill_color_same_cnt++;
    }
  }

  oss << "seriesList : [\n";
  for ( auto series : main->series_list ) {
    oss << "{";
    if ( series_legend_map.count( series ) > 0 ) {
      BoundaryBox bb = series_legend_map[ series ];
      if ( !series->global_legend ) {
        bb.min.x += main->g_dx; bb.min.y += main->g_dy;
        bb.max.x += main->g_dx; bb.max.y += main->g_dy;
      }
      std::swap( bb.min.y, bb.max.y );
      bb.min.y = -bb.min.y;
      bb.max.y = -bb.max.y;
      oss << "legendBB:{";
      oss << "x1:" << bb.min.x.SVG( false ) << ',';
      oss << "y1:" << bb.min.y.SVG( false ) << ',';
      oss << "x2:" << bb.max.x.SVG( false ) << ',';
      oss << "y2:" << bb.max.y.SVG( false ) << "},";
    }
    int idx_x = -1;
    int idx_y = -1;
    for ( int i = 0; i < 2; i++ ) {
     if ( series->axis_x == main->html.x_axis[ i ].axis ) idx_x = i;
     if ( series->axis_y == main->html.x_axis[ i ].axis ) idx_x = i;
     if ( series->axis_x == main->html.y_axis[ i ].axis ) idx_y = i;
     if ( series->axis_y == main->html.y_axis[ i ].axis ) idx_y = i;
    }
    if ( idx_x >= 0 ) {
      oss << "axisX:" << idx_x << ',';
    }
    if ( idx_y >= 0 ) {
      oss << "axisY:" << idx_y << ',';
    }
    {
      Color fg;
      {
        Color c1{ series->line_color };
        Color c2{ series->fill_color };
        if ( !series->line_color_shown ) c1.Set( &bg_color );
        if ( !series->fill_color_shown ) c2.Set( &bg_color );
        c1.SetTransparency( 0.0 );
        c2.SetTransparency( 0.0 );
        float d1 = Color::Diff( &c1, &bg_color );
        float d2 = Color::Diff( &c2, &bg_color );
        fg.Set( &c1 );
        if ( series->fill_color_shown ) {
          if (
            !series->line_color_shown ||
            (d1 < 0.1 && d2 > d1) ||
            ( d1 > 0.1 && d2 > 0.1 &&
              series->fill_color_same_cnt < series->line_color_same_cnt
            )
          ) {
            fg.Set( &c2 );
          }
        }
      }
      Color bg{ &bg_color };
      if ( !fg.IsClear() ) {
        bg.Set( &bg_color, &fg, 0.2 );
      }
      Color tx{ main->AxisColor() };
      MakeColorVisible( &tx, &bg_color );
      if ( !fg.IsDefined() ) fg.Clear();
      if ( !bg.IsDefined() ) bg.Clear();
      if ( !tx.IsDefined() ) tx.Clear();
      oss << "fgColor:" << fg.SVG() << ',';
      oss << "bgColor:" << bg.SVG() << ',';
      oss << "txColor:" << tx.SVG() << ',';
    }
    oss << "},\n";
  }
  oss << "],\n";

  // Resolution of snap points in points, i.e. how close the snap points are
  // placed (to reduce HTML size). Mouse events are in SVG point unit steps, so
  // a finer (smaller) resolution than 1.0 does not make much sense.
  double snap_resolution = 1.0;
  double snap_f = 1.0 / snap_resolution;

  // When there are relatively few snap point, there is no need to prune them.
  bool few_snaps = main->html.snap_points.size() <= 1000;

  std::unordered_set< uint64_t > snap_set;
  std::unordered_set< uint32_t > cat_set;

  // Returns true if point did not exist and was added to the set.
  auto SnapAdd = [&]( Point p ) {
    uint64_t key =
      (static_cast< uint64_t >( p.y * snap_f ) << 32) |
      (static_cast< uint64_t >( p.x * snap_f ) <<  0);
    return snap_set.insert( key ).second;
  };

  if ( !main->category_list.empty() ) {
    auto base_it = main->html.snap_points.begin();
    while ( base_it != main->html.snap_points.end() ) {
      auto it = base_it;
      auto id = base_it->series_id;
      while ( it != main->html.snap_points.end() && it->series_id == id ) {
        if ( cat_set.count( it->cat_idx ) > 0 ) {
          SnapAdd( it->p );
        }
        ++it;
      }
      it = base_it;
      while ( it != main->html.snap_points.end() && it->series_id == id ) {
        bool added = SnapAdd( it->p );
        bool dont_prune = dont_prune_set.count( it->p ) > 0;
        if ( added || dont_prune ) {
          cat_set.insert( it->cat_idx );
        }
        ++it;
      }
      base_it = it;
    }
  }

  if ( !main->category_list.empty() ) {
    std::unordered_set< int32_t > snap_cat_set;
    for ( uint32_t i = 0; i < main->category_list.size(); ++i ) {
      U coor = main->axis_x->Coor( i );
      int32_t key = static_cast< int32_t >( std::floor( coor * snap_f ) );
      if ( snap_cat_set.insert( key ).second ) {
        cat_set.insert( i );
      }
    }
  }

  oss << "snapPoints : [\n";
  for (
    auto it = main->html.snap_points.rbegin();
    it != main->html.snap_points.rend(); ++it
  ) {
    const auto& sp = *it;
    bool add = few_snaps;
    if ( sp.tag_x.empty() ) {
      add = add || cat_set.count( sp.cat_idx ) > 0;
    } else {
      add = add || SnapAdd( sp.p );
    }
    if ( add ) {
      U X = +(sp.p.x + main->g_dx);
      U Y = -(sp.p.y + main->g_dy);
      oss << "{s:" << sp.series_id << ',';
      if ( sp.tag_x.empty() ) {
        oss << "x:" << sp.cat_idx << ',';
      } else {
        oss << "x:" << quoteJS( sp.tag_x ) << ',';
      }
      oss << "y:" << quoteJS( sp.tag_y ) << ",";
      oss << "X:" << X.SVG( false ) << ',';
      oss << "Y:" << Y.SVG( false ) << "},\n";
    }
  }
  oss << "],\n";

  if ( !main->category_list.empty() ) {
    oss << "catCnt : " << main->category_list.size() << ",\n";
    oss << "categories : [\n";
    uint32_t i = 0;
    uint32_t j = 0;
    for ( const auto& s : main->category_list ) {
      if ( !s.empty() && (few_snaps || cat_set.count( i ) > 0) ) {
        if ( j < i ) {
          oss << i << ',';
          j = i;
        }
        oss << quoteJS( s ) << ",\n";
        ++j;
      }
      ++i;
    }
    oss << "],\n";
  }

  oss << "},\n";

  return;
}

//------------------------------------------------------------------------------

std::string HTML::GenHTML( SVG::Canvas* canvas )
{
  std::ostringstream oss;
  oss << std::boolalpha;

  #include <chart_html_part1.h>

  BoundaryBox ensemble_bb = canvas->TopGroup()->GetBB();

  {
    U ensemble_w = ensemble_bb.max.x - ensemble_bb.min.x;
    U ensemble_h = ensemble_bb.max.y - ensemble_bb.min.y;
    oss << "<div style=\"";
    oss << "width:" << ensemble_w.SVG( false ) << "px;";
    oss << "height:" << ensemble_h.SVG( false ) << "px;";
    oss << "position:relative;margin:0 auto;\">\n";
  }

  oss << canvas->GenSVG( 0, "style=\"pointer-events: none;\" id=\"svgChart\"" );

  {
    Canvas cursor_canvas;
    Group* g = cursor_canvas.TopGroup();
    g->Add( new Rect( ensemble_bb.min, ensemble_bb.max ) );
    g->Attr()->SetLineWidth( 0 );
    g->Attr()->LineColor()->Clear();
    g->Attr()->FillColor()->Clear();
    oss << cursor_canvas.GenSVG( 0, "style=\"pointer-events: none;\" id=\"svgCursor\"" );
  }

  {
    Canvas snap_canvas;
    Group* g = snap_canvas.TopGroup();
    g->Add( new Rect( ensemble_bb.min, ensemble_bb.max ) );
    g->Attr()->SetLineWidth( 0 );
    g->Attr()->LineColor()->Clear();
    g->Attr()->FillColor()->Clear();
    oss << snap_canvas.GenSVG( 0, "id=\"svgSnap\"" );
  }

  oss << "</div>\n";

  oss << "\n<script>\n\n";

  oss << "const chart_list = [" << '\n';
  for ( auto main : main_list ) {
    GenChartData( main, oss );
  }
  oss << "];" << '\n';

  #include <chart_html_part2.h>

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
