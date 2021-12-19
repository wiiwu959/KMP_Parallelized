#include <ctime>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

#define THREAD_PER_BLOCK 256
#define WORD_PER_THREAD 16

#define FIRST_PASS 0
#define SECOND_PASS 1

using namespace std;

// build the kmp table for the subsequent operations
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

// Kernel failuretion. Implement the KMP algorithm
__global__ void KMP(char *pattern, char *target, int failure[], int answer[], int pattern_length, int target_length, int pass)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    // int i = WORD_PER_THREAD * threadIdx.x;
    // int j = (WORD_PER_THREAD * (threadIdx.x + 2) - 1) >= target_length ? target_length - 1 : WORD_PER_THREAD * (threadIdx.x + 2) - 1;
    int i, j;

    switch (pass)
    {
        case FIRST_PASS:
            i = WORD_PER_THREAD * index;
            j = WORD_PER_THREAD * (index + 1) > target_length ? target_length : WORD_PER_THREAD * (index + 1);
            break;
        case SECOND_PASS:
            i = WORD_PER_THREAD * (index + 1) - (pattern_length - 1);
            j = WORD_PER_THREAD * (index + 1) + (pattern_length);
            break;
        default:
            break;
    }

    if (i >= target_length)
        return;

    __shared__ char pattern_cache[32];
    __shared__ char failure_cache[32];
    if (threadIdx.x == 0)
    {
        for (int i = 0; i < pattern_length; i++)
        {
            pattern_cache[i] = pattern[i];
            failure_cache[i] = failure[i];
        }
    }

    // __shared__ char target_cache[WORD_PER_THREAD * (THREAD_PER_BLOCK + 1)];
    // for (int i = 0; i < WORD_PER_THREAD * 2; i++)
    //     target_cache[WORD_PER_THREAD * threadIdx.x + i] = target[(WORD_PER_THREAD * index + i) >= target_length ? target_length - 1 : WORD_PER_THREAD * index + i];

    int k = 0;
    while (i < j)
    {
        if (k == -1)
        {
            i++;
            k = 0;
        }
        else if (target[i] == pattern_cache[k])
        {
            i++;
            k++;

            if (k == pattern_length)
            {
                // answer[(blockIdx.x * blockDim.x * WORD_PER_THREAD) + i - pattern_length] = 1;
                k--;
                answer[i - pattern_length] = 1;
                i = i - pattern_length + 1;
            }
        }
        else
            k = failure_cache[k];
    }

    return;
}

int main(int argc, char *argv[])
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

    char *device_target, *device_pattern;
    int *device_failure, *device_answer;

    cudaEvent_t start, end;
    float elapsed_time;

    cudaMalloc((void **)&device_target, target_length * sizeof(char));
    cudaMalloc((void **)&device_pattern, pattern_length * sizeof(char));
    cudaMalloc((void **)&device_failure, target_length * sizeof(int));
    cudaMalloc((void **)&device_answer, target_length * sizeof(int));

    cudaMemcpy(device_target, target, target_length * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(device_pattern, pattern, pattern_length * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(device_failure, failure, pattern_length * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(device_answer, answer, target_length * sizeof(int), cudaMemcpyHostToDevice);

    cudaEventCreate(&start);
    cudaEventCreate(&end);
    cudaEventRecord(start, 0);

    KMP<<<(target_length / THREAD_PER_BLOCK / WORD_PER_THREAD), THREAD_PER_BLOCK>>>(device_pattern, device_target, device_failure, device_answer, pattern_length, target_length, FIRST_PASS);
    KMP<<<(target_length / THREAD_PER_BLOCK / WORD_PER_THREAD), THREAD_PER_BLOCK>>>(device_pattern, device_target, device_failure, device_answer, pattern_length, target_length, SECOND_PASS);

    cudaEventRecord(end, 0);
    cudaEventSynchronize(end);
    cudaEventElapsedTime(&elapsed_time, start, end);
    cudaEventDestroy(start);
    cudaEventDestroy(end);

    cudaMemcpy(answer, device_answer, target_length * sizeof(int), cudaMemcpyDeviceToHost);

    cout << "----- This is parallel results using KMP Algorithm on CUDA. -----" << endl;
    cout << "When the target length is " << target_length << ", pattern length is " << pattern_length << ", the elapsed time is " << elapsed_time << " ms." << endl;

    int counter = 0;

    for (int i = 0; i < target_length; i++)
    {
        if (answer[i])
        {
            cout << "Find a matching substring starting at: " << i << "." << endl;
            counter++;
        }
    }

    cout << counter << endl;

    cudaFree(device_target);
    cudaFree(device_pattern);
    cudaFree(device_failure);
    cudaFree(device_answer);

    delete[] target;
    delete[] pattern;
    delete[] failure;
    delete[] answer;

    return 0;
}