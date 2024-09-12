#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>

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
void handleScreenCommand(const std::string &command)
{
    std::istringstream iss(command);
    std::string screenCmd, option, name;

    iss >> screenCmd >> option >> name;

    if (option == "-r")
    {
        std::cout << "screen -r command recognized. Doing something..." << "\n";
    }
    else if (option == "-ls")
    {
        std::cout << "screen -ls command recognized. Doing something...\n";
    }
    else if (option == "-s")
    {
        std::cout << "screen -s command recognized. Doing something..." << "\n";
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
    std::string input;

    while (active)
    {

        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "exit")
        {
            active = false;
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
            handleScreenCommand(input);
            // TODO add function
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