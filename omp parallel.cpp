#include <bits/stdc++.h>
#include <omp.h>

using namespace std;
using namespace std::chrono;

struct token {
    int offset;
    int length;
    char next;
};

vector<token> lz77_compress(string input, int window_size, int buffer_size) {
    vector<token> output;
    int input_length = input.length();
    int i = 0;

    while (i < input_length) {
        int j = max(0, i - window_size);
        int longest_match_length = 0;
        int best_match_offset = 0;

        while (j < i) {
            int k = 0;
            while (i + k < input_length && input[j + k] == input[i + k] && k < buffer_size) {
                k++;
            }
            if (k > longest_match_length) {
                longest_match_length = k;
                best_match_offset = i - j;
            }

            j++;
        }
        if (longest_match_length > 0) {
            output.push_back({best_match_offset, longest_match_length, input[i + longest_match_length]});
            i += longest_match_length + 1;
        } else {
            output.push_back({0, 0, input[i]});
            i++;
        }
    }

    return output;
}

void omp_lz77_compress(string input, int window_size, int buffer_size, int threads, vector<vector<token>> *array_tokens) {
    omp_set_num_threads(threads);
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < threads; i++) {
            array_tokens->push_back(
                    lz77_compress(input.substr(i * (input.size() / threads), input.size() / threads),
                                  100,
                                  10));
        }
    }
}


int main() {
    int NUM_THREADS = 1;
    string filename = "test.txt";
    ifstream infile(filename);
    string input((istreambuf_iterator<char>(infile)),
                 istreambuf_iterator<char>());
    cout << "Size of actual file: " << input.size() << endl;
    ofstream timeResult("Time result.csv");
    ofstream sizeResult("Size result.csv");
    timeResult << "1,6,7,8,9,10,11,12,13,14,15,16\n";
    sizeResult << "1,6,7,8,9,10,11,12,13,14,15,16\n";
    for (int j = 0; j < 10; ++j) {
        auto start = high_resolution_clock::now();
        vector<token> a = lz77_compress(input, 100, 10);
        auto stop = high_resolution_clock::now();

        cout << "Size of compress file without parallel: " << a.size() << endl;
        sizeResult << a.size();

        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time taken by one thread :"
             << duration.count() << " microseconds" << endl;
        timeResult << duration.count();

        for (int i = 6; i <= 16; ++i) {
            NUM_THREADS = i;
            vector<vector<token>> array_tokens(NUM_THREADS + 1);
            auto start = high_resolution_clock::now();
            omp_lz77_compress(input, 100, 10, i, &array_tokens);
            array_tokens.push_back(lz77_compress(input.substr(NUM_THREADS * (input.size() / NUM_THREADS)), 100, 10));
            auto stop = high_resolution_clock::now();
            int sum = 0;
            for (int i = 0; i < array_tokens.size(); i++) {
                sum += array_tokens[i].size();
            }
            cout << "Size of compress file with " << i << "threads: " << sum << endl;
            sizeResult << "," << sum;
            auto duration = duration_cast<microseconds>(stop - start);
            cout << "Time taken by " << i << " threads: "
                 << duration.count() << " microseconds" << endl;
            timeResult << "," << duration.count();
        }
        timeResult << endl;
        sizeResult << endl;
    }
    timeResult.close();
    sizeResult.close();
    return 0;
}