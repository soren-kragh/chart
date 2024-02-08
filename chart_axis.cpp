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

#include <chart_axis.h>
#include <set>

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Axis::Axis( int angle )
{
  this->angle = angle;
  digits = 0;
  decimals = 0;
  num_max_len = 0;
  exp_max_len = 0;
  log_scale = false;
  number_format = Fixed;
  number_format_auto = true;
  show_minor_mumbers = false;
  show_minor_mumbers_auto = true;
  min = 0;
  max = 0;
  orth_axis_cross = 0;
  major = 0;
  sub_divs = 0;
  number_pos = Auto;
  major_grid_enable = true;
  minor_grid_enable = true;
  unit_pos = Auto;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::SetLogScale( bool log_scale )
{
  this->log_scale = log_scale;
}

void Axis::SetNumberFormat( NumberFormat number_format )
{
  this->number_format = number_format;
  this->number_format_auto = false;
}

void Axis::SetNumberUnit( std::string unit )
{
  this->number_unit = unit;
}

void Axis::ShowMinorNumbers( bool show_minor_mumbers )
{
  this->show_minor_mumbers = show_minor_mumbers;
  this->show_minor_mumbers_auto = false;
}

void Axis::SetRange( double min, double max, double orth_axis_cross )
{
  this->min = min;
  this->max = max;
  if ( orth_axis_cross < min ) orth_axis_cross = min;
  if ( orth_axis_cross > max ) orth_axis_cross = max;
  this->orth_axis_cross = orth_axis_cross;
}

void Axis::SetRange( double min, double max )
{
  SetRange( min, max, min );
}

void Axis::SetTick( double major, int sub_divs )
{
  this->major = major;
  this->sub_divs = sub_divs;
}

void Axis::SetGrid( bool major_enable, bool minor_enable )
{
  major_grid_enable = major_enable;
  minor_grid_enable = minor_enable;
}

void Axis::SetNumberPos( Pos pos )
{
  this->number_pos = pos;
}

void Axis::SetLabel( std::string label )
{
  this->label = label;
}

void Axis::SetUnit( std::string unit )
{
  this->unit = unit;
}

void Axis::SetUnitPos( Pos pos )
{
  this->unit_pos = pos;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::LegalizeMinor( void ) {
  if ( major <= 0 ) {
    sub_divs = 0;
    return;
  }

  U max_coor = Coor( max );

  if ( log_scale ) {
    if ( major > 10 ) sub_divs = 1;
    if ( sub_divs < 1 ) {
      sub_divs = 10;
      while ( true ) {
        U coor = Coor( max - max / sub_divs );
        if ( max_coor - coor <= 32 ) break;
        if ( sub_divs == 100 ) break;
        do sub_divs++; while ( 100 % sub_divs );
      }
      while ( true ) {
        U coor = Coor( max - max / sub_divs );
        if ( max_coor - coor >= 8 ) break;
        if ( sub_divs == 1 ) break;
        do sub_divs--; while ( 100 % sub_divs );
      }
    }
    if ( sub_divs > 100 ) sub_divs = 100;
    while ( sub_divs > 1 ) {
      U coor = Coor( max - max / sub_divs );
      if ( max_coor - coor >= 4 ) break;
      do sub_divs--; while ( 100 % sub_divs );
    }
  } else {
    if ( sub_divs < 1 ) sub_divs = 1;
    while ( sub_divs > 1 ) {
      U coor = Coor( max - major / sub_divs );
      if ( max_coor - coor >= 10 ) break;
      do sub_divs--; while ( 1000 % sub_divs );
    }
  }

  return;
}

void Axis::LegalizeMajor( void ) {
  double mag = std::max( std::abs( min ), std::abs( max ) );

  while ( true ) {

    if (
      mag < num_lo || mag > num_hi || (max - min) < num_lo ||
      mag > (max - min) * 1e9
    ) {
      major = 0;
      break;
    }

    if ( log_scale ) {
      if ( min < num_lo || max > num_hi ) {
        major = 0;
        break;
      }
      if ( show_minor_mumbers_auto ) show_minor_mumbers = true;
      if ( number_format_auto ) {
        number_format = (min < 10e-30 || max > 0.1e30) ? Scientific : Magnitude;
      }
    } else {
      if ( show_minor_mumbers_auto ) show_minor_mumbers = false;
      if ( number_format_auto ) number_format = Fixed;
    }
    if ( number_format == Fixed ) {
      if (
        mag < (number_format_auto ? 0.01 : lim) ||
        mag > (number_format_auto ? 1e6 : 1e15)
      ) {
        number_format = Scientific;
      }
    }

    U max_coor = Coor( max );

    if ( log_scale ) {
      bool auto_major = major < 10;
      if ( auto_major ) major = 10;
      major =
        std::round(
          std::pow( double( 10 ), std::round( std::log10( major ) ) )
        );
      while ( true ) {
        U coor = Coor( max / major );
        if ( max_coor - coor >= (auto_major ? 40 : 20) ) break;
        if ( number_format == Magnitude ) {
          major = major * ((major > 10) ? 1000 : 100);
        } else {
          major = major * 10;
        }
      }
    } else {
      if ( major > 0 ) {
        // Minimum allowed major spacing.
        U min_space = 12;
        if ( length * major < min_space * (max - min) ) {
          major = 0;
        }
      }
      if ( major <= 0 ) {
        // Minimum major spacing to aim for.
        U min_space = 100;
        int32_t p = 0;
        int32_t m = 1;
        int32_t d = 1;
        while ( 1 ) {
          major = std::pow( double( 10 ), p ) * m / d;
          if ( (max - min) * 2 * min_space > length * major ) break;
          switch ( d ) {
            case 1  : d = 2; break;
            case 2  : d = 4; break;
            case 4  : d = 5; break;
            default : d = 1; p--;
          }
        }
        while ( p >= 0 && d == 1 ) {
          major = std::pow( double( 10 ), p ) * m / d;
          if ( (max - min) * min_space < length * major ) break;
          switch ( m ) {
            case 1  : m = 2; break;
            case 2  : m = 5; break;
            default : m = 1; p++;
          }
        }
        sub_divs = 2;
      }
    }

    break;
  }

  if ( major == 0 ) {
    log_scale = false;
    number_format = Scientific;
  }

  return;
}

void Axis::LegalizeMinMax(
  double series_min,
  double series_max
)
{
  if ( series_min == series_max ) {
    if ( log_scale ) {
      series_min = series_min / 10;
      series_max = series_max * 10;
    } else {
      series_min = series_min - 1;
      series_max = series_max + 1;
    }
  }

  bool automatic = false;

  if ( min >= max ) {
    automatic = true;
    min = series_min;
    max = series_max;
  }
  if ( log_scale && min <= 0 ) {
    min = series_min;
    if ( max <= min ) max = 1000 * min;
  }

  if ( angle == 90 && automatic && !log_scale ) {
    if ( min > 0 && (max - min) / max > 0.5 ) min = 0;
    if ( max < 0 && (min - max) / min > 0.5 ) max = 0;
  }

  LegalizeMajor();

  if ( automatic ) {
    double p;
    if ( major > 0 ) {
      if ( log_scale ) {
        int32_t u = std::lround( std::log10( major ) );
        p = std::log10( min ) / u + epsilon;
        min = std::pow( std::pow( double( 10 ), u ), std::floor( p ) );
        p = std::log10( max ) / u - epsilon;
        max = std::pow( std::pow( double( 10 ), u ), std::ceil( p ) );
        if ( max < 10 * min ) max = 10 * min;
      } else {
        double e = (max - min) * epsilon;
        p = (min + e) / major;
        min = std::floor( p ) * major;
        p = (max - e) / major;
        max = std::ceil( p ) * major;
      }
    }
    if ( angle == 0 ) {
      orth_axis_cross = min;
    } else {
      orth_axis_cross = (max <= 0) ? max : min;
      if ( min < 0 && max > 0 ) orth_axis_cross = 0;
      if ( log_scale ) orth_axis_cross = min;
    }
  }

  LegalizeMinor();

  if ( orth_axis_cross < min ) orth_axis_cross = min;
  if ( orth_axis_cross > max ) orth_axis_cross = max;

  return;
}

///////////////////////////////////////////////////////////////////////////////

U Axis::Coor( double v )
{
  if ( log_scale ) {
    if ( v > 0 ) {
      double a = std::log10( min );
      double b = std::log10( max );
      return (std::log10( v ) - a) * length / (b - a);
    } else {
      return -1e20;
    }
  } else {
    return (v - min) * length / (max - min);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Compute number of required decimals. If update=true then the digits and
// decimals member class variables are updated to reflect the new max.
int32_t Axis::ComputeDecimals( double v, bool update )
{
  if ( v > -lim && v < lim ) v = 0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision( precision ) << v;
  int dp = -1;
  int nz = -1;
  int i = 0;
  for ( const char c : oss.str() ) {
    if ( c != '0' && dp >= 0 ) nz = i;
    if ( c == '.' && dp <  0 ) dp = i;
    i++;
  }
  int dig = (dp < 0) ? 0 : dp;
  int dec = (nz < 0) ? 0 : (nz - dp);
  if ( update ) {
    digits = std::max( dig, digits );
    decimals = std::max( dec, decimals );
  }
  return dec;
}

int32_t Axis::NormalizeExponent( double& num )
{
  int32_t exp = 0;
  if ( num != 0 && number_format != Fixed ) {
    double sign = (num < 0) ? -1 : 1;
    num = num * sign;
    while ( num < 1 ) {
      num = num * 10;
      exp--;
    }
    while ( num > (10 - lim) ) {
      num = num / 10;
      exp++;
    }
    if ( num > (1 - lim) && num < (1 + lim) ) {
      num = 1;
    }
    if ( number_format == Magnitude ) {
      while ( exp % 3 ) {
        num = num * 10;
        exp--;
      }
      if ( exp == -3 && num >= 100 ) {
        num = num / 1000;
        exp = 0;
      }
    }
    num = num * sign;
  }
  return exp;
}

void Axis::ComputeNumFormat( void )
{
  digits = 0;
  decimals = 0;
  num_max_len = 0;
  exp_max_len = 0;

  if ( major <= 0 || number_format == Magnitude ) return;

  U min_coor = 0;
  U max_coor = length;
  U eps_coor = (max_coor - min_coor) * epsilon;

  std::vector< double > v_list;
  if ( log_scale ) {
    int32_t pow_inc = std::round( std::log10( major ) );
    int32_t pow_min = std::floor( std::log10( min ) ) - pow_inc;
    int32_t pow_max = std::ceil( std::log10( max ) ) + pow_inc;
    while ( pow_min % pow_inc ) pow_min--;
    while ( pow_max % pow_inc ) pow_max++;
    for ( int32_t pow_cur = pow_min; pow_cur <= pow_max; pow_cur += pow_inc ) {
      for ( int32_t sn = 0; sn < sub_divs; sn++ ) {
        if ( sn > 0 && !show_minor_mumbers ) break;
        double m0 = std::pow( double( 10 ), pow_cur );
        double m1 = std::pow( double( 10 ), pow_cur + pow_inc );
        double v = m1 * sn / sub_divs;
        if ( sn == 0 ) v = m0;
        U v_coor = Coor( v );
        if ( v_coor < min_coor - eps_coor ) continue;
        if ( v_coor > max_coor + eps_coor ) continue;
        v_list.push_back( v );
      }
    }
  } else {
    int64_t mn_min = std::floor( (min - major) / major );
    int64_t mn_max = std::ceil( (max + major) / major );
    for ( int64_t mn = mn_min; mn <= mn_max; mn++ ) {
      for ( int32_t sn = 0; sn < sub_divs; sn++ ) {
        if ( sn > 0 && !show_minor_mumbers ) break;
        double v = mn * major + sn * major / sub_divs;
        U v_coor = Coor( v );
        if ( v_coor < min_coor - eps_coor ) continue;
        if ( v_coor > max_coor + eps_coor ) continue;
        v_list.push_back( v );
      }
    }
  }

  if ( number_format == Fixed ) {
    for ( double v : v_list ) {
      ComputeDecimals( v, true );
    }
  }

  if ( number_format == Scientific ) {
    for ( double v : v_list ) {
      int32_t exp = NormalizeExponent( v );
      ComputeDecimals( v, true );
      std::ostringstream oss;
      oss << exp;
      exp_max_len = std::max( exp_max_len, int32_t( oss.str().length() ) );
    }
  }

  num_max_len = digits + decimals;
  if ( decimals > 0 ) num_max_len++;

  if ( angle == 0 ) {
    if ( number_format != Fixed ) decimals = 0;
    num_max_len = 0;
    exp_max_len = 0;
  } else {
    if ( number_pos == Left  ) num_max_len = 0;
    if ( number_pos == Right ) exp_max_len = 0;
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

std::string Axis::NumToStr( double v )
{
  int32_t dec = std::max( ComputeDecimals( v ), decimals );
  std::ostringstream oss;
  oss << std::fixed << std::setprecision( dec ) << v;
  return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

SVG::Group* Axis::BuildNum( SVG::Group* g, double v, bool bold )
{
  if ( std::abs( v ) < num_lo ) v = 0;
  double num = v;
  int32_t exp = NormalizeExponent( num );

  NumberFormat number_format = this->number_format;
  if ( number_format == Magnitude && (exp < -30 || exp > 30) ) {
    number_format = Scientific;
  }

  std::string s = NumToStr( num );

  if ( number_format == Magnitude ) {
    const char sym[] = "qryzafpn\xE6m kMGTPEZYRQ";
    exp = exp / 3;
    if ( exp != 0 ) s += sym[ exp + 10 ];
    s += number_unit;
    g = Label( g, s );
    if ( bold ) g->Attr()->TextFont()->SetBold();
    return g;
  }

  int32_t leading_ws = num_max_len - s.length();
  if ( angle == 90 && leading_ws > 0 ) {
    s.insert( 0, leading_ws, ' ' );
  } else {
    leading_ws = 0;
  }

  if ( number_format == Fixed ) {
    s += number_unit;
    g = Label( g, s );
    if ( bold ) g->Attr()->TextFont()->SetBold();
    return g;
  }

  // number_format is Scientific.
  BoundaryBox bb;

  g = g->AddNewGroup();
  SVG::Group* num_g = g->AddNewGroup();
  SVG::Group* exp_g = g->AddNewGroup();

  // Build non-exponent part,
  do {
    if ( num == 0 ) {
      size_t pos = s.find( '.' );
      if ( pos != std::string::npos ) s.erase( pos );
      num_g->Add( new Text( 0, 0, s ) );
      break;
    }
    if ( std::abs( num ) == 1 && (angle == 0 || number_pos == Left) ) {
      num_g->Add( new Text( 0, 0, (num < 0) ? "-10" : "10" ) );
      leading_ws = 0;
      break;
    }
    num_g->Add( new Text( 0, 0, s ) );
    bb = num_g->GetBB();
    U cr = (bb.max.y - bb.min.y) * 0.15;
    U dx = cr * 0.75;
    U cx = bb.max.x + dx + cr;
    U cy = (bb.max.y - bb.min.y) * 0.45 + cr * 0.5;
    SVG::Group* x_g = num_g->AddNewGroup();
    x_g->Attr()->SetLineWidth(
      cr * (bold ? 0.75 : 0.5)
    )->LineColor()->Set( Black );
    x_g->Add( new Line( cx - cr, cy - cr, cx + cr, cy + cr ) );
    x_g->Add( new Line( cx - cr, cy + cr, cx + cr, cy - cr ) );
    bb = num_g->GetBB();
    num_g->Add( new Text( 0, 0, "10" ) );
    num_g->Last()->MoveTo( MinX, MinY, bb.max.x + dx, bb.min.y );
  } while ( false );

  int32_t trailing_ws = 0;

  // Build exponent part,
  do {
    if ( num == 0 ) {
      s = "";
    } else {
      std::ostringstream oss;
      oss << exp;
      s = oss.str();
    }
    if ( angle != 0 || num != 0 ) {
      trailing_ws = exp_max_len - s.length();
      if ( trailing_ws > 0 ) {
        s.insert( s.length(), trailing_ws, ' ' );
      } else {
        trailing_ws = 0;
      }
    }
    bb = num_g->GetBB();
    U h = bb.max.y - bb.min.y;
    exp_g->Attr()->TextFont()->SetSize( h * 0.9 );
    exp_g->Add( new Text( 0, 0, s ) );
    bool center = num == 0 && angle == 0;
    exp_g->MoveTo(
      center ? MidX : MinX, MaxY,
      center ? (bb.max.x - bb.min.x)/2 : bb.max.x/1, bb.max.y + h * 0.3
    );
  } while ( false );

  if ( number_unit != "" ) {
    bb = num_g->GetBB();
    U y = bb.min.y;
    bb = exp_g->GetBB();
    U x = bb.max.x;
    num_g->Add( new Text( 0, 0, number_unit ) );
    num_g->Last()->MoveTo( MinX, MinY, x, y );
  }

  Attributes attr;

  // Add number background.
  num_g->Attr()->Collect( attr );
  bb = num_g->GetBB();
  TextBG( g, bb, bb.max.y - bb.min.y );
  if ( leading_ws > 0 ) {
    g->Last()->Attr()->FillColor()->Clear();
    bb.min.x += attr.TextFont()->GetWidth( leading_ws );
    TextBG( g, bb, bb.max.y - bb.min.y );
  }

  // Add exponent background.
  exp_g->Attr()->Collect( attr );
  bb = exp_g->GetBB();
  TextBG( g, bb, bb.max.y - bb.min.y );
  if ( trailing_ws > 0 || num == 0 ) {
    g->Last()->Attr()->FillColor()->Clear();
    if ( num != 0 ) {
      bb.max.x -= attr.TextFont()->GetWidth( trailing_ws );
      TextBG( g, bb, bb.max.y - bb.min.y );
    }
  }

  if ( bold ) g->Attr()->TextFont()->SetBold();
  return g;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::BuildTicsNumsLinear(
  Axis& orth_axis,
  std::vector< SVG::Object* >& axes_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g,
  SVG::U sx, SVG::U sy, SVG::U ex, SVG::U ey
)
{
  std::vector< int64_t > mn_list;
  if ( major == 0 ) {
    mn_list.push_back( 0 );
    mn_list.push_back( 1 );
  } else {
    std::set< int64_t > mn_set;
    int64_t mn_min = std::floor( (min - major) / major );
    int64_t mn_max = std::ceil( (max + major) / major );
    auto add = [&]( int64_t mn )
    {
      if ( mn >= mn_min && mn <= mn_max && mn_set.find( mn ) == mn_set.end() ) {
        mn_list.push_back( mn );
        mn_set.insert( mn );
      }
    };
    add( 0 );
    int64_t step = mn_max - mn_min;
    while ( step & (step - 1) ) step++;
    while ( step > 0 ) {
      for ( int64_t mn = (mn_min / step) * step; mn <= mn_max; mn += step ) {
        add( mn );
      }
      step = step / 2;
    }
  }

  std::vector< int32_t > sn_list;
  if ( major == 0 ) {
    sn_list.push_back( 0 );
  } else {
    std::set< int32_t > sn_set;
    auto add = [&]( int32_t sn )
    {
      if ( sn < sub_divs && sn_set.find( sn ) == sn_set.end() ) {
        sn_list.push_back( sn );
        sn_set.insert( sn );
      }
    };
    int32_t step = sub_divs;
    while ( step > 0 ) {
      for ( int32_t sn = 0; sn < sub_divs; sn += step ) {
        add( sn );
      }
      do step--; while ( step > 0 && sub_divs % step );
    }
  }

  std::vector< SVG::Object* > num_objects;

  U oca_coor = Coor( orth_axis_cross );
  U min_coor = 0;
  U max_coor = length;
  U eps_coor = (max_coor - min_coor) * epsilon;
  U zro_coor = 1e9;
  if ( min < epsilon && max > -epsilon ) zro_coor = Coor( 0 );

  for ( int32_t sn : sn_list ) {
    for ( int64_t mn : mn_list ) {
      double v = (mn == 0) ? min : max;
      if ( major > 0 ) {
        v = mn * major + sn * major / sub_divs;
      }
      U v_coor = Coor( v );
      if ( v_coor < min_coor - eps_coor ) continue;
      if ( v_coor > max_coor + eps_coor ) continue;
      bool near_crossing_axis =
        v_coor >= oca_coor - eps_coor &&
        v_coor <= oca_coor + eps_coor;
      bool at_zero =
        v_coor >= zro_coor - eps_coor &&
        v_coor <= zro_coor + eps_coor;
      U x = (angle == 0) ? v_coor : sx;
      U y = (angle == 0) ? sy : v_coor;
      if ( !near_crossing_axis ) {
        U d = (sn == 0) ? tick_major_len : tick_minor_len;
        U gx1 = 0;
        U gy1 = 0;
        U gx2 = orth_axis.length;
        U gy2 = orth_axis.length;
        if ( angle == 0 ) {
          gx1 = gx2 = x;
          line_g->Add( new Line( x, y - d, x, y + d ) );
        } else {
          gy1 = gy2 = y;
          line_g->Add( new Line( x - d, y, x + d, y ) );
        }
        bool mg = sn == 0 && major_grid_enable;
        if ( mg || minor_grid_enable ) {
          if ( mg ) {
            if ( at_zero ) {
              zero_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
            } else {
              major_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
            }
          } else
          if ( minor_grid_enable ) {
            minor_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
          }
        }
      }
      if (
        (sn == 0 || show_minor_mumbers) &&
        ( !near_crossing_axis ||
          ( orth_axis.orth_axis_cross == orth_axis.min &&
            ((angle == 0) ? (number_pos == Below) : (number_pos == Left))
          )
        )
      )
      {
        U d = tick_major_len;
        Object* obj = BuildNum( num_g, v, sn == 0 );
        if ( angle == 0 ) {
          if ( number_pos == Above ) {
            obj->MoveTo( MidX, MinY, x, y + d + num_space_y );
          } else {
            obj->MoveTo( MidX, MaxY, x, y - d - num_space_y );
          }
        } else {
          if ( number_pos == Right ) {
            obj->MoveTo( MinX, MidY, x + d + num_space_x, y );
          } else {
            obj->MoveTo( MaxX, MidY, x - d - num_space_x, y );
          }
        }
        U mx = (angle == 0) ? 4 : 0;
        if (
          Chart::Collides( obj, axes_objects, mx, 0 ) ||
          Chart::Collides( obj, num_objects, mx, 0 )
        ) {
          num_g->DeleteFront();
        } else {
          num_objects.push_back( obj );
        }
      }
    }
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::BuildTicsNumsLogarithmic(
  Axis& orth_axis,
  std::vector< SVG::Object* >& axes_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g,
  SVG::U sx, SVG::U sy, SVG::U ex, SVG::U ey
)
{
  if ( major <= 0 ) return;

  int32_t pow_inc = std::round( std::log10( major ) );

  std::vector< int32_t > pow_list;
  {
    std::set< int32_t > pow_set;
    int32_t pow_inc = std::round( std::log10( major ) );
    int32_t pow_min = std::floor( std::log10( min ) ) - pow_inc;
    int32_t pow_max = std::ceil( std::log10( max ) ) + pow_inc;
    while ( pow_min % pow_inc ) pow_min--;
    while ( pow_max % pow_inc ) pow_max++;
    auto add = [&]( int32_t pow )
    {
      if ( pow >= pow_min && pow <= pow_max && pow_set.find( pow ) == pow_set.end() ) {
        pow_list.push_back( pow );
        pow_set.insert( pow );
      }
    };
    add( 0 );
    int32_t step = (pow_max - pow_min) / pow_inc;
    while ( step & (step - 1) ) step++;
    while ( step > 0 ) {
      int32_t p = pow_min / pow_inc;
      while ( p % step ) p--;
      while ( p <= pow_max / pow_inc ) {
        add( p * pow_inc );
        p += step;
      }
      step = step / 2;
    }
  }

  std::vector< int32_t > sn_list;
  {
    std::set< int32_t > sn_set;
    auto add = [&]( int32_t sn )
    {
      if ( sn < sub_divs && sn_set.find( sn ) == sn_set.end() ) {
        sn_list.push_back( sn );
        sn_set.insert( sn );
      }
    };
    add( 0 );
    add( (sub_divs % 10 == 0) ? (sub_divs / 10) : 1 );
    if ( sub_divs % 10 == 0 ) {
      add( 2 * sub_divs / 10 );
      add( 5 * sub_divs / 10 );
      add( 3 * sub_divs / 10 );
      add( 7 * sub_divs / 10 );
    }
    for ( int32_t i = 2; i <= sub_divs; i++ ) {
      if ( sub_divs % i == 0 ) {
        int32_t j = sub_divs / i;
        for ( int32_t sn = j; sn < sub_divs; sn += j ) add( sn );
      }
    }
  }

  std::vector< SVG::Object* > num_objects;

  U oca_coor = Coor( orth_axis_cross );
  U min_coor = 0;
  U max_coor = length;
  U eps_coor = (max_coor - min_coor) * epsilon;
  U one_coor = 1e9;
  if ( min < (1 + epsilon) && max > (1 - epsilon) ) one_coor = Coor( 1 );

  for ( int32_t sn : sn_list ) {
    if ( sn >= sub_divs ) continue;
    for ( int32_t pow_cur : pow_list ) {
      double m0 = std::pow( double( 10 ), pow_cur );
      double m1 = std::pow( double( 10 ), pow_cur + pow_inc );
      double v = m1 * sn / sub_divs;
      if ( sn == 0 ) v = m0;
      U v_coor = Coor( v );
      U m0_coor = Coor( m0 );
      if ( sn > 0 && v_coor <= m0_coor ) continue;
      if ( v_coor < min_coor - eps_coor ) continue;
      if ( v_coor > max_coor + eps_coor ) continue;
      bool near_crossing_axis =
        v_coor >= oca_coor - eps_coor &&
        v_coor <= oca_coor + eps_coor;
      bool at_one =
        v_coor >= one_coor - eps_coor &&
        v_coor <= one_coor + eps_coor;
      U x = (angle == 0) ? v_coor : sx;
      U y = (angle == 0) ? sy : v_coor;
      if ( !near_crossing_axis ) {
        U d = (sn == 0) ? tick_major_len : tick_minor_len;
        U gx1 = 0;
        U gy1 = 0;
        U gx2 = orth_axis.length;
        U gy2 = orth_axis.length;
        if ( angle == 0 ) {
          gx1 = gx2 = x;
          line_g->Add( new Line( x, y - d, x, y + d ) );
        } else {
          gy1 = gy2 = y;
          line_g->Add( new Line( x - d, y, x + d, y ) );
        }
        bool mg = sn == 0 && major_grid_enable;
        if ( mg || minor_grid_enable ) {
          if ( mg ) {
            if ( at_one ) {
              zero_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
            } else {
              major_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
            }
          } else
          if ( minor_grid_enable ) {
            minor_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
          }
        }
      }
      if (
        (sn == 0 || show_minor_mumbers) &&
        ( !near_crossing_axis ||
          ( orth_axis.orth_axis_cross == orth_axis.min &&
            ((angle == 0) ? (number_pos == Below) : (number_pos == Left))
          )
        )
      )
      {
        U d = tick_major_len;
        Object* obj = BuildNum( num_g, v, sn == 0 );
        if ( angle == 0 ) {
          if ( number_pos == Above ) {
            obj->MoveTo( MidX, MinY, x, y + d + num_space_y );
          } else {
            obj->MoveTo( MidX, MaxY, x, y - d - num_space_y );
          }
        } else {
          if ( number_pos == Right ) {
            obj->MoveTo( MinX, MidY, x + d + num_space_x, y );
          } else {
            obj->MoveTo( MaxX, MidY, x - d - num_space_x, y );
          }
        }
        U mx = (angle == 0) ? 4 : 0;
        if (
          Chart::Collides( obj, axes_objects, mx, 0 ) ||
          Chart::Collides( obj, num_objects, mx, 0 )
        ) {
          num_g->DeleteFront();
        } else {
          num_objects.push_back( obj );
        }
      }
    }
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::Build(
  uint32_t phase,
  Axis& orth_axis,
  std::vector< SVG::Object* >& axes_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g, SVG::Group* label_g
)
{
  U start = (orth_axis_cross == min) ? -tick_major_len : 0;
  U ap = orth_axis.Coor( orth_axis.orth_axis_cross );
  U sx = (angle == 0) ? start : ap;
  U sy = (angle == 0) ? ap : start;
  U ex = (angle == 0) ? (length + overhang) : +sx;
  U ey = (angle == 0) ? +sy : (length + overhang);

  // Limit for when axes are near min or max.
  double near = 0.3;

  if ( angle == 0 ) {
    if ( number_pos != Below && number_pos != Above ) {
      number_pos = (ap > (orth_axis.length * (1 - near))) ? Above : Below;
    }
  } else {
    if ( number_pos != Left && number_pos != Right ) {
      number_pos = (ap > (orth_axis.length * (1 - near))) ? Right : Left;
    }
  }

  if ( phase == 0 && unit != "" ) {
    Object* obj = Label( label_g, unit, 16 );
    obj->Attr()->TextFont()->SetBold();
    if ( angle == 0 ) {
      Pos unit_pos = this->unit_pos;
      if ( unit_pos != Below && unit_pos != Above && unit_pos != Right ) {
        unit_pos = (number_pos == Below) ? Above : Below;
      }
      switch ( unit_pos ) {
        case Above : obj->MoveTo( MaxX, MinY, ex, ey + tick_major_len + num_space_y ); break;
        case Below : obj->MoveTo( MaxX, MaxY, ex, ey - tick_major_len - num_space_y ); break;
        default    : obj->MoveTo( MinX, MidY, ex + 4, ey );
      }
      if (
        this->unit_pos == Auto &&
        (obj->GetBB().min.x - 50) < Coor( orth_axis_cross )
      ) {
        // Move if too close to Y-axis.
        if ( orth_axis.orth_axis_cross == orth_axis.min ) {
          obj->MoveTo( MaxX, MaxY, ex, ey - tick_major_len - num_space_y );
        } else {
          obj->MoveTo( MinX, MidY, ex + 4, ey );
        }
      }
    } else {
      switch ( unit_pos ) {
        case Right : obj->MoveTo( MinX, MaxY, ex + tick_major_len + num_space_x, ey ); break;
        case Left  : obj->MoveTo( MaxX, MaxY, ex - tick_major_len - num_space_x, ey ); break;
        case Below : obj->MoveTo( MidX, MaxY, ex, 0 - tick_major_len - num_space_y ); break;
        default    : obj->MoveTo( MidX, MinY, ex, ey + 4 );
      }
    }
    axes_objects.push_back( obj );
  }

  if ( phase == 0 ) return;

  line_g = line_g->AddNewGroup();
  num_g  = num_g->AddNewGroup();

  line_g->Add( new Line( sx, sy, ex, ey ) );
  Poly* poly =
    new Poly(
      { ex, ey,
        ex - arrow_length, ey + arrow_width/2,
        ex - arrow_length, ey - arrow_width/2
      }
    );
  line_g->Add( poly );
  poly->Close();
  poly->Attr()->FillColor()->Set( Black );
  poly->Rotate( angle, ex, ey );

  // Add DMZ rectangle for orthogonal axis to trigger collision for numbers
  // that are too close.
  {
    U oc = Coor( orth_axis_cross );
    U zc = 2 * tick_major_len;
    if ( angle == 0 ) {
      axes_objects.push_back(
        new Rect( oc - zc, 0, oc + zc, orth_axis.length + orth_axis.overhang )
      );
    } else {
      axes_objects.push_back(
        new Rect( 0, oc - zc, orth_axis.length + orth_axis.overhang, oc + zc )
      );
    }
  }

  ComputeNumFormat();

  if ( log_scale ) {
    BuildTicsNumsLogarithmic(
      orth_axis, axes_objects,
      minor_g, major_g, zero_g, line_g, num_g,
      sx, sy, ex, ey
    );
  } else {
    BuildTicsNumsLinear(
      orth_axis, axes_objects,
      minor_g, major_g, zero_g, line_g, num_g,
      sx, sy, ex, ey
    );
  }

  // Remove DMZ rectangle.
  delete axes_objects.back();
  axes_objects.pop_back();

  if ( label != "" ) {
    BoundaryBox b1 = line_g->GetBB();
    BoundaryBox b2;
    if ( !num_g->Empty() ) {
      b2 = num_g->GetBB();
    } else {
      b2 = b1;
    }
    U gap = 10;
    Object* obj = MultiLineText( label_g, label, 24 );
    obj->Rotate( angle );
    if ( angle == 0 ) {
      U x = length / 2;
      U y = 0;
      if ( y > b1.min.y ) y = b1.min.y;
      if ( y > b2.min.y ) y = b2.min.y;
      obj->MoveTo( MidX, MaxY, x, y - gap );
    } else {
      U y = length / 2;
      if (
        (orth_axis.orth_axis_cross - orth_axis.min) /
        (orth_axis.max - orth_axis.min) > (1 - near)
      ) {
        U x = orth_axis.length;
        if ( x < b1.max.x ) x = b1.max.x;
        if ( x < b2.max.x ) x = b2.max.x;
        obj->MoveTo( MinX, MidY, x + gap, y );
      } else {
        U x = 0;
        if ( x > b1.min.x ) x = b1.min.x;
        if ( x > b2.min.x ) x = b2.min.x;
        obj->MoveTo( MaxX, MidY, x - gap, y );
      }
    }
    axes_objects.push_back( obj );
  }

  axes_objects.push_back( line_g );
  axes_objects.push_back( num_g );

  return;
}

///////////////////////////////////////////////////////////////////////////////
