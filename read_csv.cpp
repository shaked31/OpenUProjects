#include <algorithm>
#include <iostream> // input/output
#include <fstream> // file operation
#include <string> // use strings
#include <vector> // store variables to heap (dynamic allocation)
#include <sstream> // lets
#include <cctype> // to use isdigit

using namespace std;

bool checkString(const string& str) { // copy of string, make sure that the function won't change str
    if (str.empty()) {
        return false;
    }
    if (str[0] == '-') {
        if (str.size() == 1) {
            return false;
        }
    }

    int countPoints = 0;
    for (int i = 1 ; i < str.size() ; i++) {
        // if (str[i] == '.') {
        //     continue;
        //     countPoints++;
        // }
        // if (!isdigit(str[i])) {
        //     return false;
        // }
        // check if digits and doesnt have more than one decimal point
    }
    return true;
}

int main() {
    string filePath;
    // cout << "Enter CSV file path" << endl;
    // getline(cin, filePath); // allows spaces in file path

    fstream file; // define file stream object
    // file.open(filePath);
    file.open("C:\\Users\\hanoi\\Desktop\\maman11\\example_no_bom.csv");
    if (!file.is_open()) {
        cout << "File did not open correctly" << endl;
        return -1;
    }
    string line;
    vector<vector<double>> array; // init 2D array

    int row = 0;

    while (getline(file, line)) { // copies line in pFile to the string variable named "line"
        array.push_back(vector<double>());
        double rowSum = 0;
        stringstream ss(line); // this way, we can treat "line" as a file
        string cell;
        int col = 0;
        while (getline(ss, cell, ',')) {
            if (checkString(cell)) {
                double num = stoi(cell);
                cout << num << endl;
                array[row].push_back(num);
                rowSum += num;
                col++;
            }
        }
        cout << "Total of line: " << rowSum << endl;
        row++;
    }
    file.close();

    cout << "Sums of columns:" << endl;
    vector<double> allColumnsSums;
    for (int i = 0 ; i < array.size() ; i++) {
        double columnSum = 0;
        for (int j = 0 ; j < array[i].size() ; j++) {
            columnSum += array[i][j];
        }
        cout << columnSum;
    }
}