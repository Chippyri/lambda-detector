// LambdaScourerCmake.cpp : Defines the entry point for the application.
//

#include "LambdaDetector.h"


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

using namespace Concurrency;
using namespace std::this_thread;
namespace fs = std::filesystem;

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
static mutex global_mtx;
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
bool detectLambda(string path) {

	std::fstream myfile(path);
	ofstream infoFile;
	infoFile.open("info.txt", std::ios_base::app);
	ofstream testFile;
	testFile.open("test.txt", std::ios_base::app);
	int lineCounter = 1;
	bool lambda = false;

	//std::regex regex("\[\][\s]*\(.*\)[\s\w]*\{[\:\(\)[\s\<\;\+a-z\d\/\*]*\}");
	//string re = R"((constexpr))";
	string bad = R"((operator|delete)\s*\[)";
	//string regex = R"(\[\s*\]\s*\(\s*\)\s*)";
	string good = R"(\[[a-z\&\s\=]*\]\s*\()"; // [ ] [ =] [= ] [        = ]
	// [\,\=\s\(\)]+ (takes a lot of time but working) Maybe use boost::regex
	
	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*(constexpr)?\s*\{)";
	// string re = R"(\s*\[[a-z\s\&\=\d]*\]\s*\([a-z\s\&\=\d]*\)\s*\{)";
	//std::regex badRegex("operator|delete) [");
	auto const badRegex = std::regex(bad, std::regex::optimize);
	auto const goodRegex = std::regex(good, std::regex::optimize);
	//std::regex goodRegex(re);
	//std::smatch match;

	//// Possible before lambda
	//string bLambda1 = ",";
	//string bLambda2 = "=";
	//string bLambda3 = " ";

	//// Possible lambdas
	//string pLambda1  = "[](";
	//string pLambda2  = "[] (";
	//string pLambda3  = "[=](";
	//string pLambda4  = "[ =](";
	//string pLambda5  = "[= ](";
	//string pLambda6  = "[ = ](";
	//string pLambda7  = "[ =] (";
	//string pLambda8  = "[= ] (";
	//string pLambda9	 = "[ = ] (";

	//string pLambda10 = "[&";


	//string pLambda11 = "[](";
	//string pLambda12 = "[](";

	string line;

	if (myfile) {
		while (getline(myfile, line)) {
			//cout << line << endl;
			
			// Would like to find another solution to this.
			if (std::regex_search(line, badRegex)) {
				
			}
			else if (std::regex_search(line, goodRegex)) {
				testFile << path << " line: " << lineCounter << endl;
				lambda = true;
			}

			//if (line.find("delete[]") != std::string::npos || line.find("delete []") != std::string::npos || line.find("operator[](") != std::string::npos || line.find("operator [](") != std::string::npos || line.find("operator[] (") != std::string::npos || line.find("operator [] (") != std::string::npos) {

			//} else if (line.find("[](") != std::string::npos || line.find("[] (") != std::string::npos) {
			//	//cout << "YES THERES [] HERE" << endl;
			//	infoFile << path << " line: " << lineCounter << endl;
			//	lambda = true;
			//}
			//else if (line.find("[=](") != std::string::npos || line.find("[ =](") != std::string::npos|| line.find("[= ](") != std::string::npos || line.find("[ = ](") != std::string::npos || line.find("[ = ] (") != std::string::npos) {
			//	//cout << "YES THERES [=] HERE" << endl;
			//	infoFile << path << " line: " << lineCounter << endl;
			//	lambda = true;
			//}
			//else {
			//	//cout << "NOT ON THIS LINE" << endl;
			//}
			lineCounter++;
		}
	}

	myfile.close();
	infoFile.close();

	return lambda;
	//cout << file << endl;
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

			// Would like to find a solution to not repeat the same code in every if-statement
			if (str == EXTENSION_CPP)
			{
				cppFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_C)
			{
				cFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_CC)
			{
				ccFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_CPLUSPLUS)
			{
				cplusplusFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_CXX)
			{
				cxxFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_H)
			{
				hFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_HPP)
			{
				hppFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_HH)
			{
				hhFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_HPLUSPLUS)
			{
				hplusplusFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
			}
			else if (str == EXTENSION_HXX)
			{
				hxxFileCount++;
				string path = entry.path().string();
				if (detectLambda(path)) {
					ptr.get()[10] = 1;
				}
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
	if (ptr.get()[10] != 1) {
		ptr.get()[10] = 0;
	}

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
			auto ptr = countFileTypes(searchPath);

			// Open lambda and nolambda files.
			ofstream lambdaFile;
			lambdaFile.open("lambda.txt", std::ios_base::app);

			ofstream noLambdaFile;
			noLambdaFile.open("nolambda.txt", std::ios_base::app);

			//sleep_for(std::chrono::seconds(1));

			std::lock_guard<std::mutex> lock{ global_mtx };
			cout << get_id() << ": " << popped << endl;

			cout << "CPP: " << ptr.get()[0] << endl;
			cout << "CC: " << ptr.get()[1] << endl;
			cout << "C: " << ptr.get()[2] << endl;
			cout << "C++: " << ptr.get()[3] << endl;
			cout << "CXX: " << ptr.get()[4] << endl;
			cout << "H: " << ptr.get()[5] << endl;
			cout << "HH: " << ptr.get()[6] << endl;
			cout << "HPP: " << ptr.get()[7] << endl;
			cout << "H++: " << ptr.get()[8] << endl;
			cout << "HXX: " << ptr.get()[9] << endl;
			
			// Add popped (name) to file if it has lambda or not.
			if (ptr.get()[10] == 1) {
				lambdaFile << popped << endl;
			}
			else {
				noLambdaFile << popped << endl;
			}

			// Close files
			lambdaFile.close();
			noLambdaFile.close();
		}
	}
};

int main(const int argc, const char* argv[])
{
	if (argc < 2)
	{
		return 1;
	}

	cout << argc << endl;
	PATH = argv[1];

	const auto repositories = createMap(PATH);
	concurrent_queue<string> workQueue = createWorkQueue(repositories);

	thread t1(thread_obj(), ref(workQueue));
	thread t2(thread_obj(), ref(workQueue));
	thread t3(thread_obj(), ref(workQueue));
	thread t4(thread_obj(), ref(workQueue));

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	
	return 0;
}

