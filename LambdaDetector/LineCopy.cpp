#pragma once

#include <fstream>
#include <string>
#include <iostream>
using std::ifstream;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;
using std::getline;
using std::stoi;

int main(const int argc, const char* argv[])
{
	const string DELIMITER = " line: ";
	const auto DELIMITER_LENGTH = DELIMITER.length();
	const string HTML_START_TAGS = "<!DOCTYPE html><html><head></head><body>";
	const string HTML_END_TAGS = "</body></html>";
	const auto NUMBER_OF_LINES = 8;

	// Open the file with the LambdaDetector-results.
	ifstream analysisFile("test.txt");
	string currentLine;

	// Create a HTML output file and initialize it.
	ofstream outputFile("linecopy_output.html", ofstream::out | ofstream::trunc);
	outputFile << HTML_START_TAGS;

	// Parse the LambdaDetector-results and output the preceding and 
	// following lines for the match.
	while (getline(analysisFile, currentLine)) {

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
		outputFile << "<pre><b>" << currentLine << "</b></pre>" << endl;

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
				outputFile << "<pre>";
				for (auto i = 0; i < NUMBER_OF_LINES; i++)
				{
					lineCounter++;
					if (getline(matchedFile, tmpStr))	// The number of rows may be past the end of the file
					{
						outputFile << "<b>" << lineCounter << "</b>" << ": ";
						if (lineCounter == rowNumberAsInt)
						{
							// Split string in to two, the second half will be made red and bold to easier notice it (after the [-character)
							auto charPos = tmpStr.find('[');
							outputFile << tmpStr.substr(0, charPos);
							outputFile << "<b><font color='red'>";
							outputFile << tmpStr.substr(charPos);
							outputFile << "</font></b>\n";
						}
						else
						{
							outputFile << tmpStr << '\n';
						}
					}
				}
				outputFile << "</pre>";
				break;
			}
		}
	}

	// "end" the HTML file
	outputFile << HTML_END_TAGS;

	// Close the files
	outputFile.close();
	analysisFile.close();
	
	return 0;
}
