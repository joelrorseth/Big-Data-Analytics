//
//  A-Priori Algorithm
//
//  Pass 1: Read baskets, count occurrences of individual items
//          Items appearing >= s times are frequent items
//  Pass 2: Read baskets, count occurrences of pairs whose two
//          items are frequent themselves
//
// This program demonstrates output of the Triangular matrix and Triples
// approach. You only need one of these, I left both for references=.
//
//  Joel Rorseth 104407927
//  February 13, 2018
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>


// Input file, retail.txt, is a big dataset with 88k rows. Each row is a
// basket with some number of elements, encoded as integers. Each row is sorted.


// Translate triangular matrix i,j into linear array index
int tri_to_lin_idx(int i, int j, int n) {
    return (n * i) + j - ((i * (i + 1)) /2);
}

// Translate the index of linear array to triangular matrix i,j
std::pair<int,int> lin_to_tri_idx(int idx, int n) {

    unsigned int i = 0, j = 0;
    unsigned int delta = n - 1;
    
    // Row index
    for( unsigned int x = delta; x < idx; x += delta-- )
        ++i;
    
    // Column index
    delta = n - 1;
    unsigned int x;
    for(x = delta; x < idx; x += delta-- )
        ++j;
    
    j = n + idx - x - 1;
    
    return std::make_pair(j, i);
}



// Pass 1 -- Make histogram to help find frequent items
// Returns the number of frequent items (beating thresh) found

int find_freq_items(const std::string & filename,
                    std::unordered_set<int> & freq, int thresh) {

    // Store all items count in first pass, but make this local so we can
    // deallocate this and keep only frequent items in a returned set
    
    std::unordered_map<int,int> hist;

    std::ifstream in_file(filename);
    std::string line;
    int num, line_count;

    while (std::getline(in_file, line)) {

        ++line_count;

        // Use stringstream to convert line into stream, read using >>
        std::stringstream ss(line);

        // Increment the element histogram
        while (ss >> num)
            ++hist[num];
    }
    
    in_file.close();

    // Calculate actual threshold using given percentage threshold
    auto support_cutoff = line_count * (((double) thresh) / 100.0);
    //std::cout << "Items must occur in " << support_cutoff << " baskets\n";

    // For each distinct item read...
    for (auto it = hist.begin(); it != hist.end(); ++it) {

        // Insert into set if occurrences > support threshold
        if ((*it).second > support_cutoff)
            freq.insert((*it).first);
    }

    return freq.size();
}



// Pass 2 -- Find pairs where both items are frequent, count such pairs
// For demonstration, store occurrences in triples and in triangular array

void find_pairs_freq_items(const std::string & filename,
        std::unordered_set<int> & freq_items,
        std::unordered_map<int,int> & item_to_index_lookup,
        std::unordered_map<int,int> & index_to_item_lookup,
        int num_freq_items) {

    std::ifstream in_file(filename);
    std::string line;
    int num;

    // Approach 1: Triangular matrix / array
    int n = (num_freq_items * (num_freq_items + 1)) / 2;
    int tri_matrix[n] = {0};
    //std::cout << "Triangular matrix is " << n << " by " << n << '\n';
    
    // Approach 2: Map for keeping triples
    std::map<std::pair<int,int>, int> pair_hist;

    
    while (std::getline(in_file, line)) {

        // Use stringstream to convert line into stream, read using >>
        std::stringstream ss(line);
        std::vector<int> basket;

        // Add element to current basket
        while (ss >> num)
            basket.push_back(num);

    
        // Test each distinct pair in the basket
        for (auto i = basket.begin(); i != basket.end(); ++i) {
            for (auto j = (i+1); j != basket.end(); ++j) {

                // Both pair elements must be frequent themselves
                if (freq_items.find(*i) != freq_items.end() &&
                    freq_items.find(*j) != freq_items.end() &&
                    *i != *j) {
                    
                    // Insert into pair triples (histogram)
                    ++pair_hist[std::make_pair(*i, *j)];

                    // Map i,j from upper triangular matrix to linear triangular array index
                    auto s_i = item_to_index_lookup[*i];
                    auto s_j = item_to_index_lookup[*j];

                    // i < j to use only the upper half of the imaginary triangular matrix
                    // Also means that 1,2 and 2,1 are interpreted as the same pair
                    auto _i = (s_i < s_j) ? s_i : s_j;
                    auto _j = (s_i < s_j) ? s_j : s_i;
                    
                    // Increment counter for this unique pair
                    int k = tri_to_lin_idx(_i, _j, num_freq_items);
                    ++tri_matrix[k];
                }
            }
        }
    }

    in_file.close();
    
    std::cout << "\nApproach 1: One dimensional triangular array\n"
        << "\nPAIR  \tOCCURRENCES\n";
    
    // Approach 1: Print triangular matrix results
    for (int i = 0; i < n; ++i) {
        
        // In our triangular matrix, every i,i element will be 0 count, skip
        if (tri_matrix[i] == 0)
            continue;
        
        auto result = lin_to_tri_idx(i, num_freq_items);
        
        std::cout << index_to_item_lookup[result.first] << ','
            << index_to_item_lookup[result.second]
            //<< " aka " << result.first << ',' << result.second
            << "  \t" << tri_matrix[i] << '\n';
    }
        
    std::cout << "\nApproach 2: Keep triples with count\n"
        << "\nPAIR  \tOCCURRENCES\n";
    
    // Approach 2: Print triples i,j,count
    for (const auto & c: pair_hist)
        std::cout << c.first.first << ',' << c.first.second
            << "  \t" << c.second << '\n';

    std::cout << std::endl;
}



// Demonstration
// Perform Pass 1 and 2, perform auxiliary preprocessing

int main(int argc, char ** argv) {

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.txt> <support thresh %>\n";
        return 1;
    }
    
    const std::string filename{argv[1]};
    const int thresh_percent = std::stoi(argv[2]);

    // Pass 1: Get histogram of individual elements
    std::unordered_set<int> freq_items;
    auto num_freq_items = find_freq_items(filename, freq_items, thresh_percent);

    //std::cout << "There are " << num_freq_items << " frequent items\n";

    // Create condensed lookup table, mapping 0,1,... to frequent_items and vice versa
    // This is a trick to reduce size of triangular matrix to num_freq_items

    std::unordered_map<int,int> item_to_index_lookup;
    std::unordered_map<int,int> index_to_item_lookup;

    for (auto it = freq_items.begin(); it != freq_items.end(); ++it) {
        
        auto index = std::distance(freq_items.begin(), it);
        //std::cout << *it << " mapped to " << index << "\n";
        index_to_item_lookup[index] = *it;
        item_to_index_lookup[*it] = index;
        
    }

    // Pass 2: Print occurrence count of each pair determined to be frequent
    find_pairs_freq_items(filename, freq_items, item_to_index_lookup,
        index_to_item_lookup, num_freq_items);
}
