#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
using std::ifstream;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;
using std::getline;
using std::stoi;
int counter = 1;

int main(const int argc, const char* argv[])
{
	const string DELIMITER = " line: ";
	const auto DELIMITER_LENGTH = DELIMITER.length();
	const string HTML_START_TAGS = "<!DOCTYPE html><html><head></head><body>";
	const string HTML_END_TAGS = "</body></html>";
	const auto NUMBER_OF_LINES = 8;
	const int NUMBEROFLAMBDASINFILE = 1000;

	// Open the file with the LambdaDetector-results.
	ifstream analysisFile("test.txt");
	string currentLine;
	
	// Create a HTML output file and initialize it.
	int fileCounter = 0;
	string fileName = "linecopy_output" + std::to_string(fileCounter) +".html";

	ofstream outputFile(fileName, ofstream::out | ofstream::trunc);
	outputFile << HTML_START_TAGS;

	// Parse the LambdaDetector-results and output the preceding and 
	// following lines for the match.
	
	ofstream outfile[20];
	outfile[fileCounter].open(fileName, ofstream::out | ofstream::trunc);
	outfile[fileCounter] << HTML_START_TAGS;
	while (getline(analysisFile, currentLine)) {

		if (counter % NUMBEROFLAMBDASINFILE == 0) {
			outfile[fileCounter] << HTML_START_TAGS;
			outfile[fileCounter].close();
			fileCounter++;
			fileName = "linecopy_output" + std::to_string(fileCounter) + ".html";
			outfile[fileCounter].open(fileName, ofstream::out | ofstream::trunc);
			outfile[fileCounter] << HTML_START_TAGS;
		}
		// Get the strings before and after the delimiter, not including the delimiter.
		const auto delimiterPosition = currentLine.find(DELIMITER);
		const auto pastDelimiterPosition = delimiterPosition + DELIMITER_LENGTH;
		const string filePath = currentLine.substr(0, delimiterPosition);
		const string rowNumberAsString = currentLine.substr(pastDelimiterPosition);

		// Get the found line number as a number.
		const auto rowNumberAsInt = stoi(rowNumberAsString);

		// Print the current line to cout so that we know there is something happening
		cout << currentLine << endl;
		
		// Print the exact line (path + row) from the LambdaDetector log to HTML output
		outfile[fileCounter] << "<pre><b>" << counter << " " << currentLine << "</b></pre>" << endl;

		// Open the file located in the filepath
		ifstream matchedFile(filePath);
		auto lineCounter = 0;
		string tmpStr;

		// Calculate on which row to start if we want preceding lines
		int startLine = rowNumberAsInt - NUMBER_OF_LINES/2;					// Center on line
		// Check that it is not beyond the beginning of the file
		if (startLine < 0)
		{
			startLine = 0;
		}

		while (getline(matchedFile, tmpStr)) {
			lineCounter++;
			if (lineCounter == startLine)
			{
				outfile[fileCounter] << "<pre>";
				for (auto i = 0; i < NUMBER_OF_LINES; i++)
				{
					lineCounter++;
					if (getline(matchedFile, tmpStr))	// The number of rows may be past the end of the file
					{
						outfile[fileCounter] << "<b>" << lineCounter << "</b>" << ": ";
						if (lineCounter == rowNumberAsInt)
						{
							// Split string in to two, the second half will be made red and bold to easier notice it (after the [-character)
							auto charPos = tmpStr.find('[');
							outfile[fileCounter] << tmpStr.substr(0, charPos);
							outfile[fileCounter] << "<b><font color='red'>";
							outfile[fileCounter] << tmpStr.substr(charPos);
							outfile[fileCounter] << "</font></b>\n";
						}
						else
						{
							outfile[fileCounter] << tmpStr << '\n';
						}
					}
				}
				outfile[fileCounter] << "</pre>";
				break;
			}
		}
		counter++;
	}

	// "end" the HTML file
	outfile[fileCounter] << HTML_END_TAGS;

	// Close the files
	outfile[fileCounter].close();
	analysisFile.close();
	
	return 0;
}
