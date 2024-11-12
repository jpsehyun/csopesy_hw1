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
#include <set>

/*##################################################################
* Instructions: Type initialize to start
*               To change the parameter, please edit config.txt

##################################################################*/

std::atomic<bool> schedulerRunning(false);  // Controls the scheduler-test state
std::atomic<bool> stopRequested(false);     // Controls when to stop scheduler-test
// Bad practice (global var), lazy way to count the number of proceess
std::atomic<int> globalProcessNumber = 1;


int minMemory = 0; // This would be minMemory += memoryFrame while its < memoryPerPrcoess (what is the minimum number of memory needed for each process)
int maxMemory;
int memoryFrame;
int memoryPerProcess;
std::vector<int> memoryBlock;

std::set<int> pidSet; // I need to know the order of pID that is being worked on to print

int tempQuantum = 0;

bool allocateMemory(int minMemory, int pid, std::vector<int>& memoryBlock) {
    for (int block : memoryBlock) {
        if (block == pid) {
            return false;
        }
    }

    int maxMemory = memoryBlock.size();

    for (int i = 0; i <= maxMemory - minMemory;) {
        if (memoryBlock[i] != 0) {
            while (i < maxMemory && memoryBlock[i] != 0) {
                i++;
            }
            continue;
        }

        bool canAllocate = true;
        for (int j = 0; j < minMemory; j++) {
            if (memoryBlock[i + j] != 0) {
                canAllocate = false;
                i += j + 1;
                break;
            }
        }

        if (canAllocate) {
            for (int j = 0; j < minMemory; ++j) {
                memoryBlock[i + j] = pid;
            }
            return true;
        }
    }

    return false;
}

void deallocateMemory(int pid, std::vector<int>& memoryBlock) {
    for (int& block : memoryBlock) {
        if (block == pid) {
            block = 0;
        }
    }
}

int calculateExternalFragmentation(std::vector<int>& memoryBlock) {
    int freeMemory = 0;

    for (int& block : memoryBlock) {
        if (block == 0) {
            freeMemory++;
        }
    }

    return freeMemory;
}

int calculateNumProcessInMemory(const std::vector<int>& memoryBlock) {
    std::set<int> uniqueProcesses;

    for (int block : memoryBlock) {
        if (block != 0) {
            uniqueProcesses.insert(block);
        }
    }

    return uniqueProcesses.size();
}

bool isProcessInMemory(const std::vector<int>& memoryBlock, int pid) {
    for (int block : memoryBlock) {
        if (block == pid) {
            return true;
        }
    }
    return false;
}

