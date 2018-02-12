
# Project 1

Below are summaries and compile commands for the algorithms implemented.

## A-Priori

The A-Priori program implements the algorithm efficiently, and avoids keeping infrequent items (and their counts)
between passes. By transforming the item counts from Pass 1 into a set of frequent items, memory is saved, and
is sufficient input to Pass 2. I have implemented both the Triangular Matrix / Array (approach 1) and Count Triples
(approach 2) in this file. You will see results for both in the output. You would obviously pick *one* in a real application,
however because the input size is manageable, *I have chosen to illustrate both*.

`g++-7 -std=c++17 -o apriori a_priori.cxx`
`./apriori retail.txt 10`

## PCY

The PCY algorithm implements the multistage extension to the traditional PCY algorithm, finishing in 3 passes. For
efficiency, hash tables are stored temporarily, as vectors. Before finishing a pass, each pass performs its respective
between-class duties. Before finishing a pass, the hash table is removed from memory and transformed / returned
as a bitmap, representing the support frequency of a bucket at the given indices.

`g++-7 -std=c++17 -o pcy pcy.cxx`
`./pcy retail.txt 10`

## Scalability Study

The scalability study performs a comparison of the performance of the A-Priori and PCY algorithms. The comparison metric
is runtime, when run over identical subsets of the original dataset (retail.txt). The results are illustrated in the images provided
in the directory, run using support thresholds of 1%, 5% and 10%.

`g++-7 -std=c++17 -o study scalability_study.cxx -lboost_iostreams -lboost_system -lboost_filesystem`
`./study retail.txt a_priori.cxx pcy.cxx`


### Dependencies

- gcc  (I have used 7.2.0, with C++17 standard)
- boost  (I have used 1.66.0)
- gnuplot  (header for C++ use is included)

### System Specs

- OS:  macOS Sierra 10.12.6
- CPU:  2.2 GHz Intel Core i7
- RAM:  16 GB 1600 MHz DDR3

### Notes

- For efficiency, I have swapped out the hash table in the PCY algorithm in favour of a `std::vector`. This is because the hash
functions used yield integers, in the range of 0 to (NUM_BUCKETS-1). As such, the hash values can be used as indices to any
indexable array for O(1) lookup.
- Frequent items are stored in `std::unordered_set`, so that O(1) lookup efficiency is achieved.
