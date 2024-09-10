#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>

void printASCII() {
    std::cout << "CSOPESY\n";
    // TODO: Replace it with an ASCII art
}

void printMessage() {
    std::cout << "Type a keyword to continue...\n";
    std::cout << "[Keywords]: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit\n\n";
    std::cout << "Enter a Command: ";
}

void acceptString(const std::string& str) {
    std::cout << str << " command recognized. Doing something...\n";
}

// Function to check whether the input string is in the pool of available keyword
bool checkKeyword(const std::string& input, const std::string keyword[], int size) {
    for (int i = 0; i < size; ++i) {
        if (input == keyword[i]) {
            return true;
        }
    }
    return false;
}

// Function to parse and handle the 'screen' command
void handleScreenCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string screenCmd, option, name;
    
    iss >> screenCmd >> option >> name;

    if (option == "-r") {
        std::cout << "screen -r command recognized. Doing something..." << "\n";
        // TODO: Add functionality 
    } else if (option == "-ls") {
        std::cout << "screen -ls command recognized. Doing something...\n";
        // TODO: Add functionality 
    } else if (option == "-s") {
        std::cout << "screen -s command recognized. Doing something..." << "\n";
        // TODO: Add functionality 
    } else {
        std::cout << "Invalid screen command! Try using -r, -ls, or -s.\n";
    }
}

int main() {
    printASCII();
    printMessage();

    bool active = true;
    std::string input;
	
	// List of Available Keywords
    std::string keyword[] = {
        "initialize", 
        "screen", 
        "scheduler-test", 
        "scheduler-stop", 
        "report-util", 
        "clear", 
        "exit"
    };

    int keywordSize = sizeof(keyword) / sizeof(keyword[0]);

    while (active) {
    	
        std::getline(std::cin, input); 
        std::istringstream iss(input);
        std::string command;
        iss >> command; 

        if (checkKeyword(command, keyword, keywordSize)) {
            if (command == "exit") {
                active = false;
            } else if (command == "clear") {
                system("cls"); 
                printASCII();
                printMessage();
            } else if (command == "initialize") {
                acceptString(command);
                // TODO add function
            } else if (command == "screen") {
                handleScreenCommand(input);
                // TODO add function
            } else if (command == "scheduler-test") {
                acceptString(command);
                // TODO add function
            } else if (command == "scheduler-stop") {
                acceptString(command);
                // TODO add function
            } else if (command == "report-util") {
                acceptString(command);
                // TODO add function
            }
        } else {
            std::cout << "Command not recognized, check your input!\n";
        }
    }

    return 0;
}
	