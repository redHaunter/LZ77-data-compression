#include <bits/stdc++.h>

using namespace std;

struct token
{
    int offset;
    int length;
    char next;
};

vector<token> lz77_compress(string input, int window_size, int buffer_size)
{
    vector<token> output;
    int input_length = input.length();
    int i = 0;

    while (i < input_length)
    {
        int j = max(0, i - window_size);
        int longest_match_length = 0;
        int best_match_offset = 0;

        while (j < i)
        {
            int k = 0;
            while (i + k < input_length && input[j + k] == input[i + k] && k < buffer_size)
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
            output.push_back({best_match_offset, longest_match_length, input[i + longest_match_length]});
            i += longest_match_length + 1;
        }
        else
        {
            output.push_back({0, 0, input[i]});
            i++;
        }
    }

    return output;
}

string lz77_decompress(vector<token> compressed)
{
    string output = "";
    for (auto t : compressed)
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
    return output;
}

int main()
{
    string filename = "sample-2mb-text-file.txt";
    ifstream infile(filename);
    string input((istreambuf_iterator<char>(infile)),
                 istreambuf_iterator<char>());
    cout << input.size() << endl;
    vector<token> compressed = lz77_compress(input, 100, 10);
    cout << compressed.size() << endl;

    string decompressed = lz77_decompress(compressed);
    cout << "Original: " << input << endl;
    cout << "Decompressed: " << decompressed << endl;
    return 0;
}
