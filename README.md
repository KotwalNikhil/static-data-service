# ResearchEx Assignment

## Overview

The assignment provides an implementation for handling large static data files with support for multithreading, priority-based data handling, and file management.

## Problem Statement

There are more than 10000 equity symbols trading in the US market today. Suppose a market data vendor provided us with market data for each symbol in individual files in the format attached below (sorted on timestamp). There will be 10000 such files each named after a symbol. We need to bring the data in these 10000 symbols into a single market data file (again sorted on timestamp) so that we can play it back in our back-testing system. For now, the result file should also be in a text format with an extra column (attached below as MultiplexedFile.txt). So -  

The problem is to achieve this result file using a program as efficiently as possible. Note that the total number of entries in all these 10000 files put together runs into billions and inefficient handling can cause the program to run into days.  

The entries in the input files are all guaranteed to be sorted by timestamp. The output file too must have all entries sorted by timestamp. In case entries of different symbols have the same timestamp (our timestamp resolution is in millisecond and hence there will be multiple quotes across symbols coming in at the same timestamp), then those entries must be sort in alphabetical order of the symbols
staticDataService.hpp: Core implementation of the static data service.

## How to build and run test

Download the zip file and extract to get the multiThreading directory

```
cd multiThreading
mkdir build
cd build
cmake .. && make -j12
ctest --output-on-failure -R fileProcessingTest
```

## Implemented Algorithms 

Algorithm can be switched from method `mergeMultipleFiles` inside `staticDataService.hpp` line 271

### 1. External merge of large files

#### Implemented methods -

**mergeMultipleFiles** - To create batches from inputFiles and assign threads to them.  
**mergeBatch** - Creates temporary output file and calls mergeTwoFiles.  
**mergeTwoFiles** - A utility method to merge two files based on the compare method.  
**compare** - To compare two lines based on the constraints. It parses and converts timestamp in string format to unix int format using `convertToUnixTimestamp`.

### 2. Using Priority Queue

It uses thread local queue of `MarketData` object to store the content from each file in the batch and internally uses custom overloaded operator <  to create min-heap of the data.

#### Implemented methods -

**mergeMultipleFiles** - To create batches from inputFiles and assign threads to them.  
**mergeBatchUsingPriorityQueue** - Creates priority queue and temporary output file tp merge the data in the batch.  

## Unit test
Used Google test to test the assignment.  
Created class `FileProcessingTest` which has overridden methods  

**setup()**  - Creates multiple input files with the static data - currently I have 512 files, which has unique rows using unique timestamp based on the year.
 Initially I creted unique rows based on the microsecond but for larger files the count crosses 1000 the microsecond becomes invalid.
 Hence I created based on the year and used logic `1972 + i` so that the converted unix timestamp is valid.  

**teardown()** - This is used to clear the auto generated input files.
