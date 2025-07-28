#include <algorithm>
#include <iostream> // input/output
#include <fstream> // file operation
#include <string> // use strings
#include <vector> // store variables to heap (dynamic allocation)
#include <sstream> //
#include <cctype> // to use isdigit
#include <ranges>

using namespace std;


bool checkString(const string& str, double& value) { // copy of string, make sure that the function won't change str
    try {
        size_t idx;
        value = stod(str, &idx); // init idx to the last char that is a number (or 0 if starts with char that is not a number)
        // value is passed by reference so it will be changed to the number until the first char in the string that is not a number
        // for example, after - value = stod("45A", &idx), value = 45, idx = 2.
        return idx == str.size(); // returns true if all the string is a number
    }
    catch (invalid_argument&) {
        return false;
    }
    catch (out_of_range&) {
        return false;
    }
}

int main() {
    string filePath;

    cout << "Enter CSV file path" << endl;
    getline(cin, filePath); // allows spaces in file path

    fstream file; // define file stream object
    file.open(filePath);

    if (!file.is_open()) {
        cout << "File did not open correctly" << endl;
        return -1;
    }
    string line;
    vector<vector<double>> array; // init 2D array of doubles

    int row = 0;
    int maxColumn = 0;
    while (getline(file, line)) { // copies line in file to the string variable named "line"
        vector<double> currRow;
        double rowSum = 0;
        stringstream ss(line); // this way, we can treat "line" as a file and access each cell
        string cell;
        int col = 0;
        while (getline(ss, cell, ',')) {
            double num = 0;
            if (checkString(cell, num)) {
                rowSum += num;
                currRow.push_back(num);
            }
            else { // if string not a valid number, push 0.
                currRow.push_back(0);
            }
            col++;
        }
        array.push_back(currRow);
        if (col > maxColumn) {
            maxColumn = col;
        }
        cout << "Total of line: " << rowSum << endl;
        row++;
    }
    file.close();

    cout << "Sums of columns:" << endl;
    vector<double> allColumnsSums;
    for (int col = 0; col < maxColumn ; col++) {
        double columnSum = 0;

        for (int row = 0; row < array.size(); row++) {
            if (array[row].size() > col) {
                columnSum += array[row][col];
            }
        }
        cout << columnSum << " ";
    }
}