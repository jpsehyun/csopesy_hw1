#include <iostream> // I/O operations
#include <cstdlib>  // rand()
#include <string>   // string
#include <sstream>  // parse string
#include <ctime>    // timestamps
#include <vector>   // vectors
#include <thread>   // multi-threading
#include <queue>    // queue
#include <memory>   // smart pointers (shared_ptr)
#include <fstream>  // file operations

/*##################################################################
* Instructions: Type screen -ls repeatedly

* Mode 0 = random number of commands per process
* Mode 1 = static 100 commands per process
* You can change the number of cores and mode at the bottom of this box

##################################################################*/

#define NUM_CORES 4
#define MODE 0 // 0 = random number of commands per process
               // 1 = 100 commands each

// New class for process (terminal =/= process right?)
class Process
{
private:
    int pid;                                    // id for each processes (e.g. Process 1)
    int numPrintCommands;                       // Total number of commands
    int finishedCommands;                       // Number of commands executed
    int coreId;                                 // Core executing this process
    std::time_t startTime;                      // Process start time
    std::vector<std::time_t> commandTimestamps; // Timestamps for each command execution

public:
    Process(int id, int commands, int core)
        : pid(id), numPrintCommands(commands), finishedCommands(0), coreId(core), startTime(0) {}

    int getPid() const
    {
        return pid;
    }

    int getFinishedCommands() const
    {
        return finishedCommands;
    }

    int getTotalCommands() const
    {
        return numPrintCommands;
    }

    int getCoreId() const
    {
        return coreId;
    }

    void setCoreId(int core)
    {
        coreId = core;
    }

    void setStartTime()
    {
        startTime = std::time(nullptr);
    }

    std::time_t getStartTime() const
    {
        return startTime;
    }

    const std::vector<std::time_t>& getCommandTimestamps() const
    {
        return commandTimestamps;
    }

    bool isFinished() const
    {
        return finishedCommands >= numPrintCommands;
    }

    void executePrintCommands()
    {
        // saves and creates the .txt file
        std::ostringstream filename;
        filename << "process_" << pid << ".txt";
        std::ofstream logFile(filename.str(), std::ios_base::app); 

        while (finishedCommands < numPrintCommands)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
            std::time_t currentTime = std::time(nullptr); 
            commandTimestamps.push_back(currentTime);

            char buffer[80];
            struct tm timeInfo;
            localtime_s(&timeInfo, &currentTime);
            std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);

           
            logFile << buffer << " core:" << coreId << " Hello world from process " << pid << std::endl;

            finishedCommands++;
        }

        logFile.close();
    }
};

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
        localtime_s(&timeInfo, &timestamp); // Safe version of localtime
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);
        std::cout << "Timestamp: " << buffer << std::endl;
    }

    // Added logic when not in the main console
    void startProcessLoop()
    {
        std::string command;
        while (true)
        {
            std::cout << ">>" << name << ": "; // Show the process prompt
            std::getline(std::cin, command);

            if (command == "exit")
            {
                break; // Exit the process and return to the main menu
            }
            else
            {
                std::cout << "Command not recognized. Only 'exit' is allowed in the screen.\n";
                incrementCurrentLine(); // Increment current line of instruction

                // After each invalid command, show the updated current line count
                printAttributes(); // Display the updated terminal state
            }
        }
    }
};

class FCFS_Scheduler
{
private:
    std::queue<std::shared_ptr<Process>> processQueue; // Queue to schedule process
    std::vector<std::thread> coreThreads; // Store threads for each core
    std::vector<std::shared_ptr<Process>> allProcesses; // Track all processes
    std::vector<bool> coreBusy; // Track which cores are busy
    bool exit = false;

public:
    FCFS_Scheduler() : coreBusy(NUM_CORES, false)
    { // Initialize core status to be free
        // Create and start a new thread for each core, executing coreThreadFunction with the core ID.
        for (int i = 0; i < NUM_CORES; i++)
        {
            coreThreads.emplace_back(&FCFS_Scheduler::coreThreadFunction, this, i);
        }
    }

    void addProcess(const Process& p)
    {
        auto processPtr = std::make_shared<Process>(p); // Create shared pointer for the process
        processQueue.push(processPtr); // Push it to the queue
        allProcesses.push_back(processPtr); // Store all processes
    }

    void coreThreadFunction(int coreId)
    {
        while (true)
        {
            std::shared_ptr<Process> processPtr = nullptr;

            // Get the next process from the queue
            if (!processQueue.empty())
            {
                processPtr = processQueue.front(); // Get the shared pointer to the process
                processQueue.pop(); // Remove it from the queue
                processPtr->setCoreId(coreId); // Assign the core ID to the process
                processPtr->setStartTime(); // Set the start time when process is assigned to core
            }

            // Execute the process if it's assigned to this core
            if (processPtr)
            {
                coreBusy[coreId] = true; // Mark the core as busy
                processPtr->executePrintCommands(); // Execute commands for this process
                coreBusy[coreId] = false; // Mark the core as free
            }

            // Check for exit condition
            if (exit && processQueue.empty())
            {
                break;
            }
        }
    }

    std::string formatTime(std::time_t time)
    {
        std::tm localTime; // Create a tm structure to hold the local time
        localtime_s(&localTime, &time); // Use localtime_s to get the local time

        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", &localTime); // Format time as m/d/y h:m:s am/pm
        return std::string(buffer);
    }

    void printCoreStatus()
    {
        system("cls"); // Clear the console

        std::cout << "Current Status of Running Processes:\n";
        for (const auto& process : allProcesses)
        {
            // Only show processes that have been assigned to a core and are not finished
            if (process->getCoreId() >= 0 && !process->isFinished())
            {
                // Get the start time and format it
                std::string startTimeFormatted = formatTime(process->getStartTime());

                std::cout << "Process " << process->getPid() << " (" << startTimeFormatted << ") (Core "
                    << process->getCoreId() << "): "
                    << process->getFinishedCommands() << "/" << process->getTotalCommands() << " commands executed.\n";
            }
        }

        // Print finished processes
        std::cout << "\nFinished Processes:\n";
        for (const auto& process : allProcesses)
        {
            if (process->isFinished())
            {   // Only show finished processes
                std::string startTimeFormatted = formatTime(process->getStartTime());
                std::cout << "Process " << process->getPid() << " (" << startTimeFormatted << "): "
                    << process->getFinishedCommands() << "/" << process->getTotalCommands() << " commands executed.\n";
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
void handleScreenCommand(const std::string& command, std::vector<Terminal>& terminals, bool& isTerminalOpen, FCFS_Scheduler& scheduler)
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
        isTerminalOpen = true;
        // Start a thread to print the core status
        std::thread lsThread([&scheduler]() {
            scheduler.printCoreStatus();
            });
        lsThread.join(); // Wait for the thread to finish

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

    srand(static_cast<unsigned int>(time(0))); // rng

    FCFS_Scheduler scheduler; // Create scheduler with default cores
    int numCommands;

    // Initializes Processes upon launch (10 processes static for test case)
    for (int i = 1; i <= 10; ++i) {
#if MODE == 0
        // Each process has a random number of print commands
        numCommands = rand() % 100 + 50;
#else MODE == 1
        // Each process has exactly 100 commands
        numCommands = 100;
#endif
        scheduler.addProcess(Process(i, numCommands, -1)); // -1 indicates it is unassigned
    }

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
            handleScreenCommand(input, terminals, isTerminalOpen, scheduler);
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