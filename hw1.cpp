#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <ctime>
#include <vector>

class Terminal
{
private:
    std::string name;
    std::time_t timestamp;
    //TODO add Current line of instruction / Total line of instruction in attribute

public:
    void setAttributes(const std::string &terminal)
    {
        name = terminal;
        // Sets the time to the current time
        timestamp = std::time(nullptr);
    }

    std::string getName() const
    {
        return name;
    }

    std::time_t getTimestamp() const
    {
        return timestamp;
    }

    void printAttributes() const
    {
        std::cout << "Terminal Name: " << name << std::endl;
        std::cout << "Timestamp: " << std::ctime(&timestamp);
    }
};

void printASCII()
{
    std::cout << " _______  _______  _______  _______  _______  _______  __   __ \n";
    std::cout << "|       ||       ||       ||       ||       ||       ||  | |  |\n";
    std::cout << "|       ||  _____||   _   ||    _  ||    ___||  _____||  |_|  |\n";
    std::cout << "|       || |_____ |  | |  ||   |_| ||   |___ | |_____ |       |\n";
    std::cout << "|      _||_____  ||  |_|  ||    ___||    ___||_____  ||_     _|\n";
    std::cout << "|     |_  _____| ||       ||   |    |   |___  _____| |  |   |  \n";
    std::cout << "|_______||_______||_______||___|    |_______||_______|  |___|  \n";
}

void printWelcomeMessage()
{
    std::cout << "Type a keyword to continue...\n";
    std::cout << "[Keywords]: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit\n";
}

void printMessage()
{
    std::cout << "\nEnter a command: ";
}

void printAcceptMessage(const std::string &str)
{
    std::cout << str << " command recognized. Doing something...\n";
}

// Function to parse and handle the 'screen' command
void handleScreenCommand(const std::string &command, std::vector<Terminal> &terminals, bool &isTerminalOpen)
{
    std::istringstream iss(command);
    std::string screenCmd, option, name;

    iss >> screenCmd >> option >> name;
	
	// -s command creates a new instance of Terminal Class
    if (option == "-s" && !name.empty())
    {
        // Check for duplicate terminal names
        for (const auto &terminal : terminals)
        {
            if (terminal.getName() == name)
            {
                std::cout << "Error: Terminal name '" << name << "' already exists!\n";
                return; 
            }
        }
		
		// Creates a new instance of the Terminal class and stores it in the vector
        Terminal newTerminal;
        newTerminal.setAttributes(name);
        terminals.push_back(newTerminal); 
        
        system("cls");
        newTerminal.printAttributes();
        // Terminal is considered open if we successfully call the printAttributes() function
        isTerminalOpen = true;
    }
    
    // -r command reloads a existing instance of Terminal Class
    else if (option == "-r" && !name.empty())
    {	
    	// Goes through every indicies of the vector to check for the name
        for (const auto &terminal : terminals)
        {
            if (terminal.getName() == name)
            {
                system("cls");
                terminal.printAttributes();
                isTerminalOpen = true;
                return;
            }
        }
        std::cout << "Error: No terminal with name '" << name << "' found!\n";
    }
    else if (option == "-ls")
    {
        std::cout << "screen -ls command recognized. Doing something...\n";
    }
    else
    {
        std::cout << "Invalid screen command! Try using -r, -ls, or -s.\n";
    }
}

int main()
{
    printASCII();
    printWelcomeMessage();
    printMessage();

    bool active = true;
    bool isTerminalOpen = false;
    std::string input;
    
	// Vector to hold Terminal instances
    std::vector<Terminal> terminals;

    while (active)
    {
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "exit")
        {	
        	// If there is an open terminal, exit will go to main screen
        	if (isTerminalOpen){
	        	system("cls");
	            printASCII();
	            printWelcomeMessage();
	            isTerminalOpen = false;
			}
            else active = false;
        }
        else if (command == "clear")
        {
            system("cls");
            printASCII();
            printWelcomeMessage();
        }
        else if (command == "initialize")
        {
            printAcceptMessage(command);
            // TODO add function
        }
        else if (command == "screen")
        {
            handleScreenCommand(input, terminals, isTerminalOpen); 
        }
        else if (command == "scheduler-test")
        {
            printAcceptMessage(command);
            // TODO add function
        }
        else if (command == "scheduler-stop")
        {
            printAcceptMessage(command);
            // TODO add function
        }
        else if (command == "report-util")
        {
            printAcceptMessage(command);
            // TODO add function
        }
        else
        {
            std::cout << "Command not recognized, check your input!\n";
        }

        if (active)
        {
            printMessage();
        }
    }

    exit(0);
    return 0;
}
