#pragma once

#include <fstream>
#include <iostream>
#include <string>

using std::string;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::cout;

// ##### GLOBALS
static const string INPUT_FILE_NAME = "test.txt";
static const string DELIMITER = " line: ";
static const auto DELIMITER_LENGTH = DELIMITER.length();

// ##### COUNTERS
int commentsAboveLambda = 0;
int commentsInsideLambda = 0;
int commentsInsideOneLiner = 0;
int commentsAfterOneLiner = 0;

int noCommentsAtAll = 0;	// one/multi
int hadAComment = 0;		// one/multi
int linesRead = 0;

// MISC
int lambdasChecked = 0;
int lambdasNotChecked = 0;	// Did not have the required style
int oneLiners = 0;

ofstream insideLambdaFile("comment_inside_lambda.txt", ofstream::out | ofstream::trunc);
ofstream aboveLambdaFile("comment_above_lambda.txt", ofstream::out | ofstream::trunc);
ofstream commentAfterOneLinerFile("comment_after_one_liner.txt", ofstream::out | ofstream::trunc);
ofstream testFile("comment_test.txt", ofstream::out | ofstream::trunc);

bool containsAComment(const string& tmpStr)
{
	return tmpStr.find("//") != string::npos || tmpStr.find("/*") != string::npos;
}

bool startingBraceIsBeforeEndingBrace(const string& tmpStr)
{
	const auto startPos = tmpStr.find('{');
	const auto endPos = tmpStr.find('}');
	if (startPos == string::npos || endPos == string::npos) return false;
	return startPos < endPos;
}

bool containsCommentInsideOneLiner(const string& tmpStr)
{
	const auto startPos = tmpStr.find('{');
	const auto endPos = tmpStr.find('}');

	if (startPos == string::npos || endPos == string::npos) return false;
	return containsAComment(tmpStr.substr(startPos, endPos - startPos));
}

// NOTE: Requires the string to contain the }-character
bool containsCommentAfterEndingBrace(const string& tmpStr)
{
	const auto endPos = tmpStr.find('}');
	if (endPos == string::npos) return false;
	return containsAComment(tmpStr.substr(endPos));
}

bool containsCommentAfterStartingBrace(const string& tmpStr)
{
	const auto startPos = tmpStr.find('{');
	if (startPos == string::npos) return false;
	return containsAComment(tmpStr.substr(startPos));
}

// NOTE: Must have contained a comment!
bool commentIsBeforeEndBrace(const string& tmpStr)
{
	auto commentPos = tmpStr.find("//");
	auto endBracePos = tmpStr.find('}');

	if (commentPos != string::npos)
	{
		return endBracePos > commentPos;
	}

	commentPos = tmpStr.find("/*");
	if (commentPos != string::npos)
	{
		return endBracePos > commentPos;
	}
	return false;
}

