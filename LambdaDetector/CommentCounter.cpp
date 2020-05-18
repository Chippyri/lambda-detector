#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <regex>
#include <algorithm>

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
using std::count;

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

bool containsStartingCurlyBrace(const string& tmpStr)
{
	return tmpStr.find('{') != string::npos;
}

bool containsEndingCurlyBrace(const string& tmpStr)
{
	return tmpStr.find('}') != string::npos;
}

int main(const int argc, const char* argv[])
{	
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
	ofstream aboveLambdaFile("comment_above_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream insideLambdaFile("comment_inside_lambda.txt", ofstream::out | ofstream::trunc);

	ofstream oneLinersFile("comment_one_liners.txt", ofstream::out | ofstream::trunc);
	ofstream testFile("comment_test.txt", ofstream::out | ofstream::trunc);
	ofstream faultFile("comment_faults.txt", ofstream::out | ofstream::trunc);
	
	// Counters
	int commentAboveLambda = 0;
	int commentInsideLambda = 0;
	int noCommentsAtAll = 0;
	int hadSomeComment = 0;
	int faults = 0;
	int lambdasChecked = 0;
	int oneLiners = 0;

	// Read from input file and copy the corresponding rows to a string to analyse
	string currentInputLine;
	while (getline(inputFile, currentInputLine))
	{
		lambdasChecked++;
		
		// Read the values from file
		const auto delimiterPosition = currentInputLine.find(DELIMITER);
		const auto pastDelimiterPosition = delimiterPosition + DELIMITER_LENGTH;
		const string filePath = currentInputLine.substr(0, delimiterPosition);
		const string rowNumberAsString = currentInputLine.substr(pastDelimiterPosition);
		const auto rowNumberAsInt = stoi(rowNumberAsString);

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
		bool lineWithLambdaStartChecked = false;
		bool lambdaHadACommentInside = false;
		bool lambdaHadACommentAbove = false;
		
		bool foundStartingBrace = false;
		bool foundEndingBrace = false;
		auto bracesCounter = 0;
						
		while (getline(fileToCopyFrom, tmpStr)) {
			inFileLineCounter++;
			if (inFileLineCounter >= startLineForCopying)
			{
				// Check the row before the lambda, does it have a comment?
				if (!lineBeforeChecked)
				{
					if (containsAComment(tmpStr))
					{
						aboveLambdaFile << currentInputLine << endl;
						aboveLambdaFile << tmpStr << endl;
						lambdaHadACommentAbove = true;
						commentAboveLambda++;
					}
					lineBeforeChecked = true;
					
				} else if (!lineWithLambdaStartChecked)
				{
					// Count the braces on the row
					auto startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');
					auto endingBraces = count(tmpStr.begin(), tmpStr.end(), '}');
					bracesCounter = startingBraces - endingBraces;

					//cout << bracesCounter;
					
					// Likely one-liners
					if (bracesCounter == 0 && startingBraces > 0)
					{
						oneLinersFile << tmpStr << endl;
						oneLiners++;
					}
					// No braces at all
					else if (bracesCounter == 0) {
						// TODO: Find the starting brace
						faultFile << tmpStr << endl;
						faults++;
					}
					// More start braces than end braces, ideal
					else if (bracesCounter == 1)
					{
						foundStartingBrace = true;
					}
					// Multiple start braces
					else if (bracesCounter > 1)
					{
						// TODO: Possibly just subtract by one?
						faultFile << tmpStr << endl;
						faults++;
					}
					// More end braces than start braces
					else
					{
						// TODO: Substract the end braces?
						faultFile << tmpStr << endl;
						faults++;
					}
					lineWithLambdaStartChecked = true;
				} else if(foundStartingBrace && !foundEndingBrace && bracesCounter != 0){

					// NOTE: Just a single comment inside each lambda
					if (!lambdaHadACommentInside && containsAComment(tmpStr))
					{
						lambdaHadACommentInside = true;
						insideLambdaFile << currentInputLine << endl;
						insideLambdaFile << tmpStr << endl;
						commentInsideLambda++;
					}
					
					bracesCounter += count(tmpStr.begin(), tmpStr.end(), '{');
					bracesCounter -= count(tmpStr.begin(), tmpStr.end(), '}');

					if (bracesCounter == 0)
					{
						foundEndingBrace = true;
						if (!lambdaHadACommentAbove && !lambdaHadACommentInside)
						{
							noCommentsAtAll++;
						} else if (lambdaHadACommentAbove || lambdaHadACommentInside)
						{
							hadSomeComment++;
						}
						
						cout << "f";
					}
				}
				else { break; }
			}
		}
	}

	aboveLambdaFile << "Count: " << commentAboveLambda << endl;
	insideLambdaFile << "Count: " << commentInsideLambda << endl;
	faultFile << "Count: " << faults << endl;

	cout << endl << "COMMENTS" << endl;
	cout << "Right above lambda: " << commentAboveLambda << endl;
	cout << "Inside the lambda: " << commentInsideLambda << endl;

	cout << "AMOUNTS" << endl;
	cout << "Lambdas checked: " << lambdasChecked << endl;
	cout << "One-liners: " << oneLiners << endl;
	cout << "Faults: " << faults << endl;
	cout << "No comments at all: " << noCommentsAtAll << endl;
	cout << "Had some comment: " << hadSomeComment << endl;
	
	return 0;
}
