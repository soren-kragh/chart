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

#pragma once

#include <svg_canvas.h>
#include <chart_common.h>
#include <chart_datum.h>

namespace Chart {

class Series;

class Tag
{
  friend class Axis;
  friend class Series;

public:

  Tag( void );
  ~Tag( void );

private:

  std::vector< SVG::BoundaryBox > recorded_tags;

  // Records and checks tag for collision detection.
  void RecordTag( const SVG::BoundaryBox& bb );
  bool Collision( const SVG::BoundaryBox& bb );

  // Used for line and point type series. The connected argument indicates if
  // this point is connected to the previous point. A tag will only be added
  // for valid datum.
  void LineTag(
    Series* series, SVG::Group* tag_g,
    SVG::Point p, const Datum& datum, bool datum_valid,
    bool connected, Pos direction
  );

  // Indicate that no more line or point data points for the current series
  // follow.
  void EndLineTag( void );

  // Used for bar-type tagging; p_base is the coordinate of the start of the bar
  // and p is the coordinate of the end of the bar. The direction indicates
  // which way the bar is pointing, which could normally be deduced from
  // p_base/p except in the case where the bar size is zero.
  void BarTag(
    Series* series, SVG::Group* tag_g,
    SVG::Point p1, SVG::Point p2, const Datum& datum,
    Pos direction
  );

  struct {
    bool valid;
    Series* series;
    SVG::Group* tag_g;
    SVG::Point p;
    Datum datum;
    bool datum_valid;
    int dir_bst;
    int dir_prv;
    int dir_nxt;
  } tag;

  const SVG::U min_base_dist = 2.0;

  SVG::Group* BuildTag(
    Series* series, SVG::Group* tag_g, const Datum& datum, SVG::U& r
  );

  // Assuming bar type series, returns how much beyond the bar end the tag
  // takes up.
  SVG::U GetBeyond( Series* series, SVG::Group* tag_g );

  // The direction indicates the preferred direction in which to place the tag.
  SVG::Group* AddLineTag( void );

  SVG::Group* AddBarTag(
    Series* series, SVG::Group* tag_g,
    SVG::Point p1, SVG::Point p2, const Datum& datum,
    Pos direction
  );

  // Get the direction of the vector in values from 0 to 7, where 0 is "east";
  // returns -1 if the direction is undefined (x and y are both zero).
  int Direction( double x, double y );

  // Checks if dir1 is equal to any direction in the range [dir2;dir2+ofs]; the
  // direction is in the range for 0 to 7; ofs is in the range from -7 to 7.
  bool DirCmp( int dir1, int dir2, int ofs );

};

}
