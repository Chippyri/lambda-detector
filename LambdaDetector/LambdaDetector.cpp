#include "LambdaDetector.h"

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::thread;
using std::ref;
using std::lock_guard;
using std::mutex;
using std::wstring;
using std::ios_base;
using std::wofstream;
using std::fstream;
using std::regex;
using std::regex_search;
using std::size_t;
using std::for_each;
using std::lock_guard;

using namespace Concurrency;
using namespace std::this_thread;
namespace fs = std::filesystem;

const string TEST = "C:/Users/jonat/Desktop/lambdatest/test1.txt";

const string EXTENSION_CPP = ".cpp";
const string EXTENSION_CC = ".cc";
const string EXTENSION_C = ".c";
const string EXTENSION_CXX = ".cxx";
const string EXTENSION_CPLUSPLUS = ".c++";

const string EXTENSION_HPP = ".hpp";
const string EXTENSION_H = ".h";
const string EXTENSION_HH = ".hh";
const string EXTENSION_HXX = ".hxx";
const string EXTENSION_HPLUSPLUS = ".h++";

// Global
static string PATH;
mutex WRITE_TO_TEST_MUTEX;
wofstream TEST_FILE_STREAM;

// Should be thread-safe?
void writeToTestFile(const wstring& fileName, const int& lineNumber)
{
	// Protects from concurrent writing, is automatically unlocked when function ends
	const lock_guard<mutex> lock(WRITE_TO_TEST_MUTEX);

	// Open the stream to the output file
	TEST_FILE_STREAM << fileName << " line: " << lineNumber << endl;
}

// Checking every row in the file of the file if it contains []( or [=](, operator[] is not considered a lambda. 
bool scanFileForLambda(const wstring& file) {

	// Open file to read
	fstream myFile(file);

	int lineCounter = 1;
	bool lambda = false;

	// TEST PRINT WHAT FILE IT IS CURRENTLY CHECKING
	//std::wcout << file << endl;

	//std::regex regex("\[\][\s]*\(.*\)[\s\w]*\{[\:\(\)[\s\<\;\+a-z\d\/\*]*\}");
	// /* comment
	//string comment = R"(\s*\/\*)";
	// // comment
	//string sComment = R"(\s*\/\/)";
	//string re = R"((constexpr))";
	const string bad = R"((operator|delete|new)\s*\[)";
	//string regex = R"(\[\s*\]\s*\(\s*\)\s*)";
	//const string good = R"([\,\=\s\(\)]*[\,\=\s\(\)]+\[[a-zA-Z0-9\*\_\&\s\=\:\<\>]*\]\s*\()"; // [ ] [ =] [= ] [        = ] (ADDED "\:")
	const string good = R"([\,\=\s\(\)]*((\[\])|(\[\s*[a-zA-Z\_\&\*\s]+[a-zA-Z0-9\*\_\&\s\=\:\<\>\,]*\]))\s*\()"; // [ ] [ =] [= ] [        = ] (ADDED "\:")
	// [\,\=\s\(\)]+ (takes a lot of time but working) Maybe use boost::regex

	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*(constexpr)?\s*\{)";
	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*\{)";
	//std::regex badRegex("operator|delete) [");
	//auto const sCommentRegex = std::regex(sComment, std::regex::optimize);
	//auto const commentRegex = std::regex(comment, std::regex::optimize);
	auto const badRegex = regex(bad, regex::optimize);
	auto const goodRegex = regex(good, regex::optimize);
	//std::regex goodRegex(re);
	//std::smatch match;

	string line;
	string subline;
	string newline;
	size_t pos = 0;
	size_t spos = 0;
	bool stopComment = false;
	if (myFile) {
		while (getline(myFile, line)) {

			// Find /* if it is first on the line, skip until */
			// If it's not the first, save what is before /* in line and check line as normal while then skipping whats in /**/

			//TODO repeated code, change
			// If the line has a comment
			stopComment = false;
			if ((spos = line.find("//")) != string::npos) {
				subline = line.substr(0, spos);
				if (regex_search(subline, badRegex)) {
				}

				else if (regex_search(subline, goodRegex)) {
					writeToTestFile(file, lineCounter);
					lambda = true;
				}
			}
			else if ((pos = line.find("/*")) != string::npos) {
				// Line up to comment is still checked if it has a lambda
				subline = line.substr(0, pos);
				if (regex_search(subline, badRegex)) {
				}

				else if (regex_search(subline, goodRegex)) {
					writeToTestFile(file, lineCounter);
					lambda = true;
				}
				// cout << lineCounter << " " << subline + line << endl;
				// If the same line that started the comment end the comment
				if (line.find("*/") != string::npos) {
					stopComment = true;
				}

				// If the line didnt end the comment, get more lines until there is a remove comment
				while (stopComment == false && getline(myFile, newline)) {
					if (newline.find("*/") != string::npos) {
						stopComment = true;
					}
					// still counting the lines
					lineCounter++;
				}

			}
			// If the line did not have a comment, search for lambda
			else {

				if (regex_search(line, badRegex)) {

				}

				else if (regex_search(line, goodRegex)) {
					writeToTestFile(file, lineCounter);
					lambda = true;
				}
			}

			lineCounter++;
		}
	}

	myFile.close();

	return lambda;
}

