// LambdaScourerCmake.cpp : Defines the entry point for the application.
//

#include "LambdaDetector.h"
//#include <boost/regex.hpp>

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::make_pair;
using std::thread;
using std::ref;
using std::lock_guard;
using std::mutex;
using std::unique_ptr;
//using boost::regex;

using namespace Concurrency;
using namespace std::this_thread;
namespace fs = std::filesystem;

const string TEST = "test.txt";
//const string LAMBDA = "l.txt";
//const string NOLAMBDA = "n.txt";

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

// TODO: Global variables, move to a class 
static string PATH;

concurrent_unordered_map<string, bool> createMap(const string& path)
{
	concurrent_unordered_map<string, bool> repositories;

	for (const auto& entry : fs::directory_iterator(path))
	{
		repositories.insert(make_pair(entry.path().filename().string(), false));
	}

	return repositories;
}

void writeRepositoryMapToFile(const concurrent_unordered_map<string, bool>& repositories)
{
	ofstream outputFile;
	outputFile.open("output.csv");
	outputFile << "repository,contains_lambda" << endl;

	for (const auto& entry : repositories)
	{
		cout << entry.first << " " << entry.second << endl;
		outputFile << entry.first << "," << entry.second << endl;
	}

	outputFile.close();
}

// Checking every row in the file of the path if it contains []( or [=](, operator[] is not considered a lambda. 
bool detectLambda(std::wstring path) {

	std::fstream myfile(path);
	ofstream infoFile;
	infoFile.open("info.txt", std::ios_base::app);
	std::wofstream testFile;
	testFile.open(TEST, std::ios_base::app);
	int lineCounter = 1;
	bool lambda = false;

	// TEST PRINT WHAT FILE IT IS CURRENTLY CHECKING
	//std::wcout << path << endl;

	//std::regex regex("\[\][\s]*\(.*\)[\s\w]*\{[\:\(\)[\s\<\;\+a-z\d\/\*]*\}");
	// /* comment
	//string comment = R"(\s*\/\*)";
	// // comment
	//string sComment = R"(\s*\/\/)";
	//string re = R"((constexpr))";
	string bad = R"((operator|delete|new)\s*\[)";
	//string regex = R"(\[\s*\]\s*\(\s*\)\s*)";
	string good = R"([\,\=\s\(\)]*[\,\=\s\(\)]+\[[a-z\&\s\=\:]*\]\s*\()"; // [ ] [ =] [= ] [        = ] (ADDED "\:")
	// [\,\=\s\(\)]+ (takes a lot of time but working) Maybe use boost::regex

	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*(constexpr)?\s*\{)";
	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*\{)";
	//std::regex badRegex("operator|delete) [");
	//auto const sCommentRegex = std::regex(sComment, std::regex::optimize);
	//auto const commentRegex = std::regex(comment, std::regex::optimize);
	auto const badRegex = std::regex(bad, std::regex::optimize);
	auto const goodRegex = std::regex(good, std::regex::optimize);
	//std::regex goodRegex(re);
	//std::smatch match;

	string line;
	string subline;
	string newline;
	std::size_t pos = 0;
	std::size_t spos = 0;
	bool stopComment = false;
	if (myfile) {
		while (getline(myfile, line)) {
			//cout << line << endl;

			// Find /* if it is first on the line, skip until */
			// If it's not the first, save what is before /* in line and check line as normal while then skipping whats in /**/

			//TODO repeated code, change
			// If the line has a comment
			if ((spos = line.find("//")) != std::string::npos) {
				subline = line.substr(0, spos);
				if (std::regex_search(subline, badRegex)) {
				}

				else if (std::regex_search(subline, goodRegex)) {
					testFile << path << " line: " << lineCounter << endl;
					lambda = true;
				}
			}
			else if ((pos = line.find("/*")) != std::string::npos) {
				// Line up to comment is still checked if it has a lambda
				subline = line.substr(0, pos);
				if (std::regex_search(subline, badRegex)) {
				}

				else if (std::regex_search(subline, goodRegex)) {
					testFile << path << " line: " << lineCounter << endl;
					lambda = true;
				}
				// cout << lineCounter << " " << subline + line << endl;
				// If the same line that started the comment end the comment
				if (line.find("*/") != std::string::npos) {
					stopComment = true;
				}

				// If the line didnt end the comment, get more lines until there is a remove comment
				while (stopComment == false && getline(myfile, newline)) {

					if (newline.find("*/") != std::string::npos) {
						stopComment = true;
					}
					// still counting the lines
					lineCounter++;
				}

			}
			// If the line did not have a comment, search for lambda
			else {

				if (std::regex_search(line, badRegex)) {

				}

				else if (std::regex_search(line, goodRegex)) {
					testFile << path << " line: " << lineCounter << endl;
					lambda = true;
				}
			}

			lineCounter++;
		}
	}

	myfile.close();
	infoFile.close();

	return lambda;
	//cout << file << endl;
}

