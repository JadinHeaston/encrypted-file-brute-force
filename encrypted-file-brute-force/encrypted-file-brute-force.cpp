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

#include <Windows.h> //Allows me to show/hide the console window. Also allows MultiByteToChar function thing.
#include <stdio.h> //Allows us to execute commands.


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


thread_pool mainThreadPool(std::thread::hardware_concurrency());

thread_pool writeFilesThreadPool(1); //Dedicated to writing to the debug file.


//Global variables.
std::wstring encryptedFile = L"C:/Users/Jadin-PC/source/repos/JadinHeaston/encrypted-file-brute-force/encrypted-file-brute-force/Encrypted.7z";
std::wstring sevenZipExePath; //Holds the path to the 7-Zip executable.


//Function prototypes
std::wstring charToWString(char* givenCharArray); //Used during beginning argument search to integrate provided char switches to fit in internally used wide strings.
std::wstring formatFilePath(std::wstring givenFile, std::wstring givenDirectorySeparator = L"");
std::wstring stringToWString(const std::string& s);
std::wstring increasePasswordLength(std::wstring givenPassword, std::unordered_map<wchar_t, bool> givenCharacterMap, size_t positionToAddTo, wchar_t initialCharacter, wchar_t finalCharacter);
std::wstring tryPassword(std::wstring givenPassword);
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring);
void writeToFile(std::wstring givenFilePath, std::chrono::system_clock::time_point givenTime, bool writeTime, std::wstring textToWrite);

