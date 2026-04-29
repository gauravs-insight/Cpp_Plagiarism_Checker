#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <iomanip>

using namespace std;

class Document {
private:
    string filename;
    string content;
    vector<string> words;
    unordered_set<string> unique_words;
    unordered_set<string> stop_words = {"a", "an", "the"};

    void normalizeAndTokenize() {
        string current_word = "";
        auto processWord = [&](string& word) {
            if (!word.empty()) {
                if (stop_words.find(word) == stop_words.end()) {
                    words.push_back(word);
                    unique_words.insert(word);
                }
                word = "";
            }
        };

        for (char c : content) {
            if (isalnum(c)) {
                current_word += (char)tolower(c);
            } else {
                processWord(current_word);
            }
        }
        processWord(current_word);
    }

public:
    Document(const string& fname) : filename(fname) {
        ifstream file(fname);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                content += line + " ";
            }
            file.close();
        }
        normalizeAndTokenize();
    }

    const vector<string>& getWords() const { return words; }
    const unordered_set<string>& getUniqueWords() const { return unique_words; }
    int getWordCount() const { return words.size(); }
    int getUniqueWordCount() const { return unique_words.size(); }
    bool isEmpty() const { return words.empty(); }
};

class TerminalRenderer {
public:
    static void printHeader() {
        cout << "\n============================================\n";
        cout << "   PlagCheck - Terminal Plagiarism Checker  \n";
        cout << "============================================\n\n";
    }

    static void printStats(const Document& d1, const Document& d2, int matches, int total_unique) {
        cout << "--- Analysis Statistics ---\n";
        cout << left << setw(25) << "Total Words File 1:" << d1.getWordCount() << endl;
        cout << left << setw(25) << "Total Words File 2:" << d2.getWordCount() << endl;
        cout << left << setw(25) << "Unique Words File 1:" << d1.getUniqueWordCount() << endl;
        cout << left << setw(25) << "Unique Words File 2:" << d2.getUniqueWordCount() << endl;
        cout << left << setw(25) << "Matched Unique Words:" << matches << endl;
        cout << left << setw(25) << "Union (Total Unique):" << total_unique << endl;
        cout << "---------------------------\n\n";
    }

    static void printResult(double similarity) {
        cout << ">>> FINAL SIMILARITY SCORE: ";
        cout << fixed << setprecision(2) << similarity << "%\n";
        
        cout << "Verdict: ";
        if (similarity >= 80.0) cout << "HIGH PLAGIARISM DETECTED!\n";
        else if (similarity >= 40.0) cout << "Moderate similarity found.\n";
        else if (similarity >= 10.0) cout << "Low similarity.\n";
        else cout << "Minimal / No plagiarism detected.\n";
        cout << "\n============================================\n";
    }

    static void printMatches(const vector<string>& matches) {
        if (matches.empty()) return;
        cout << "Common Words: ";
        for (size_t i = 0; i < min(matches.size(), (size_t)20); ++i) {
            cout << matches[i] << (i == matches.size() - 1 || i == 19 ? "" : ", ");
        }
        if (matches.size() > 20) cout << "... (and " << matches.size() - 20 << " more)";
        cout << "\n\n";
    }

    static void printError(const string& msg) {
        cerr << "[ERROR] " << msg << endl;
    }
};

int main(int argc, char* argv[]) {
    TerminalRenderer::printHeader();

    if (argc < 3) {
        TerminalRenderer::printError("Insufficient arguments.");
        cout << "Usage: cli_plagchecker.exe <file1> <file2>\n";
        return 1;
    }

    string path1 = argv[1];
    string path2 = argv[2];

    Document doc1(path1);
    Document doc2(path2);

    if (doc1.isEmpty() && doc2.isEmpty()) {
        TerminalRenderer::printError("Both files are empty or could not be read.");
        return 1;
    }

    const auto& set1 = doc1.getUniqueWords();
    const auto& set2 = doc2.getUniqueWords();
    vector<string> matched_words;

    for (const string& word : set1) {
        if (set2.find(word) != set2.end()) {
            matched_words.push_back(word);
        }
    }
    sort(matched_words.begin(), matched_words.end());

    int union_count = set1.size() + set2.size() - matched_words.size();
    if (union_count <= 0) union_count = 1;

    double similarity = ((double)matched_words.size() / (double)union_count) * 100.0;

    TerminalRenderer::printMatches(matched_words);
    TerminalRenderer::printStats(doc1, doc2, matched_words.size(), union_count);
    TerminalRenderer::printResult(similarity);

    return 0;
}
