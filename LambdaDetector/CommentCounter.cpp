#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
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
	ofstream oneLinersCommentsFile("comment_one_liners_with_comments.txt", ofstream::out | ofstream::trunc);
	ofstream faultFile("comment_faults.txt", ofstream::out | ofstream::trunc);

	ofstream aboveLambdaPathFile("comment_above_paths_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream insideLambdaPathFile("comment_inside_paths_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream noCommentPathFile("comment_none_paths_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream commentAfterOneLinerFile("comment_after_one_liner_lambda.txt", ofstream::out | ofstream::trunc);
	
	// Counters
	int commentAboveLambda = 0;
	int commentInsideLambda = 0;
	int noCommentsAtAll = 0;
	int hadSomeComment = 0;
	int hadBothComment = 0;
	int faults = 0;
	int lambdasChecked = 0;
	int oneLiners = 0;
	int couldNotOpenFile = 0;
	int oneLinerAfterComment = 0;
	
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

		// Open file and read specified rows
		ifstream fileToCopyFrom(filePath);

		if (!fileToCopyFrom.good())
		{
			couldNotOpenFile++;
			continue;
		}

		lambdasChecked++;
		
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
		bool isOneLiner = false;
		auto bracesCounter = 0;

		string tmpStorageForOneLiner;
		
		while (getline(fileToCopyFrom, tmpStr)) {
			inFileLineCounter++;
			if (inFileLineCounter >= startLineForCopying)
			{
				// Check the row before the lambda, does it have a comment?
				if (!lineBeforeChecked)
				{
					if (containsAComment(tmpStr))
					{
						aboveLambdaPathFile << currentInputLine << endl;
						aboveLambdaFile << tmpStr << endl;
						lambdaHadACommentAbove = true;
						commentAboveLambda++;
					}
					lineBeforeChecked = true;
					
				} else if (!lineWithLambdaStartChecked)
				{
					lineWithLambdaStartChecked = true;
					
					// Count the braces on the row
					auto startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');
					auto endingBraces = count(tmpStr.begin(), tmpStr.end(), '}');
					bracesCounter = startingBraces - endingBraces;
					
					if (bracesCounter == 0 && startingBraces > 0)	// One-liners
					{
						oneLinersFile << tmpStr << endl;
						oneLiners++;
						isOneLiner = true;
						foundEndingBrace = true;
						tmpStorageForOneLiner = tmpStr;
					}
					// No braces at all
					else if (bracesCounter == 0) { // Has the starting brace on a later row
						// Keep looking!
					}
					// More start braces than end braces, ideal
					else if (bracesCounter >= 1)
					{
						foundStartingBrace = true;
					}
					// More end braces than start braces
					else
					{
						if (startingBraces == 0)	// Has the starting brace on a later row
						{
							// Keep looking! 
						} else if (startingBraces > 0)	// NOTE: Looking at the output, all 47 were one-liners.
						{
							oneLinersFile << tmpStr << endl;
							oneLiners++;
							isOneLiner = true;
							foundEndingBrace = true;
							tmpStorageForOneLiner = tmpStr;
						}	
					}
				} else if (isOneLiner)
				{
					// Check for comment inside the {}
					auto startPos = tmpStorageForOneLiner.find('{');
					auto endPos = tmpStorageForOneLiner.find('}');

					string tmpStorageCut = tmpStorageForOneLiner.substr(startPos, endPos - startPos);

					if (endPos <= tmpStorageForOneLiner.length())	// Out of range error?
					{
						string tmpStorageCutEndOfLine = tmpStorageForOneLiner.substr(endPos);
						if (containsAComment(tmpStorageCutEndOfLine))	// After the end of the one-liner
						{
							oneLinerAfterComment++;
							commentAfterOneLinerFile << tmpStorageForOneLiner << endl;
						}
					}
					
					if (containsAComment(tmpStorageCut))	// Inside the one-liner
					{
						lambdaHadACommentInside = true;
						commentInsideLambda++;
						oneLinersCommentsFile << tmpStorageForOneLiner << endl;
						insideLambdaPathFile << currentInputLine << endl;
					}

					if (!lambdaHadACommentAbove && !lambdaHadACommentInside)
					{
						noCommentsAtAll++;
						noCommentPathFile << currentInputLine << endl;
						
					} else if (lambdaHadACommentAbove && lambdaHadACommentInside)
					{
						hadBothComment++;
					}
					else if (lambdaHadACommentAbove || lambdaHadACommentInside)
					{
						hadSomeComment++;
					}
					
					break;
				}
				else if (foundStartingBrace && !foundEndingBrace && bracesCounter != 0) {

					// NOTE: Just a single comment inside each lambda
					if (!lambdaHadACommentInside && containsAComment(tmpStr))
					{
						lambdaHadACommentInside = true;
						//insideLambdaFile << currentInputLine << endl;
						insideLambdaFile << tmpStr << endl;
						commentInsideLambda++;
						insideLambdaPathFile << currentInputLine << endl;
					}
					
					bracesCounter += count(tmpStr.begin(), tmpStr.end(), '{');
					bracesCounter -= count(tmpStr.begin(), tmpStr.end(), '}');

					if (bracesCounter == 0)
					{
						foundEndingBrace = true;
						if (!lambdaHadACommentAbove && !lambdaHadACommentInside)
						{
							noCommentsAtAll++;
							noCommentPathFile << currentInputLine << endl;
							
						} else if (lambdaHadACommentAbove && lambdaHadACommentInside)
						{
							hadBothComment++;
						}
						else if (lambdaHadACommentAbove || lambdaHadACommentInside)
						{
							hadSomeComment++;
						}
						
						cout << "f";
					}
				} else if (!foundStartingBrace) // There were no braces present at all previously, so we have to find a starting brace and continue from there.
				{
					// Count the starting braces on the row
					auto startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');

					if (startingBraces < 1) // Keep looking! We don't care about bracesCounter until we find the start.
					{
						continue;
					}

					foundStartingBrace = true;
					bracesCounter += startingBraces;
					bracesCounter -= count(tmpStr.begin(), tmpStr.end(), '}');

					if (bracesCounter < 0) // More ending braces than starting braces
					{
						faultFile << tmpStr << endl;
						faults++;
						break;
					}

					if (bracesCounter == 0) // One-liner?
					{
						oneLinersFile << tmpStr << endl;
						oneLiners++;
						isOneLiner = true;
						foundEndingBrace = true;
						tmpStorageForOneLiner = tmpStr;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	aboveLambdaFile << "Count: " << commentAboveLambda << endl;
	insideLambdaFile << "Count: " << commentInsideLambda << endl;
	faultFile << "Count: " << faults << endl;

	int totalChecked = noCommentsAtAll + hadBothComment + hadSomeComment;
	int missingLambdas = lambdasChecked - totalChecked;

	cout << endl << "------------------------------------------------" << endl;
	cout << "COMMENTS" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Right above lambda: " << commentAboveLambda << endl;
	cout << "Inside the lambda: " << commentInsideLambda << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Lambdas with no comments at all: " << noCommentsAtAll << endl;
	cout << "Lambdas that had some comment, not both: " << hadSomeComment << endl;
	cout << "Lambdas that had both types of comment: " << hadBothComment << endl;
	cout << "Comment after one-liner on same row: " << oneLinerAfterComment << endl;
	cout << "Total checked: " << totalChecked << "/" << lambdasChecked << ", missing " << missingLambdas << endl;

	cout << endl << "------------------------------------------------" << endl;
	cout << "MISC" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "One-liners: " << oneLiners << endl;
	cout << "Faults+missing: " << faults+missingLambdas << endl;	
	
	return 0;
}
