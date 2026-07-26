#include <headers/helpers.hpp>
#include <iostream>
using namespace std;

int error(const string &msg)
{
    cerr << "Error: " << msg << endl;
    return EXIT_FAILURE;
}