// Function that returns a set of number which is the starting and end point of each pid in memory block
std::vector<int> findNonZeroRanges(const std::vector<int>& memoryBlock) {
    std::vector<int> ranges;
    int n = memoryBlock.size();
    bool inBlock = false;
    int start = -1;
    int prevValue = -1;

    for (int i = 0; i < n; i++) {
        if (memoryBlock[i] != 0) {

            if (!inBlock || memoryBlock[i] != prevValue) {
                if (inBlock) {
                    ranges.push_back(start);
                    ranges.push_back(i);
                }
                start = i + 1;
                inBlock = true;
            }
            prevValue = memoryBlock[i];
        }
        else {
            if (inBlock) {
                ranges.push_back(start);
                ranges.push_back(i);
                inBlock = false;
            }
        }
    }

    if (inBlock) {
        ranges.push_back(start);
        ranges.push_back(n);
    }

    return ranges;
}

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

    Process(const std::string& procName, int id, int commands, int core)
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

    void setFinished(int n) {
        numFinishedCommands = n;
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

    void executePrintCommands(int delay, std::string name, std::vector<Process>& processes)
    {
        int iterationCount = 0;
        const int updateThreshold = 5;

        int tempTotal = 0;

        for (auto& p : processes)
        {
            if (p.getName() == name)
            {
                tempTotal = p.getTotalCommands();
            }
        }

        while (numFinishedCommands < numCommands)
        {
            // TODO make this cpuCycle dependent

            for (int i = 0; i < delay; i++)
            {
                // Does nothing during the delay
                volatile int temp = 0;
                temp++;
            }

            std::time_t currentTime = std::time(nullptr);
            commandTimestamps.push_back(currentTime);

            char buffer[80];
            struct tm timeInfo;
            localtime_s(&timeInfo, &currentTime);
            std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);

            numFinishedCommands++;

            iterationCount++;

            if (numFinishedCommands >= tempTotal) {
                for (auto& p : processes)
                {
                    if (p.getName() == name)
                    {
                        p.setFinished(numFinishedCommands);
                        break;
                    }
                }
            }

            // Only update after 5 iterations or else the program crashes/abort 
            else if (numFinishedCommands < tempTotal)
            {
                for (auto& p : processes)
                {
                    if (p.getName() == name)
                    {
                        p.setFinished(numFinishedCommands);
                        break;
                    }
                }
                iterationCount = 0;
            }
        }
    }

    void executePrintCommandsRR(int delay, std::string name, std::vector<Process>& processes)
    {
        static int iterationCount = 0;
        const int updateThreshold = 5;

        int tempTotal = 0;

        for (auto& p : processes)
        {
            if (p.getName() == name)
            {
                tempTotal = p.getTotalCommands();
            }
        }

        if (numFinishedCommands < numCommands)
        {
            // TODO make this cpuCycle dependent
            // std::this_thread::sleep_for(std::chrono::milliseconds(delay));

            for (int i = 0; i < delay; i++)
            {
                // Does nothing during the delay
                volatile int temp = 0;
                temp++;
            }

            std::time_t currentTime = std::time(nullptr);
            commandTimestamps.push_back(currentTime);

            char buffer[80];
            struct tm timeInfo;
            localtime_s(&timeInfo, &currentTime);
            std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", &timeInfo);

            numFinishedCommands++;

            iterationCount++;

            if (numFinishedCommands >= tempTotal) {
                for (auto& p : processes)
                {
                    if (p.getName() == name)
                    {
                        p.setFinished(numFinishedCommands);
                        break;
                    }
                }
            }

            // Only update after 5 iterations or else the program crashes/abort 
            else if (iterationCount >= updateThreshold)
            {
                for (auto& p : processes)
                {
                    if (p.getName() == name)
                    {
                        p.setFinished(numFinishedCommands);
                        break;
                    }
                }
                iterationCount = 0;
            }
        }
    }

    void startProcessLoop()
    {
        std::string command;
        while (true)
        {
            std::cout << ">>" << pname << ": ";
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
        std::cout << "Process: " << pname << std::endl;
        std::cout << "ID: " << pid << std::endl;
        if (numFinishedCommands < numCommands) {
            std::cout << "\nCurrent Instruction Line: " << numFinishedCommands << std::endl;
            std::cout << "Lines of Code: " << numCommands << std::endl;
        }
        else if (numFinishedCommands >= numCommands)
            std::cout << "\nFinished!" << std::endl;
        std::cout << std::endl;
    }
};

// Vector of processes moved to global var
std::vector<Process> processes;

class Scheduler { // Allows different schedulers (like FCFS or RR) to be used interchangeably
public:
    virtual ~Scheduler() = default;
    virtual void addProcess(const Process& p) = 0;
    virtual void printCoreStatus() = 0;
    virtual std::string formatTime(std::time_t time) = 0;
    bool memoryFull = false;
};

