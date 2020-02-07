#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>

// Parses application arguments for example.
void parse_arguments(
        int argc,
        char *argv[],
        unsigned int *domain_id,
        unsigned int *sample_count, 
        unsigned int *verbosity)
{
    int arg_processing = 1;
    bool show_usage_and_exit = false;

    while (arg_processing < argc) {
        if (strcmp(argv[arg_processing], "-d") == 0
            || strcmp(argv[arg_processing], "--domain") == 0) {
            *domain_id = atoi(argv[arg_processing + 1]);
            arg_processing += 2;
            continue;
        }
        if (strcmp(argv[arg_processing], "-s") == 0
            || strcmp(argv[arg_processing], "--sample_count") == 0) {
            *sample_count = atoi(argv[arg_processing + 1]);
            arg_processing += 2;
            continue;
        }
        if (strcmp(argv[arg_processing], "-v") == 0
            || strcmp(argv[arg_processing], "--verbosity") == 0) {
            *verbosity = atoi(argv[arg_processing + 1]);
            arg_processing += 2;
            continue;
        }
        if (strcmp(argv[arg_processing], "-h") == 0
            || strcmp(argv[arg_processing], "--help") == 0) {
            std::cout << "Example application." << std::endl;
            show_usage_and_exit = true;
            break;
        } else {
            std::cout << "Bad parameter." << std::endl;
            show_usage_and_exit = true;
            break;
        }
    }
    if (show_usage_and_exit) {
        std::cout << "Usage:\n"\
                     "    -d, --domain       <int>   Domain ID this application will\n" \
                     "                               subscribe in.  \n"
                     "                               Default: 0\n"\
                     "    -s, --sample_count <int>   Number of samples to receive before\n"\
                     "                               cleanly shutting down. \n"
                     "                               Default: infinite\n"
                     "    -v, --verbosity    <int>   How much debugging output to show.\n"\
                     "                               Range: 0-5 \n"
                     "                               Default: 0"
                << std::endl;
        exit(0);
    }
}



#endif