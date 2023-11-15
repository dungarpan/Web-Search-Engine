#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

using namespace std;

// Defining a structure to hold the inverted index entry
struct InvertedIndexEntry {
    int wordID;
    int fileNumber;
    vector<pair<int, int>> docOccurrences;
};

// Creating a minheap
struct CompareInvertedIndexEntry {
    bool operator()(const InvertedIndexEntry& a, const InvertedIndexEntry& b) {
        return a.wordID > b.wordID;
    }
};

int main() {
    // Opening 324 input files and 100 output files
    vector<ifstream> inputFiles(324);
    vector<ofstream> outputFiles(100);

    for (int i = 0; i < 324; i++) {
        string inputFileName = "inverted_index_" + to_string(i) + ".txt";
        inputFiles[i].open(inputFileName);
    }

    for (int i = 0; i < 100; i++) {
        string outputFileName = "final_inverted_index_" + to_string(i) + ".txt";
        outputFiles[i].open(outputFileName);
    }

    // Priority queue to keep track of the smallest word ID
    priority_queue<InvertedIndexEntry, vector<InvertedIndexEntry>, CompareInvertedIndexEntry> minHeap;

    // Initializing the priority queue with the first entry from each input file
    for (int i = 0; i < 324; i++) {
        InvertedIndexEntry entry;
        entry.fileNumber = i;
        if (inputFiles[i] >> entry.wordID) {
            cout<<i<<endl;
            string line;
            getline(inputFiles[i], line);
            size_t startPos = line.find('(');
            size_t endPos = line.find(')');

            while (startPos != string::npos && endPos != string::npos) {
                string pair = line.substr(startPos + 1, endPos - startPos - 1);
                int docID, occurrence;

                if (sscanf(pair.c_str(), "%d,%d", &docID, &occurrence) == 2) {
                    //cout<<"docID:"<<docID<<endl;
                    entry.docOccurrences.push_back({docID, occurrence});
                }
                startPos = line.find('(', endPos);
                endPos = line.find(')', startPos);
            }
            minHeap.push(entry);
        }
    }

    cout << "end first heap entry loop" << endl;


    int firstFlag = 0;
    int lastWordID = -1;
    // Merging the inverted index entries so that all docids belonging to a wordid is in the same line
    while (!minHeap.empty()) {
        InvertedIndexEntry minEntry = minHeap.top();
        minHeap.pop();

        // if current wordid and previous wordis is same write in the same line
        if (lastWordID == minEntry.wordID){
            int outputFileNumber = minEntry.wordID % 100;
            for (const auto& docOccurrence : minEntry.docOccurrences) {
                outputFiles[outputFileNumber] << "(" << docOccurrence.first << ", " << docOccurrence.second << ")";
            }
        }

        //else going to a new line
        else{
            int outputFileNumber = minEntry.wordID % 100;
            outputFiles[outputFileNumber] << endl;
            outputFiles[outputFileNumber] << minEntry.wordID << ": ";
            for (const auto& docOccurrence : minEntry.docOccurrences) {
                outputFiles[outputFileNumber] << "(" << docOccurrence.first << ", " << docOccurrence.second << ")";
            }
            
        }

        // Reading the next entry from the same input file
        InvertedIndexEntry nextEntry;
        int inputFileIndex = minEntry.fileNumber;
        if (inputFiles[inputFileIndex] >> nextEntry.wordID) {
            string line;
            getline(inputFiles[inputFileIndex], line);
            size_t startPos = line.find('(');
            size_t endPos = line.find(')');

            while (startPos != string::npos && endPos != string::npos) {
                string pair = line.substr(startPos + 1, endPos - startPos - 1);
                int docID, occurrence;

                if (sscanf(pair.c_str(), "%d,%d", &docID, &occurrence) == 2) {
                    //cout<<"docID:"<<docID<<endl;
                    nextEntry.docOccurrences.push_back({docID, occurrence});
                }
                startPos = line.find('(', endPos);
                endPos = line.find(')', startPos);
            }
            nextEntry.fileNumber = inputFileIndex; 
            //cout<<"nextEntry:"<<nextEntry.wordID<<endl;
            minHeap.push(nextEntry);
        }

        if (minEntry.wordID%10000==0){
            cout<<"wid:"<<minEntry.wordID<<endl;
        }
        lastWordID = minEntry.wordID;
        firstFlag = 1;
    }

    // Close all input and output files
    for (int i = 0; i < 324; i++) {
        inputFiles[i].close();
    }

    for (int i = 0; i < 100; i++) {
        outputFiles[i].close();
    }

    return 0;
}