class RR_Scheduler : public Scheduler {
private:
    std::queue<std::shared_ptr<Process>> processQueue; // Queue to hold processes
    std::vector<std::thread> coreThreads;              // Store threads for each core
    std::vector<std::shared_ptr<Process>> allProcesses; // Track all processes
    std::vector<bool> coreBusy;                         // Track core availability
    int quantumCycle;                                   // Time slice for each process
    std::mutex queueMutex;
    int numCores;

public:
    RR_Scheduler(int numCore, int quantum, int delay)
        : coreBusy(numCore, false), quantumCycle(quantum), numCores(numCore)
    {
        // Create and start a new thread for each core
        for (int i = 0; i < numCore; i++) {
            coreThreads.emplace_back(&RR_Scheduler::coreThreadFunction, this, i, delay);
        }
    }

    void addProcess(const Process& p) override {
        auto processPtr = std::make_shared<Process>(p);
        std::lock_guard<std::mutex> lock(queueMutex);
        processQueue.push(processPtr);
        allProcesses.push_back(processPtr);
    }

    void coreThreadFunction(int coreId, int delay) {
        while (true) {
            std::shared_ptr<Process> processPtr = nullptr;
            {
                std::lock_guard<std::mutex> lock(queueMutex);

                bool lowestAvailableCore = true;
                for (int i = 0; i < coreId; i++)
                {
                    if (!coreBusy[i])
                    {
                        lowestAvailableCore = false;
                        break;
                    }
                }

                // Fetch process only if this core is not busy and there is a process available
                if (lowestAvailableCore && !coreBusy[coreId] && !processQueue.empty()) {
                    processPtr = processQueue.front();
                    processQueue.pop();  // Pop it from the queue

                    allocateMemory(minMemory, processPtr->getPid(), memoryBlock);

                    if (isProcessInMemory(memoryBlock, processPtr->getPid())) {
                        processPtr->setCoreId(coreId);
                        if (processPtr->getStartTime() == 0) {
                            processPtr->setStartTime();
                        }
                        coreBusy[coreId] = true;  // Mark the core as busy
                        pidSet.insert(processPtr->getPid());

                        //std::cout << "Core " << coreId << " is handling Process " << processPtr->getPid() << std::endl;
                    }
                    else {
                        // If memory allocation fails, requeue the process and skip this core's cycle
                        processQueue.push(processPtr);  // Requeue the process
                        continue;  // Skip the rest of the loop iteration
                    }
                }
            }

            // If there's no process to execute, set it to not busy
            if (!processPtr) {
                coreBusy[coreId] = false;  // Mark the core as idle
                continue;
            }

            // Execute commands until quantum cycle is exhausted or process is finished
            int cyclesExecuted = 0;  // Count the number of cycles executed
            try {
                while (cyclesExecuted < quantumCycle && !processPtr->isFinished()) {
                    //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    processPtr->executePrintCommandsRR(delay, processPtr->getName(), processes);  // Execute one command
                    //std::cout << "Core " << coreId << " is handling Process " << processPtr->getPid() << std::endl;
                    cyclesExecuted++;
                }
            }
            catch (const std::exception&) {}

            int totalFreeSpace = calculateExternalFragmentation(memoryBlock);
            int numProcessesInMemory = calculateNumProcessInMemory(memoryBlock);

            // Get current time for timestamp
            std::time_t timestamp = std::time(nullptr);
            std::tm localTime;
            localtime_s(&localTime, &timestamp);
            char timeBuffer[100];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S %p", &localTime);

            std::string fileName = "memory_stamp_" + std::to_string(tempQuantum += quantumCycle) + ".txt";

            std::vector<int> ranges = findNonZeroRanges(memoryBlock);

            std::ofstream logFile(fileName, std::ios::out);
            if (logFile.is_open()) {
                logFile << "Timestamp: " << timeBuffer << "\n"
                    << "Processes in memory: " << numProcessesInMemory << "\n"
                    << "Total external fragmentation in KB: " << totalFreeSpace << " units\n"
                    << "--------------------------------------------\n";

                logFile << "--------end-------- = " << maxMemory << "\n";
                auto setPtr = pidSet.rbegin();

                for (size_t i = ranges.size(); i > 0; i -= 2) {
                    logFile << ranges[i - 1] << "\n";

                    if (setPtr != pidSet.rend()) {
                        logFile << "P" << *setPtr << "\n";
                        setPtr++;
                    }

                    logFile << ranges[i - 2] - 1 << "\n\n";
                }
                logFile << "--------start------ = " << 0 << "\n";

                // Close the file
                logFile.close();
            }

            // If the process is not finished after its quantum, requeue it
            if (!processPtr->isFinished()) {
                std::lock_guard<std::mutex> lock(queueMutex);
                // Requeue if there are fewer processes than cores
                processQueue.push(processPtr);  // Requeue to the back of the queue
            }
            else {
                deallocateMemory(processPtr->getPid(), memoryBlock);
                pidSet.erase(processPtr->getPid());
            }

            coreBusy[coreId] = false;  // Mark the core as idle after processing
        }
    }


