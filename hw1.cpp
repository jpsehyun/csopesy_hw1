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
    int currentLine;
    int totalLines;

public:
    Terminal() : currentLine(1), totalLines(10) {} // For now our default total line of instructions is 10

    void setAttributes(const std::string& terminal)
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

    // Number of lines functions
    int getCurrentLine() const
    {
        return currentLine;
    }

    int getTotalLines() const
    {
        return totalLines;
    }

    void incrementCurrentLine()
    {
        if (currentLine < totalLines)
        {
            currentLine++;
        }
    }

    void printAttributes() const
    {
        std::cout << "Terminal Name: " << name << std::endl;

        std::cout << "Instruction Line: " << currentLine << " / " << totalLines << std::endl;

        // Formatted timestamp
        char buffer[80];
        struct tm timeInfo;
        localtime_s(&timeInfo, &timestamp);  // Safe version of localtime
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);
        std::cout << "Timestamp: " << buffer << std::endl;
    }

    // Added logic when not in the main console
    void startProcessLoop()
    {
        std::string command;
        while (true)
        {
            std::cout << ">>" << name << ": ";  // Show the process prompt
            std::getline(std::cin, command);

            if (command == "exit")
            {
                break;  // Exit the process and return to the main menu
            }
            else
            {
                std::cout << "Command not recognized. Only 'exit' is allowed in the screen.\n";
                incrementCurrentLine();  // Increment current line of instruction, simply for tracking purposes right now

                // After each invalid command, show the updated current line count
                printAttributes();  // Display the updated terminal state
            }
        }
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

void printAcceptMessage(const std::string& str)
{
    std::cout << str << " command recognized. Doing something...\n";
}

// Function to parse and handle the 'screen' command
void handleScreenCommand(const std::string& command, std::vector<Terminal>& terminals, bool& isTerminalOpen)
{
    std::istringstream iss(command);
    std::string screenCmd, option, name;

    iss >> screenCmd >> option >> name;

    // -s command creates a new instance of Terminal Class
    if (option == "-s" && !name.empty())
    {
        // Check for duplicate terminal names
        for (const auto& terminal : terminals)
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
        newTerminal.startProcessLoop(); // enter the screen's command loop
        
        // Update the terminal object in the vector after the session ends
        for (auto& terminal : terminals)
        {
            if (terminal.getName() == name)
            {
                terminal = newTerminal;  // Save the updated terminal state (with currentLine)
                break;
            }
        }

        isTerminalOpen = false; // reset after exiting process

        // Print main menu static text after exiting screen terminal
        system("cls");
        printASCII();
        printWelcomeMessage();
    }

    // -r command reloads a existing instance of Terminal Class
    else if (option == "-r" && !name.empty())
    {
        // Goes through every indicies of the vector to check for the name
        for (auto& terminal : terminals) // removed const
        {
            if (terminal.getName() == name)
            {
                system("cls");
                terminal.printAttributes();
                isTerminalOpen = true;
                terminal.startProcessLoop(); // enter the screen's command loop

                // Update the terminal object in the vector after the session ends
                for (auto& t : terminals)
                {
                    if (t.getName() == name)
                    {
                        t = terminal;  // Save the updated terminal state (with currentLine)
                        break;
                    }
                }

                isTerminalOpen = false; // reset after exiting process

                // Print main menu static text after exiting screen terminal
                system("cls");
                printASCII();
                printWelcomeMessage();
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
            if (isTerminalOpen) {
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