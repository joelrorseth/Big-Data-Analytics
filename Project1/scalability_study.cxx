//
//  Scalability Study
//
//  Joel Rorseth 104407927
//  February 11, 2018
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <chrono>
#include "gnuplot-iostream/gnuplot-iostream.h"



// Get string filename for chunk percentage size
std::string chunk_filename(const std::string & filename, const int size) {
    //return "temp_" + std::to_string(size) + ".txt";
    return "percent" + std::to_string(size) + '_' + filename;
}


// Create a temporary file for each chunk size
void create_temp_chunk_files(const std::string & filename,
                             std::vector<int> & chunk_sizes) {
    
    // First obtain number of baskets (lines) in entire, original input file
    std::ifstream in_file(filename);
    std::string line;
    std::vector<std::string> lines;
    int baskets;
    
    while (std::getline(in_file, line)) {
        ++baskets;
        lines.push_back(line);
    }
    
    in_file.close();
    
    // Now for each chunk_size, create a file which contains the first
    // <chunk_size>% of the lines out of line_count
    
    std::cout << "Total baskets: " << baskets << '\n';
    std::ofstream out_file;
    
    for (const auto size: chunk_sizes) {
        
        out_file.open(chunk_filename(filename, size));
        
        // The number of lines in this chunk size of the original file
        int chunk_lines = baskets * ((double) size / 100.0);
        
        // Write the first chunk_lines lines to its own file
        for (int i = 0; i < chunk_lines; ++i)
            out_file << lines[i] << '\n';
        
        out_file.close();
    }
}


// Delete all the temporary chunk files from the directory
void delete_temp_chunk_files(const std::string & filename,
                             const std::vector<int> & chunk_sizes) {

    for (const auto size: chunk_sizes)
        std::remove(chunk_filename(filename, size).c_str());
}


// Main line graph generation algorithm
int main(int argc, char ** argv) {
    
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <input.txt> <a_priori.cxx> <pcy.cxx>\n";
        return 1;
    }
    
    const std::string input_filename{argv[1]};
    const std::string a_priori_file{argv[2]};
    const std::string pcy_file{argv[3]};
    const std::vector<std::string> algorithms{a_priori_file, pcy_file};
    
    std::vector<int> thresholds{1,5,10};
    std::vector<int> chunk_sizes{1,5,10,20,30,40,50,60,70,80,90,100};
    
    // Generate chunked files to pass to algorithm
    create_temp_chunk_files(input_filename, chunk_sizes);
    
    // For each tested support threshold, derive a performance line chart
    for (const auto thresh: thresholds) {
        
        // Initialize new line chart for Support Threshold: <thresh>%
        Gnuplot gp;
        const std::string title = "Support Threshold: " + std::to_string(thresh) + "%";
        const std::string pic_filename = "./study_threshold_" + std::to_string(thresh) + ".png";
        
        gp << "set title \"" << title << "\"\nset xrange[0:100]\n";
        gp << "set xlabel \"Dataset Size\"\nset ylabel \"Runtime (ms)\"\n";
        gp << "set terminal png\nset output \"study_threshold_" <<
            std::to_string(thresh) << ".png\"\n";
        
        std::vector<std::pair<int,int>> points_ap;
        std::vector<std::pair<int,int>> points_pcy;
        
        // Compare runtime for A-Priori algorithm vs PCY
        for (auto i = 0; i < algorithms.size(); ++i) {
            
            const auto algorithm = algorithms[i];
            
            // For each chunk size..
            for (const auto size: chunk_sizes) {
             
                // Pass entire chunk to frequent itemset miner
                const auto cmd = "g++-7 -std=c++17 -o miner " + algorithm + " && ./miner "
                    + chunk_filename(input_filename, size) + ' ' + std::to_string(thresh);
                
                // Start timer
                const auto start = std::chrono::high_resolution_clock::now();
                
                // Execute 'cmd'
                system(cmd.c_str());
                
                // End timer upon return
                const auto finish = std::chrono::high_resolution_clock::now();
                
                // Obtain time difference (runtime taken to perform mining)
                const std::chrono::milliseconds dif =
                    std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
                const double dif_mseconds = dif.count();
                
                // Put {size, dif} into points vector
                if (i == 0)
                    points_ap.push_back( std::make_pair(size, dif_mseconds) );
                else
                    points_pcy.push_back( std::make_pair(size, dif_mseconds) );
            }
        }
        
        // Plot chunk_size, runtime pair on current line chart
        gp << "plot" << gp.file1d(points_ap) << " title \"A-Priori\" with lines, "
            << gp.file1d(points_pcy) << " title \"Multistage\" with lines\n";
    }
    
    // Remove files now, they were only temporary
    delete_temp_chunk_files(input_filename, chunk_sizes);
}