    std::string formatTime(std::time_t time) override {
        std::tm localTime;
        localtime_s(&localTime, &time);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", &localTime);
        return std::string(buffer);
    }

    void printCoreStatus() override {
        system("cls");

        std::cout << "Current Status of Running Processes (RR Scheduler):\n";

        std::vector<std::shared_ptr<Process>> activeProcesses(numCores, nullptr);

        for (const auto& process : allProcesses) {
            if (!process->isFinished() && process->getCoreId() >= 0) {
                activeProcesses[process->getCoreId()] = process; // Assign the process to its core
            }
        }

        for (int coreId = 0; coreId < numCores; coreId++) {
            if (activeProcesses[coreId]) {
                std::string startTimeFormatted = formatTime(activeProcesses[coreId]->getStartTime());
                std::cout << "Core " << coreId << " is processing: " << activeProcesses[coreId]->getName()
                    << " (" << startTimeFormatted << "): "
                    << activeProcesses[coreId]->getFinishedCommands() << "/"
                    << activeProcesses[coreId]->getTotalCommands() << " commands executed.\n";
            }
            else {
                std::cout << "Core " << coreId << " is idle.\n";
            }
        }

        // Finished processes
        std::cout << "\nFinished Processes:\n";
        for (const auto& process : allProcesses) {
            if (process->isFinished()) {
                std::string startTimeFormatted = formatTime(process->getStartTime());
                std::cout << "Process " << process->getName() << " (" << startTimeFormatted << "): "
                    << process->getFinishedCommands() << "/"
                    << process->getTotalCommands() << " commands executed.\n";
            }
        }
    }
};

class FCFS_Scheduler : public Scheduler
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
    FCFS_Scheduler(int numCore, int delay) : coreBusy(numCore, false), numCores(numCore)
    {
        // Create and start a new thread for each core
        // Each core gets their coreThreadFunctions
        for (int i = 0; i < numCore; i++)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            coreThreads.emplace_back(&FCFS_Scheduler::coreThreadFunction, this, i, delay);
        }
    }

    void addProcess(const Process& p) override
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

    void coreThreadFunction(int coreId, int delay)
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
            // afterwards, core is no longer busy
            if (processPtr)
            {
                coreBusy[coreId] = true;
                processPtr->executePrintCommands(delay, processPtr->getName(), processes);
                coreBusy[coreId] = false;
            }

            // standby for few timeframe if no job is assigned
            if (!processPtr)
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    std::string formatTime(std::time_t time) override
    {
        std::tm localTime; // Create a tm structure to hold the local time
        localtime_s(&localTime, &time); // Use localtime_s to get the local time

        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", &localTime); // Format time as m/d/y h:m:s am/pm
        return std::string(buffer);
    }

    void printCoreStatus() override
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
void handleScreenCommand(const std::string& command, std::vector<Process>& processes, bool& isTerminalOpen, int min, int max, std::unique_ptr<Scheduler>& scheduler)
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

        srand(static_cast<unsigned int>(time(0)));
        int commandSize = rand() % (max - min + 1) + min;

        // Creates a new instance of the Terminal class and stores it in the vector
        Process newProcess(name, globalProcessNumber, commandSize, -1);
        globalProcessNumber++;

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
            scheduler->printCoreStatus();
            });
        lsThread.join(); // Wait for the thread to finish

    }
    else
    {
        std::cout << "Invalid screen command! Try using -r, -ls, or -s.\n";
    }
}

