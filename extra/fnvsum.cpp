#include "../src/common/mb_hash.h"

#include <iostream>
#include <iomanip>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cerr << "Missing file name" << endl;
        return EXIT_FAILURE;
    }

    for(int i = 1; i < argc; i++)
    {
        auto [hash, success] = mb::file_hash(argv[i]);

        if(success)
        {
            cout << hex << setw(6) << setfill('0') << hash << "    " << argv[i] << "\n";
        }
    }

    return EXIT_SUCCESS;
}
