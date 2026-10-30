/*
  Program name: EECS 348 Assignment 5 (revision of Assignment 4 Sudoku Solver)
  Description: Recursive, object-oriented backtracking Sudoku solver that finds ALL solutions
  Inputs: puzzle1.txt - puzzle5.txt (81 tokens each, digits 1-9 or "_" for empty), or filenames given as arguments
  Outputs: solution1.txt - solution5.txt (every solution in the input format, then "Solutions found: n",
           or the single line "No solution found.") plus a short summary on the console
  Collaborators: Eric Chen (original Assignment 4 author), Kimi K2.5 | Thinking (generated the original code)
  Other sources: Claude (Anthropic) helped with the revisions marked "edited by Josselyn" below
  Original author: Eric Chen
  Revised by: Josselyn Bui
  Revision date: 10/29/2026
  Notes: Compile on the Cycle servers with: g++ -std=c++17 -o Assignment5.exe Assignment5.cpp
*/

#include <array>    // fixed-size 9x9 grid type (original code)
#include <cctype>   // std::isdigit for checking tokens (original code)
#include <fstream>  // std::ifstream to read puzzles; std::ofstream to write solutions (ofstream added by Josselyn)
#include <iostream> // std::cout / std::cerr for console messages (original code)
#include <string>   // std::string for filenames and tokens (original code)
#include <vector>   // std::vector for the list of filenames (original code)

class Sudoku { // class holding the puzzle data and the recursive solver (original structure, Eric Chen / Kimi)
public:
    // read(): loads one puzzle file. Returns false only for an unopenable file or bad format.
    // edited by Josselyn with assistance from Claude: conflicting clues no longer make read() fail
    bool read(const std::string& filename) {
        name = filename;                          // remember the file name for messages (original)
        std::ifstream in(filename);               // open the puzzle file (original)
        if (!in) {                                // file could not be opened (original)
            std::cerr << "Could not open " << filename << "\n"; // tell the user (original)
            return false;                         // main counts this as a failed file (original)
        }

        grid.fill({});                            // clear every cell to 0 (original)
        std::string tok;                          // holds one whitespace-separated token (original)
        int count = 0;                            // how many tokens we have stored so far (original)

        while (in >> tok) {                       // read tokens until end of file (original)
            if (count >= 81) return false;        // more than 81 tokens means a bad puzzle (original)
            int value = 0;                        // value for this cell, 0 means empty (original)
            if (tok == "_") {                     // underscore marks an empty cell (original)
                value = 0;                        // keep it empty (original)
            } else if (tok.size() == 1 && std::isdigit(static_cast<unsigned char>(tok[0]))) { // single digit (original)
                value = tok[0] - '0';             // convert char to int (original)
                if (value < 1 || value > 9) return false; // 0 is not a legal clue (original)
            } else {
                return false;                     // anything else is a format error (original)
            }
            grid[count / 9][count % 9] = value;   // map linear index to row and column (original)
            ++count;                              // move to the next cell (original)
        }

        if (count != 81) return false;            // a puzzle needs exactly 81 cells (original)

        // edited by Josselyn with assistance from Claude: record whether the clues conflict instead of failing the read
        consistent = buildMasks();                // false if any clue repeats in a row, column or box
        return true;                              // file was readable and well formed
    }

    // run(): solves the puzzle and writes solutionN.txt
    // edited by Josselyn with assistance from Claude: writes to a file, uses the required messages
    void run(int puzzleNumber) {
        std::string outName = "solution" + std::to_string(puzzleNumber) + ".txt"; // e.g. solution3.txt
        std::ofstream out(outName);               // open the solution file for writing
        if (!out) {                               // could not create the file
            std::cerr << "Could not create " << outName << "\n"; // tell the user
            return;                               // nothing more we can do for this puzzle
        }

        solutions = 0;                            // reset the counter for this puzzle

        if (!consistent) {                        // clues already break the rules
            out << "No solution found.\n";        // required single line
            std::cout << name << ": No solution found.\n"; // console summary
            return;                               // skip the search
        }

        solve(0, out);                            // start recursion at cell index 0

        if (solutions == 0) {                     // search finished with nothing found
            out << "No solution found.\n";        // required single line
        } else {
            out << "Solutions found: " << solutions << "\n"; // required final line
        }
        std::cout << name << ": " << solutions << " solution(s) written to " << outName << "\n"; // console summary
    }

private:
    std::string name;                             // current puzzle file name (original)
    std::array<std::array<int, 9>, 9> grid{};     // the 9x9 grid, 0 = empty (original)
    bool consistent = true;                       // false if the starting clues conflict (original)
    int solutions = 0;                            // number of complete solutions found (original)

    // edited by Josselyn with assistance from Claude: bitmasks replace the repeated scans and vectors
    int rowMask[9] = {};                          // bit v is set when digit v is used in that row
    int colMask[9] = {};                          // bit v is set when digit v is used in that column
    int boxMask[9] = {};                          // bit v is set when digit v is used in that 3x3 box

