#include <mpi/mpi.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>

#define MASTER 0

using namespace std;

int *kmp(char *target, char *pattern, int *table);
int *preKMP(char *pattern, int len); // Build failure function (also known as KMP match table)

// KMP match table
// public to every process
int *preKMP(char *pattern, int len)
{
    int k = 0;
    int i = 1;
    int *table = (int *)malloc(len * sizeof(int));
    table[0] = k;
    for (i = 1; i < len; i++)
    {
        while (k > 0 && pattern[i - 1] != pattern[i])
        {
            table[i] = k - 1;
            k = table[i];
        }
        if (pattern[i] == pattern[k])
            k++;
        table[i] = k;
    }
    return table;
}

// key func kmp
int *kmp(char *target, char *pattern, int *table)
{
    int n = strlen(target);
    int m = strlen(pattern);
    int *answer = (int *)calloc(n, sizeof(int));
    int j = 0;
    int i = 0;
    int index = 0;
    while (i < n)
    {
        if (pattern[j] == target[i])
        {
            j++;
            i++;
        }
        if (j == m)
        {
            // printf("this is matching %d.\n", i-j);
            answer[i - j] = 1;
            j = table[j - 1];
        }
        else if (i < n && pattern[j] != target[i])
        {
            if (j != 0)
                j = table[j - 1];
            else
                i++;
        }
    }
    return answer;
}

void fill_result_from_partial(int *result, int result_start_idx, int *partial_res, int partial_size)
{

    for (int i = 0; i < partial_size; i++)
    {
        result[result_start_idx + i] = partial_res[i];
    }
}

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

    char *target = (char *)malloc(sizeof(char) * (target_length + 1));
    char *pattern = (char *)malloc(sizeof(char) * (pattern_length + 1));

    strcpy(target, target_string.c_str());
    strcpy(pattern, pattern_string.c_str());

    int *kmp_table = preKMP(pattern, pattern_length); // Build failure function (also known as KMP match table)

    int world_rank, world_size;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    /* Print processor info in each worker(process) */
    // char processor_name[MPI_MAX_PROCESSOR_NAME];
    // int name_len;
    // MPI_Get_processor_name(processor_name, &name_len);
    // // Print off a hello world message
    // printf("Hello world from processor %s, rank %d out of %d processors\n",
    //         processor_name, world_rank, world_size);

    int start_pos = target_length / world_size;
    int block_connection = pattern_length - 1; //
    int block_per_proc_no_connect = target_length / world_size;
    int block_per_proc = block_per_proc_no_connect;
    block_per_proc += (world_size != 1) ? block_connection : 0; // 0 for corner case - only one process is no connection part
    int block_endProc = block_per_proc_no_connect + target_length % world_size;

    // Task distribution method
    // For example:
    // target:[a b c d e f g h i j], pattern = "fgh",  world_size = 3, block_connection = 2
    // proc1 = a b c d e (d, e is connection block)
    // proc2 = d e f g h (g, h is connection block)
    // proc3 = g h i j

    int *result;          // Final KMP result is used by Master process
    int *partial_result;  // Parital KMP result for each worker
    char *partial_target; // Partial target string

    struct timespec start, end;
    double elapsed_time;

    if (world_rank == MASTER)
    {
        cout << "----- This is MPI results using KMP Algorithm. -----" << endl;

        clock_gettime(CLOCK_MONOTONIC, &start);

        result = (int *)calloc(target_length, sizeof(int));
        partial_target = (char *)malloc(sizeof(char) * (block_per_proc + 1));
        strncpy(partial_target, target, block_per_proc);
        partial_target[block_per_proc] = '\0';
        partial_result = kmp(partial_target, pattern, kmp_table);

        fill_result_from_partial(result, 0, partial_result, block_per_proc);

        if (world_size > 1)
        {
            // Wait for result from other processes
            MPI_Request requests[world_size - 1];
            MPI_Status status[world_size - 1];

            int shift_idx = block_per_proc_no_connect;
            for (int i = 1; i < world_size - 1; i++)
            {
                MPI_Irecv(result + shift_idx, block_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD, &(requests[i - 1]));
                shift_idx += block_per_proc_no_connect;
            }
            // Wait for final prococess
            MPI_Irecv(result + shift_idx, block_endProc, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, &(requests[world_size - 2]));

            MPI_Waitall(world_size - 1, requests, status);
        }
    }
    else if (world_rank == world_size - 1)
    {
        // Final process
        partial_target = (char *)malloc(sizeof(char) * (block_endProc + 1));
        int target_start_idx = block_per_proc_no_connect * (world_rank);
        strncpy(partial_target, target + target_start_idx, block_endProc);
        partial_target[block_endProc] = '\0';
        partial_result = kmp(partial_target, pattern, kmp_table);

        // Send result to MASTER
        MPI_Request req;
        MPI_Isend(partial_result, block_endProc, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &req);
    }
    else
    {
        // Process is neither Master nor final process

        partial_target = (char *)malloc(sizeof(char) * (block_per_proc + 1));
        int target_start_idx = block_per_proc_no_connect * (world_rank);
        strncpy(partial_target, target + target_start_idx, block_per_proc);
        partial_target[block_per_proc] = '\0';

        partial_result = kmp(partial_target, pattern, kmp_table);

        // Send result to MASTER
        MPI_Request req;
        MPI_Isend(partial_result, block_per_proc, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &req);
    }

    // print result
    if (world_rank == MASTER)
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        elapsed_time = (end.tv_sec - start.tv_sec) * 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;
        cout << "When the target length is " << target_length << ", pattern length is " << pattern_length << ", the elapsed time is " << elapsed_time << " ms." << endl;

        int counter = 0;
        for (int i = 0; i < target_length; i++)
        {
            if (result[i] == 1)
            {
                // cout << "Find a matching substring starting at: " << i << "." << endl;
                counter++;
            }
        }

        cout << counter << endl;
    }

    MPI_Finalize();

    free(target);
    free(pattern);

    return 0;
}