bool matchesExtension(const string& extensionToTest, const string& extensionWanted)
{
	return (extensionToTest == extensionWanted);
}

bool hasEligibleExtension(const string& file_extension)
{
	return matchesExtension(file_extension, EXTENSION_CPP) ||
		matchesExtension(file_extension, EXTENSION_H) ||
		matchesExtension(file_extension, EXTENSION_C) ||
		matchesExtension(file_extension, EXTENSION_HPP) ||
		matchesExtension(file_extension, EXTENSION_CPLUSPLUS) ||
		matchesExtension(file_extension, EXTENSION_CXX) ||
		matchesExtension(file_extension, EXTENSION_HH) ||
		matchesExtension(file_extension, EXTENSION_HXX) ||
		matchesExtension(file_extension, EXTENSION_HPLUSPLUS);
}

// Put all repository names in a queue for threads to take from
concurrent_queue<string> createWorkQueue()
{
	concurrent_queue<string> queue;

	for (const auto& entry : fs::directory_iterator(PATH))
	{
		queue.push(entry.path().filename().string());
	}

	return queue;
}

struct AtomicCounter {
	std::atomic<int> value = 0;

	int get() {
		++value;
		return value.load();
	}
};

AtomicCounter counter;

bool isFileAndHasCorrectExtension(const std::filesystem::directory_entry& file)
{
	if (file.is_regular_file() && file.path().has_extension())
	{
		string file_extension = file.path().extension().string();

		// Change extension name to lowercase
		for_each(file_extension.begin(), file_extension.end(), [](char& c) {
			c = ::tolower(c);
			});

		if (hasEligibleExtension(file_extension))
		{
			return true;
		}
	}
	return false;
}

// A callable object 
class thread_obj {
public:

	void operator()(concurrent_queue<string>& queue) const
	{
		string popped;
		while (queue.try_pop(popped))
		{
			string searchPath;
			searchPath.append(PATH);
			searchPath.append("/");
			searchPath.append(popped);

			// Iterate files looking for C++-files and scan those files for lambdas
			for (const auto& file : fs::recursive_directory_iterator(searchPath))
			{
				if (isFileAndHasCorrectExtension(file))
				{
					scanFileForLambda(file.path().wstring());
				}
			}

			// Written when a repository has finished scanning
			cout << counter.get() << ": " << popped << endl;
		}
	}
};

int main(const int argc, const char* argv[])
{
	// Check the amount of arguments given
	if (argc < 2)
	{
		return 1;
	}

	PATH = argv[1];

	TEST_FILE_STREAM.open(TEST, ios_base::app);
	
	// Create a concurrent queue with the repository names
	concurrent_queue<string> workQueue = createWorkQueue();

	thread t1(thread_obj(), ref(workQueue));
	thread t2(thread_obj(), ref(workQueue));
	thread t3(thread_obj(), ref(workQueue));
	thread t4(thread_obj(), ref(workQueue));
	thread t5(thread_obj(), ref(workQueue));
	thread t6(thread_obj(), ref(workQueue));
	thread t7(thread_obj(), ref(workQueue));
	thread t8(thread_obj(), ref(workQueue));
	thread t9(thread_obj(), ref(workQueue));

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	t8.join();
	t9.join();

	TEST_FILE_STREAM.close();

	return 0;
}

