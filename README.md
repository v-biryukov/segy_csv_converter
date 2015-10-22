Segy_Csv_converter
Version: 1.0
Author: Biryukov V.

Simple SEG Y to CSV converter and vice versa
Usage: segy_converter [OPTIONS]
Option                  |  Description                                       | Default
-c, --convertion          tosegy or tocsv                                      tosegy
-d, --dims                number of dimmensions: 2 or 3                        2
-s, --segyfile            .segy file (without _x.segy at the end)              segy_file
-f, --csvfile             .csv file (without .csv extension at the end)        csv_file
-i, --interpolation_coef  time interpolation coefficient                       1.0
-h, --help                print this help and exit


Example: segy_converter --segyfile seismo --csvfile input

