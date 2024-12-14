#pragma once

#include <vector>
#include <mutex>
#include <fstream>
#include <thread>
#include <queue>
#include <regex>
#include <condition_variable>
#include <atomic>
#include "marketData.hpp"

constexpr size_t maxTotalThreads = 100;
std::atomic<int> tempCounter(0);

// Used for min-heap priority queue
auto comparator = [](const MarketData& lhs, const MarketData& rhs) {
        return lhs < rhs;
    };

// Converts string Type into enum object for MarketData object creation
Type stringToType(const std::string& str) {
    if (str == "Bid") return BID;
    if (str == "Ask") return ASK;
    if (str == "TRADE") return TRADE;
    throw std::invalid_argument("Invalid Type: " + str);
}

// Converts Type enum object into string for writing in file
std::string typeToString(const Type type) {
    if (type == BID) return "Bid";
    if (type == ASK) return "Ask";
    if (type == TRADE) return "TRADE";
    throw std::invalid_argument("Invalid Type: " + type);
}

// Converts string timestamp in unix format for comparison
int64_t convertToUnixTimestamp(const std::string& timestamp) {
    std::tm tm = {};
    int milliseconds = 0;

    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    // Extract milliseconds if present
    if (ss.peek() == '.') {
        ss.ignore();
        ss >> milliseconds;
    }

    time_t timeSinceEpoch = std::mktime(&tm);

    return static_cast<int64_t>(timeSinceEpoch) * 1000 + milliseconds;
}

// To fetch Symbol using fileName if symbol is not already present in file
std::string fileNameFromPath(std::string path) {
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');
    if (lastSlash == std::string::npos) {
        return path.substr(0, lastDot);
    }
    return path.substr(lastSlash + 1, (lastDot > lastSlash ? lastDot : path.size()) - lastSlash - 1);
}

// To find if the file contains the symbol
bool inline checkSymbolInFile(std::string str) {
    if (str.find("Symbol") != std::string::npos) {
        return true;
    }

    return false;
}

// Compare method used by mergeTwoFiles - Returns true if line1 < line2
bool inline compare(std::string line1 , std::string line2, std::string file_name1 , std::string file_name2, bool hasSymbol1, bool hasSymbol2) {
    size_t comma_pos, second_comma_pos;
    std::string timestamp;
    int64_t unixTimeStamp1, unixTimeStamp2;

    comma_pos = line1.find(',');
    second_comma_pos = line1.find(',', comma_pos + 1);

    if (hasSymbol1) {
        timestamp = line1.substr(comma_pos + 2, second_comma_pos - comma_pos - 2);
        file_name1 = line1.substr(0, comma_pos);
    } else {
        timestamp = line1.substr(0, comma_pos);
    }

    unixTimeStamp1 = convertToUnixTimestamp(timestamp);

    comma_pos = line2.find(',');
    second_comma_pos = line2.find(',', comma_pos + 1);

    if (hasSymbol2) {
        timestamp = line2.substr(comma_pos + 2, second_comma_pos - comma_pos - 2);
        file_name2 = line2.substr(0, comma_pos);
    } else {
        timestamp = line2.substr(0, comma_pos);
    }

    unixTimeStamp2 = convertToUnixTimestamp(timestamp);

    if ((unixTimeStamp1 < unixTimeStamp2) || ((unixTimeStamp1 == unixTimeStamp2) && (file_name1 <= file_name2))) {
        return true;
    }

    return false;
}

// A utility method to merge two files based on the compare method
void inline mergeTwoFiles(const std::string file1, const std::string file2, const std::string& outputFile) {
    std::cout<<"merging files "<<file1<<" "<<file2<<" "<<"\n";
    std::ifstream inputFile_1(file1), inputFile_2(file2);
    std::ofstream out(outputFile);

    if (!inputFile_1.is_open() || !inputFile_2.is_open() || !out.is_open()) {
        std::cerr << "Error opening file: " << file1 << " " << file2 <<" "<< outputFile << std::endl;
        return;
    }
    
    std::string file_name1 = "";
    std::string file_name2 = "";
    bool hasSymbol1 = true;
    bool hasSymbol2 = true;

    std::string line1, line2;
    // Skip headers from input files
    std::getline(inputFile_1, line1);
    std::getline(inputFile_2, line2);

    // using header check if symbol is present in the file content
    if (not checkSymbolInFile(line1)) {
        file_name1 = fileNameFromPath(file1);
        hasSymbol1 = false;
    }
    if (not checkSymbolInFile(line2)) {
        file_name2 = fileNameFromPath(file2);;
        hasSymbol2 = false;
    }

    // add header in output file
    out << "Symbol, Timestamp, Price, Size, Exchange, Type\n";

    bool hasline1 = static_cast<bool>(std::getline(inputFile_1, line1));
    bool hasline2 = static_cast<bool>(std::getline(inputFile_2, line2));

    while(hasline1 || hasline2) {
        if (hasline1 && (!hasline2 || compare(line1, line2, file_name1, file_name2, hasSymbol1, hasSymbol2))) {
            if (!hasSymbol1)out << file_name1 << ", " << line1 << "\n";
            else out << line1 << "\n";
            hasline1 = static_cast<bool>(std::getline(inputFile_1, line1));
        }
        else {
            if (!hasSymbol2)out << file_name2 << ", " << line2 << "\n";
            else out << line2 << "\n";
            hasline2 = static_cast<bool>(std::getline(inputFile_2, line2));
        }
    }

    inputFile_1.close();
    inputFile_2.close();
    out.close();
}