    // buildMasks(): fills the masks from the clues, returns false on a duplicate
    // edited by Josselyn with assistance from Claude: replaces initialGridIsConsistent (one pass instead of nested loops)
    bool buildMasks() {
        for (int i = 0; i < 9; ++i) {             // reset all masks first
            rowMask[i] = colMask[i] = boxMask[i] = 0;
        }
        for (int r = 0; r < 9; ++r) {             // visit every row
            for (int c = 0; c < 9; ++c) {         // visit every column
                int v = grid[r][c];               // digit in this cell
                if (v == 0) continue;             // empty cells have no clue to check
                int bit = 1 << v;                 // bit that represents this digit
                int b = (r / 3) * 3 + (c / 3);    // which of the 9 boxes this cell is in
                if ((rowMask[r] & bit) || (colMask[c] & bit) || (boxMask[b] & bit)) {
                    return false;                 // digit already used in row, column or box
                }
                rowMask[r] |= bit;                // mark digit used in the row
                colMask[c] |= bit;                // mark digit used in the column
                boxMask[b] |= bit;                // mark digit used in the box
            }
        }
        return true;                              // no conflicts found
    }

    // solve(): recursive backtracking from cell index pos (0..80); prints each full solution to out
    // edited by Josselyn with assistance from Claude: takes a start position and uses masks (original logic by Eric Chen / Kimi)
    void solve(int pos, std::ostream& out) {
        while (pos < 81 && grid[pos / 9][pos % 9] != 0) ++pos; // skip cells that are already filled

        if (pos == 81) {                          // no empty cell left, so the grid is a solution
            if (solutions > 0) out << "\n";       // blank line between solutions
            ++solutions;                          // count this solution
            writeGrid(out);                       // write it in the input format
            return;                               // go back and look for more
        }

        int r = pos / 9;                          // row of the empty cell
        int c = pos % 9;                          // column of the empty cell
        int b = (r / 3) * 3 + (c / 3);            // box of the empty cell
        int used = rowMask[r] | colMask[c] | boxMask[b]; // digits that are not allowed here

        for (int v = 1; v <= 9; ++v) {            // try each digit in increasing order
            int bit = 1 << v;                     // bit for this digit
            if (used & bit) continue;             // illegal here, try the next digit
            grid[r][c] = v;                       // place the guess
            rowMask[r] |= bit;                    // update the masks to match
            colMask[c] |= bit;
            boxMask[b] |= bit;
            solve(pos + 1, out);                  // recurse to the next cell
            grid[r][c] = 0;                       // undo the guess (backtrack)
            rowMask[r] &= ~bit;                   // undo the mask updates
            colMask[c] &= ~bit;
            boxMask[b] &= ~bit;
        }
    }

    // writeGrid(): prints the grid as 9 lines of space-separated tokens, "_" for empty
    // edited by Josselyn with assistance from Claude: writes to any stream, no trailing spaces
    void writeGrid(std::ostream& out) const {
        for (int r = 0; r < 9; ++r) {             // one output line per row
            for (int c = 0; c < 9; ++c) {         // one token per column
                if (c > 0) out << ' ';            // space only between tokens
                if (grid[r][c] == 0) out << '_';  // empty cell
                else out << grid[r][c];           // digit
            }
            out << "\n";                          // end of row
        }
    }
};

// program entry point
// edited by Josselyn with assistance from Claude: defaults to puzzle1-5.txt, passes puzzle number to run()
int main(int argc, char** argv) {
    std::vector<std::string> files;               // puzzle file names to process (original)

    for (int i = 1; i < argc; ++i) {              // filenames may come from the command line (original)
        files.emplace_back(argv[i]);
    }

    if (files.empty()) {                          // nothing given, so use the assignment's file names
        for (int i = 1; i <= 5; ++i) {            // puzzle1.txt through puzzle5.txt
            files.push_back("puzzle" + std::to_string(i) + ".txt");
        }
    }

    int failedFiles = 0;                          // how many files were unreadable or badly formatted (original)
    int number = 0;                               // puzzle number used in solutionN.txt
    for (const std::string& filename : files) {   // process each puzzle (original)
        ++number;                                 // 1 for the first file, 2 for the second, ...
        Sudoku sudoku;                            // fresh solver object for each file (original)
        if (sudoku.read(filename)) {              // load and validate the format (original)
            sudoku.run(number);                   // solve and write solutionN.txt
        } else {
            ++failedFiles;                        // count it and keep going with the rest (original)
        }
    }

    if (failedFiles > 0) {                        // report problems at the end (original)
        std::cerr << "Finished with " << failedFiles
                  << " file(s) that had errors. Check the files for bad characters or formatting.\n";
        return 1;                                 // non-zero exit means something failed (original)
    }
    return 0;                                     // everything worked (original)
}