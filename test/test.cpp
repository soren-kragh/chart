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
  {
    std::ostringstream oss;
    oss << "Footnote" << '\n';
    oss << "More" << '\n';
    chart.SetFootnote( oss.str() );
  }

//  chart.AxisX()->SetLogScale();
//  chart.AxisX()->SetNumberFormat( Chart::Plain );
//  chart.AxisX()->SetLabel( "Awesomeness" );
//  chart.AxisX()->SetUnit( "monsters\nper hour" );
  chart.AxisX()->SetRange( -2, 2 );
  chart.AxisX()->SetTick( 0.1, 0 );
//  chart.AxisX()->SetGrid( true, true );
  chart.AxisX()->SetNumberPos( Chart::Above );
//  chart.AxisX()->SetUnitPos( Chart::Above );

//  chart.AxisY()->SetLogScale();
  chart.AxisY()->ShowMinorNumbers();
//  chart.AxisY()->SetNumberFormat( Chart::Scientific );
//  chart.AxisY()->SetLabel( "Scariness" );
//  chart.AxisY()->SetUnit( "Panic/s" );
  chart.AxisY()->SetRange( -1, 7 );
  chart.AxisY()->SetTick( 1.0, 9 );
//  chart.AxisY()->SetGrid( true, true );
  chart.AxisY()->SetNumberPos( Chart::Right );
//  chart.AxisY()->SetUnitPos( Chart::Above );

//  for ( int i = 0; i < 12; i++ ) {
//    std::ostringstream oss;
//    oss << "Series " << static_cast<char>('A' + i );
//    Chart::Series* series = chart.AddSeries( oss.str() );
//    for ( int p = 0; p <= 100; p++ ) {
//      double x = 1.1214e-10 * p;
//      double y = std::cos( p/10.0 + i / 5.0 ) + 1.4;
//      series->Add( x + 33, 0.5e+12 + y * 1e+12 );
//    }
//  }

//  chart.SetLegendPos( Chart::Auto );
  chart.SetChartArea( 1600, 800 );

  SVG::Canvas* canvas = chart.Build();
  canvas->Background()->Set( SVG::Black, 0.5 );
  std::cout << canvas->GenSVG( 2 );
  delete canvas;

  return 0;
}