// Use priority queue to merge data 
void mergeBatchUsingPriorityQueue(std::vector<std::string> inputFiles, std::vector<std::string>& outputFiles) {
    std::priority_queue<MarketData, std::vector<MarketData>, decltype(comparator)> queue(comparator);
    for (auto& file: inputFiles) {
        std::ifstream inputFile(file);
        std::string line;
        std::string file_name = fileNameFromPath(file);

        if (!inputFile.is_open()) {
            std::cerr << "Error opening file: " << file << std::endl;
            continue;
        }

        std::string regexPattern(R"(^\s*(.*?),\s*([\d.]+),\s*(\d+),\s*(.*?),\s*(.*)\s*$)");
        
        // get header
        std::getline(inputFile, line);
        bool hasSymbol = checkSymbolInFile(line);
        if(hasSymbol) {
            regexPattern = R"(^(\w+), ([\d-]+\s[\d:.]+), ([\d.]+), (\d+), ([^,]+), (\w+)$)";
        }

        std::regex pattern(regexPattern);
        std::smatch match;
        
        
        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, match, pattern)) {
                std::cout<<"matched record\n";
                MarketData record;
                if(!hasSymbol)record = MarketData(file_name, convertToUnixTimestamp(match[1]), match[1], match[4], std::stod(match[2]), std::stoi(match[3]), stringToType(match[5]));
                else record = MarketData(match[1], convertToUnixTimestamp(match[2]), match[2], match[5], std::stod(match[3]), std::stoi(match[4]), stringToType(match[6]));
                queue.push(record);
            }    
        }
        
        inputFile.close();
    }
    std::string outputFilePath = "temp_" + std::to_string(tempCounter++) + ".txt";

    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputFilePath << std::endl;
        return;
    }
    // Write headers
    outputFile << "Symbol, Timestamp, Price, Size, Exchange, Type\n";

    while (!queue.empty()) {
        auto record = queue.top();
        record.print();
        queue.pop();
        outputFile << record.symbol_ << ", " << record.timestamp_ << ", " << record.price_ << ", " << record.size_ << ", " << record.exchange_ << ", " << typeToString(record.type_) << std::endl;
    }
    outputFiles.emplace_back(outputFilePath);
}

// Use external merge algorithm to merge two files at a time
void mergeBatch(std::vector<std::string> inputFiles, std::vector<std::string>& outputFiles)
{
    for (size_t i = 0; i < inputFiles.size(); i+=2) {
        if (i + 1 < inputFiles.size()) {
            std::string outputFile = "temp_" + std::to_string(tempCounter++) + ".txt";
            mergeTwoFiles(inputFiles[i], inputFiles[i+1], outputFile);

            outputFiles.emplace_back(outputFile);

            // remove the files after merging
            std::remove(inputFiles[i].c_str());
            std::remove(inputFiles[i+1].c_str());
        }
        else {
            // single file remaining
            outputFiles.emplace_back(inputFiles[i]);
        }
    }
}

// Creates chunks and assigns threads to them for merging into single output file
// It can use different merging techinqes. implemented ones are 1. External merge algorithm 2. Using Priority Queue
// Current impl is using external merge algorithm logic 
void mergeMultipleFiles(std::vector<std::string>inputFiles, std::string& outputFile) {
    std::vector<std::string> currentFiles = inputFiles;
    int totalFiles = currentFiles.size();
    std::vector<std::thread> threads;
    size_t chunkSize = totalFiles / maxTotalThreads;
    std::mutex mutex;
    if(chunkSize <= 1) {
        chunkSize = totalFiles;
    }

    while(currentFiles.size() > 1) {
        std::vector<std::string> nextFiles;

        // process files in batches
        for (int i = 0; i < currentFiles.size(); i += chunkSize) {

            size_t endIndex = std::min(i + chunkSize, currentFiles.size());
            std::vector<std::string> batchFiles(currentFiles.begin() + i, currentFiles.begin() + endIndex);
            threads.emplace_back([batchFiles, &nextFiles, &mutex](){
                std::vector<std::string> batchOutput;
                mergeBatch(batchFiles, batchOutput); // Use external sort algorithm
                // mergeBatchUsingPriorityQueue(batchFiles, batchOutput); // comment out to use Priority Queue method for merging the batch

                std::lock_guard<std::mutex> lock(mutex);
                nextFiles.insert(nextFiles.end(), batchOutput.begin(), batchOutput.end());
            });
        }

        for (auto& thread : threads) {
            if(thread.joinable())thread.join();
        }

        currentFiles = nextFiles;
    }

    // final outputfile
    if (!currentFiles.empty()) {
        std::rename(currentFiles[0].c_str(), outputFile.c_str());
    }
}
