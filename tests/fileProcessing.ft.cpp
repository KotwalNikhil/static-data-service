#include <gtest/gtest.h>
#include <filesystem>
#include "../src/staticDataService.hpp"

namespace fs = std::filesystem;

class FileProcessingTest : public ::testing::Test {
protected:
    std::vector<std::string> testFiles;
    std::string outputFilePath = "test_output.txt";

    void SetUp() override {
        // Create test files with sorted content
        for (size_t i = 0; i < 512; ++i) {
            std::string fileName = "test_file_" + std::to_string(i) + ".txt";
            std::ofstream file(fileName);

            // add header
            file << "Timestamp, Price, Size, Exchange, Type\n";
            for (size_t j = 0; j < 10; ++j) {
                file << std::to_string(1970 + (i * 10 + j)) + "-03-05 10:00:00.123, 228.5, 120, NYSE, TRADE\n";
            }
            file.close();
            testFiles.push_back(fileName);
        }
    }

    // Removes all the generated test_files after completing test fixture
    void TearDown() override {
        for (const auto& file : testFiles) {
            fs::remove(file);
        }
        fs::remove(outputFilePath);
    }
};

TEST_F(FileProcessingTest, MergeSortedFiles) {

    mergeMultipleFiles(testFiles, outputFilePath);

    std::ifstream outputFile(outputFilePath);
    ASSERT_TRUE(outputFile.is_open());

    std::string line;
    size_t lineCount = 0;
    std::getline(outputFile, line);
    // Check header
    ASSERT_EQ(line, "Symbol, Timestamp, Price, Size, Exchange, Type");

    while (std::getline(outputFile, line)) {
        ASSERT_EQ(line, "test_file_" + std::to_string(lineCount/10) + ", " + std::to_string(1970 + (lineCount)) + "-03-05 10:00:00.123, 228.5, 120, NYSE, TRADE");
        ++lineCount;
    }

    ASSERT_EQ(lineCount, 5120); // 512 files with 5120 lines each
    outputFile.close();
}