//Password used for testing: ThisIsATestPassword
int main(int argc, char* argv[])
{
	const std::wstring sevenZipProgramFilesPath = L"C:/Program Files/7-Zip/7z.exe";
	std::wstring givenSevenZipPath;
	std::wstring givenEncryptedFilePath;
	std::wstring recordsOutputLocation;

	bool noLowercaseCharacters = false;
	bool noNumbers = false;
	bool noSpaces = false;
	bool noSpecialCharacters = false;
	bool noUppercaseCharacters = false;

	bool noRecords = false; //Specifies if files are written containing debugging and general information.

	std::wstring excludedCharacters = L""; //Holds manuallly excluded characters given by the user.

	size_t minimumLength = 0;
	size_t maximumLength = 99;

	std::wstring lowercaseCharacters = L"abcdefghijklmnopqrstuvwxyz";
	std::wstring numbers = L"0123456789";
	std::wstring space = L" ";
	std::wstring specialCharacters = L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	std::wstring uppercaseCharacters = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::wstring allCharacters = lowercaseCharacters + numbers + space + specialCharacters + uppercaseCharacters; //A combination of the above.

	char userInput[1]; //Holds user input character.

	//Expected arguments:
	//Optional: path to 7z.exe
		//The program will check common places it is installed.
		//It will also check if it is a PATH Variable.
	//Required: File path to file to brute force
	//Several options that help speed up the process by eliminating possibilities.
		//-l (no lowercase)
		//-n (no numbers)
		//-p (no spaces)
		//-s (no special)
		//-u (no uppercase)
		//-r (no records)
		//--exclude-characters "<CHARACTERS>"
		//--minimum-length <NUMBER> (Default is 0, max is size_t)
		//--maximum-length <NUMBER> (Default is 99, max is size_t)
		//--ignored-password-list "<PATH_TO_LIST>"
		//--threads <NUMBER_OF_THREADS> (Defaults to system amount)
		//--records-output-location <PATH_TO_OUTPUT_DIRECTORY>
		//--rainbow <PATH_TO_RAINBOW_TABLE>

	for (int i = 0; i < argc; i++) // Cycle through all arguments.
	{
		//Check if the argument contains a single or double slash
		if (strncmp(argv[i], "--", 2) == 0) //Check for double slash
		{
			if (strcmp(argv[i], "--help") == 0) //Checking second argument for if it is "-h" or "-help".
			{
				//Display help
				std::cout << "HELP PROVIDED. GET FUCKED" << std::endl;

				system("PAUSE");
				return 0;
			}
			else if (strcmp(argv[i], "--exclude-characters") == 0)
				excludedCharacters = charToWString(argv[i + 1]);
			else if (strcmp(argv[i], "--records-output-location") == 0)
				recordsOutputLocation = formatFilePath(charToWString(argv[i + 1]));
		}
		else if (strncmp(argv[i], "-", 1) == 0) //Check for single dash.
		{
			for (int iterator = 1; iterator < sizeof(argv[i]); ++iterator) //Iterating through all characters, after the slash. (Starting at 1 to skip the initial dash)
				singleCharArguments[tolower(argv[i][iterator])] = 1; //Ensuring keys are lowercase for easy use later.
		}

		//Iterating through argument array and applying arguments.
		for (size_t iterator = 0; iterator < sizeof(singleCharArguments); ++iterator)
		{
			//std::cout << singleCharArguments['h'] << std::endl;
			if (singleCharArguments['h']) //Short help message.
			{
				//Display help message.
				std::cout << "HELP" << std::endl;
				system("PAUSE");
				return 0;
			}
			if (singleCharArguments['l']) //No lowercase characters
				noLowercaseCharacters = true;
			if (singleCharArguments['n']) //No numbers
				noNumbers = true;
			if (singleCharArguments['p']) //No spaces
				noSpaces = true;
			if (singleCharArguments['r'])
				noRecords = true;
			if (singleCharArguments['s']) //No special characters
				noSpecialCharacters = true;
			if (singleCharArguments['u']) //No uppercase characters
				noUppercaseCharacters = true;
		}
	}
	//ARGUMENTS FINISHED.

	//Determining if records are kept, creating the folder required and files inside of it that are needed.
	//
	if (!noRecords)
	{
		//Check that folder exists.
		if (!std::filesystem::is_directory(recordsOutputLocation))
		{
			system("PAUSE");
			//Notify user. Ask if they want to use the default, not record stuff, or end the program.

			//Ask user if they want to use the default location.
				//If not, ask the user if they want to continue without recording or end the program.
		}
	}





	//*****
	//The exe needs verified. Maybe a command can be run and an output used as verification?
	//Verify that given 7z.exe path is legitimate.
		//If it is not legitimate, look for 7z.exe in common locations.
		//If the given path is a directory, check the top level files for 7z.exe
	if (std::filesystem::is_directory(givenSevenZipPath))
	{
		//Check through the top level of the directory for 7z.exe.
		//system("PAUSE");
	}



	if (std::filesystem::is_regular_file(givenSevenZipPath))
		sevenZipExePath = givenSevenZipPath; //Use the provided path.
	else if (std::filesystem::is_regular_file(sevenZipProgramFilesPath))
	{
		std::wcout << L"A valid 7z.exe was not found from the given path - " << givenSevenZipPath << std::endl;
		std::wcout << L"A valid 7z.exe was found under '" << sevenZipProgramFilesPath << L"'. Would you like to use this exe? (Y/N)" << std::endl;


		std::cin >> userInput[0]; //Awaiting user input...

		//Verify if the user is okay with continuing.
		if (tolower(userInput[0]) == 'y') //The input is NOT a "Y".
			sevenZipExePath = sevenZipProgramFilesPath; //Use the Program Files path.
		else //The user gave the okay. Continue.
		{
			std::cout << "No valid 7z.exe to use. Program ending." << std::endl;
			system("PAUSE");
			return 0;
		}
	}


	//Verify that file to brute-force is legitimate.
		//Does the file exist?
		//Is it actually encrypted?
	if (std::filesystem::is_regular_file(givenEncryptedFilePath))
	{
		//Check that the file is encrypted.
			//If file starts with 7z (7z), or pk (zip), then it is a compressed file.
		//system("PAUSE");
	}
	//else if () //Check for if it is a PATH variable.

	//Use a provided rainbow table first, if provided.


	std::unordered_map<wchar_t, bool> characterArray;


	for (size_t iterator = 0; iterator < allCharacters.size(); ++iterator)
		characterArray[allCharacters[iterator]] = true; //Initialize MAP to use ALL characters.

	//Removing certain charactes based on user input character array.
	if (noLowercaseCharacters)
	{
		//Iterate through string and add it to the main characterArray.
		for (size_t iterator = 0; iterator < lowercaseCharacters.size(); ++iterator)
			characterArray.erase(lowercaseCharacters[iterator]);
	}
	if (noNumbers)
	{
		//Iterate through string and add it to the main characterArray.
		for (size_t iterator = 0; iterator < numbers.size(); ++iterator)
			characterArray.erase(numbers[iterator]);
	}
	if (noSpaces)
	{
		//Iterate through string and add it to the main characterArray.
		for (size_t iterator = 0; iterator < space.size(); ++iterator)
			characterArray.erase(space[iterator]);
	}
	if (noSpecialCharacters)
	{
		//Iterate through string and add it to the main characterArray.
		for (size_t iterator = 0; iterator < specialCharacters.size(); ++iterator)
			characterArray.erase(specialCharacters[iterator]);
	}
	if (noUppercaseCharacters)
	{
		//Iterate through string and add it to the main characterArray.
		for (size_t iterator = 0; iterator < uppercaseCharacters.size(); ++iterator)
			characterArray.erase(uppercaseCharacters[iterator]);
	}

	std::unordered_map<wchar_t, bool>::iterator characterArrayIterator; //Creating an iterator to see the key using "iterator->first"

	//Iterate through excluded characters list, and make sure they are excluded.
	if (excludedCharacters != L"")
	{
		for (characterArrayIterator = characterArray.begin(); characterArrayIterator != characterArray.end(); ++characterArrayIterator) //Iterate through character array.
		{
			for (size_t iteratorTwo = 0; iteratorTwo < excludedCharacters.size(); ++iteratorTwo) //Iterate through all excluded characters
				characterArray.erase(excludedCharacters[iteratorTwo]); //Remove the excluded character.
		}
	}


	std::cout << "Minimum Length: " << minimumLength << std::endl;
	std::cout << "Maximum Length: " << maximumLength << std::endl;
	std::cout << "characterArray Size: " << characterArray.size() << std::endl;
	//Calculate possible number of passwords.
	unsigned long long possiblePasswordsCount = pow(maximumLength, characterArray.size()) - pow(minimumLength, characterArray.size());
	std::cout << "Maximum possible passwords: " << possiblePasswordsCount << std::endl;
	//Generate passwords to use and output them if the user says to.
	//if (outputAllPossiblePasswords)
	//{
	//	for (size_t iterator = minimumLength; iterator < maximumLength; ++iterator)
	//	{
			
	//	}
	//}
	


	characterArrayIterator = characterArray.begin(); //Initializing iterator.
	wchar_t firstCharacter = characterArrayIterator->first; //Holds first value of map.
	wchar_t finalCharacter; //Will hold last value in the map.
	for (characterArrayIterator; characterArrayIterator != characterArray.end(); ++characterArrayIterator) //Iterating through the map to get the final value.
		finalCharacter = characterArrayIterator->first;
	std::wcout << L"First character: " << firstCharacter << std::endl;
	std::wcout << L"Final character: " << finalCharacter << std::endl;

	//Begin brute forcing.
	for (characterArrayIterator = characterArray.begin(); characterArrayIterator != characterArray.end(); ++characterArrayIterator)
		std::wcout << characterArrayIterator->first; //Initialize MAP to use ALL characters.

	std::cout << std::endl;
	std::wcout << allCharacters << std::endl;

	std::wstring currentPassword;
	currentPassword.push_back(firstCharacter); //Holds the current password.
	size_t counter = 0;
	//system("PAUSE");
	//std::wcout << currentPassword << std::endl;
	//system("PAUSE");
	//CConsoleLogger another_console;
	//another_console.Create("This is the first console");
	::ShowWindow(GetConsoleWindow(), SW_HIDE);
	writeFilesThreadPool.push_task(writeToFile, L"output.txt", std::chrono::system_clock::now(), true, L"Starting cracking!");
	while (currentPassword.size() <= maximumLength) //Iterate through all lengths.
	{
		currentPassword = increasePasswordLength(currentPassword, characterArray, currentPassword.size(), firstCharacter, finalCharacter);

		for (characterArrayIterator = characterArray.begin(); characterArrayIterator != characterArray.end(); ++characterArrayIterator)
		{
			currentPassword.pop_back();
			currentPassword.push_back(characterArrayIterator->first);
			//if (currentPassword == L"#1")
			//writeFilesThreadPool.push_task(writeToFile, L"password.txt", std::chrono::system_clock::now(), true, counter + L" - " + currentPassword);
			//Send to console, if it is desired.
			//if (!hideConsole)

			//tryPassword(currentPassword);
			mainThreadPool.push_task(tryPassword, currentPassword);
			counter++; //Increment counter.
		}


		//Once one million passwords are assigned, wait for them to be completed.
		if (counter >= 1000000)
		{
			writeFilesThreadPool.push_task(writeToFile, L"output.txt", std::chrono::system_clock::now(), true, L"Current Password at " + std::to_wstring(counter) + L": " + currentPassword);
			mainThreadPool.wait_for_tasks();
			writeFilesThreadPool.push_task(writeToFile, L"output.txt", std::chrono::system_clock::now(), true, L"Finished waiting...");
			counter = 0; //Reset counter.
		}

	}


	writeFilesThreadPool.wait_for_tasks();
	system("PAUSE");
	return 1;
}

