#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>

using namespace std;

// Defining a struct to store document information
struct DocumentInfo {
    int docID;
    string url;
    int numTerms;
    vector<int> terms; // Now stores word IDs as integers
};

// Function to tokenize a string into words
vector<string> tokenize(const string& text) {
    vector<string> words;
    istringstream iss(text);
    string word;
    while (iss >> word) {
        // Remove punctuation and convert to lowercase
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        words.push_back(word);
    }
    return words;
}

int main() {
    const string inputFileName = "fulldocs-new.trec";  // Replace with your file path

    // Set the number of documents per inverted index file
    const int documentsPerIndex = 10000;

    unordered_map<int, DocumentInfo> docIdToInfo;
    unordered_map<string, int> wordToId;
    unordered_map<int, vector<pair<int, int>>> invertedIndex; // Word ID to (Doc ID, Term Count)

    int docID = 0;
    int wordID = 0;
    int invertedIndexFileNumber = 312; // Counter for inverted index files

    ifstream inputFile(inputFileName);
    if (!inputFile) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    string line, documentContent, url;
    int lineCount = 0;  // Line counter
    bool isInsideText = false;

    while (getline(inputFile, line)) {
        if (line == "<DOC>") {
            documentContent = line;
            url = "";  // Reset URL
            isInsideText = false;
            while (getline(inputFile, line)) {
                documentContent += line;
                if (line == "</DOC>")
                    break;

                if (isInsideText && url.empty()) {
                    url = line;  // Store the URL
                }

                if (line == "<TEXT>") {
                    isInsideText = true;
                }
                lineCount++;
            }

            if (!url.empty()) {
                size_t textStart = documentContent.find("<TEXT>");
                size_t textEnd = documentContent.find("</TEXT>");
                if (textStart != string::npos && textEnd != string::npos) {
                    string textContent = documentContent.substr(textStart + 6, textEnd - textStart - 6);
                    vector<string> words = tokenize(textContent);

                    DocumentInfo currentDoc;
                    currentDoc.docID = docID;
                    currentDoc.numTerms = words.size();
                    currentDoc.url = url;
                    for (const string& word : words) {
                        if (wordToId.find(word) == wordToId.end()) {
                            wordToId[word] = wordID;
                            wordID++;
                        }

                        int currentWordID = wordToId[word];
                        bool found = false;

                        // Check if this word and doc pair already exists
                        if (docID>=3100000){
                            for (auto& docTermPair : invertedIndex[currentWordID]) {
                                if (docTermPair.first == docID) {
                                    docTermPair.second++; // Increment term count
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                invertedIndex[currentWordID].push_back({docID, 1}); // Add a new pair
                            }
                        }
                        currentDoc.terms.push_back(currentWordID);
                    }

                    docID++;
                    if(docID % 500 == 0)
                        cout<<docID<<endl;
                    if (docID>=3100000){
                        if (docID % documentsPerIndex == 0) {
                            // Save the inverted index to a new file
                            string invertedIndexFileName = "inverted_index_" + to_string(invertedIndexFileNumber) + ".txt";
                            ofstream invertedIndexFile(invertedIndexFileName);
                            for (const auto& entry : invertedIndex) {
                                invertedIndexFile << entry.first << ": ";
                                for (const auto& docTermPair : entry.second) {
                                    invertedIndexFile << "(" << docTermPair.first << ", " << docTermPair.second << ")";
                                }
                                invertedIndexFile << endl;
                            }
                            invertedIndexFile.close();

                            // Increment the inverted index file number
                            invertedIndexFileNumber++;

                            // Clear the inverted index for the next batch of documents
                            invertedIndex.clear();
                        }
                    }
                    docIdToInfo[docID] = currentDoc;
                }
            }
            lineCount++;  // Increment line counter
        }
    }

    inputFile.close();

    // Save invertedIndex to a file
    /*
    ofstream invertedIndexFile("inverted_index.txt");
    for (const auto& entry : invertedIndex) {
        invertedIndexFile << entry.first << ": ";
        for (const auto& docTermPair : entry.second) {
            invertedIndexFile << "(" << docTermPair.first << ", " << docTermPair.second << ")";
        }
        invertedIndexFile << endl;
    }
    invertedIndexFile.close();
    cout<<"Finished inverted index write"<<endl;
    */

    // Save wordToId to a file
    ofstream wordToIdFile("word_to_id_new.txt");
    for (const auto& entry : wordToId) {
        wordToIdFile << entry.first << ": " << entry.second << endl;
    }
    wordToIdFile.close();
    cout<<"Finished word to id write"<<endl;

    // Now you can write the currentDoc information to a CSV file
    ofstream currentDocFile("current_doc_new.txt");
    currentDocFile << "DocID,URL,NumTerms,WordIDs" << endl;
    for (const auto& pair : docIdToInfo) {
        const DocumentInfo& docInfo = pair.second;
        currentDocFile << docInfo.docID << "," << docInfo.url << "," << docInfo.numTerms << ",";
        for (size_t i = 0; i < docInfo.terms.size(); ++i) {
            currentDocFile << docInfo.terms[i];
            if (i < docInfo.terms.size() - 1) {
                currentDocFile << ",";
            }
        }
        currentDocFile << endl;
    }
    currentDocFile.close();
    cout<<"Finished current doc write"<<endl;

    return 0;
}
