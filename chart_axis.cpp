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

////////////////////////////////////////////////////////////////////////////////

Axis::Axis( bool is_x_axis, Label* label_db )
{
  show = false;
  this->is_x_axis = is_x_axis;
  this->label_db = label_db;
  angle = is_x_axis ? 0 : 90;
  category_axis = false;
  reverse = false;
  y_dual = false;
  orth_dual = false;
  length = 0;
  chart_box = false;
  style = AxisStyle::Auto;
  pos = Pos::Auto;
  pos_base_axis_y_n = 0;
  digits = 0;
  decimals = 0;
  num_max_len = 0;
  exp_max_len = 0;
  log_scale = false;
  number_format = NumberFormat::Auto;
  number_sign = false;
  show_minor_mumbers = false;
  show_minor_mumbers_auto = true;
  number_size = 1.0;
  label_size = 1.0;
  data_def = false;
  data_min = 0;
  data_max = 0;
  data_min_is_base = false;
  data_max_is_base = false;
  min = 0;
  max = 0;
  orth_axis_cross = 0;
  major = 0;
  sub_divs = 0;
  number_pos = Pos::Auto;
  grid_style = GridStyle::Auto;
  major_grid_enable = true;
  minor_grid_enable = true;
  grid_set = false;
  unit_pos = Pos::Auto;

  orth_length = 0;
  orth_style[ 0 ] = AxisStyle::Auto;
  orth_style[ 1 ] = AxisStyle::Auto;
  orth_axis_coor[ 0 ] = 0;
  orth_axis_coor[ 1 ] = 0;
  orth_reverse[ 0 ] = false;
  orth_reverse[ 1 ] = false;

  orth_coor_is_min = false;
  orth_coor_is_max = false;
  orth_coor = 0;

  cat_coor_is_min = false;
  cat_coor_is_max = false;
  cat_coor = 0;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::SetAngle( int angle )
{
  this->angle = angle;
}

void Axis::SetReverse( bool reverse )
{
  this->reverse = reverse;
}

void Axis::SetStyle( AxisStyle style )
{
  this->style = style;
  show = true;
}

void Axis::SetPos( Pos pos, int axis_y_n )
{
  this->pos = pos;
  pos_base_axis_y_n = axis_y_n;
}

void Axis::SetLogScale( bool log_scale )
{
  this->log_scale = log_scale;
  show = true;
}

void Axis::SetNumberFormat( NumberFormat number_format )
{
  this->number_format = number_format;
  show = true;
}

void Axis::SetNumberSign( bool number_sign )
{
  this->number_sign = number_sign;
  show = true;
}

void Axis::SetNumberUnit( const std::string& txt )
{
  number_unit = txt;
  show = true;
}

void Axis::ShowMinorNumbers( bool show_minor_mumbers )
{
  this->show_minor_mumbers = show_minor_mumbers;
  this->show_minor_mumbers_auto = false;
  show = true;
}

void Axis::SetRange( double min, double max, double orth_axis_cross )
{
  this->min = min;
  this->max = max;
  if ( orth_axis_cross < min ) orth_axis_cross = min;
  if ( orth_axis_cross > max ) orth_axis_cross = max;
  this->orth_axis_cross = orth_axis_cross;
  show = true;
}

void Axis::SetRange( double min, double max )
{
  SetRange( min, max, 0 );
}

void Axis::SetTick( double major, int sub_divs )
{
  this->major = major;
  this->sub_divs = sub_divs;
  show = true;
}

void Axis::SetTickSpacing( int32_t start, int32_t stride )
{
  cat_start = std::max( 0, start );
  cat_stride = std::max( 1, stride );
}

void Axis::SetGridStyle( GridStyle gs )
{
  grid_style = gs;
}

void Axis::SetGrid( bool major_enable, bool minor_enable )
{
  major_grid_enable = major_enable;
  minor_grid_enable = minor_enable;
  grid_set = true;
  show = true;
}

void Axis::SetNumberPos( Pos pos )
{
  this->number_pos = pos;
}

void Axis::SetLabel( const std::string& txt )
{
  label = txt;
  show = true;
}

void Axis::SetSubLabel( const std::string& txt )
{
  sub_label = txt;
  show = true;
}

void Axis::SetUnit( const std::string& txt )
{
  unit = txt;
  show = true;
}

void Axis::SetUnitPos( Pos pos )
{
  this->unit_pos = pos;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::LegalizeMinor( void ) {
  if ( category_axis || major <= 0 ) {
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
        if ( std::abs( max_coor - coor ) <= 32 ) break;
        if ( sub_divs == 100 ) break;
        do sub_divs++; while ( 100 % sub_divs );
      }
      while ( true ) {
        U coor = Coor( max - max / sub_divs );
        if ( std::abs( max_coor - coor ) >= 8 ) break;
        if ( sub_divs == 1 ) break;
        do sub_divs--; while ( 100 % sub_divs );
      }
    }
    if ( sub_divs > 100 ) sub_divs = 100;
    while ( sub_divs > 1 ) {
      U coor = Coor( max - max / sub_divs );
      if ( std::abs( max_coor - coor ) >= 4 ) break;
      do sub_divs--; while ( 100 % sub_divs );
    }
  } else {
    if ( sub_divs < 1 ) sub_divs = 1;
    while ( sub_divs > 1 ) {
      U coor = Coor( max - major / sub_divs );
      if ( std::abs( max_coor - coor ) >= 10 ) break;
      do sub_divs--; while ( 1000 % sub_divs );
    }
  }

  return;
}

