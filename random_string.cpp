#include <iostream>
#include <fstream>
#include <random>

using namespace std;

int main()
{
    fstream output("target_new.txt", fstream::app);

    random_device device;
    minstd_rand seed(device());
    uniform_int_distribution<minstd_rand::result_type> rand_char(0, 25);

    for (unsigned long long i = 0; i < 1 << 16; i++)
        output << char(rand_char(seed) + 'a');

    output.close();

    return 0;
}
