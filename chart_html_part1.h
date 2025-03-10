oss << R"EOF(

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Interactive SVG</title>
  <style>
    text {
      font-family: "Noto Mono", "Lucida Console", "Courier New", monospace;
      font-size: 12;
    }
    html, body {
      margin: 0;
      padding: 0;
    }
    .hide-cursor {
      cursor: none;
    }
    #svgChart, #svgCursor, #svgSnap {
      position: absolute;
      top: 0;
      left: 0;
      overflow: visible;
    }
    #svgChart {
      transform: translateZ(0);
    }
  </style>
</head>
<body>

)EOF";
