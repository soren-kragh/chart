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

using namespace SVG;
using namespace Chart;

///////////////////////////////////////////////////////////////////////////////

Axis::Axis( void )
{
  min = 0;
  max = 0;
  orth_axis_cross = 0;
  major = 0;
  sub_divs = 0;
  number_pos = Auto;
  major_grid_enable = true;
  minor_grid_enable = false;

  unit_pos = Auto;
}

///////////////////////////////////////////////////////////////////////////////

void Axis::SetRange( double min, double max, double orth_axis_cross )
{
  this->min = min;
  this->max = max;
  if ( orth_axis_cross < min ) orth_axis_cross = min;
  if ( orth_axis_cross > max ) orth_axis_cross = max;
  this->orth_axis_cross = orth_axis_cross;
}

void Axis::SetTick( double major, int sub_divs )
{
  this->major = major;
  this->sub_divs = (sub_divs > 0) ? sub_divs : 1;
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
  if ( (max - min) < 1e-15 || (max - min) > 1e+15 ) {
    major = 0;
    return;
  }
  if ( major > 0 ) {
    if ( ((max - min) / major) * sub_divs > 1000 ) {
      major = 0;
    }
  }
  if ( major <= 0 ) {
    uint64_t m = 1;
    uint64_t d = 1;
    bool f = false;
    while ( 1 ) {
      double q = (max - min) / (1.0 * m / d);
      if ( !f && q <= 10 ) d = d * 2; else
      if (  f && q <=  4 ) d = d * 5; else
      break;
      f = !f;
    }
    f = false;
    while ( d == 1 ) {
      double q = (max - min) / (1.0 * m / d);
      if ( !f && q > 20 ) m = m * 2; else
      if (  f && q > 20 ) m = m * 5; else
      break;
      f = !f;
    }
    major = 1.0 * m / d;
    sub_divs = 2;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Axis::Build(
  int angle, const Axis& orth_axis,
  std::vector< SVG::Object* >& axes_objects,
  SVG::Group* minor_g, SVG::Group* major_g, SVG::Group* zero_g,
  SVG::Group* line_g, SVG::Group* num_g, SVG::Group* label_g
)
{
  line_g = line_g->AddNewGroup();
  num_g  = num_g->AddNewGroup();

  U start = (orth_axis_cross == min) ? -tick_major_len : 0;
  U s =
    (orth_axis.orth_axis_cross - orth_axis.min) * orth_axis.length /
    (orth_axis.max - orth_axis.min);
  U sx = (angle == 0) ? start : s;
  U sy = (angle == 0) ? s : start;
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

  Pos number_pos = this->number_pos;
  if ( angle == 0 ) {
    if ( number_pos != Below && number_pos != Above ) {
      number_pos =
        ( (orth_axis.max - orth_axis.orth_axis_cross) <
          (orth_axis.max - orth_axis.min) * near
        )
        ? Above
        : Below;
    }
  } else {
    if ( number_pos != Left && number_pos != Right ) {
      number_pos =
        ( (orth_axis.max - orth_axis.orth_axis_cross) <
          (orth_axis.max - orth_axis.min) * near
        )
        ? Right
        : Left;
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
        (obj->GetBB().min.x - 50) <
        ((orth_axis_cross - min) * length / (max - min))
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
    U oc = (orth_axis_cross - min) * length / (max - min);
    U zc = 2 * tick_major_len;
    if ( angle == 0 ) {
      axes_objects.push_back(
        new Rect( oc - zc, 0, oc + zc, orth_axis.length )
      );
    } else {
      axes_objects.push_back(
        new Rect( 0, oc - zc, orth_axis.length, oc + zc )
      );
    }
  }

  int decimals = 0;
  if ( std::floor( major ) != major ) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision( 20 ) << major;
    int state = 0;
    int i = 0;
    for ( char c : oss.str() ) {
      switch ( state ) {
        case 1 : {
          i++;
          if ( c != '0' ) state = 2;
          break;
        }
        case 2 : {
          if ( c == '0' ) decimals = i;
          i++;
          break;
        }
        default : {
          if ( c == '.' ) state = 1;
        }
      }
      if ( decimals > 0 ) break;
    }
  }

  if ( major > 0 ) {
    int64_t mn = std::floor( (min - major) / major );
    int sn = 0;
    double e = major / sub_divs / 1000; // To account to rounding errors.
    Object* prev_num = NULL;
    Object* this_num = NULL;
    while ( 1 ) {
      double p = mn * major + sn * major / ((sub_divs > 0) ? sub_divs : 1);
      if ( p >= max+e ) break;
      bool near_crossing_axis =
        p > orth_axis_cross-e &&
        p < orth_axis_cross+e;
      if ( p >= min-e ) {
        U q = (p - min) * length / (max - min);
        U x = (angle == 0) ? q : sx;
        U y = (angle == 0) ? sy : q;
        U d = (sn == 0) ? tick_major_len : tick_minor_len;
        if ( !near_crossing_axis ) {
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
          sn == 0 &&
          ( !near_crossing_axis ||
            ( orth_axis.orth_axis_cross == orth_axis.min &&
              ((angle == 0) ? (number_pos == Below) : (number_pos == Left))
            )
          )
        )
        {
          std::ostringstream oss;
          oss << std::fixed << std::setprecision( decimals ) << p;
          this_num = Label( num_g, oss.str() );
          if ( angle == 0 ) {
            if ( number_pos == Above ) {
              this_num->MoveTo( MidX, MinY, x, y + d + 2 );
            } else {
              this_num->MoveTo( MidX, MaxY, x, y - d - 2 );
            }
          } else {
            if ( number_pos == Right ) {
              this_num->MoveTo( MinX, MidY, x + d + 2, y );
            } else {
              this_num->MoveTo( MaxX, MidY, x - d - 2, y );
            }
          }
          if (
            Chart::Collides( this_num, axes_objects, 0, 0 ) ||
            SVG::Collides( this_num, prev_num )
          ) {
            num_g->DeleteFront();
          } else {
            prev_num = this_num;
          }
        }
      }
      if ( sub_divs > 0 ) {
        sn = (sn + 1) % sub_divs;
      }
      if ( sn == 0 ) {
        mn++;
      }
    }
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
