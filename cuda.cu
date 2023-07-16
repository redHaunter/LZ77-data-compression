#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>

using namespace std;
using namespace std::chrono;

struct token
{
    int offset;
    int length;
    char next;
};

__global__ void lz77_compress_kernel(char *input, int input_length, token **output, int window_size, int buffer_size, int num_theards, int string_length)
{
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    int start_index = input_length * idx;
    int i = 0;
    int end = input_length;
    if (idx == num_theards - 1)
    {
        end += string_length - (input_length * num_theards);
    }
    int index_output = 0;
    while (i < end)
    {
        int j = max(0, i - window_size);
        int longest_match_length = 0;
        int best_match_offset = 0;

        while (j < i)
        {
            int k = 0;
            while (i + k < end && input[j + k + start_index] == input[i + k + start_index] && k < buffer_size)
            {
                k++;
            }
            if (k > longest_match_length)
            {
                longest_match_length = k;
                best_match_offset = i - j;
            }

            j++;
        }
        if (longest_match_length > 0)
        {
            output[idx][index_output++] = {best_match_offset, longest_match_length, input[i + longest_match_length + start_index]};
            i += longest_match_length + 1;
        }
        else
        {
            output[idx][index_output++] = {0, 0, input[i + start_index]};
            i++;
        }
    }
}

vector<vector<token>> lz77_compress_cuda(string input, int window_size, int buffer_size, int num_blocks, int num_theards)
{
    char *input_dev;
    token **output_dev;
    int input_length = input.length();

    cudaMalloc((void **)&input_dev, input_length * sizeof(char));
    cudaMalloc((void **)&output_dev, num_theards * num_blocks * sizeof(token *));

    cudaMemcpy(input_dev, input.c_str(), input_length * sizeof(char), cudaMemcpyHostToDevice);

    token **output = new token *[num_theards * num_blocks];
    for (int i = 0; i < num_theards * num_blocks; i++)
    {
        cudaMalloc((void **)&output[i], input_length * sizeof(token));
    }
    cudaMemcpy(output_dev, output, num_theards * num_blocks * sizeof(token *), cudaMemcpyHostToDevice);
    int t = input_length / (num_theards * num_blocks);
    vector<vector<token>> output_vectors(num_theards * num_blocks);
    lz77_compress_kernel<<<num_blocks, num_theards>>>(input_dev, t, output_dev, window_size, buffer_size, num_theards * num_blocks, input_length);
    for (int i = 0; i < num_theards * num_blocks; i++)
    {
        output_vectors[i].resize(input_length);
        cudaMemcpy(&(output_vectors[i][0]), output[i], input_length * sizeof(token), cudaMemcpyDeviceToHost);
        cudaFree(output[i]);
    }
    cudaFree(input_dev);
    cudaFree(output_dev);
    delete[] output;

    return output_vectors;
}

string lz77_decompress(vector<vector<token>> compressed)
{
    string output = "";
    for (auto block : compressed)
    {
        for (auto t : block)
        {
            if (t.length == 0)
            {
                output += t.next;
            }
            else
            {
                int start = output.length() - t.offset;
                for (int i = 0; i < t.length; i++)
                {
                    output += output[start + i];
                }
                output += t.next;
            }
        }
    }
    return output;
}

int main()
{
    ofstream timeResult("Time result2.csv");
    ofstream sizeResult("Size result2.csv");
    timeResult << "2,4,8,16,32,64,128,256,512\n";
    sizeResult << "2,4,8,16,32,64,128,256,512\n";
    for (int i = 2; i <= 32; i *= 2)
    {
        for (int j = 2; j <= 32; j *= 2)
        {
            string filename = "C:\\Users\\Asus\\Desktop\\cuda_cpp\\8.txt";
            ifstream infile(filename);
            string input((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
            cout << "Size of actual file: " << input.size() << endl;
            int window_size = 100;
            int buffer_size = 10;
            auto start = high_resolution_clock::now();
            vector<vector<token>> compressed = lz77_compress_cuda(input, window_size, buffer_size, i, j);
            auto stop = high_resolution_clock::now();

            int sum = 0;
            for (vector<token> v : compressed)
            {
                for (token vv : v)
                {
                    if (vv.next == '\0')
                        break;
                    sum++;
                }
            }
            cout << "Size of compress file: " << sum << endl;
            auto duration = duration_cast<microseconds>(stop - start);
            cout << "Time taken " << duration.count() << " microseconds" << endl;
            sizeResult << sum << ",";
            timeResult << duration.count() << ",";
            cudaDeviceReset();
        }
        sizeResult << endl;
        timeResult << endl;
    }

    timeResult.close();
    sizeResult.close();

    // string decompressed = lz77_decompress(compressed);
    // cout << decompressed << endl;
    return 0;
}