void processSchedulerAutoAdder(std::unique_ptr<Scheduler>& scheduler, std::vector<Process>& processes)
{
    std::size_t lastCheckedSize = 0; // Keep track of how many processes have been added

    while (true) {
        // Check for new processes in the vector
        if (processes.size() > lastCheckedSize) {
            for (std::size_t i = lastCheckedSize; i < processes.size(); i++) {
                scheduler->addProcess(processes[i]);  // Add new processes to the scheduler
            }
            lastCheckedSize = processes.size();  // Update the last checked size
        }
    }
}

void readConfigFile(int& numCore, std::string& mode, int& quantumCycle, int& batchFrequency, int& minCommandNum, int& maxCommandNum, int& delay, int& maxMemory, int& memoryFrame, int& memoryPerProcess) {
    std::ifstream configFile("config.txt");

    if (!configFile.is_open()) {
        std::cerr << "Error: config.txt could not be opened!\n";
        exit(1);
    }

    std::string line, param;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        iss >> param;

        if (param == "num-cpu") {
            iss >> numCore;
        }
        else if (param == "scheduler") {
            iss >> mode;
        }
        else if (param == "quantum-cycles") {
            iss >> quantumCycle;
        }
        else if (param == "batch-process-freq") {
            iss >> batchFrequency;
        }
        else if (param == "min-ins") {
            iss >> minCommandNum;
        }
        else if (param == "max-ins") {
            iss >> maxCommandNum;
        }
        else if (param == "delays-per-exec") {
            iss >> delay;
        }
        else if (param == "max-overall-mem") {
            iss >> maxMemory;
        }
        else if (param == "mem-per-frame") {
            iss >> memoryFrame;
        }
        else if (param == "mem-per-proc") {
            iss >> memoryPerProcess;
        }
    }

    do {
        minMemory += memoryFrame;
    } while (minMemory < memoryPerProcess);

    configFile.close();
}

// Write whats being printed on printCoreStatus into .txt
void handleReportUtilCommand(std::unique_ptr<Scheduler>& scheduler)
{
    std::ofstream logFile("csopesy-log.txt");

    if (logFile.is_open())
    {
        std::stringstream ss;

        // Redirects the cout output into ss
        std::streambuf* originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(ss.rdbuf());

        scheduler->printCoreStatus();

        // Redirects the cout output to its original place
        std::cout.rdbuf(originalCoutBuffer);

        // Write the content of ss into .txt
        logFile << ss.str();

        logFile.close();
    }
}

// Generate dummy processes in scheduler-test
void schedulerTestFunction(int batchFrequency, std::vector<Process>& processes, int minCommandNum, int maxCommandNum) {
    schedulerRunning = true;
    stopRequested = false;

    // Reinforce minimum batch frequency
    if (batchFrequency < 1) {
        batchFrequency = 1;
    }

    while (schedulerRunning && !stopRequested) {
        // Sleep based on the batch frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        for (int i = 0; i < batchFrequency; i++) {
            volatile int x = 0;
            x++;
        }

        // Generate a new process with a random number of commands
        int commandSize = rand() % (maxCommandNum - minCommandNum + 1) + minCommandNum;
        std::string processName = "p" + std::to_string(globalProcessNumber);

        Process newProcess(processName, globalProcessNumber, commandSize, -1);
        processes.push_back(newProcess);
        globalProcessNumber++;
        // Do not print anything while the scheduler is running
    }

    if (stopRequested) {
        // Provide a summary of all processes generated when `scheduler-stop` is called
        std::cout << "\nScheduler-test stopped. Summary of all generated processes:\n";
        for (const auto& process : processes) {
            std::cout << process.getName() << " with " << process.getTotalCommands() << " commands.\n";
        }

        // After the summary is printed, ensure the prompt is placed after the summary
        std::cout << "\nEnter a command: " << std::flush;
    }
}

