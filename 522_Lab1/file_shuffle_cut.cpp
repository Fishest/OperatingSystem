// Program: file_shuffle_cut.cpp
// Author: Chris Gill
// Purpose: reads lines of a text file and numbers them, shuffles them, and
//          cuts them into separate files with fragments of the original text

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
using namespace std;

// return codes for success or failure
const int success = 0;
const int wrong_number_of_arguments = -1;
const int input_file_open_failed = -2;
const int output_file_open_failed = -3;

// constants for command line indexing
const int program_name_index = 0;
const int file_name_index = 1;
const int fragments_index = 2;
const int expected_argc = 3;

// struct to hold a numbered line of text
struct numbered_line {
    numbered_line() : number(0) {}
    int number;
    string text;
};

// outputs proper usage syntax for the program
int usage (const char *program_name, int result) {
    cout << "usage: " << program_name
         << " <file name> <number of fragments>" << endl;
    return result;
}

// writes out a range of lines into a named output file
int write_fragment (vector<numbered_line>::const_iterator iter,
                    vector<numbered_line>::const_iterator stop,
                    const char * filename)
{
    ofstream ofs (filename);
    if (!ofs) {
        cout << "Could not open output file " <<  filename << endl;
        return output_file_open_failed;
    }

    while (iter != stop) {
        ofs << iter->number << " " << iter->text << endl;
        iter++;
    }

    return success;
}


int main (int argc, char *argv[]) {

    // check command line argument count
    if (argc != expected_argc) {
        // suggest how to run the program correctly
        return usage(argv[program_name_index], wrong_number_of_arguments);
    }

    // check ability to open input file
    ifstream ifs (argv[file_name_index]);
    if (!ifs) {
        cout << "Could not open file " <<  argv[file_name_index] << endl;
        return usage(argv[program_name_index], input_file_open_failed);
    }

    // fill a vector with numbered lines from the file
    vector<numbered_line> nlv;
    numbered_line nl;
    while (getline(ifs, nl.text)) {
        nlv.push_back(nl);
        nl.number++;
    }

    // shuffle the numbered lines in the vector
    random_shuffle (nlv.begin(), nlv.end());

    // extract (and possibly reduce) number of fragments to create
    int fragments;
    istringstream iss (argv[fragments_index]);
    iss >> fragments;
    if (fragments > nlv.size()) {
        fragments = nlv.size();
    }

    // calculate number of lines per fragment, initialize iterators
    int lines_per_fragment = nlv.size() / fragments;
    vector<numbered_line>::const_iterator start = nlv.begin();
    vector<numbered_line>::const_iterator stop = start + lines_per_fragment;

    // output fragments of shuffled text into files
    for (int fragment = 1; fragment <= fragments; ++fragment) {
        if (stop > nlv.end() || fragment == fragments) stop = nlv.end();

        ostringstream file_name_stream;
        file_name_stream << argv[file_name_index] << "_" << fragment;
        int result =  write_fragment (start, stop,
                                      file_name_stream.str().c_str());
        if (result != success) return result;
        start += lines_per_fragment;
        stop = start + lines_per_fragment;
    }

    cout << nlv.size() << " lines" << endl;
    cout << fragments << " fragments" << endl;
    cout << lines_per_fragment << " lines_per_fragment" << endl;


    return success;
}
