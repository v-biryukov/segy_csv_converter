Segy_Csv_converter <br />
Version: 1.0 <br />
Author: Biryukov V. <br />

Simple SEG Y to CSV converter and vice versa <br />
Usage: segy_converter [OPTIONS] <br />
Option                  |  Description                                       | Default <br />
-c, --convertion          tosegy or tocsv                                      tosegy <br />
-d, --dims                number of dimmensions: 2 or 3                        2 <br />
-s, --segyfile            .segy file (without _x.segy at the end)              segy_file <br />
-f, --csvfile             .csv file (without .csv extension at the end)        csv_file <br />
-i, --interpolation_coef  time interpolation coefficient                       1.0 <br />
-h, --help                print this help and exit <br />


Example: segy_converter --segyfile seismo --csvfile input <br />

