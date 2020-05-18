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

bool containsAComment(const string & tmpStr)
{
	return tmpStr.find("//") != string::npos || tmpStr.find("/*") != string::npos;
}

bool containsACurlyBrace(const string& tmpStr)
{
	return tmpStr.find('}') != string::npos;
}



int main(const int argc, const char* argv[])
{
	// Regexes
	
	// Input file
	const string INPUT_FILE_NAME = "test.txt";
	const string DELIMITER = " line: ";
	const auto DELIMITER_LENGTH = DELIMITER.length();



	// Open the file with the LambdaDetector-results.
	ifstream inputFile(INPUT_FILE_NAME);
	if (!inputFile.good())
	{
		return 1;
	}
	
	// Files to output to
	ofstream beforeLambdaFile("comment_before_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream onSameLineFile("comment_on_same_line.txt", ofstream::out | ofstream::trunc);
	ofstream inLambdaFile("comment_in_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream rightAfterLambdaFile("comment_right_after_lambda.txt", ofstream::out | ofstream::trunc);

	// Counters
	int commentOnLineBeforeLambda = 0;
	int commentOnSameRowAsLambda = 0;
	int commentInsideTheLambda = 0;
	int commentRightAfterLambda = 0;
	
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
		int startLineForCopying = rowNumberAsInt - 1;	// Check the line before
		if (startLineForCopying < 0) // Check that it is not beyond the beginning of the file
		{
			startLineForCopying = 0;
		}

		vector<string> linesFromFile;
		string tmpStr;
		auto inFileLineCounter = 0;

		bool lineBeforeChecked = false;
		bool lineWithLambdaChecked = false;
		bool linesInsideLambdaChecked = false;
		bool haveFoundACommentInsideLambda = false;
						
		while (getline(fileToCopyFrom, tmpStr)) {
			inFileLineCounter++;
			if (inFileLineCounter >= startLineForCopying)
			{
				// Check the row before the lambda, does it have a comment?
				if (!lineBeforeChecked)
				{
					if (containsAComment(tmpStr))
					{
						beforeLambdaFile << tmpStr << endl;
						commentOnLineBeforeLambda++;
					}
					lineBeforeChecked = true;
					
				} else if (!lineWithLambdaChecked)
				{
					// Check the row with the lambda, does it have a comment?
					if(containsAComment(tmpStr))
					{
						commentOnSameRowAsLambda++;
						onSameLineFile << tmpStr << endl;
					}
					lineWithLambdaChecked = true;
					
				}
				else if (!linesInsideLambdaChecked)
				{
					// Does the lambda end?
					if (containsACurlyBrace(tmpStr))
					{
						// Check right after the lambda ending brace, does it have a comment?
						linesInsideLambdaChecked = true;
						if(containsAComment(tmpStr))
						{
							commentRightAfterLambda++;
							rightAfterLambdaFile << tmpStr << endl;
						}
					} else
					{
						// Check inside the lambda, does it have a comment?
						if (!haveFoundACommentInsideLambda && containsAComment(tmpStr))
						{
							haveFoundACommentInsideLambda = true;
							inLambdaFile << tmpStr << endl;
							commentInsideTheLambda++;
						}
					}
				}
				else { break; }
			}
		}
	}
	cout << "Before: " << commentOnLineBeforeLambda << endl;
	cout << "On the same line: " << commentOnSameRowAsLambda << endl;
	cout << "Inside lambda: " << commentInsideTheLambda << endl;
	cout << "Right after end of lambda: " << commentRightAfterLambda << endl;
	
	return 0;
}
