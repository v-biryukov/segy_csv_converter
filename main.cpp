#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "seismogram.h"

#define MAX_NAME_LENGTH 200

using namespace std;


int main(int argc, char ** argv)
{
    char * convertion = "tosegy";
    int dims = 2;
    char csv_file[MAX_NAME_LENGTH] = "csv_file";
    char segy_file[MAX_NAME_LENGTH] = "segy_file";
    float interpolation_coef = 1.0;

    int c;
    opterr = 0;

    const char    * short_opt = "hc:d:f:s:i:";
    struct option   long_opt[] =
    {
        {"help",          no_argument,       NULL, 'h'},
        {"convertion",    required_argument, NULL, 'c'},
        {"dims",          required_argument, NULL, 'd'},
        {"csvfile",       required_argument, NULL, 'f'},
        {"segyfile",      required_argument, NULL, 's'},
        {"interpolation_coef",      required_argument, NULL, 'i'},
        {NULL,            0,                 NULL, 0  }
    };

    while((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch(c)
        {
            case -1:       /* no more arguments */
            case 0:        /* long options toggles */
            break;

            case 'c':
            convertion = optarg;
            printf("you entered \"%s\"\n", optarg);
            break;

            case 'd':
            dims = ::atoi(optarg);
            printf("you entered \"%s\"\n", optarg);
            break;

            case 'f':
            strcpy(csv_file, optarg);
            //csv_file = optarg;
            printf("you entered \"%s\"\n", optarg);
            break;

            case 's':
            strcpy(segy_file, optarg);
            //segy_file = optarg;
            printf("you entered \"%s\"\n", optarg);
            break;

            case 'i':
            interpolation_coef = ::atof(optarg);
            printf("you entered \"%s\"\n", optarg);
            break;

            case 'h':
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("  Option                  |  Description                                       | Default \n");
            printf("  -c, --convertion          \"tosegy\" or \"tocsv\"                                  tosegy\n");
            printf("  -d, --dims                number of dimmensions: 2 or 3                        2\n");
            printf("  -s, --segyfile            .segy file (without _x.segy at the end)              segy_file\n");
            printf("  -f, --csvfile             .csv file (without .csv extension at the end)        csv_file\n");
            printf("  -i, --interpolation_coef  time interpolation coefficient                       1.0\n");
            printf("  -h, --help                print this help and exit\n");
            printf("Example: %s --segyfile seismo --csvfile input\n", argv[0]);
            printf("\n");
            return(0);

            case ':':
            case '?':
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);

            default:
            fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);
        };
    };
    if (dims != 2 && dims != 3)
    {
        fprintf(stderr, "Invalid value for option dims (should be equal to 2 or 3, but equal to %d)\n", dims);
        fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
        return(-2);
    }
    if (strcmp(convertion,"tosegy") != 0 && strcmp(convertion,"tocsv") != 0 )
    {
        fprintf(stderr, "Invalid value for option convertion (should be equal to \"tosegy\" or \"tocsv\", but equal to %s)\n", convertion);
        fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
        return(-2);
    }

    if (dims == 2)
    {
        char * segy_file_x = strdup(segy_file);
        strcat(segy_file_x, "_x.segy");
        char * segy_file_y = strdup(segy_file);
        strcat(segy_file_y, "_y.segy");
        typedef CombinedSeismogramm<float, 2>::Elastic SeismoElastic;
        CombinedSeismogramm < float, 2 > s = CombinedSeismogramm < float, 2 >(interpolation_coef);
        s.AddComponent(segy_file_x, new VxGetter<SeismoElastic, 2>());
        s.AddComponent(segy_file_y, new VyGetter<SeismoElastic, 2>());

        std::vector<std::string> csv_files;
        csv_files.push_back(csv_file);


        std::vector<std::string> segy_files;
        segy_files.push_back(segy_file_x);
        segy_files.push_back(segy_file_y);

        if (!strcmp(convertion,"tosegy"))
        {
            s.Load(CSV, csv_files);
            s.Save(SEG_Y, segy_files);
        }
        else if (!strcmp(convertion,"tocsv"))
        {
            s.Load(SEG_Y, segy_files);
            s.Save(CSV, csv_files);
        }
        free(segy_file_x);
        free(segy_file_y);
    }
    else if (dims == 3)
    {
        char * segy_file_x = strdup(segy_file);
        strcat(segy_file_x, "_x.segy");
        char * segy_file_y = strdup(segy_file);
        strcat(segy_file_y, "_y.segy");
        char * segy_file_z = strdup(segy_file);
        strcat(segy_file_z, "_z.segy");
        typedef CombinedSeismogramm<float, 3>::Elastic SeismoElastic;
        CombinedSeismogramm < float, 3 > s = CombinedSeismogramm < float, 3 >(interpolation_coef);
        s.AddComponent(segy_file_x, new VxGetter<SeismoElastic, 3>());
        s.AddComponent(segy_file_y, new VyGetter<SeismoElastic, 3>());
        s.AddComponent(segy_file_z, new VzGetter<SeismoElastic, 3>());

        std::vector<std::string> csv_files;
        csv_files.push_back(csv_file);


        std::vector<std::string> segy_files;
        segy_files.push_back(segy_file_x);
        segy_files.push_back(segy_file_y);
        segy_files.push_back(segy_file_z);

        if (!strcmp(convertion,"tosegy"))
        {
            s.Load(CSV, csv_files);
            s.Save(SEG_Y, segy_files);
        }
        else if (!strcmp(convertion,"tocsv"))
        {
            s.Load(SEG_Y, segy_files);
            s.Save(CSV, csv_files);
        }
        free(segy_file_x);
        free(segy_file_y);
        free(segy_file_z);
    }
    return 0;
}



//#include <iostream>
//#include <algorithm>
//#include "seismogram.h"

//using namespace std;

//int main(int argc, char ** argv)
//{
//    typedef CombinedSeismogramm<float, 2>::Elastic SeismoElastic;
//    CombinedSeismogramm < float, 2 > s = CombinedSeismogramm < float, 2 >(1);
//    s.AddComponent("seismo_vx.segy", new VxGetter<SeismoElastic, 2>());
//    s.AddComponent("seismo_vy.segy", new VyGetter<SeismoElastic, 2>());

//    std::vector<std::string> in_files;
//    in_files.push_back("seismo_in_x.segy");
//    in_files.push_back("seismo_in_y.segy");


//    std::vector<std::string> out_files;
//    out_files.push_back("seismo_out_x.segy");
//    out_files.push_back("seismo_out_y.segy");

//    std::vector<std::string> out_files2;
//    out_files2.push_back("seismo_out2_x.segy");
//    out_files2.push_back("seismo_out2_y.segy");


//    s.Load(SEG_Y, in_files);
//    s.Save(SEG_Y, out_files);
//    s.Save(SEG_Y, out_files2);

////    s.Load(SEG_Y, out_files);
////    s.Save(CSV, in_files);


//    return 0;
//}
