#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/replace.hpp>

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
	const auto NUMBER_OF_LAMBDAS_IN_FILE = 1000;
	const auto MAXIMUM_NUMBER_OF_OUTPUT_FILES = 100;
	const string ANALYSIS_FILE_NAME = "test.txt";
	const string OUTPUT_FILE_PREFIX = "linecopy_output";
	
	// Open the file with the LambdaDetector-results.
	ifstream analysisFile(ANALYSIS_FILE_NAME);
	if (!analysisFile.good())
	{
		return 1;
	}
	
	string currentLine;
	
	// Create the first HTML output file and initialize it.
	int lineCounter = 1;
	int fileCounter = 0;
	string fileName = OUTPUT_FILE_PREFIX + std::to_string(fileCounter) + ".html";
	ofstream outfile[MAXIMUM_NUMBER_OF_OUTPUT_FILES];
	outfile[fileCounter].open(fileName, ofstream::out | ofstream::trunc);
	outfile[fileCounter] << HTML_START_TAGS;

	// Parse the LambdaDetector-results and output the preceding and following lines for the match.
	while (getline(analysisFile, currentLine)) {

		// If maximum number of lines in a file has been reached, end the file and open the next one.
		if (lineCounter % NUMBER_OF_LAMBDAS_IN_FILE == 0) {

			// Close the current file.
			outfile[fileCounter] << HTML_END_TAGS;
			outfile[fileCounter].close();
			fileCounter++;

			// Just so that we don't go past the end of the array
			if (fileCounter >= MAXIMUM_NUMBER_OF_OUTPUT_FILES)
			{
				break;
			}
			
			// Create and open a new file in the array.
			fileName = OUTPUT_FILE_PREFIX + std::to_string(fileCounter) + ".html";
			outfile[fileCounter].open(fileName, ofstream::out | ofstream::trunc);
			outfile[fileCounter] << HTML_START_TAGS;
		}
		
		// Get the strings before and after the delimiter, not including the delimiter.
		const auto delimiterPosition = currentLine.find(DELIMITER);
		const auto pastDelimiterPosition = delimiterPosition + DELIMITER_LENGTH;
		const string filePath = currentLine.substr(0, delimiterPosition);
		const string rowNumberAsString = currentLine.substr(pastDelimiterPosition);

		// Get the parsed line number as a number.
		const auto rowNumberAsInt = stoi(rowNumberAsString);

		// Print the current line to cout so that we know there is something happening.
		cout << currentLine << endl;
		
		// Print the exact line (path + row) from the LambdaDetector log to HTML output.
		outfile[fileCounter] << "<pre><b>" << lineCounter << " " << currentLine << "</b></pre>" << endl;

		// Open the file located in the filepath.
		ifstream matchedFile(filePath);
		auto inFileLineCounter = 0;
		string tmpStr;

		// Calculate on which row to start if we want preceding lines
		int startLine = rowNumberAsInt - NUMBER_OF_LINES/2;					// Center on line
		// Check that it is not beyond the beginning of the file
		if (startLine < 0)
		{
			startLine = 0;
		}

		while (getline(matchedFile, tmpStr)) {
			inFileLineCounter++;

			// Once the starting line is found, start copying the lines of code into the HTML-file.
			if (inFileLineCounter == startLine)
			{
				outfile[fileCounter] << "<pre>";
				for (auto i = 0; i < NUMBER_OF_LINES; i++)
				{
					inFileLineCounter++;
					if (getline(matchedFile, tmpStr))	// The number of rows may be past the end of the file
					{
						// Write out the line number copied
						outfile[fileCounter] << "<b>" << inFileLineCounter << "</b>" << ": ";

						// Replace all characters which could be construed as HTML
						boost::replace_all(tmpStr, "&", "&amp;");
						boost::replace_all(tmpStr, "<", "&lt;");
						boost::replace_all(tmpStr, ">", "&gt;");
						
						// If the matched line is the current one, highlight the start of the lambda
						if (inFileLineCounter == rowNumberAsInt)
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
							// Does the line contain the characters //? If so, mark everything starting from them
							// on that line in green.
							if (tmpStr.find("//") != string::npos)
							{
								const int foundPos = tmpStr.find("//");
								outfile[fileCounter] << tmpStr.substr(0, foundPos);
								outfile[fileCounter] << "<font color='green'>" << tmpStr.substr(foundPos) << "</font>\n";
							} else
							{
								outfile[fileCounter] << tmpStr << '\n';
							}
						}
					}
				}
				outfile[fileCounter] << "</pre>";
				break;
			}
		}
		lineCounter++;
	}

	// "end" the HTML file
	outfile[fileCounter] << HTML_END_TAGS;

	// Close the files
	outfile[fileCounter].close();
	analysisFile.close();
	
	return 0;
}