std::wstring increasePasswordLength(std::wstring givenPassword, std::unordered_map<wchar_t, bool> givenCharacterMap, size_t positionToAddTo, wchar_t initialCharacter, wchar_t finalCharacter)
{
	positionToAddTo = positionToAddTo - 1; //Remove one to account for .size not being the index, and recursion.

	if (givenPassword[positionToAddTo] == finalCharacter) //If the last character is at it's limit, then we need to set it to the initial character and add one to the previous character.
	{
		givenPassword[positionToAddTo] = initialCharacter; //Set this character to be the initial character

		if (positionToAddTo == 0) //If the position is the first character, then add a new character at the end with the intitial character.
		{
			givenPassword.push_back(initialCharacter); 
			return givenPassword;
		}

		givenPassword = increasePasswordLength(givenPassword, givenCharacterMap, positionToAddTo, initialCharacter, finalCharacter);
	}
	else
	{
		std::unordered_map<wchar_t, bool>::iterator givenCharacterMapIterator = givenCharacterMap.begin();
		for (givenCharacterMapIterator; givenCharacterMapIterator != givenCharacterMap.end(); ++givenCharacterMapIterator) //Iterate through character array.
		{
			if (givenPassword[positionToAddTo] == givenCharacterMapIterator->first)
			{
				givenCharacterMapIterator++;
				givenPassword[positionToAddTo] = givenCharacterMapIterator->first; //Once a match is found, set the value to the NEXT character in the array.
			}
		}
	}

	return givenPassword;
}

