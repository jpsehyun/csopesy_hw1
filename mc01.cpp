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
#include <mutex>
#include <atomic>

/*##################################################################
* Instructions: Type screen -s <name> to create a process
* For now it applies FCFS automatically to the process created in a queue

* You can change the number of cores at the bottom of this box

##################################################################*/

#define NUM_CORES 4

// Bad practice (global var), lazy way to count the number of proceess
std::atomic<int> globalProcessNumber = 1;

// New class for process (terminal =/= process right?)
class Process
{
private:
    std::string pname;                          // user-defined process name 
    int pid;                                    // id for each processes (e.g. Process 1)
    int numCommands;                            // Total number of commands
    int numFinishedCommands;                    // Number of commands executed
    int coreId;                                 // Core executing this process
    std::time_t startTime;                      // Process start time
    std::vector<std::time_t> commandTimestamps; // Timestamps for each command execution

public:
    Process(int id, int commands, int core)
        : pid(id), numCommands(commands), numFinishedCommands(0), coreId(core), startTime(0)
    {
        pname = std::to_string(id);
    }

    Process(const std::string& procName, int id,  int commands, int core)
        : pname(procName), pid(id), numCommands(commands), numFinishedCommands(0), coreId(core), startTime(0) {}

    std::string getName() const
    {
        return pname;
    }

    int getPid() const
    {
        return pid;
    }

    int getFinishedCommands() const
    {
        return numFinishedCommands;
    }

    int getTotalCommands() const
    {
        return numCommands;
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
        return numFinishedCommands >= numCommands;
    }

    void executePrintCommands()
    {
        // saves and creates the .txt file
        std::ostringstream filename;
        filename << "process_" << pid << ".txt";
        std::ofstream logFile(filename.str(), std::ios_base::app);

        while (numFinishedCommands < numCommands)
        {
            // Need to sleep or else the system process the process too fast
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::time_t currentTime = std::time(nullptr);
            commandTimestamps.push_back(currentTime);

            char buffer[80];
            struct tm timeInfo;
            localtime_s(&timeInfo, &currentTime);
            std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);


            logFile << buffer << " core:" << coreId << " Hello world from process " << pname << std::endl;

            numFinishedCommands++;
        }

        logFile.close();
    }

    void startProcessLoop()
    {
        std::string command;
        while (true)
        {
            std::cout << ">>" << pname << ": "; // Show the process prompt
            std::getline(std::cin, command);

            if (command == "exit")
            {
                break;
            }
            else
            {
                std::cout << "Command not recognized. Only 'exit' is allowed in the screen.\n";

                // After each invalid command, show the updated current line count
                printAttributes(); // Display the updated terminal state
            }
        }
    }

    void printAttributes() const
    {
        std::cout << "Terminal Name: " << pname << std::endl;
        std::cout << "Instruction Line: " << numFinishedCommands << " / " << numCommands << std::endl;
    }
};

class FCFS_Scheduler
{
private:
    // shared_ptr is a smart pointer that automatically manages the memory of an object, 
    // allocating it when created and deleting it when no longer referenced. As we do manually noramlly.
    // AKA better version of doing Process* (I just learned this)

    std::queue<std::shared_ptr<Process>> processQueue; // Queue to schedule process
    std::vector<std::thread> coreThreads; // Store threads for each core
    std::vector<std::shared_ptr<Process>> allProcesses; // Track all processes
    std::vector<bool> coreBusy; // Track which cores are busy

    std::mutex queueMutex;
    int numCores;

public:
    FCFS_Scheduler(int numCore) : coreBusy(numCore, false), numCores(numCore)
    {
        // Create and start a new thread for each core
        // Each core gets their coreThreadFunctions
        for (int i = 0; i < numCore; i++)
        {
            coreThreads.emplace_back(&FCFS_Scheduler::coreThreadFunction, this, i);
        }
    }

    void addProcess(const Process& p)
    {
        // new Process object is created through the p reference we gave at the parameter
        // make_shared creates a shared_ptr
        auto processPtr = std::make_shared<Process>(p);
        std::lock_guard<std::mutex> lock(queueMutex);

        // Add the process to the queue (at the front)
        // The core thread will pop the process inside this queue when they work on it
        processQueue.push(processPtr);

        // Also add the process to the vector, (at the back).
        // This is to keep track of all processes
        allProcesses.push_back(processPtr);

    }

