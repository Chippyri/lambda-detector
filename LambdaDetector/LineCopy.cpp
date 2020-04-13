#pragma once

#include <fstream>
#include <string>
#include <iostream>
using std::ifstream;
using std::string;
using std::cout;
using std::endl;
using std::getline;
using std::stoi;

int main(const int argc, const char* argv[])
{
	ifstream testFile("test.txt");
	string firstLine;
	string delimiter = " line: ";
	while (getline(testFile, firstLine)) {
		cout << "-------------------------------------------------" << endl;
		auto position = firstLine.find(delimiter);
		auto endPosition = position + 7;
		string token = firstLine.substr(0, position);
		string endToken = firstLine.substr(endPosition);
		cout << firstLine << endl;
		cout << token << endl;
		cout << endToken << endl;
		auto value = stoi(endToken);
		cout << value << endl;

		ifstream firstFile(token);
		auto linecounter = 0;
		string tempStr;

		int numberOfLines = 8;
		int startLine = value - (numberOfLines / 2); // Center on line

		if (startLine >= -1)
		{
			while (getline(firstFile, tempStr)) {
				linecounter++;
				if (linecounter == startLine)
				{
					cout << tempStr << endl;
					for (auto i = 0; i < numberOfLines; i++)
					{
						getline(firstFile, tempStr);
						cout << tempStr << endl;
					}
				}
			}
		}
		cout << "-------------------------------------------------" << endl;
	}
	testFile.close(); // place last in code
	return 0;
}