// Stop process generation (scheduler-stop)
void stopSchedulerTest() {
    stopRequested = true;
    schedulerRunning = false;
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
    int numCore;
    std::string mode;
    int quantumCycle;
    int batchFrequency;
    int minCommandNum;
    int maxCommandNum;
    int delay;


    while (true) {
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "initialize") {
            // TODO: read from config.txt and assign the values
            readConfigFile(numCore, mode, quantumCycle, batchFrequency, minCommandNum, maxCommandNum, delay, maxMemory, memoryFrame, memoryPerProcess);

            system("cls");

            std::cout << "======================================\n";
            std::cout << "        Configuration Loaded!         \n";
            std::cout << "======================================\n";
            std::cout << "  -> Number of CPUs        : " << numCore << "\n";
            std::cout << "  -> Scheduler Mode        : " << mode << "\n";
            std::cout << "  -> Quantum Cycles        : " << quantumCycle << "\n";
            std::cout << "  -> Batch Process Freq    : " << batchFrequency << "\n";
            std::cout << "  -> Min Instructions/Proc : " << minCommandNum << "\n";
            std::cout << "  -> Max Instructions/Proc : " << maxCommandNum << "\n";
            std::cout << "  -> Delay per Execution   : " << delay << "\n";
            std::cout << "  -> Overall Memory        : " << maxMemory << "\n";
            std::cout << "  -> Memory Per Frame      : " << memoryFrame << "\n";
            std::cout << "  -> Memory Per Process    : " << memoryPerProcess << "\n";
            std::cout << "======================================\n";

            std::cout << "\nLoading main menu in 3... ";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "2... ";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "1...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));

            memoryBlock.resize(maxMemory, 0);

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

    // Vector to hold Process instances
    //std::vector<Process> processes;

    std::unique_ptr<Scheduler> scheduler;

    if (mode == "fcfs") {
        scheduler = std::make_unique<FCFS_Scheduler>(numCore, delay);
    }
    else if (mode == "rr") {
        scheduler = std::make_unique<RR_Scheduler>(numCore, quantumCycle, delay);
    }


    // Thread which constantly accepts new process into the process vector
    // (Since users is now able to add processes into the ready queue while the scheduler is running)
    std::thread processAdderThread(processSchedulerAutoAdder, std::ref(scheduler), std::ref(processes));

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
            else {
                active = false;
                stopSchedulerTest();  // Ensure scheduler-test stops if running
            }
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
            handleScreenCommand(input, processes, isTerminalOpen, minCommandNum, maxCommandNum, scheduler);
        }
        else if (command == "scheduler-test")
        {
            //printAcceptMessage(command);
            if (!schedulerRunning) {
                printAcceptMessage(command);
                std::thread schedulerThread(schedulerTestFunction, batchFrequency, std::ref(processes), minCommandNum, maxCommandNum);
                schedulerThread.detach();  // Run scheduler in a separate thread
            }
            else {
                std::cout << "Scheduler-test is already running!\n";
            }
        }
        else if (command == "scheduler-stop")
        {
            //printAcceptMessage(command);
            if (schedulerRunning) {
                stopSchedulerTest();
            }
            else {
                std::cout << "Scheduler-test has already stopped running!\n";
            }
        }
        else if (command == "report-util")
        {
            handleReportUtilCommand(scheduler);
            system("cls");
            printASCII();
            printWelcomeMessage();
            std::cout << "\n[Logs successfully saved to csopesy-log.txt!]\n";
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

    // Ensure scheduler-test stops on exit
    stopSchedulerTest();
    exit(0);
    return 0;
}