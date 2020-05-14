#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <regex>

using std::ifstream;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;
using std::getline;
using std::stoi;
using std::vector;
using std::regex;
using std::regex_search;

bool searchAndWriteToFileIfExisted(const string& match, const regex& reg, ofstream& file, const string& path)
{
	if (regex_search(match, reg))
	{
		file << path << endl;
		file << match << endl;
		return true;
	}

	return false;
}



int main(const int argc, const char* argv[])
{
	// Regexes
	const string assignRegexString = R"(\=\s*\[)";
	regex assignRegex(assignRegexString);

	const string returnRegexString = R"(return\s+\[)";
	regex returnRegex(returnRegexString);

	const string argumentCommaRegexString = R"(,\s*\[)";
	regex argumentCommaRegex(argumentCommaRegexString);

	const string argumentParenthesisRegexString = R"(\(\s*\[)";
	regex argumentParenthesisRegex(argumentParenthesisRegexString);
	
	// Input file
	const string INPUT_FILE_NAME = "test.txt";
	const string DELIMITER = " line: ";
	const auto DELIMITER_LENGTH = DELIMITER.length();

	// Output file
	const string ASSIGN_FILE_NAME = "categorizer_assign_output.txt";
	const string RETURN_FILE_NAME = "categorizer_return_output.txt";
	const string ARGUMENT_COMMA_FILE_NAME = "categorizer_comma_arg_output.txt";
	const string ARGUMENT_PARENTHESIS_OUTPUT_FILE_NAME = "categorizer_parenthesis_arg_output.txt";
	const string OTHER_OUTPUT_FILE_NAME = "categorizer_other_output.txt";
	
	// How many lines to copy and inspect
	const auto NUMBER_OF_LINES = 8;

	// Open the file with the LambdaDetector-results.
	ifstream inputFile(INPUT_FILE_NAME);
	if (!inputFile.good())
	{
		return 1;
	}
	
	// File to output to
	ofstream assignOutputFile(ASSIGN_FILE_NAME, ofstream::out | ofstream::trunc);
	ofstream returnOutputFile(RETURN_FILE_NAME, ofstream::out | ofstream::trunc);
	ofstream argCommaOutputFile(ARGUMENT_COMMA_FILE_NAME, ofstream::out | ofstream::trunc);
	ofstream argParenthesisOutputFile(ARGUMENT_PARENTHESIS_OUTPUT_FILE_NAME, ofstream::out | ofstream::trunc);
	ofstream otherOutputFile(OTHER_OUTPUT_FILE_NAME, ofstream::out | ofstream::trunc);

	// Read from input file and copy the corresponding rows to a string to analyse
	string currentInputLine;
	while (getline(inputFile, currentInputLine))
	{
		// Read the values from file
		const auto delimiterPosition = currentInputLine.find(DELIMITER);
		const auto pastDelimiterPosition = delimiterPosition + DELIMITER_LENGTH;
		const string filePath = currentInputLine.substr(0, delimiterPosition);
		const string rowNumberAsString = currentInputLine.substr(pastDelimiterPosition);
		const auto rowNumberAsInt = stoi(rowNumberAsString);

		cout << currentInputLine << endl;

		// Open file and read specified rows
		ifstream fileToCopyFrom(filePath);
		int startLineForCopying = rowNumberAsInt - (NUMBER_OF_LINES / 2);
		if (startLineForCopying < 0) // Check that it is not beyond the beginning of the file
		{
			startLineForCopying = 0;
		}

		vector<string> linesFromFile;
		string tmpStr;
		auto inFileLineCounter = 0;
						
		while (getline(fileToCopyFrom, tmpStr)) {
			inFileLineCounter++;
			if (inFileLineCounter == startLineForCopying)
			{
				linesFromFile.push_back(tmpStr);
				for (auto i = 0; i < NUMBER_OF_LINES; i++)
				{
					inFileLineCounter++;
					if (getline(fileToCopyFrom, tmpStr))	// The number of rows may be past the end of the file
					{
						linesFromFile.push_back(tmpStr);	// Copy to vector
					}
				}

				// Write all lines to file
				/*
				outputFile << currentInputLine << endl;
				for (const string& line : linesFromFile)
				{
					outputFile << line << endl;
				}
				*/

				/*
				int foundPos;
				if ((foundPos = linesFromFile.at(4).find('=')) != string::npos)
				{	
					outputFile << currentInputLine << endl;
					outputFile << linesFromFile.at(4) << endl;
				}
				*/
				string lambdaLine = linesFromFile.at(4);
				
				if (searchAndWriteToFileIfExisted(lambdaLine, returnRegex, returnOutputFile, currentInputLine))
				{
					
				} else if (searchAndWriteToFileIfExisted(lambdaLine, assignRegex, assignOutputFile, currentInputLine))
				{
					
				} else if (searchAndWriteToFileIfExisted(lambdaLine, argumentCommaRegex, argCommaOutputFile, currentInputLine))
				{
					
				} else if (searchAndWriteToFileIfExisted(lambdaLine, argumentParenthesisRegex, argParenthesisOutputFile, currentInputLine))
				{

				} else
				{
					otherOutputFile << currentInputLine << endl;
					otherOutputFile << lambdaLine << endl;
				}
				
				// Clear vector
				linesFromFile.clear();
				break;
			}
		}
	}
	return 0;
}