int main()
{
	// Parse results from Lambda Finder
	ifstream inputFile(INPUT_FILE_NAME);
	
	if (!inputFile.good())
	{
		return 1;
	}

	string currentInputLine;

	int counter = 0;

	while (getline(inputFile, currentInputLine))
	{
		linesRead++;
		counter++;
		const auto delimiterPosition = currentInputLine.find(DELIMITER);
		const auto pastDelimiterPosition = delimiterPosition + DELIMITER_LENGTH;
		const string filePath = currentInputLine.substr(0, delimiterPosition);
		const string rowNumberAsString = currentInputLine.substr(pastDelimiterPosition);
		const auto rowNumberAsInt = stoi(rowNumberAsString);

		ifstream fileToCopyFrom(filePath);
		if (!fileToCopyFrom.good())
		{
			continue;
		}

		int rowToCheckFrom = rowNumberAsInt - 1;	// Check the line before
		if (rowToCheckFrom < 0) // Check that it is not beyond the beginning of the file
		{
			rowToCheckFrom = 0;
		}

		string tmpStr;

		bool lambdaHadACommentAbove = false;
		bool lambdaHadACommentInside = false;
		bool lambdaOneLinerHadACommentAfter = false;

		bool foundStartingBrace = false;

		auto bracesCounter = 0;
		auto inFileLineCounter = 0;

		while (true) {
			if (getline(fileToCopyFrom, tmpStr))
			{
				inFileLineCounter++;
				if (inFileLineCounter == rowToCheckFrom)
				{
					// ##### ROW ABOVE LAMBDA #####
					if (containsAComment(tmpStr))
					{
						aboveLambdaFile << tmpStr << endl;
						lambdaHadACommentAbove = true;
						commentsAboveLambda++;
					}
					
					// ##### ROW WITH LAMBDA #####
					if (getline(fileToCopyFrom, tmpStr))
					{
						testFile << tmpStr << endl;
						
						auto startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');
						auto endingBraces = count(tmpStr.begin(), tmpStr.end(), '}');
						bracesCounter = startingBraces - endingBraces;

						// ##### ONE-LINER #####
						if (bracesCounter == 0 && startingBraces > 0 && startingBraceIsBeforeEndingBrace(tmpStr))
						{
							oneLiners++;

							// ##### COMMENT INSIDE #####
							if (containsCommentInsideOneLiner(tmpStr))
							{
								commentsInsideOneLiner++;
								lambdaHadACommentInside = true;
							}

							// ##### COMMENT AFTER ONE LINER #####
							if (containsCommentAfterEndingBrace(tmpStr))
							{
								commentsAfterOneLiner++;
								commentAfterOneLinerFile << tmpStr << endl;
								lambdaOneLinerHadACommentAfter = true;
							}
						}

						// ##### MULTI-LINER #####
						else
						{
							if (containsCommentAfterStartingBrace(tmpStr))
							{
								lambdaHadACommentInside = true;
								insideLambdaFile << tmpStr << endl;
								commentsInsideLambda++;
								lambdasChecked++;
								break;
							}

							while (true)
							{
								if (getline(fileToCopyFrom, tmpStr))
								{
									testFile << tmpStr << endl;
									// NOTE: Just a single comment inside each multi-line lambda is checked, then it breaks
									if (!lambdaHadACommentInside && containsAComment(tmpStr) && commentIsBeforeEndBrace(tmpStr))
									{
										lambdaHadACommentInside = true;
										insideLambdaFile << tmpStr << endl;
										commentsInsideLambda++;
										lambdasChecked++;
										testFile << endl << endl << endl;
										break;
									}

									bracesCounter += count(tmpStr.begin(), tmpStr.end(), '{');
									bracesCounter -= count(tmpStr.begin(), tmpStr.end(), '}');

									if (bracesCounter <= 0)
									{
										//cout << ":"; // End brace was found without a comment.
										lambdasChecked++;
										testFile << endl << endl << endl;
										break;
									}
								}
								else
								{
									// Could not find an even or more end braces before the end of the file.
									lambdasNotChecked++;
									break;
								}
							}
						}


					} else
					{
						break;
					}
					break;
				}
			}
		}

		if (!lambdaHadACommentAbove && !lambdaHadACommentInside && !lambdaOneLinerHadACommentAfter)
		{
			noCommentsAtAll++;
		}
		else if (lambdaHadACommentAbove || lambdaHadACommentInside || lambdaOneLinerHadACommentAfter)
		{
			hadAComment++;
		}

		if (counter == 12000)
		{
			int totalChecked = lambdasChecked + lambdasNotChecked + oneLiners;
			int missingLambdas = linesRead - totalChecked;

			cout << endl << "------------------------------------------------" << endl;
			cout << "COMMENTS" << endl;
			cout << "------------------------------------------------" << endl;
			cout << "Comments inside (one): " << commentsInsideOneLiner << endl;
			cout << "Comments after (one): " << commentsAfterOneLiner << endl;
			cout << "Right above lambda (one/multi): " << commentsAboveLambda << endl;
			cout << "Comments inside (multi): " << commentsInsideLambda << endl;

			cout << "Lambdas that had some comment: " << hadAComment << endl;
			//cout << "Lambdas with no comments at all: " << noCommentsAtAll << endl;
			cout << "Lambdas with no comments at all (minus not checked): " << noCommentsAtAll - lambdasNotChecked << endl;

			cout << endl << "------------------------------------------------" << endl;
			cout << "MISC" << endl;
			cout << "------------------------------------------------" << endl;
			cout << "Lambdas checked: " << lambdasChecked << endl;
			cout << "Lambdas not checked: " << lambdasNotChecked << endl;
			cout << "One-liners: " << oneLiners << endl;
			cout << "Total tested: " << totalChecked << "/" << linesRead << ", missing " << missingLambdas << endl;
		}
	}

	int totalChecked = lambdasChecked + lambdasNotChecked + oneLiners;
	int missingLambdas = linesRead - totalChecked;

	cout << endl << "------------------------------------------------" << endl;
	cout << "COMMENTS" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Comments inside (one): " << commentsInsideOneLiner << endl;
	cout << "Comments after (one): " << commentsAfterOneLiner << endl;
	cout << "Right above lambda (one/multi): " << commentsAboveLambda << endl;
	cout << "Comments inside (multi): " << commentsInsideLambda << endl;

	cout << "Lambdas that had some comment: " << hadAComment << endl;
	//cout << "Lambdas with no comments at all: " << noCommentsAtAll << endl;
	cout << "Lambdas with no comments at all (minus not checked): " << noCommentsAtAll - lambdasNotChecked << endl;

	cout << endl << "------------------------------------------------" << endl;
	cout << "MISC" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Lambdas checked: " << lambdasChecked << endl;
	cout << "Lambdas not checked: " << lambdasNotChecked << endl;
	cout << "One-liners: " << oneLiners << endl;
	cout << "Total tested: " << totalChecked << "/" << linesRead << ", missing " << missingLambdas << endl;
	
	
	return 0;
}