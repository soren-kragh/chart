//
//  Copyright (C) 2023, S. Kragh
//
//  This file is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License for more details.
//  <http://www.gnu.org/licenses/>.
//

#include <chart_axis.h>
#include <set>

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Axis::Axis( int angle )
{
  this->angle = angle;
  decimals = -1;
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

void Axis::AutoTick( void ) {
  double mag = std::max( std::abs( min ), std::abs( max ) );

  if ( mag < num_lo || mag > num_hi || (max - min) < num_lo ) {
    major = 0;
    return;
  }
  if ( log_scale ) {
    if ( min < num_lo || max > num_hi ) {
      major = 0;
      return;
    }
    if ( show_minor_mumbers_auto ) show_minor_mumbers = true;
    if ( number_format_auto ) {
      number_format = (mag < 5e-29 || mag > 5e29) ? Scientific : Magnitude;
    }
    if ( number_format == Fixed ) {
      if ( min < 1e-8 || max > 1e20 ) {
        number_format = Scientific;
      }
    }
  } else {
    if ( show_minor_mumbers_auto ) show_minor_mumbers = false;
    if ( number_format_auto ) number_format = Fixed;
    if ( number_format == Fixed ) {
      if ( mag < 1e-8 || mag > (number_format_auto ? 1e10 : 1e20) ) {
        number_format = Scientific;
      }
    }
  }

  if ( log_scale ) {
    bool auto_major = major < 9.5;
    if ( auto_major ) major = 10;
    major = std::pow( double( 10 ), std::round( std::log10( major ) ) );
    int32_t e1 = std::floor( std::log10( min ) * (1 + cre) );
    int32_t e2 = std::ceil( std::log10( max ) * (1 - cre) );
    if ( e2 <= e1 ) e2 = e1 + 1;
    if ( auto_major ) {
      while ( (e2 - e1) / std::round( std::log10( major ) ) > length / 35 ) {
        if ( number_format == Magnitude ) {
          major = major * ((major > 10) ? 1000 : 100);
        } else {
          major = major * 10;
        }
      }
    }
    if ( sub_divs < 1 ) {
      int32_t mc = (e2 - e1) / std::round( std::log10( major ) );
      if ( mc < 1 ) mc = 1;
      sub_divs = 10;
      if ( length / (mc * sub_divs) < 15 ) {
        sub_divs = std::lround( length / (15.0 * mc) );
      }
      if ( length / (mc * sub_divs) > 50 ) {
        sub_divs = std::lround( length / (50.0 * mc) );
      }
    }
    if ( sub_divs < 1 || major > 10 ) sub_divs = 1;
    if ( sub_divs > 100 ) sub_divs = 100;
    while ( 100 % sub_divs ) sub_divs--;
  } else {
    if ( major > 0 ) {
      if ( sub_divs < 1 ) sub_divs = 1;
      if ( length * major < 10 * (max - min) ) {
        major = 0;
      } else
      while ( length * major < 5 * (max - min) * sub_divs ) {
        sub_divs = sub_divs / 2;
        while ( sub_divs % 2 && sub_divs % 5 ) sub_divs--;
      }
      if ( sub_divs < 1 ) sub_divs = 1;
    }
    if ( major <= 0 ) {
      double m = 1;
      double d = 1;
      bool f = false;
      while ( 1 ) {
        double q = (max - min) / (m / d);
        if ( !f && q <= 10 ) d = d * 2; else
        if (  f && q <=  4 ) d = d * 5; else
        break;
        f = !f;
      }
      f = false;
      while ( d == 1 ) {
        double q = (max - min) / (m / d);
        if ( !f && q > 20 ) m = m * 2; else
        if (  f && q > 20 ) m = m * 5; else
        break;
        f = !f;
      }
      major = m / d;
      sub_divs = 2;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

U Axis::Coor( double v )
{
  if ( log_scale ) {
    if ( v > 0 ) {
      return
        (std::log10( v ) - std::log10( min )) * length /
        (std::log10( max ) - std::log10( min ));
    } else {
      return -1e20;
    }
  } else {
    return
      (v - min) * length / (max - min);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Updates (increases) digits and decimals veriables based on v.
void Axis::ComputeDecimals( double v )
{
  const double lim = std::pow( double( 10 ), -precision ) / 2;
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
  digits = std::max( dig, digits );
  decimals = std::max( dec, decimals );
}

int32_t Axis::NormalizeExponent( double& num )
{
  int32_t exp = 0;
  if ( num != 0 && number_format != Fixed ) {
    double sign = (num < 0 ) ? -1 : 1;
    num = num * sign;
    while ( num >= 10 ) {
      num = num / 10;
      exp++;
    }
    while ( num < 1 ) {
      num = num * 10;
      exp--;
    }
    if ( num > 10 ) {
      num = 1;
      exp++;
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
  decimals = -1;
  num_max_len = 0;
  exp_max_len = 0;

  if ( number_format == Magnitude ) return;

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
        if ( v > max * (1 + cre) ) continue;
        if ( v < min * (1 - cre) ) continue;
        v_list.push_back( v );
      }
    }
  } else {
    const double e = (max - min) * cre; // To account to rounding errors.
    int32_t mn_min = std::floor( (min - major) / major );
    int32_t mn_max = std::ceil( (max + major) / major );
    for ( int32_t mn = mn_min; mn <= mn_max; mn++ ) {
      for ( int32_t sn = 0; sn < sub_divs; sn++ ) {
        if ( sn > 0 && !show_minor_mumbers ) break;
        double p = mn * major + sn * major / sub_divs;
        if ( p < min-e ) continue;
        if ( p > max+e ) continue;
        v_list.push_back( p );
      }
    }
  }

  if ( number_format == Fixed ) {
    for ( double v : v_list ) {
      ComputeDecimals( v );
    }
  }

  if ( number_format == Scientific ) {
    for ( double v : v_list ) {
      int32_t exp = NormalizeExponent( v );
      ComputeDecimals( v );
      std::ostringstream oss;
      oss << exp;
      if ( exp_max_len < oss.str().length() ) exp_max_len = oss.str().length();
    }
  }

  num_max_len = digits + decimals;
  if ( decimals > 0 ) num_max_len++;

  if ( angle == 0 ) {
    if ( number_format != Fixed ) decimals = -1;
    num_max_len = 0;
    exp_max_len = 0;
  } else {
    if ( number_pos == Left  ) num_max_len = 0;
    if ( number_pos == Right ) exp_max_len = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////

std::string Axis::NumToStr( double v )
{
  const double lim = std::pow( double( 10 ), -precision ) / 2;
  if ( v > -lim && v < lim ) v = 0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision( precision ) << v;
  bool dp_seen = false;
  bool nz_seen = false;
  int i = 0;
  std::string s;
  for ( const char c : oss.str() ) {
    if ( c == '.' ) {
      dp_seen = true;
    } else {
      if ( dp_seen ) {
        if ( (v == 0 || nz_seen) && c == '0' && i >= decimals ) break;
        if ( i == 0 ) s += '.';
        s += c;
        i++;
      } else {
        s += c;
      }
      nz_seen = nz_seen || c != '0';
    }
  }
  return s;
}

///////////////////////////////////////////////////////////////////////////////

SVG::Object* Axis::BuildNum( SVG::Group* g, double v, bool bold )
{
  if ( std::abs( v ) < num_lo ) v = 0;
  double num = v;
  int32_t exp = NormalizeExponent( num );

  NumberFormat number_format = this->number_format;
  if ( number_format == Magnitude && (exp < -30 || exp > 30) ) {
    number_format = Scientific;
  }

  Object* obj = NULL;

  std::string s = NumToStr( num );

  if ( number_format == Magnitude ) {
    const char sym[] = "qryzafpn\xE6m kMGTPEZYRQ";
    exp = exp / 3;
    if ( exp != 0 ) s += sym[ exp + 10 ];
    obj = Label( g, s );
    if ( bold ) obj->Attr()->TextFont()->SetBold();
    return obj;
  }

  if ( angle == 90 && num_max_len > s.length() ) {
    s.insert( 0, num_max_len - s.length(), ' ' );
  }

  if ( number_format == Fixed ) {
    obj = Label( g, s );
    if ( bold ) obj->Attr()->TextFont()->SetBold();
    return obj;
  }

  g = g->AddNewGroup();
  BoundaryBox bb;
  if ( angle == 0 && num == 0 ) {
    obj = Label( g, s );
  } else
  if ( angle == 0 && num == 1 ) {
    obj = Label( g, "10" );
  } else {
    obj = Label( g, s );
    bb = obj->GetBB();
    U cr = (bb.max.y - bb.min.y) * 0.15;
    U cx = bb.max.x + cr;
    U cy = (bb.max.y - bb.min.y) * 0.45 + cr * 0.5;
    obj = Label( g, (num == 0) ? "  " : "10" );
    obj->MoveTo( MinX, MinY, cx + cr, bb.min.y );
    if ( num == 0 ) {
      obj->Attr()->FillColor()->Clear();
    } else {
      g = g->AddNewGroup();
      g->Attr()->SetLineWidth( cr * (bold ? 0.75 : 0.5))->LineColor()->Set( Black );
      g->Add( new Line( cx - cr, cy - cr, cx + cr, cy + cr ) );
      g->Add( new Line( cx - cr, cy + cr, cx + cr, cy - cr ) );
      g = g->ParrentGroup();
      g->Add( new Rect( cx - cr*2, bb.min.y, cx + cr*2, bb.max.y ) );
      g->LastToBack();
    }
  }
  bb = obj->GetBB();
  if ( angle != 0 || num != 0 ) {
    if ( num == 0 ) {
      s = "";
    } else {
      std::ostringstream oss;
      oss << exp;
      s = oss.str();
    }
    if ( angle == 90 && exp_max_len > s.length() ) {
      s.insert( s.length(), exp_max_len - s.length(), ' ' );
    }
    U h = bb.max.y - bb.min.y;
    obj = g->Add( new Text( 0, 0, s ) );
    obj->Attr()->TextFont()->SetSize( h * 0.9 );
    obj->MoveTo( MinX, MaxY, bb.max.x - h * 0.05, bb.max.y + h * 0.4 );
    bb = obj->GetBB();
    g->Add( new Rect( bb.min.x - h * 0.12, bb.min.y, bb.max.x + h * 0.12, bb.max.y ) );
    g->Last()->Attr()->LineColor()->Clear();
    if ( num == 0 ) {
      g->Last()->Attr()->FillColor()->Clear();
    }
    g->LastToBack();
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
  if ( major <= 0 ) return;

  std::vector< int32_t > mn_list;
  {
    std::set< int32_t > mn_set;
    int32_t mn_min = std::floor( (min - major) / major );
    int32_t mn_max = std::ceil( (max + major) / major );
    auto add = [&]( int32_t mn )
    {
      if ( mn >= mn_min && mn <= mn_max && mn_set.find( mn ) == mn_set.end() ) {
        mn_list.push_back( mn );
        mn_set.insert( mn );
      }
    };
    int32_t step = mn_max - mn_min;
    while ( step & (step - 1) ) step++;
    while ( step > 0 ) {
      for ( int32_t mn = (mn_min / step) * step; mn <= mn_max; mn += step ) {
        add( mn );
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
    int32_t step = sub_divs;
    while ( step > 0 ) {
      for ( int32_t sn = 0; sn < sub_divs; sn += step ) {
        add( sn );
      }
      do step--; while ( step > 0 && sub_divs % step );
    }
  }

  std::vector< SVG::Object* > num_objects;

  const double e = (max - min) * cre; // To account to rounding errors.
  for ( int32_t sn : sn_list ) {
    for ( int32_t mn : mn_list ) {
      double p = mn * major + sn * major / sub_divs;
      if ( p < min-e ) continue;
      if ( p > max+e ) continue;
      bool near_crossing_axis =
        p > orth_axis_cross-e &&
        p < orth_axis_cross+e;
      U q = Coor( p );
      U x = (angle == 0) ? q : sx;
      U y = (angle == 0) ? sy : q;
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
            if ( p > -e && p < +e ) {
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
        Object* obj = BuildNum( num_g, p, sn == 0 );
        if ( angle == 0 ) {
          if ( number_pos == Above ) {
            obj->MoveTo( MidX, MinY, x, y + d + 2 );
          } else {
            obj->MoveTo( MidX, MaxY, x, y - d - 2 );
          }
        } else {
          if ( number_pos == Right ) {
            obj->MoveTo( MinX, MidY, x + d + 2, y );
          } else {
            obj->MoveTo( MaxX, MidY, x - d - 2, y );
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
    add( 1 );
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

  for ( int32_t sn : sn_list ) {
    if ( sn >= sub_divs ) continue;
    for ( int32_t pow_cur : pow_list ) {
      double m0 = std::pow( double( 10 ), pow_cur );
      double m1 = std::pow( double( 10 ), pow_cur + pow_inc );
      double v = m1 * sn / sub_divs;
      if ( sn == 0 ) v = m0;
      if ( v > max * (1 + cre) ) continue;
      if ( v < min * (1 - cre) ) continue;
      if ( sn > 0 && v < m0 * (1 + cre) ) continue;
      bool near_crossing_axis =
        v > orth_axis_cross * (1 - cre) &&
        v < orth_axis_cross * (1 + cre);
      U q = Coor( v );
      U x = (angle == 0) ? q : sx;
      U y = (angle == 0) ? sy : q;
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
            if ( v > (1 - cre) && v < (1 + cre) ) {
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
            obj->MoveTo( MidX, MinY, x, y + d + 2 );
          } else {
            obj->MoveTo( MidX, MaxY, x, y - d - 2 );
          }
        } else {
          if ( number_pos == Right ) {
            obj->MoveTo( MinX, MidY, x + d + 2, y );
          } else {
            obj->MoveTo( MaxX, MidY, x - d - 2, y );
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
  Axis& orth_axis,
  std::vector< SVG::Object* >& axes_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g, SVG::Group* label_g
)
{
  line_g = line_g->AddNewGroup();
  num_g  = num_g->AddNewGroup();

  U start = (orth_axis_cross == min) ? -tick_major_len : 0;
  U ap = orth_axis.Coor( orth_axis.orth_axis_cross );
  U sx = (angle == 0) ? start : ap;
  U sy = (angle == 0) ? ap : start;
  U ex = (angle == 0) ? (length + overhang) : +sx;
  U ey = (angle == 0) ? +sy : (length + overhang);

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

  Pos unit_pos = this->unit_pos;
  if ( angle == 0 ) {
    if ( unit_pos != Below && unit_pos != Above && unit_pos != Right ) {
      unit_pos = number_pos;
    }
  } else {
    if ( unit_pos != Left && unit_pos != Right && unit_pos != Above ) {
      unit_pos = Above;
    }
  }

  if ( unit != "" ) {
    Object* obj = Label( label_g, unit, 16 );
    obj->Attr()->TextFont()->SetBold();
    if ( angle == 0 ) {
      if ( unit_pos == Above ) {
        obj->MoveTo( MaxX, MinY, ex, ey + tick_major_len + 2 );
      } else
      if ( unit_pos == Right ) {
        obj->MoveTo( MinX, MidY, ex + 2, ey );
      } else {
        obj->MoveTo( MaxX, MaxY, ex, ey - tick_major_len - 2 );
      }
      if (
        (obj->GetBB().min.x - 50) < Coor( orth_axis_cross )
        && orth_axis.orth_axis_cross > orth_axis.min
        && this->unit_pos == Auto
      ) {
        // Move to right if too close to Y-axis.
        obj->MoveTo( MinX, MidY, ex + 2, ey );
      }
    } else {
      if ( unit_pos == Right ) {
        obj->MoveTo( MinX, MaxY, ex + tick_major_len + 2, ey );
      } else
      if ( unit_pos == Above ) {
        obj->MoveTo( MidX, MinY, ex, ey + 2 );
      } else {
        obj->MoveTo( MaxX, MaxY, ex - tick_major_len - 2, ey );
      }
    }
    axes_objects.push_back( obj );
  }

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
    Object* obj = Label( label_g, label, 24 );
    obj->Rotate( angle );
    if ( angle == 0 ) {
      U x = length / 2;
      U y = 0;
      if ( y > b1.min.y ) y = b1.min.y;
      if ( y > b2.min.y ) y = b2.min.y;
      obj->MoveTo( MidX, MaxY, x, y - 4 );
    } else {
      U y = length / 2;
      if (
        (orth_axis.orth_axis_cross - orth_axis.min) /
        (orth_axis.max - orth_axis.min) > (1 - near)
      ) {
        U x = orth_axis.length;
        if ( x < b1.max.x ) x = b1.max.x;
        if ( x < b2.max.x ) x = b2.max.x;
        obj->MoveTo( MinX, MidY, x + 4, y );
      } else {
        U x = 0;
        if ( x > b1.min.x ) x = b1.min.x;
        if ( x > b2.min.x ) x = b2.min.x;
        obj->MoveTo( MaxX, MidY, x - 4, y );
      }
    }
    axes_objects.push_back( obj );
  }

  axes_objects.push_back( line_g );
  axes_objects.push_back( num_g );
}

///////////////////////////////////////////////////////////////////////////////
