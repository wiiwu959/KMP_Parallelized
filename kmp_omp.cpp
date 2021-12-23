#include <ctime>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <omp.h>

#define CORE_NUM 4

using namespace std;

void preKMP(char *pattern, int failure[]);
void KMP(char *target, char *pattern, int *failure, int *answer, int pattern_length, int target_length, int part_start);

int main(int argc, char **argv)
{
    fstream target_file("target.txt"), pattern_file("pattern.txt");
    stringstream target_stream, pattern_stream;

    target_stream << target_file.rdbuf();
    string target_string = target_stream.str();

    pattern_stream << pattern_file.rdbuf();
    string pattern_string = pattern_stream.str();

    int target_length = target_string.length();
    int pattern_length = pattern_string.length();

    char *target = new char[target_length + 1];
    char *pattern = new char[pattern_length + 1];

    strcpy(target, target_string.c_str());
    strcpy(pattern, pattern_string.c_str());

    int *failure = new int[pattern_length];
    int *answer = new int[target_length]();

    preKMP(pattern, failure);

    struct timespec start, end;
    double elapsed_time;

    cout << "----- This is OpenMP results using KMP Algorithm. -----" << endl;

    clock_gettime(CLOCK_MONOTONIC, &start);
    int first_partial = target_length / CORE_NUM;
    // first round
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < CORE_NUM; i++)
        {
            int part_start = i * first_partial;
            KMP(target, pattern, failure, answer, pattern_length, first_partial, part_start);
        }

        // second round
        int second_partial = (pattern_length - 1) * 2;
        #pragma omp for
        for (int i = 0; i < CORE_NUM - 1; i++)
        {
            int part_start = (i + 1) * first_partial - (pattern_length - 1);
            KMP(target, pattern, failure, answer, pattern_length, second_partial, part_start);
        }
    }

    // last part
    int last_partial = (target_length % CORE_NUM) + pattern_length - 1;
    if(last_partial != 0)
    {
        int part_start = CORE_NUM * first_partial - (pattern_length - 1);
        KMP(target, pattern, failure, answer, pattern_length, last_partial, part_start);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time = (end.tv_sec - start.tv_sec) * 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;
    cout << "When the target length is " << target_length << ", pattern length is " << pattern_length << ", the elapsed time is " << elapsed_time << " ms." << endl;

    int counter = 0;

    for (int i = 0; i < target_length; i++)
    {
        if (answer[i])
        {
            // cout << "Find a matching substring starting at: " << i << "." << endl;
            counter++;
        }
    }

    cout << counter << endl;

    delete[] target;
    delete[] pattern;
    delete[] failure;
    delete[] answer;

    return 0;
}

void KMP(char *target, char *pattern, int *failure, int *answer, int pattern_length, int target_length, int part_start)
{
    int i = part_start, j = part_start + target_length;

    int k = 0;
    while (i < j)
    {
        if (k == -1)
        {
            i++;
            k = 0;
        }
        else if (target[i] == pattern[k])
        {
            i++;
            k++;

            if (k == pattern_length)
            {
                k--;
                answer[i - pattern_length] = 1;
                i = i - pattern_length + 1;
            }
        }
        else
            k = failure[k];
    }

    return;
}

void preKMP(char *pattern, int failure[])
{
    int m = strlen(pattern);
    int k;

    failure[0] = -1;

    for (int i = 1; i < m; i++)
    {
        k = failure[i - 1];

        while (k >= 0)
        {
            if (pattern[k] == pattern[i - 1])
                break;
            else
                k = failure[k];
        }

        failure[i] = k + 1;
    }

    return;
}