    void coreThreadFunction(int coreId)
    {
        while (true)
        {
            std::shared_ptr<Process> processPtr = nullptr;

            {
                std::lock_guard<std::mutex> lock(queueMutex);

                // Smart way to check if a certain coreId is the lowest available core
                bool lowestAvailableCore = true;
                for (int i = 0; i < coreId; i++)
                {
                    if (!coreBusy[i])
                    {
                        lowestAvailableCore = false;
                        break;
                    }
                }

                // if the Queue is not empty and this core is the lowest empty core
                // grab the process from the queue, and remove it from the queue
                // assign the core Id and start time
                if (lowestAvailableCore && !processQueue.empty())
                {
                    processPtr = processQueue.front();
                    processQueue.pop();
                    processPtr->setCoreId(coreId);
                    processPtr->setStartTime();
                }
            }

            // if there is a process that is being worked on mark the core as busy
            // execute the print command for the process
            // afterwards, core is no longer busy
            if (processPtr)
            {
                coreBusy[coreId] = true;
                processPtr->executePrintCommands();
                coreBusy[coreId] = false;
            }

            // standby for few timeframe if no job is assigned
            if (!processPtr)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        system("cls");

        std::cout << "Current Status of Running Processes:\n";
        for (const auto& process : allProcesses)
        {
            // Only show processes that have been assigned to a core and are not finished
            if (process->getCoreId() >= 0 && !process->isFinished())
            {
                std::string startTimeFormatted = formatTime(process->getStartTime());

                std::cout << "Process " << process->getName() << " (" << startTimeFormatted << ") (Core "
                    << process->getCoreId() << "): "
                    << process->getFinishedCommands() << "/" << process->getTotalCommands() << " commands executed.\n";
            }
        }

        std::cout << "\nFinished Processes:\n";
        for (const auto& process : allProcesses)
        {
            if (process->isFinished())
            {   // Only show finished processes
                std::string startTimeFormatted = formatTime(process->getStartTime());
                std::cout << "Process " << process->getName() << " (" << startTimeFormatted << "): "
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
void handleScreenCommand(const std::string& command, std::vector<Process>& processes, bool& isTerminalOpen, FCFS_Scheduler& scheduler)
{
    std::istringstream iss(command);
    std::string screenCmd, option, name;

    iss >> screenCmd >> option >> name;

    // -s command creates a new instance of Terminal Class
    if (option == "-s" && !name.empty())
    {
        // Check for duplicate terminal names
        for (const auto& p : processes)
        {
            if (p.getName() == name)
            {
                std::cout << "Error: Terminal name '" << name << "' already exists!\n";
                return;
            }
        }

        // Creates a new instance of the Terminal class and stores it in the vector
        Process newProcess(name, globalProcessNumber, 100 ,-1);
        globalProcessNumber++;
        /* //////////////////////////////////////////////////////*/

        // TODO set the command line from config.txt (it's currently static 100)
        // Need to pass the # of command line from config.txt to the parameter

        /* //////////////////////////////////////////////////////*/
        processes.push_back(newProcess);

        system("cls");
        newProcess.printAttributes();
        // Terminal is considered open if we successfully call the printAttributes() function
        isTerminalOpen = true;
        newProcess.startProcessLoop(); // enter the screen's command loop

        // Update the terminal object in the vector after the session ends
        for (auto& p : processes)
        {
            if (p.getName() == name)
            {
                p = newProcess;  // Save the updated terminal state (with currentLine)
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
        for (auto& p : processes) // removed const
        {
            if (p.getName() == name)
            {
                system("cls");
                p.printAttributes();
                isTerminalOpen = true;
                p.startProcessLoop(); // enter the screen's command loop

                // Update the terminal object in the vector after the session ends
                for (auto& process : processes)
                {
                    if (process.getName() == name)
                    {
                        process = p;  // Save the updated terminal state (with currentLine)
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

void processSchedulerAutoAdder(FCFS_Scheduler& scheduler, std::vector<Process>& processes)
{
    std::size_t lastCheckedSize = 0; // Keep track of how many processes have been added

    while (true) {
        // Check for new processes in the vector
        if (processes.size() > lastCheckedSize) {
            for (std::size_t i = lastCheckedSize; i < processes.size(); i++) {
                scheduler.addProcess(processes[i]);  // Add new processes to the scheduler
            }
            lastCheckedSize = processes.size();  // Update the last checked size
        }
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

    // TODO: Declare a variable which are to be retrieved from config.txt
    int numCore = NUM_CORES;

    while (true) {
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "initialize") {
            // TODO: read from config.txt and assign the values

            system("cls");

            printASCII();
            printWelcomeMessage();
            printMessage();
            break;
        }
        else if (command == "exit") {
            exit(0);
        }
        else {
            std::cout << "You must initialize before performing any command! [initialize]\n";
            printMessage();
        }
    }

    // Potential Design
    // if (mode == "fcfs")
    FCFS_Scheduler scheduler(numCore);
    // else if (mode == "rr")
    // RR_Scheduler scheduler;

    // Vector to hold Process instances
    std::vector<Process> processes;

    // Thread which constantly accepts new process into the process vector
    // (Since users is now able to add processes into the ready queue while the scheduler is running)
    std::thread processAdderThread(processSchedulerAutoAdder, std::ref(scheduler), std::ref(processes));
    processAdderThread.detach();

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
            std::cout << "Already Initialized!\n";
        }
        else if (command == "screen")
        {
            handleScreenCommand(input, processes, isTerminalOpen, scheduler);
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