//Converts character arrays to wstring.
std::wstring charToWString(char* givenCharArray)
{
	std::string intermediaryString = givenCharArray;
	int wchars_num = MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(65001, 0, intermediaryString.c_str(), -1, wstr, wchars_num);

	return wstr;
}

//Uniformly sets directory separators.
std::wstring formatFilePath(std::wstring givenString, std::wstring givenDirectorySeparator)
{


	if (givenDirectorySeparator == L"\\" || givenString.find(L"\\\\?\\") != std::wstring::npos) //If the windows max_path bypass is in the path, then all separators must be backslashes.
	{
		//Formating givenFile to have the slashes ALL be \.
		for (int i = 0; i < (int)givenString.length(); ++i)
		{
			if (givenString[i] == '/')
				givenString[i] = '\\';
		}
	}
	else
	{
		//Formating givenFile to have the slashes ALL be /.
		for (int i = 0; i < (int)givenString.length(); ++i)
		{
			if (givenString[i] == '\\')
				givenString[i] = '/';
		}
	}


	return givenString;
}

//Converts string to WString.
std::wstring stringToWString(const std::string& s)
{
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

//As said.
std::wstring tryPassword(std::wstring givenPassword)
{
	//7z.exe - p"ThisIsATestPassword" - w - y t "T:\temp transfers\AES.zip"//
	size_t lastFoundPosition = 0; //Initialize Variable
	while (givenPassword.find(L"\"", lastFoundPosition) != std::wstring::npos) //Find the next quotation mark.
	{
		givenPassword.replace(givenPassword.find(L"\""), 1, L"\\\""); //Replace it with an escaped version.
		lastFoundPosition = givenPassword.find_last_of(L"\\\"") + 2; //The position after the newly placed escaped version of a quote.
	}
	std::wstring commandToRun = L"\"\"" + sevenZipExePath + L"\" -p\"" + givenPassword + L"\" -w -y t \"" + encryptedFile + L"\"\"";
	//writeFilesThreadPool.push_task(writeToFile, L"commands.txt", std::chrono::system_clock::now(), true, givenPassword + L" - " + commandToRun);
	
	wchar_t buffer[128];
	std::wstring result = L"";

	// Open pipe to file
	FILE* pipe = _wpopen(commandToRun.c_str(), L"r");
	if (!pipe)
	{
		std::cout << "YO. WE DEAD BRO" << std::endl;
		system("PAUSE");
		return L"FAILURE"; //Popen failed.
	}

	// read till end of process:
	while (!feof(pipe))
	{
		//Use buffer to read and add to result
		if (fgetws(buffer, 128, pipe) != NULL)
			result += buffer;

		if (result.find(L"Can not open encrypted archive. Wrong password?")) // :(
		{
			_pclose(pipe); //Close pipe.
			//writeFilesThreadPool.push_task(writeToFile, L"all.txt", std::chrono::system_clock::now(), true, givenPassword + L" - " + commandToRun + L" - " + result);
			return L"";
		}
		else if (result.find(L"Command Line Error"))
			writeFilesThreadPool.push_task(writeToFile, L"debug.txt", std::chrono::system_clock::now(), true, givenPassword + L" - " + commandToRun);
		else if (result.find(L"Everything is Ok")) // :)
		{
			_pclose(pipe); //Close pipe.
			::ShowWindow(GetConsoleWindow(), SW_SHOW);
			std::wcout << "PASSWORD FOUND: " << givenPassword << std::endl;
			system("PAUSE");
			return givenPassword;
		}
		//fflush(pipe);
	}

	_pclose(pipe); //Close pipe.


	//Final check.
	if (result.find(L"ERROR: Wrong password")) // :(
		return L"";
	else if (result.find(L"Command Line Error: The command must be specified."))
		writeFilesThreadPool.push_task(writeToFile, L"debug.txt", std::chrono::system_clock::now(), true, givenPassword + L" - " + commandToRun);
	else if (result.find(L"Everything is Ok")) // :)
	{
		::ShowWindow(GetConsoleWindow(), SW_SHOW);
		std::wcout << "PASSWORD FOUND: " << givenPassword << std::endl;
		system("PAUSE");
		return givenPassword;
	}
	return L"";
}

//Writes to the the debug file.
void writeToFile(std::wstring givenFilePath, std::chrono::system_clock::time_point givenTime, bool writeTime, std::wstring textToWrite)
{
	////open the current debug file.
	//std::ifstream temporaryDebugHandle(debugFilePath + debugFileName, std::ios::in | std::ios::binary | std::ios::ate); //Start the cursor at the very end.
	//size_t fileSize = temporaryDebugHandle.tellg(); //Get the file size in bytes.
	//temporaryDebugHandle.close(); //Close temporary handle reading 

	//if (fileSize >= 102857600) //100 MB
	//{
	//	//Change the file name.
	//	if (debugFileName.find(L"."))
	//		debugFileName = debugFileName.insert(debugFileName.find(L"."), L" - " + std::to_wstring(debugFileCount));
	//	else
	//		debugFileName = debugFileName.append(L" - " + std::to_wstring(debugFileCount));
	//	++debugFileCount; //Incrememnt debug file count.
	//}
	std::ofstream fileHandle(givenFilePath, std::ios::out | std::ios::binary | std::ios::app); //Open the debug file for writing.

	//Get the size.
	//If that size is larger than 
	if (writeTime) //Check if we are writing time.
	{
		char buff[20]; //Create buffer.

		std::time_t newTimeValue = std::chrono::system_clock::to_time_t(givenTime); //Convert given time to time_t.

		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&newTimeValue)); //Format time. Place it into buffer.

		std::string temporaryString = buff; //Put the buffer into the string;

		std::wstring timeValue = stringToWString(temporaryString); //Converting string to wstring.

		timeValue = timeValue + L": "; //Appending "delimiter".

		//Add the current time of writing.
		writeUnicodeToFile(fileHandle, timeValue + textToWrite + newLine); //Write this to the debug file.
	}
	else
	{
		writeUnicodeToFile(fileHandle, textToWrite + newLine); //Write this to the debug file.
	}

	fileHandle.close(); //Close file.

}

//Writing unicode to file. Stolen from https://cppcodetips.wordpress.com/2014/09/16/reading-and-writing-a-unicode-file-in-c/
void writeUnicodeToFile(std::ofstream& outputFile, std::wstring inputWstring)
{
	outputFile.write((char*)inputWstring.c_str(), inputWstring.length() * sizeof(wchar_t));
}