bool hasExtensionAndUpdateCount(const string& extensionToTest, const string& extensionWanted, int& fileCountVariable)
{
	if (extensionToTest == extensionWanted)
	{
		fileCountVariable++;
		return true;
	}
	return false;
}

void detectAndUpdateIfRepoHasLambda(unique_ptr<int[]>& pointer, const std::filesystem::directory_entry& pathToFile)
{
	std::wstring nPath;
	nPath.append(pathToFile.path().wstring());
	if (detectLambda(nPath)) {
		pointer.get()[10] = 1;
	}
}

unique_ptr<int[]> countFileTypes(const string& path)
{
	int cppFileCount = 0;
	int ccFileCount = 0;
	int cFileCount = 0;
	int cplusplusFileCount = 0;
	int cxxFileCount = 0;

	int hppFileCount = 0;
	int hhFileCount = 0;
	int hFileCount = 0;
	int hxxFileCount = 0;
	int hplusplusFileCount = 0;

	auto ptr = std::unique_ptr<int[]>(new int[11]);

	for (const auto& entry : fs::recursive_directory_iterator(path))
	{
		if (entry.is_regular_file() && entry.path().has_extension())
		{
			string str = entry.path().extension().string();
			std::for_each(str.begin(), str.end(), [](char& c) {
				c = ::tolower(c);
				});

			if (hasExtensionAndUpdateCount(str, EXTENSION_CPP, cppFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_H, hFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_C, cFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_HPP, hppFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_CPLUSPLUS, cplusplusFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_CXX, cxxFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_HH, hhFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_HXX, hxxFileCount) ||
				hasExtensionAndUpdateCount(str, EXTENSION_HPLUSPLUS, hplusplusFileCount))
			{
				detectAndUpdateIfRepoHasLambda(ptr, entry);
			}
		}
	}

	ptr.get()[0] = cppFileCount;
	ptr.get()[1] = ccFileCount;
	ptr.get()[2] = cFileCount;
	ptr.get()[3] = cplusplusFileCount;
	ptr.get()[4] = cxxFileCount;

	ptr.get()[5] = hFileCount;
	ptr.get()[6] = hhFileCount;
	ptr.get()[7] = hppFileCount;
	ptr.get()[8] = hplusplusFileCount;
	ptr.get()[9] = hxxFileCount;

	// If the file does not have a lambda, [10] to zero
	//if (ptr.get()[10] != 1) {
	//	ptr.get()[10] = 0;
	//}

	return ptr;
}

concurrent_queue<string> createWorkQueue(const concurrent_unordered_map<string, bool> repositories)
{
	concurrent_queue<string> queue;

	for (const auto& entry : repositories)
	{
		queue.push(entry.first);
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
// A callable object 
class thread_obj {
private:

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
			auto ptr = countFileTypes(searchPath);

			// Open lambda and nolambda files.
			//ofstream lambdaFile;
			//lambdaFile.open(LAMBDA, std::ios_base::app);

			//ofstream noLambdaFile;
			//noLambdaFile.open(NOLAMBDA, std::ios_base::app);

			//sleep_for(std::chrono::seconds(1));

			//std::lock_guard<std::mutex> lock{ global_mtx };
			cout << counter.get() << ": " << popped << endl;

			//cout << "CPP: " << ptr.get()[0] << endl;
			//cout << "CC: " << ptr.get()[1] << endl;
			//cout << "C: " << ptr.get()[2] << endl;
			//cout << "C++: " << ptr.get()[3] << endl;
			//cout << "CXX: " << ptr.get()[4] << endl;
			//cout << "H: " << ptr.get()[5] << endl;
			//cout << "HH: " << ptr.get()[6] << endl;
			//cout << "HPP: " << ptr.get()[7] << endl;
			//cout << "H++: " << ptr.get()[8] << endl;
			//cout << "HXX: " << ptr.get()[9] << endl;

			// Add popped (name) to file if it has lambda or not.
			//if (ptr.get()[10] == 1) {
			//	lambdaFile << popped << endl;
			//}
			//else {
			//	noLambdaFile << popped << endl;
			//}

			// Close files
			//lambdaFile.close();
			//noLambdaFile.close();
		}
	}
};

int main(const int argc, const char* argv[])
{
	if (argc < 2)
	{
		return 1;
	}

	//cout << argc << endl;
	PATH = argv[1];

	// TODO: Test, feel free to remove!
	//boost:regex reg("hello");

	const auto repositories = createMap(PATH);
	concurrent_queue<string> workQueue = createWorkQueue(repositories);

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

	return 0;
}

