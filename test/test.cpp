///////////////////////////////////////////////////////////////////////////////

#include <chart_main.h>
#include <cfenv>

///////////////////////////////////////////////////////////////////////////////

int main()
{
  feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW );
//  feenableexcept( FE_ALL_EXCEPT );
//  double x = 10;
//  while ( x > 0 ) {
//    SVG_DBG( ">>>" << (x / (x - 1)) );
//    x--;
//  }

  Chart::Main chart;

  chart.SetTitle( "Title" );

  chart.SetSubTitle( "Sub Title" );
//  {
//    std::ostringstream oss;
//    oss << "ComputeFocusPoints( f1, f2, a );" << '\n';
//    oss << "f1 = TransformPoint( transforms, f1 );" << '\n';
//    oss << "f2 = TransformPoint( transforms, f2 );" << '\n';
//    chart.SetSubSubTitle( oss.str() );
//  }
  {
    std::ostringstream oss;
    oss << "Footnote" << '\n';
    oss << "More" << '\n';
    chart.SetFootnote( oss.str() );
  }

  chart.AxisX()->SetLabel( "Awesomeness" );
  chart.AxisX()->SetUnit( "monsters\nper hour" );
//  chart.AxisX()->SetRange( -20, 80, 50 );
//  chart.AxisX()->SetTick( 2.0, 2 );
  chart.AxisX()->SetGrid( true, true );
  chart.AxisX()->SetNumberPos( Chart::Below );
  chart.AxisX()->SetUnitPos( Chart::Above );

  chart.AxisY()->SetLabel( "Scariness" );
  chart.AxisY()->SetUnit( "Panic/s" );
//  chart.AxisY()->SetRange( -1.3, 1.9, -999 );
//  chart.AxisY()->SetTick( 0.11, 2 );
  chart.AxisY()->SetGrid( true, true );
  chart.AxisY()->SetNumberPos( Chart::Left );
//  chart.AxisY()->SetUnitPos( Chart::Above );

  for ( int i = 0; i < 12; i++ ) {
    std::ostringstream oss;
    oss << "Series " << static_cast<char>('A' + i );
//    oss << "nt200 initial" << '\n';
//    oss << "(4:256k:2)";
    if ( i % 2 ) {
      oss = std::ostringstream();
    }
    Chart::Series* series = chart.AddSeries( oss.str() );
    for ( int p = 0; p <= 100; p++ ) {
      double x = 1.1214e-10 * p;
      double y = std::cos( p/10.0 + i / 5.0 ) + 1.4;
      series->Add( x - 33, 0.5e+12 + y * 1e+12 );
    }
  }

  chart.SetLegendPos( Chart::Auto );
  chart.SetChartArea( 1000, 600 );


  SVG::Canvas* canvas = chart.Build();
  canvas->Background()->Set( SVG::Black, 0.5 );
  std::cout << canvas->GenSVG( 2 );
  delete canvas;

  return 0;
}
