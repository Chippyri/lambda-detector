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
using std::count;

bool containsAComment(const string & tmpStr)
{
	return tmpStr.find("//") != string::npos || tmpStr.find("/*") != string::npos;
}

bool startingBraceIsBeforeEndingBrace(const string & tmpStr)
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

void evaluateOneLiner(ofstream& oneLinersFile, int& oneLiners, 
	int& commentsInsideOneLiner, int& commentsAfterOneLiner, 
	ofstream& commentInsideOneLinerFile, ofstream& commentAfterOneLinerFile, 
	string tmpStr, bool& lambdaHadACommentInside, bool& lambdaOneLinerHadCommentAfter)
{
	oneLiners++;
	oneLinersFile << tmpStr << endl;

	// ##### COMMENT INSIDE #####
	if (containsCommentInsideOneLiner(tmpStr))
	{
		commentsInsideOneLiner++;
		commentInsideOneLinerFile << tmpStr << endl;
		lambdaHadACommentInside = true;
	}
							
	// ##### COMMENT AFTER ONE LINER #####
	if (containsCommentAfterEndingBrace(tmpStr))
	{
		commentsAfterOneLiner++;
		commentAfterOneLinerFile << tmpStr << endl;
		lambdaOneLinerHadCommentAfter = true;
	}
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
	
	// #####################
	string currentInputLine;

	// ##### COUNTERS #####
	int commentsAboveLambda = 0;
	int commentsInsideLambda = 0;

	int commentsInsideOneLiner = 0;
	int commentsAfterOneLiner = 0;

	int noCommentsAtAll = 0;	// one/multi
	int hadAComment = 0;		// one/multi

	// ##### MISC COUNTERS #####
	int lambdasChecked = 0;
	int oneLiners = 0;
	int fakeLambdas = 0;
	int otherFaults = 0;

	// ##### OUTPUT FILES #####
	// ONE
	ofstream commentInsideOneLinerFile("comment_inside_one_liner.txt", ofstream::out | ofstream::trunc);
	ofstream commentAfterOneLinerFile("comment_after_one_liner.txt", ofstream::out | ofstream::trunc);

	// MULTI
	ofstream insideLambdaFile("comment_inside_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream insideLambdaFilePath("comment_inside_lambda_path.txt", ofstream::out | ofstream::trunc);
	
	// ONE/MULTI
	ofstream aboveLambdaFile("comment_above_lambda.txt", ofstream::out | ofstream::trunc);

	// FOR TESTING PURPOSES
	ofstream testFile("comment_test.txt", ofstream::out | ofstream::trunc);

	// MISC
	ofstream oneLinersFile("comment_one_liners.txt", ofstream::out | ofstream::trunc);
	ofstream fakeLambdaFile("comment_fake_lambda.txt", ofstream::out | ofstream::trunc);
	ofstream faultFile("comment_faults.txt", ofstream::out | ofstream::trunc);
	
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
			continue;
		}

		lambdasChecked++;
		
		int rowToCheckFrom = rowNumberAsInt - 1;	// Check the line before
		if (rowToCheckFrom < 0) // Check that it is not beyond the beginning of the file
		{
			rowToCheckFrom = 0;
		}

		vector<string> linesFromFile;
		string tmpStr;
		
		bool lambdaHadACommentInside = false;
		bool lambdaHadACommentAbove = false;
		bool lambdaOneLinerHadACommentAfter = false;

		string tmpStorageForOneLiner;

		bool foundStartingBrace = false;
		bool foundEndingBrace = false;
		auto bracesCounter = 0;
		auto inFileLineCounter = 0;
		
		while (true){
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
						auto startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');
						auto endingBraces = count(tmpStr.begin(), tmpStr.end(), '}');
						bracesCounter = startingBraces - endingBraces;

						// ##### ONE-LINER #####
						if (bracesCounter == 0 && startingBraces > 0 && startingBraceIsBeforeEndingBrace(tmpStr))
						{
							evaluateOneLiner(oneLinersFile, 
								oneLiners, 
								commentsInsideOneLiner,
								commentsAfterOneLiner, 
								commentInsideOneLinerFile,
							    commentAfterOneLinerFile, tmpStr,
								lambdaHadACommentInside, lambdaOneLinerHadACommentAfter);
							break;
						}
						// ##### MULTI-LINER #####
						else
						{
							if (bracesCounter == 0 && startingBraces == 0) {

								if (tmpStr.find(';') != string::npos)	// Fake lambda?
								{	
									fakeLambdaFile << tmpStr << endl;
									fakeLambdas++;
									break;
								} else
								{
									// Has the starting brace on a later row
									// Keep looking!
								}
							}
							
							// More start braces than end braces, ideal
							else if (bracesCounter >= 1)
							{
								foundStartingBrace = true;
							}

							// More end braces than start braces
							else
							{
								if (startingBraces == 0) // Only end-braces
								{
									// Has the starting brace on a later row
									// Keep looking! V
								}
								else if (startingBraces > 0)	// NOTE: Looking at the output, all 47 were one-liners.
								{
									evaluateOneLiner(oneLinersFile,
										oneLiners,
										commentsInsideOneLiner,
										commentsAfterOneLiner,
										commentInsideOneLinerFile,
										commentAfterOneLinerFile, tmpStr,
										lambdaHadACommentInside, lambdaOneLinerHadACommentAfter);
									break;
								}
							}
						}
						if (foundStartingBrace)
						{
							while(true)
							{
								if (getline(fileToCopyFrom, tmpStr))
								{
									if (!foundEndingBrace && bracesCounter != 0) {

										// NOTE: Just a single comment inside each multi-line lambda is checked, then it breaks
										if (!lambdaHadACommentInside && containsAComment(tmpStr) && commentIsBeforeEndBrace(tmpStr))
										{
											lambdaHadACommentInside = true;
											insideLambdaFile << tmpStr << endl;
											commentsInsideLambda++;
											break;
										}

										bracesCounter += count(tmpStr.begin(), tmpStr.end(), '{');
										bracesCounter -= count(tmpStr.begin(), tmpStr.end(), '}');

										if (bracesCounter == 0)
										{
											cout << ":"; // End brace was found.
											break;
										}
									}
								} else
								{
									cout << ".";	// Could not find an even amount of braces before the end of the file.
									// TODO: Faults?
									break;
								}
							}
						}
						else // !foundStartingBrace
						{
							while(true)
							{
								if (getline(fileToCopyFrom, tmpStr))
								{
									startingBraces = count(tmpStr.begin(), tmpStr.end(), '{');

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
										otherFaults++;
										break;
									}

									if (bracesCounter == 0) // "One-liner"
									{
										if (tmpStr.find("});") != string::npos || tmpStr.find("} );") != string::npos)
										{
											testFile << tmpStr << endl;
											
											evaluateOneLiner(oneLinersFile,
												oneLiners,
												commentsInsideOneLiner,
												commentsAfterOneLiner,
												commentInsideOneLinerFile,
												commentAfterOneLinerFile, tmpStr,
												lambdaHadACommentInside, lambdaOneLinerHadACommentAfter);
											break;
										} else
										{
											// NOTE: There are more that are one-liners...
											faultFile << tmpStr << endl;
											break;
										}
									}
								} else
								{
									cout << "F";	// TODO: Investigate why it arrives here. *thinking*
									break;
								}
							}
						}

						break;
					}
					else
					{
						cout << "ERROR: Line with lambda did not exist!?" << endl;
						break;
					}
				}
			} else
			{
				cout << "ERROR: There was no next row when expected!" << endl;
				break;
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

		if (lambdaHadACommentInside)
		{
			insideLambdaFilePath << currentInputLine << endl;
		}
	}

	int totalChecked = noCommentsAtAll + hadAComment;
	int missingLambdas = lambdasChecked - totalChecked;

	cout << endl << "------------------------------------------------" << endl;
	cout << "COMMENTS" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Comments inside (one): " << commentsInsideOneLiner << endl;
	cout << "Comments after (one): " << commentsAfterOneLiner << endl;
	cout << "Right above lambda (one/multi): " << commentsAboveLambda << endl;
	cout << "Comments inside (multi): " << commentsInsideLambda << endl;
	cout << "Lambdas with no comments at all: " << noCommentsAtAll << endl;
	cout << "Lambdas that had some comment: " << hadAComment << endl;

	cout << endl << "------------------------------------------------" << endl;
	cout << "MISC" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Possibly incorrectly matched lambda: " << fakeLambdas << endl;
	cout << "One-liners: " << oneLiners << endl;
	cout << "Total checked: " << totalChecked << "/" << lambdasChecked << ", missing " << missingLambdas << endl;

	return 0;
}
