#include <iostream>
#include <cstdlib>
#include <string>

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
        std::cin >> input;
		
        if (checkKeyword(input, keyword, keywordSize)) {
            if (input == "exit") {
                active = false;
            } else if (input == "clear") {
                system("cls"); 
                printASCII();
                printMessage();
            } else {
                acceptString(input);
            }
        } else {
            std::cout << "Command not recognized, check your input!\n";
        }
    }

    return 0;
}