void Axis::LegalizeMajor( void ) {
  double mag = std::max( std::abs( min ), std::abs( max ) );

  while ( true ) {

    if ( category_axis ) {
      number_format = NumberFormat::None;
      if ( major < 1 ) major = 1;
      sub_divs = 0;
      break;
    }

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
      if ( number_format == NumberFormat::Auto ) {
        number_format =
          (min < 10e-30 || max > 0.1e30)
          ? NumberFormat::Scientific
          : NumberFormat::Magnitude;
      }
    } else {
      if ( show_minor_mumbers_auto ) show_minor_mumbers = false;
      if ( number_format == NumberFormat::Auto ) {
        number_format = NumberFormat::Fixed;
      }
    }
    if ( number_format == NumberFormat::Fixed ) {
      if (
        mag < ((number_format == NumberFormat::Auto) ? 0.01 : lim) ||
        mag > ((number_format == NumberFormat::Auto) ? 1e6 : 1e15)
      ) {
        number_format = NumberFormat::Scientific;
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
        if ( std::abs( max_coor - coor ) >= (auto_major ? 40 : 20) ) break;
        if ( number_format == NumberFormat::Magnitude ) {
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
        U min_space = std::min( 100.0, length / 4 );
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
    if ( number_format != NumberFormat::None ) {
      number_format = NumberFormat::Scientific;
    }
  }

  return;
}

void Axis::LegalizeMinMax(
  SVG::Group* tag_g,
  std::vector< Series* >* series_list
)
{
  bool min_is_base = false;
  bool max_is_base = false;

  if ( data_min == data_max ) {
    if ( log_scale ) {
      data_min = data_min / 10;
      data_max = data_max * 10;
    } else {
      data_min = data_min - 1;
      data_max = data_max + 1;
    }
    data_min_is_base = false;
    data_max_is_base = false;
  }

  bool automatic = false;

  if ( min >= max ) {
    automatic = true;
    min = data_min;
    max = data_max;
    min_is_base = data_min_is_base;
    max_is_base = data_max_is_base;
  }
  if ( log_scale && min <= 0 ) {
    min = data_min;
    min_is_base = data_min_is_base;
    if ( max <= min ) {
      max = 1000 * min;
      max_is_base = false;
    }
  }

  if ( automatic && !log_scale && !is_x_axis ) {
    if ( min > 0 && (max - min) / max > 0.5 && !min_is_base ) min = 0;
    if ( max < 0 && (min - max) / min > 0.5 && !max_is_base ) max = 0;
  }

  LegalizeMajor();

  if ( automatic ) {
    double p;

    if ( major > 0 ) {

      if ( log_scale ) {
        int32_t u = std::lround( std::log10( major ) );
        if ( !min_is_base ) {
          p = std::log10( min ) / u + epsilon;
          min = std::pow( std::pow( double( 10 ), u ), std::floor( p ) );
        }
        if ( !max_is_base ) {
          p = std::log10( max ) / u - epsilon;
          max = std::pow( std::pow( double( 10 ), u ), std::ceil( p ) );
          if ( max < 10 * min ) max = 10 * min;
        }
      } else {
        double e = (max - min) * epsilon;
        if ( !min_is_base ) {
          p = (min + e) / major;
          min = std::floor( p ) * major;
        }
        if ( !max_is_base ) {
          p = (max - e) / major;
          max = std::ceil( p ) * major;
        }
      }

      // Possibly expand min/max to make room for series tag.
      if ( !is_x_axis && show ) {
        double saved_min = min;
        double saved_max = max;
        int trial = 0;
        while ( ++trial ) {
          bool ok = true;
          for ( auto series : *series_list ) {
            if (
              series->axis_y != this ||
              !series->tag_enable ||
              ( series->type != SeriesType::Bar &&
                series->type != SeriesType::StackedBar &&
                series->type != SeriesType::Lollipop
              )
            )
              continue;
            U tag_beyond = series->tag_db->GetBeyond( series, tag_g );
            if ( !series->def_y || tag_beyond == 0 ) continue;
            if ( Valid( series->min_y ) && !series->min_y_is_base ) {
              U coor =
                Coor( series->min_y ) + (reverse ? +tag_beyond : -tag_beyond);
              if ( coor < 0 || coor > length ) {
                if ( log_scale ) {
                  min = min / major;
                } else {
                  min = min - major;
                }
                ok = false;
              }
            }
            if ( Valid( series->max_y ) && !series->max_y_is_base ) {
              U coor =
                Coor( series->max_y ) + (reverse ? -tag_beyond : +tag_beyond);
              if ( coor < 0 || coor > length ) {
                if ( log_scale ) {
                  max = max * major;
                } else {
                  max = max + major;
                }
                ok = false;
              }
            }
          }
          if ( ok ) break;
          if ( trial == 3 ) {
            // No success, restore.
            min = saved_min;
            max = saved_max;
            break;
          }
        }
      }

    }

    if ( is_x_axis && orth_style[ 0 ] == AxisStyle::None ) {
      orth_axis_cross = min;
    } else {
      orth_axis_cross = (max <= 0) ? max : min;
      if ( min < 0 && max > 0 && !chart_box ) orth_axis_cross = 0;
      if ( log_scale ) orth_axis_cross = min;
    }
  }

  LegalizeMinor();

  if ( orth_axis_cross < min ) orth_axis_cross = min;
  if ( orth_axis_cross > max ) orth_axis_cross = max;

  return;
}

////////////////////////////////////////////////////////////////////////////////

U Axis::Coor( double v )
{
  double c = -coor_hi;
  if ( log_scale ) {
    if ( v > 0 ) {
      double a = std::log10( min );
      double b = std::log10( max );
      c = (std::log10( v ) - a) * length / (b - a);
    }
  } else {
    c = (v - min) * length / (max - min);
  }
  c = reverse ? (length - c) : c;
  c = std::max( -coor_hi, c );
  c = std::min( +coor_hi, c );
  return c;
}

////////////////////////////////////////////////////////////////////////////////

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
  if (
    num != 0 &&
    ( number_format == NumberFormat::Scientific ||
      number_format == NumberFormat::Magnitude
    )
  ) {
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
    if ( number_format == NumberFormat::Magnitude ) {
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

  if ( number_format == NumberFormat::None      ) return;
  if ( number_format == NumberFormat::Magnitude ) return;
  if ( major <= 0 ) return;

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

  if ( number_format == NumberFormat::Fixed ) {
    for ( double v : v_list ) {
      ComputeDecimals( v, true );
    }
  }

  if ( number_format == NumberFormat::Scientific ) {
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
    if ( number_format != NumberFormat::Fixed ) decimals = 0;
    num_max_len = 0;
    exp_max_len = 0;
  } else {
    if ( number_pos == Pos::Left  ) num_max_len = 0;
    if ( number_pos == Pos::Right ) exp_max_len = 0;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

std::string Axis::NumToStr( double v, bool showpos )
{
  int32_t dec = std::max( ComputeDecimals( v ), decimals );
  std::ostringstream oss;
  if ( showpos && v > 0 ) oss << '+';
  oss << std::fixed << std::setprecision( dec ) << v;
  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Axis::BuildNum( SVG::Group* g, double v, bool bold )
{
  static const char magnitude_sym[] = "qryzafpnum kMGTPEZYRQ";

  if ( std::abs( v ) < num_lo ) v = 0;
  double num = v;
  int32_t exp = NormalizeExponent( num );

  NumberFormat number_format = this->number_format;
  if ( number_format == NumberFormat::Magnitude && (exp < -30 || exp > 30) ) {
    number_format = NumberFormat::Scientific;
  }

  std::string s = NumToStr( num, number_sign );

  if ( number_format == NumberFormat::Magnitude ) {
    int i = exp / 3;
    if ( i != 0 ) {
      if ( i == -2 ) {
        s += "µ";
      } else {
        s += magnitude_sym[ i + 10 ];
      }
    }
    s += number_unit;
    g = label_db->CreateInDB( g, s );
    if ( bold ) g->Attr()->TextFont()->SetBold();
    return g;
  }

  {
    int32_t leading_ws = num_max_len - s.length();
    if ( angle == 90 && leading_ws > 0 ) {
      s.insert( 0, leading_ws, ' ' );
    }
  }

  if ( number_format == NumberFormat::Fixed ) {
    s += number_unit;
    g = label_db->CreateInDB( g, s );
    if ( bold ) g->Attr()->TextFont()->SetBold();
    return g;
  }

  // number_format is NumberFormat::Scientific.
  SVG::Group* container;
  BoundaryBox bb;

  // Build non-exponent part,
  {
    if ( num == 0 ) {
      size_t pos = s.find( '.' );
      if ( pos != std::string::npos ) s.erase( pos );
    } else
    if ( std::abs( num ) == 1 && (angle == 0 || number_pos == Pos::Left) ) {
      s = (num < 0) ? "-10" : number_sign ? "+10" : "10";
    } else {
      s += "×10";
    }
    container = label_db->CreateInDB( g, s );
  }

  // Build exponent part,
  {
    if ( num == 0 ) {
      s = "";
    } else {
      std::ostringstream oss;
      oss << exp;
      s = oss.str();
    }
    if ( angle != 0 || num != 0 ) {
      int32_t trailing_ws = exp_max_len - s.length();
      if ( trailing_ws > 0 ) {
        s.insert( s.length(), trailing_ws, ' ' );
      }
    }
    bb = container->GetBB();
    U h = bb.max.y - bb.min.y;
    label_db->CreateInDB( container, s, h * 0.8, true );
    bool center = (num == 0 && angle == 0);
    container->Last()->MoveTo(
      center ? AnchorX::Mid : AnchorX::Min, AnchorY::Max,
      center ? (bb.max.x - bb.min.x)/2 : bb.max.x + h * 0.1, bb.max.y + h * 0.3
    );
    label_db->Update( container );
  }

  if ( !number_unit.empty() ) {
    bb = container->GetBB();
    label_db->CreateInDB( container, number_unit, 0, true );
    container->Last()->MoveTo(
      AnchorX::Min, AnchorY::Min, bb.max.x, bb.min.y
    );
    label_db->Update( container );
  }

  if ( bold ) container->Attr()->TextFont()->SetBold();
  return container;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::BuildTicksHelper(
  double v, SVG::U v_coor, int32_t sn, bool at_zero,
  SVG::U min_coor, SVG::U max_coor, SVG::U eps_coor,
  std::vector< SVG::Object* >& avoid_objects,
  std::vector< SVG::Object* >& num_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g
)
{
  if ( v_coor < min_coor - eps_coor ) return;
  if ( v_coor > max_coor + eps_coor ) return;

  bool near_crossing_axis[ 2 ];
  for ( int i : { 0, 1 } ) {
    near_crossing_axis[ i ] =
      orth_style[ i ] != AxisStyle::None &&
      CoorNear( v_coor, orth_axis_coor[ i ] );
  }
  bool not_near_crossing_axis =
    !near_crossing_axis[ 0 ] && !near_crossing_axis[ 1 ];

  bool near_chart_box_min = chart_box && CoorNear( v_coor, 0 );
  bool near_chart_box_max = chart_box && CoorNear( v_coor, length );
  bool not_near_chart_box = !near_chart_box_min && !near_chart_box_max;

  bool centered_tick =
    style == AxisStyle::Arrow || style == AxisStyle::Line;

  // Tick collides with orthogonal axis.
  bool collision = false;
  for ( int i : { 0, 1 } ) {
    if ( !near_crossing_axis[ i ] ) continue;
    bool at_orth_arrow =
      near_crossing_axis[ i ] &&
      orth_style[ i ] == AxisStyle::Arrow &&
      (orth_reverse[ i ] ? orth_coor_is_min : orth_coor_is_max);
    if ( !at_orth_arrow ) {
      if ( angle == 0 ) {
        if ( orth_coor_is_min && number_pos == Pos::Bottom ) continue;
        if ( orth_coor_is_min && centered_tick             ) continue;
        if ( orth_coor_is_max && number_pos == Pos::Top    ) continue;
        if ( orth_coor_is_max && centered_tick             ) continue;
      } else {
        if ( orth_coor_is_min && number_pos == Pos::Left   ) continue;
        if ( orth_coor_is_min && centered_tick             ) continue;
        if ( orth_coor_is_max && number_pos == Pos::Right  ) continue;
        if ( orth_coor_is_max && centered_tick             ) continue;
      }
    }
    collision = true;
  }

  // Tick collides with chart box.
  while ( 1 ) {
    if ( not_near_chart_box ) break;
    if ( angle == 0 ) {
      if ( orth_coor_is_min && number_pos == Pos::Bottom ) break;
      if ( orth_coor_is_min && centered_tick             ) break;
      if ( orth_coor_is_max && number_pos == Pos::Top    ) break;
      if ( orth_coor_is_max && centered_tick             ) break;
    } else {
      if ( orth_coor_is_min && number_pos == Pos::Left   ) break;
      if ( orth_coor_is_min && centered_tick             ) break;
      if ( orth_coor_is_max && number_pos == Pos::Right  ) break;
      if ( orth_coor_is_max && centered_tick             ) break;
    }
    collision = true;
    break;
  }

  U x = (angle == 0) ? v_coor : orth_coor;
  U y = (angle == 0) ? orth_coor : v_coor;

  U d = (sn == 0) ? tick_major_len : tick_minor_len;
  U gx1 = 0;
  U gy1 = 0;
  U gx2 = orth_length;
  U gy2 = orth_length;
  if ( angle == 0 ) {
    gx1 = gx2 = x;
    U y1 = y - d;
    U y2 = y + d;
    if ( (!not_near_crossing_axis || !not_near_chart_box) && centered_tick ) {
      if ( orth_coor_is_max ) y1 = y;
      if ( orth_coor_is_min ) y2 = y;
    }
    if ( style == AxisStyle::Edge ) {
      if ( number_pos == Pos::Top    ) y1 = y;
      if ( number_pos == Pos::Bottom ) y2 = y;
    }
    if ( style != AxisStyle::None && !collision ) {
      line_g->Add( new Line( x, y1, x, y2 ) );
    }
  } else {
    gy1 = gy2 = y;
    U x1 = x - d;
    U x2 = x + d;
    if ( (!not_near_crossing_axis || !not_near_chart_box) && centered_tick ) {
      if ( orth_coor_is_max ) x1 = x;
      if ( orth_coor_is_min ) x2 = x;
    }
    if ( style == AxisStyle::Edge ) {
      if ( number_pos == Pos::Right ) x1 = x;
      if ( number_pos == Pos::Left  ) x2 = x;
    }
    if ( style != AxisStyle::None && !collision ) {
      line_g->Add( new Line( x1, y, x2, y ) );
    }
  }

  if ( not_near_crossing_axis && not_near_chart_box ) {
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
    number_format != NumberFormat::None &&
    (sn == 0 || show_minor_mumbers)
  )
  {
    U d = tick_major_len;
    Group* obj = BuildNum( num_g, v, sn == 0 );
    if ( angle == 0 ) {
      if ( number_pos == Pos::Top ) {
        obj->MoveTo( AnchorX::Mid, AnchorY::Min, x, y + d + num_space_y );
      } else {
        obj->MoveTo( AnchorX::Mid, AnchorY::Max, x, y - d - num_space_y );
      }
    } else {
      if ( number_pos == Pos::Right ) {
        obj->MoveTo(
          AnchorX::Min, AnchorY::Mid,
          x + d + num_space_x * number_size, y
        );
      } else {
        obj->MoveTo(
          AnchorX::Max, AnchorY::Mid,
          x - d - num_space_x * number_size, y
        );
      }
    }
    U mx = num_char_w;
    if (
      Chart::Collides( obj, avoid_objects, mx, 0 ) ||
      Chart::Collides( obj, num_objects, mx, 0 )
    ) {
      label_db->Delete( obj );
      num_g->DeleteFront();
    } else {
      num_objects.push_back( obj );
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Axis::BuildTicksNumsLinear(
  std::vector< SVG::Object* >& avoid_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g
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
      bool at_zero = CoorNear( v_coor, zro_coor );
      BuildTicksHelper(
        v, v_coor, sn, at_zero,
        min_coor, max_coor, eps_coor,
        avoid_objects, num_objects,
        minor_g, major_g, zero_g, line_g, num_g
      );
    }
  }

  return;
}

//------------------------------------------------------------------------------

void Axis::BuildTicksNumsLogarithmic(
  std::vector< SVG::Object* >& avoid_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g
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
      if ( reverse ) {
        if ( sn > 0 && v_coor >= m0_coor ) continue;
      } else {
        if ( sn > 0 && v_coor <= m0_coor ) continue;
      }
      bool at_zero = CoorNear( v_coor, one_coor );
      BuildTicksHelper(
        v, v_coor, sn, at_zero,
        min_coor, max_coor, eps_coor,
        avoid_objects, num_objects,
        minor_g, major_g, zero_g, line_g, num_g
      );
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::BuildCategories(
  const std::vector< std::string >& category_list,
  std::vector< SVG::Object* >& avoid_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* cat_g
)
{
  bool normal_width = true;
  for ( const auto& cat : category_list ) {
    normal_width = normal_width && NormalWidthUTF8( cat );
  }
  if ( normal_width ) {
    cat_g->Attr()->TextFont()
      ->SetWidthFactor( 1.0 )
      ->SetHeightFactor( 1.0 )
      ->SetBaselineFactor( 1.0 );
  }

  U cat_char_w;
  U cat_char_h;
  {
    cat_g->Add( new Text( "X" ) );
    BoundaryBox bb = cat_g->Last()->GetBB();
    cat_g->DeleteFront();
    cat_char_w = bb.max.x - bb.min.x;
    cat_char_h = bb.max.y - bb.min.y;
  }

  int text_angle = 45;
  AnchorX ax = AnchorX::Mid;
  AnchorY ay = AnchorY::Mid;
  U dx = 0;
  U dy = 0;
  if ( angle == 0 ) {
    if ( number_pos == Pos::Top ) {
      ay = AnchorY::Min;
      dy = 0 + tick_major_len + num_space_y;
    } else {
      ay = AnchorY::Max;
      dy = 0 - tick_major_len - num_space_y;
    }
    U x1 = Coor( 0 );
    U x2 = Coor( category_list.size() );
    if (
      std::abs( x2 - x1 ) <
      ( category_list.size() * cat_char_h * 1.5
        / std::max( cat_stride, cat_stride_empty )
      )
    ) {
      text_angle = 90;
    }
  } else {
    if ( number_pos == Pos::Right ) {
      ax = AnchorX::Min;
      dx = 0 + tick_major_len + num_space_x * number_size;
    } else {
      ax = AnchorX::Max;
      dx = 0 - tick_major_len - num_space_x * number_size;
    }
  }

  std::vector< SVG::Object* > cat_objects;
  std::vector< uint32_t > mn_list;

  uint32_t trial = 0;
  for ( bool commit : { false, true } ) {
    while ( true ) {
      bool collision = false;
      int32_t n = -1;
      for ( const auto& cat : category_list ) {
        n++;
        if ( cat.empty() ) continue;
        if ( n < cat_start ) continue;
        if ( (n - cat_start) % cat_stride ) continue;

        Object* obj = cat_g->Add( new Text( cat ) );
        U x = (angle == 0) ? Coor( n ) : cat_coor;
        U y = (angle != 0) ? Coor( n ) : cat_coor;
        if ( trial == 0 ) {
          obj->MoveTo( ax, ay, x + dx, y + dy );
        }
        if ( trial == 1 ) {
          U sy = (n % 2) ? (cat_char_h + num_space_y) : 0;
          if ( dy < 0 ) sy = -sy;
          obj->MoveTo( ax, ay, x + dx, y + dy + sy );
        }
        if ( trial == 2 ) {
          ax = (number_pos == Pos::Top) ? AnchorX::Min : AnchorX::Max;
          ay = AnchorY::Mid;
          obj->MoveTo( ax, ay, x + dx, y + dy );
          obj->Rotate( text_angle, ax, ay );
        }
        if (
          (trial < 2 || (text_angle % 90 == 0)) &&
          Chart::Collides(
            obj, cat_objects, ((trial < 2) ? (1.5 * cat_char_w) : 0), 0
          )
        ) {
          collision = true;
          cat_g->DeleteFront();
        } else {
          U mx = (angle == 0) ? 4 : 0;
          bool aoc = Chart::Collides( obj, avoid_objects, mx, 0 );
          if ( commit && aoc ) {
            cat_g->DeleteFront();
          } else {
            cat_objects.push_back( obj );
            if ( commit ) mn_list.push_back( n );
          }
        }
      }
      if ( commit ) break;
      while ( !cat_objects.empty() ) {
        cat_g->DeleteFront();
        cat_objects.pop_back();
      }
      if ( !collision ) break;
      if ( angle != 0 ) break;
      if ( trial == 2 ) break;
      ++trial;
    }
  }

  if ( (minor_grid_enable || major_grid_enable) && major > 0 ) {
    uint32_t mm = std::llround( major );
    if ( mm < 1 ) mm = 1;
    for ( uint32_t mn : mn_list ) {
      if ( mn % mm ) continue;
      double v = mn;
      if ( v > max ) break;
      U v_coor = Coor( v );
      U gx1 = 0;
      U gy1 = 0;
      U gx2 = orth_length;
      U gy2 = orth_length;
      if ( angle == 0 ) {
        gx1 = gx2 = v_coor;
      } else {
        gy1 = gy2 = v_coor;
      }
      bool near_crossing_axis[ 2 ];
      for ( int i : { 0, 1 } ) {
        near_crossing_axis[ i ] =
          orth_style[ i ] != AxisStyle::None &&
          CoorNear( v_coor, orth_axis_coor[ i ] );
      }
      bool not_near_crossing_axis =
        !near_crossing_axis[ 0 ] && !near_crossing_axis[ 1 ];
      bool near_chart_box_min = chart_box && CoorNear( v_coor, 0 );
      bool near_chart_box_max = chart_box && CoorNear( v_coor, length );
      bool not_near_chart_box = !near_chart_box_min && !near_chart_box_max;
      if ( not_near_crossing_axis && not_near_chart_box ) {
        if ( major_grid_enable ) {
          major_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
        } else {
          minor_g->Add( new Line( gx2, gy2, gx1, gy1 ) );
        }
      }
      mn++;
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::BuildUnit(
  SVG::Group* unit_g,
  std::vector< SVG::Object* >& avoid_objects
)
{
  if ( unit.empty() ) return;

  bool at_orth_min = category_axis ? cat_coor_is_min : orth_coor_is_min;
  bool at_orth_max = category_axis ? cat_coor_is_max : orth_coor_is_max;

  U coor = category_axis ? cat_coor : orth_coor;

  U outer_max = length;
  U outer_min = 0;
  U inner_max = outer_max;
  U inner_min = outer_min;

  outer_max += (angle == 0) ? (num_space_x * number_size) : +num_space_y;
  outer_min -= (angle == 0) ? (num_space_x * number_size) : +num_space_y;
  if ( style == AxisStyle::Arrow ) {
    if ( reverse ) {
      outer_max += tick_major_len;
      outer_min -= overhang;
    } else {
      outer_max += overhang;
      outer_min -= tick_major_len;
    }
  } else {
    outer_max += tick_major_len;
    outer_min -= tick_major_len;
  }
  if ( chart_box ) {
    inner_max -= tick_major_len;
    inner_min += tick_major_len;
  } else {
    inner_max = outer_max;
    inner_min = outer_min;
  }

  Object* obj = label_db->CreateInDB( unit_g, unit, 16 * number_size );
  obj->Attr()->TextFont()->SetBold();
  bool collision = false;

  auto place = [&]( Pos px, Pos py )
  {
    AnchorX ax = AnchorX::Mid;
    AnchorY ay = AnchorY::Mid;
    U cx = length / 2;
    U cy = length / 2;

    if ( px == Pos::Left ) {
      if ( angle == 0 ) {
        cx = (py == Pos::Center) ? outer_min : inner_min;
        ax = (py == Pos::Center) ? AnchorX::Max : AnchorX::Min;
      } else {
        cx = coor - tick_major_len - num_space_x * number_size;
        ax = AnchorX::Max;
      }
    }
    if ( px == Pos::Right ) {
      if ( angle == 0 ) {
        cx = (py == Pos::Center) ? outer_max : inner_max;
        ax = (py == Pos::Center) ? AnchorX::Min : AnchorX::Max;
      } else {
        cx = coor + tick_major_len + num_space_x * number_size;
        ax = AnchorX::Min;
      }
    }
    if ( px == Pos::Center ) {
      cx = (angle == 0) ? U( length / 2 ) : coor;
      ax = AnchorX::Mid;
    }

    if ( py == Pos::Bottom ) {
      if ( angle == 0 ) {
        cy = coor - tick_major_len - num_space_y;
        ay = AnchorY::Max;
      } else {
        cy = (px == Pos::Center) ? outer_min : inner_min;
        ay = (px == Pos::Center) ? AnchorY::Max : AnchorY::Min;
      }
    }
    if ( py == Pos::Top ) {
      if ( angle == 0 ) {
        cy = coor + tick_major_len + num_space_y;
        ay = AnchorY::Min;
      } else {
        cy = (px == Pos::Center) ? outer_max : inner_max;
        ay = (px == Pos::Center) ? AnchorY::Min : AnchorY::Max;
      }
    }
    if ( py == Pos::Center ) {
      cy = (angle == 0) ? coor : U( length / 2 );
      ay = AnchorY::Mid;
    }

    if ( chart_box ) {
      if ( angle == 0 ) {
        if ( cy < 0 || cy > orth_length ) {
          if ( cx == inner_max ) cx = outer_max;
          if ( cx == inner_min ) cx = outer_min;
        }
      } else {
        if ( cx < 0 || cx > orth_length ) {
          if ( cy == inner_max ) cy = outer_max;
          if ( cy == inner_min ) cy = outer_min;
        }
      }
    }

    obj->MoveTo( ax, ay, cx, cy );
    BoundaryBox bb = obj->GetBB();
    collision = false;
    for ( int i = 0; i < 2; i++ ) {
      U mx = 48;
      U my = 32;
      if ( angle == 0 ) {
        if ( orth_axis_coor[ i ] <= bb.min.x - mx )
          continue;
        if ( orth_axis_coor[ i ] >= bb.max.x + mx )
          continue;
        if (
          ( orth_reverse[ i ]
            ? (at_orth_max && py == Pos::Top   )
            : (at_orth_min && py == Pos::Bottom)
          ) &&
          px == (reverse ? Pos::Left : Pos::Right) &&
          style == AxisStyle::Arrow
        )
          continue;
      } else {
        if ( orth_axis_coor[ i ] <= bb.min.y - my )
          continue;
        if ( orth_axis_coor[ i ] >= bb.max.y + my )
          continue;
        if (
          ( orth_reverse[ i ]
            ? (at_orth_max && px == Pos::Right)
            : (at_orth_min && px == Pos::Left )
          ) &&
          py == (reverse ? Pos::Bottom : Pos::Top) &&
          style == AxisStyle::Arrow
        )
          continue;
      }
      collision = true;
    }
    if ( chart_box ) {
      U mx = tick_major_len - epsilon;
      U my = tick_major_len - epsilon;
      bb.min.x -= mx; bb.max.x += mx;
      bb.min.y -= my; bb.max.y += my;
      BoundaryBox cb;
      cb.min.x = 0;
      cb.min.y = 0;
      cb.max.x = (angle == 0) ? length : orth_length;
      cb.max.y = (angle != 0) ? length : orth_length;
      if (
        (bb.min.x < cb.min.x && bb.max.x > cb.min.x) ||
        (bb.min.x < cb.max.x && bb.max.x > cb.max.x)
      ) {
        if ( bb.min.y < cb.max.y && bb.max.y > cb.min.y ) collision = true;
      }
      if (
        (bb.min.y < cb.min.y && bb.max.y > cb.min.y) ||
        (bb.min.y < cb.max.y && bb.max.y > cb.max.y)
      ) {
        if ( bb.min.x < cb.max.x && bb.max.x > cb.min.x ) collision = true;
      }
    }

    return collision;
  };

  bool automatic =
    unit_pos != Pos::Bottom && unit_pos != Pos::Top &&
    unit_pos != Pos::Left && unit_pos != Pos::Right;

  if ( angle == 0 ) {
    if ( automatic ) {
      if ( category_axis ) {
        unit_pos = reverse ? Pos::Left : Pos::Right;
      } else {
        unit_pos = (number_pos == Pos::Bottom) ? Pos::Top : Pos::Bottom;
        if ( chart_box ) {
          if ( at_orth_min && number_pos == Pos::Top ) {
            unit_pos = Pos::Top;
          }
          if ( at_orth_max && number_pos == Pos::Bottom ) {
            unit_pos = Pos::Bottom;
          }
          if ( !at_orth_min && !at_orth_max ) {
            unit_pos = (number_pos == Pos::Top) ? Pos::Top : Pos::Bottom;
          }
        } else {
          if ( orth_dual && style == AxisStyle::Arrow ) {
            unit_pos = reverse ? Pos::Left : Pos::Right;
          }
        }
      }
    }
    if ( unit_pos == Pos::Bottom || unit_pos == Pos::Top ) {
      if ( orth_dual || category_axis ) {
        place( Pos::Center, unit_pos );
      } else {
        if ( style == AxisStyle::Arrow ) {
          place( reverse ? Pos::Left : Pos::Right, unit_pos    ) &&
          place( reverse ? Pos::Left : Pos::Right, Pos::Center );
        } else {
          place( reverse ? Pos::Left  : Pos::Right, unit_pos ) &&
          place( reverse ? Pos::Right : Pos::Left , unit_pos ) &&
          place( Pos::Center, unit_pos );
        }
      }
    }
    if ( unit_pos == Pos::Left || unit_pos == Pos::Right ) {
      place( unit_pos, Pos::Center );
      collision = false;
    }
    if ( collision ) {
      place( reverse ? Pos::Left : Pos::Right, Pos::Center );
    }
  }

  if ( angle != 0 ) {
    if ( automatic ) {
      if ( category_axis ) {
        unit_pos = Pos::Top;
      } else {
        unit_pos = (number_pos == Pos::Left) ? Pos::Right : Pos::Left;
        if ( chart_box ) {
          if ( at_orth_min && number_pos == Pos::Right ) {
            unit_pos = Pos::Right;
          }
          if ( at_orth_max && number_pos == Pos::Left ) {
            unit_pos = Pos::Left;
          }
          if ( !at_orth_min && !at_orth_max ) {
            unit_pos = (number_pos == Pos::Left) ? Pos::Left : Pos::Right;
          }
        } else {
          if (
            (orth_dual && style == AxisStyle::Arrow) ||
            style == AxisStyle::None
          ) {
            unit_pos = reverse ? Pos::Bottom : Pos::Top;
          }
        }
      }
    }
    if ( unit_pos == Pos::Left || unit_pos == Pos::Right ) {
      if ( orth_dual ) {
        place( unit_pos, Pos::Center );
      } else {
        if ( style == AxisStyle::Arrow ) {
          place( unit_pos   , reverse ? Pos::Bottom : Pos::Top ) &&
          place( Pos::Center, reverse ? Pos::Bottom : Pos::Top );
        } else {
          place( unit_pos, reverse ? Pos::Bottom : Pos::Top    ) &&
          place( unit_pos, reverse ? Pos::Top    : Pos::Bottom ) &&
          place( unit_pos, Pos::Center );
        }
      }
    }
    if ( unit_pos == Pos::Bottom || unit_pos == Pos::Top ) {
      place( Pos::Center, unit_pos );
      collision = false;
    }
    if ( collision ) {
      place( Pos::Center, reverse ? Pos::Bottom : Pos::Top );
    }
  }

  avoid_objects.push_back( obj );

  return;
}

//------------------------------------------------------------------------------

void Axis::Build(
  const std::vector< std::string >& category_list,
  uint32_t phase,
  std::vector< SVG::Object* >& avoid_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g, SVG::Group* unit_g
)
{
  if ( !show ) return;

  // Limit for when axes are near min or max.
  double near = 0.3;

  if ( category_axis ) {
    if ( angle == 0 ) {
      if ( number_pos != Pos::Top && number_pos != Pos::Bottom ) {
        number_pos = Pos::Auto;
      }
      if ( pos == Pos::Base && orth_coor_is_max ) pos = Pos::Top;
      if ( pos != Pos::Top && pos != Pos::Bottom ) {
        pos = (number_pos != Pos::Auto) ? number_pos : Pos::Bottom;
      }
      if ( number_pos == Pos::Auto ) number_pos = pos;
      cat_coor = (pos == Pos::Top) ? orth_length : U( 0 );
      cat_coor_is_min = (pos != Pos::Top);
      cat_coor_is_max = (pos == Pos::Top);
    } else {
      if ( number_pos != Pos::Right && number_pos != Pos::Left ) {
        number_pos = Pos::Auto;
      }
      if ( pos == Pos::Base && orth_coor_is_max ) pos = Pos::Right;
      if ( pos != Pos::Right && pos != Pos::Left ) {
        pos = (number_pos != Pos::Auto) ? number_pos : Pos::Left;
      }
      if ( number_pos == Pos::Auto ) number_pos = pos;
      cat_coor = (pos == Pos::Right) ? orth_length : U( 0 );
      cat_coor_is_min = (pos != Pos::Right);
      cat_coor_is_max = (pos == Pos::Right);
    }
  } else {
    if ( angle == 0 ) {
      if ( number_pos != Pos::Bottom && number_pos != Pos::Top ) {
        number_pos =
          (orth_coor > (orth_length * (1 - near))) ? Pos::Top : Pos::Bottom;
      }
    } else {
      if ( number_pos != Pos::Left && number_pos != Pos::Right ) {
        number_pos =
          (orth_coor > (orth_length * (1 - near))) ? Pos::Right : Pos::Left;
      }
    }
  }

  if ( phase == 0 ) {
    BuildUnit( unit_g, avoid_objects );
    return;
  }

  line_g = line_g->AddNewGroup();
  num_g = num_g->AddNewGroup();
  num_g->Attr()->TextFont()->SetSize( 14 * number_size );
  if ( !category_axis ) {
    // Reset letter spacing to default, but offset the baseline such that
    // numbers are vertically centered in their boundary box. Numbers have no
    // glyph below the baseline and will therefore appear vertically un-centered
    // without this adjustment.
    num_g->Attr()->TextFont()
      ->SetWidthFactor( 1.0 )
      ->SetHeightFactor( 1.0 )
      ->SetBaselineFactor( 0.6 );
  }
  {
    num_g->Add( new Text( "X" ) );
    BoundaryBox bb = num_g->Last()->GetBB();
    num_g->DeleteFront();
    num_char_w = bb.max.x - bb.min.x;
    num_char_h = bb.max.y - bb.min.y;
  }

  U as = 0;
  U ae = length;
  if ( reverse ) {
    std::swap( as, ae );
    if ( style == AxisStyle::Arrow ) ae -= overhang;
  } else {
    if ( style == AxisStyle::Arrow ) ae += overhang;
  }
  U sx = (angle == 0) ? as : orth_coor;
  U sy = (angle == 0) ? orth_coor : as;
  U ex = (angle == 0) ? ae : orth_coor;
  U ey = (angle == 0) ? orth_coor : ae;

  bool axis_at_chart_box = chart_box && (orth_coor_is_min || orth_coor_is_max);

  if ( style != AxisStyle::None ) {
    if ( style == AxisStyle::Arrow ) {
      U sv = reverse ? -1 : +1;
      U dx = (angle == 0) ? (sv * arrow_length/2) : 0;
      U dy = (angle != 0) ? (sv * arrow_length/2) : 0;
      if ( axis_at_chart_box ) {
        if ( angle == 0 ) {
          sx = reverse ? U( 0 ) : length;
        } else {
          sy = reverse ? U( 0 ) : length;
        }
      }
      line_g->Add( new Line( sx, sy, ex - dx, ey - dy) );
      Poly* poly =
        new Poly(
          { ex, ey,
            ex - sv * arrow_length, ey + sv * arrow_width/2,
            ex - sv * arrow_length, ey - sv * arrow_width/2
          }
        );
      line_g->Add( poly );
      poly->Close();
      poly->Rotate( angle, ex, ey );
    } else {
      if ( !axis_at_chart_box ) {
        line_g->Add( new Line( sx, sy, ex, ey ) );
      }
    }
  }

  // Add DMZ rectangles for orthogonal axis to trigger collision for numbers
  // that are too close.
  int dmz_cnt = 0;
  for ( int i : { 0, 1 } ) {
    if ( orth_style[ i ] == AxisStyle::None ) continue;
    U oc = orth_axis_coor[ i ];
    U zc = 2 * tick_major_len;
    U os = 0;
    U oe = orth_length;
    if ( orth_style[ i ] == AxisStyle::Arrow ) {
      if ( orth_reverse[ i ] ) {
        os -= overhang;
      } else {
        oe += overhang;
      }
    }
    if ( angle == 0 ) {
      avoid_objects.push_back( new Rect( oc - zc, os, oc + zc, oe ) );
    } else {
      avoid_objects.push_back( new Rect( os, oc - zc, oe, oc + zc ) );
    }
    dmz_cnt++;
  }

  // Add DMZ rectangles for chart box to trigger collision for numbers that are
  // too close.
  if ( chart_box ) {
    for ( int i : { 0, 1 } ) {
      U oc = (i == 0) ? U( 0 ) : length;
      U zc = tick_major_len;
      U os = 0;
      U oe = orth_length;
      if ( angle == 0 ) {
        avoid_objects.push_back( new Rect( oc - zc, os, oc + zc, oe ) );
      } else {
        avoid_objects.push_back( new Rect( os, oc - zc, oe, oc + zc ) );
      }
      dmz_cnt++;
    }
  }

  minor_g = minor_g->AddNewGroup();
  major_g = major_g->AddNewGroup();
  zero_g  = zero_g->AddNewGroup();
  if ( grid_style == GridStyle::Solid ) {
    minor_g->Attr()
      ->SetLineWidth( 0.25 )
      ->LineColor()->Set( GridColor() );
    major_g->Attr()
      ->SetLineWidth( 0.50 )
      ->LineColor()->Set( GridColor() );
    zero_g->Attr()
      ->SetLineWidth( 0.50 )
      ->LineColor()->Set( GridColor() );
  } else {
    minor_g->Attr()
      ->SetLineWidth( 0.25 )
      ->SetLineDash( 2, 3 )
      ->LineColor()->Set( GridColor() );
    major_g->Attr()
      ->SetLineWidth( 0.50 )
      ->SetLineDash( 5, 3 )
      ->LineColor()->Set( GridColor() );
    zero_g->Attr()
      ->SetLineWidth( 1.00 )
      ->SetLineDash( 5, 3 )
      ->LineColor()->Set( GridColor() );
  }

  if ( category_axis ) {
    BuildCategories(
      category_list, avoid_objects,
      minor_g, major_g, num_g
    );
  } else {
    ComputeNumFormat();
    if ( log_scale ) {
      BuildTicksNumsLogarithmic(
        avoid_objects,
        minor_g, major_g, zero_g, line_g, num_g
      );
    } else {
      BuildTicksNumsLinear(
        avoid_objects,
        minor_g, major_g, zero_g, line_g, num_g
      );
    }
  }

  // Remove DMZ rectangles.
  while ( dmz_cnt > 0 ) {
    delete avoid_objects.back();
    avoid_objects.pop_back();
    dmz_cnt--;
  }

  avoid_objects.push_back( line_g );
  avoid_objects.push_back( num_g );

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Axis::BuildLabel(
  std::vector< SVG::Object* >& avoid_objects,
  SVG::Group* label_g
)
{
  U space_x = 25;
  U space_y = 10;
  if ( angle != 0 ) std::swap( space_x, space_y );

  std::vector< SVG::Object* > label_objs;

  Object* lab0 = nullptr;
  Object* lab1 = nullptr;
  if ( !label.empty() ) {
    lab0 = Label::CreateLabel( label_g, label, 24 * label_size );
    label_objs.push_back( lab0 );
  }
  if ( !sub_label.empty() ) {
    lab1 = Label::CreateLabel( label_g, sub_label, 16 * label_size );
    label_objs.push_back( lab1 );
  }

  if ( lab0 == nullptr && lab1 == nullptr ) return;

  bool at_orth_min = category_axis ? cat_coor_is_min : orth_coor_is_min;
  bool at_orth_max = category_axis ? cat_coor_is_max : orth_coor_is_max;

  Dir dir = Dir::Down;
  if ( angle == 0 ) {
    if ( y_dual && at_orth_max ) {
      dir = Dir::Up;
    }
  }
  if ( angle != 0 ) {
    if ( at_orth_max || (number_pos == Pos::Right && !at_orth_min) ) {
      dir = Dir::Right;
      if ( lab0 ) lab0->Rotate( -90 );
      if ( lab1 ) lab1->Rotate( -90 );
    } else {
      dir = Dir::Left;
      if ( lab0 ) lab0->Rotate( +90 );
      if ( lab1 ) lab1->Rotate( +90 );
    }
  }

  if ( dir == Dir::Down ) {
    U y = 0 - space_y;
    if ( lab0 ) {
      lab0->MoveTo( AnchorX::Mid, AnchorY::Max, length / 2, y );
      BoundaryBox bb = lab0->GetBB();
      y -= bb.max.y - bb.min.y + 3;
    }
    if ( lab1 ) {
      lab1->MoveTo( AnchorX::Mid, AnchorY::Max, length / 2, y );
    }
  } else
  if ( dir == Dir::Up ) {
    U y = orth_length + space_y;
    if ( lab1 ) {
      lab1->MoveTo( AnchorX::Mid, AnchorY::Min, length / 2, y );
      BoundaryBox bb = lab1->GetBB();
      y += bb.max.y - bb.min.y + 3;
    }
    if ( lab0 ) {
      lab0->MoveTo( AnchorX::Mid, AnchorY::Min, length / 2, y );
    }
  } else {
    U x        = (dir == Dir::Left) ? (0 - space_x) : (orth_length + space_x);
    AnchorX ax = (dir == Dir::Left) ? AnchorX::Max : AnchorX::Min;
    double vx  = (dir == Dir::Left) ? -1 : 1;
    if ( lab1 ) {
      lab1->MoveTo( ax, AnchorY::Mid, x, length / 2 );
      BoundaryBox bb = lab1->GetBB();
      x += (bb.max.x - bb.min.x + 3) * vx;
    }
    if ( lab0 ) {
      lab0->MoveTo( ax, AnchorY::Mid, x, length / 2 );
    }
  }

  MoveObjs( dir, label_objs, avoid_objects, space_x, space_y );

  if ( lab0 ) avoid_objects.push_back( lab0 );
  if ( lab1 ) avoid_objects.push_back( lab1 );

  return;
}

////////////////////////////////////////////////////////////////////////////////
