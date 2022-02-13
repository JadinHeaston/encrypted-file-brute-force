//LIBRARIES
#include <iostream> //Yeah.
#include <fstream> //File input/output.
#include <string> //Strings!
#include <sstream> //Stringstream
#include <filesystem> //Great for ease and function overloading things that allow wide character sets compared to Boost.
#include <chrono> //Time tracking.
#include <vector> //vector.
#include <unordered_map> //Map.
#include "thread_pool.hpp" //Thread pool stuff.


//Sets global delimiter used for reading and writing DB files. Tilde typically works well. (CONSIDER USING MULTIPLE CHARACTER DELIMITER FOR SAFETY)
std::wstring delimitingCharacter = L"▼";
//Simple newline dude.
std::wstring newLine = L"\n";

//Verbose debugging variables
std::ofstream verboseDebugOutput; //Hold potential future file handle if verbose debugging is enabled.
std::wstring debugFilePath = L"";
std::wstring debugFileName = L"debug.log";
int debugFileCount = 1;

//Holds an array of single letter arguments that need to be applied.
std::unordered_map<char, int> singleCharArguments;





int main(int argc, char* argv[])
{
	//Expected arguments:
		//Optional: path to 7z.exe
		//Required: File path to file to brute force
		//Several options that disable what characters to use
			//--no-special
			//--no-lowercase
			//--no-uppercase
			//--no-numbers
			//--exclude-characters "<CHARACTERS>"
			//--minimum-length
			//--maximum-length 

	//Verify that given 7z.exe path is legitimate.
		//If it is not legitimate, look for 7z.exe in common locations.
	
	//Verify that file to brute-force is legitimate.
		//Does the file exist?
		//Is it actually encrypted?

	//

	return 1;
}