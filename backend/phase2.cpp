#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// Constants
const int PAGE_SIZE = 1024;           // 1KB page size
const int PHYSICAL_MEMORY_SIZE = 64;  // 64 pages in physical memory
const int VIRTUAL_MEMORY_SIZE = 256;  // 256 pages per process
const int TLB_SIZE = 4;               // Translation Lookaside Buffer size

// Interrupt Types
enum InterruptType {
    PAGE_FAULT,
    INVALID_ACCESS,
    SEGMENTATION_FAULT,
    TIMER_INTERRUPT
};

// Process State
enum ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

// Page Table Entry
struct PageTableEntry {
    int frameNumber;
    bool valid;
    bool dirty;
    bool referenced;
    
    PageTableEntry() : frameNumber(-1), valid(false), dirty(false), referenced(false) {}
};

// TLB Entry
struct TLBEntry {
    int pid;
    int pageNumber;
    int frameNumber;
    bool valid;
    
    TLBEntry() : pid(-1), pageNumber(-1), frameNumber(-1), valid(false) {}
};

// Process Control Block
class PCB {
public:
    int pid;
    ProcessState state;
    int programCounter;
    int priority;
    vector<PageTableEntry> pageTable;
    int allocatedPages;
    int pageFaults;
    
    PCB(int id, int pages) : pid(id), state(NEW), programCounter(0), 
                             priority(0), allocatedPages(pages), pageFaults(0) {
        pageTable.resize(VIRTUAL_MEMORY_SIZE);
    }
};

// Memory Management Unit
class MMU {
private:
    vector<bool> physicalMemory;  // Frame allocation bitmap
    queue<int> freeFrames;
    vector<TLBEntry> tlb;
    int tlbHits;
    int tlbMisses;
    map<int, PCB*> processTable;
    stringstream output;  // Changed from ofstream& to stringstream
    
    // FIFO page replacement
    queue<pair<int, int>> fifoQueue;  // (pid, pageNumber)
    
public:
    // Original constructor for file output
    MMU(ofstream& out) : tlbHits(0), tlbMisses(0) {
        physicalMemory.resize(PHYSICAL_MEMORY_SIZE, false);
        tlb.resize(TLB_SIZE);
        
        // Initialize free frames
        for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
            freeFrames.push(i);
        }
    }
    
    // New constructor for API output
    MMU() : tlbHits(0), tlbMisses(0) {
        physicalMemory.resize(PHYSICAL_MEMORY_SIZE, false);
        tlb.resize(TLB_SIZE);
        
        // Initialize free frames
        for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
            freeFrames.push(i);
        }
    }
    
    // Execute commands from string input
    string executeCommands(const string& inputContent) {
        output.str(""); // Clear previous output
        output.clear();
        
        output << "=== OS SIMULATOR - PHASE 2 ===\n";
        output << "Page Size: " << PAGE_SIZE << " bytes\n";
        output << "Physical Memory: " << PHYSICAL_MEMORY_SIZE << " frames\n";
        output << "Virtual Memory: " << VIRTUAL_MEMORY_SIZE << " pages per process\n\n";
        
        istringstream input(inputContent);
        string line;
        int lineNum = 0;
        
        while (getline(input, line)) {
            lineNum++;
            
            if (line.empty() || line[0] == '#') {
                continue;  // Skip empty lines and comments
            }
            
            output << "Command [" << lineNum << "]: " << line << "\n";
            
            istringstream iss(line);
            string command;
            iss >> command;
            
            if (command == "CREATE") {
                int pid, pages;
                iss >> pid >> pages;
                createProcess(pid, pages);
            }
            else if (command == "ACCESS") {
                int pid, addr;
                iss >> pid >> addr;
                output << "Accessing virtual address " << addr << " of process " << pid << "\n";
                int physAddr = translateAddress(pid, addr, false);
                if (physAddr != -1) {
                    output << "Physical address: " << physAddr << "\n";
                }
            }
            else if (command == "WRITE") {
                int pid, addr;
                iss >> pid >> addr;
                output << "Writing to virtual address " << addr << " of process " << pid << "\n";
                int physAddr = translateAddress(pid, addr, true);
                if (physAddr != -1) {
                    output << "Physical address: " << physAddr << "\n";
                }
            }
            else if (command == "TERMINATE") {
                int pid;
                iss >> pid;
                terminateProcess(pid);
            }
            else if (command == "STATS") {
                printStatistics();
            }
            else if (command == "MEMMAP") {
                printMemoryMap();
            }
            else {
                output << "Unknown command: " << command << "\n";
            }
            
            output << "\n";
        }
        
        output << "\n=== FINAL STATISTICS ===\n";
        printStatistics();
        printMemoryMap();
        
        return output.str();
    }
    
    // Create a new process
    void createProcess(int pid, int pages) {
        if (processTable.find(pid) != processTable.end()) {
            output << "Error: Process " << pid << " already exists\n";
            return;
        }
        
        PCB* pcb = new PCB(pid, pages);
        pcb->state = READY;
        processTable[pid] = pcb;
        output << "Process " << pid << " created with " << pages << " pages\n";
    }
    
    // Allocate a frame
    int allocateFrame() {
        if (!freeFrames.empty()) {
            int frame = freeFrames.front();
            freeFrames.pop();
            physicalMemory[frame] = true;
            return frame;
        }
        return -1;  // No free frames
    }
    
    // Free a frame
    void freeFrame(int frame) {
        if (frame >= 0 && frame < PHYSICAL_MEMORY_SIZE) {
            physicalMemory[frame] = false;
            freeFrames.push(frame);
        }
    }
    
    // Page replacement using FIFO
    int replacePage() {
        if (fifoQueue.empty()) {
            return -1;
        }
        
        auto victim = fifoQueue.front();
        fifoQueue.pop();
        
        int victimPid = victim.first;
        int victimPage = victim.second;
        
        if (processTable.find(victimPid) != processTable.end()) {
            PCB* pcb = processTable[victimPid];
            if (victimPage < pcb->pageTable.size() && pcb->pageTable[victimPage].valid) {
                int frame = pcb->pageTable[victimPage].frameNumber;
                
                output << "Replacing page " << victimPage << " of process " << victimPid;
                if (pcb->pageTable[victimPage].dirty) {
                    output << " (dirty - writing back to disk)";
                }
                output << "\n";
                
                pcb->pageTable[victimPage].valid = false;
                pcb->pageTable[victimPage].frameNumber = -1;
                
                // Invalidate TLB entry
                for (auto& entry : tlb) {
                    if (entry.valid && entry.pid == victimPid && entry.pageNumber == victimPage) {
                        entry.valid = false;
                    }
                }
                
                return frame;
            }
        }
        
        return allocateFrame();
    }
    
    // Handle page fault
    void handlePageFault(int pid, int pageNumber) {
        output << "PAGE FAULT: Process " << pid << ", Page " << pageNumber << "\n";
        
        if (processTable.find(pid) == processTable.end()) {
            handleInterrupt(INVALID_ACCESS, pid, pageNumber);
            return;
        }
        
        PCB* pcb = processTable[pid];
        pcb->pageFaults++;
        
        if (pageNumber >= pcb->allocatedPages) {
            handleInterrupt(SEGMENTATION_FAULT, pid, pageNumber);
            return;
        }
        
        int frame = allocateFrame();
        if (frame == -1) {
            frame = replacePage();
            if (frame == -1) {
                output << "Error: Cannot allocate frame for page " << pageNumber << "\n";
                return;
            }
        }
        
        pcb->pageTable[pageNumber].frameNumber = frame;
        pcb->pageTable[pageNumber].valid = true;
        pcb->pageTable[pageNumber].referenced = true;
        
        fifoQueue.push({pid, pageNumber});
        
        output << "Allocated frame " << frame << " to page " << pageNumber << " of process " << pid << "\n";
    }
    
    // Translate virtual address to physical address
    int translateAddress(int pid, int virtualAddr, bool write = false) {
        int pageNumber = virtualAddr / PAGE_SIZE;
        int offset = virtualAddr % PAGE_SIZE;
        
        // Check TLB first
        for (int i = 0; i < TLB_SIZE; i++) {
            if (tlb[i].valid && tlb[i].pid == pid && tlb[i].pageNumber == pageNumber) {
                tlbHits++;
                output << "TLB Hit: Process " << pid << ", Page " << pageNumber << "\n";
                
                if (write && processTable.find(pid) != processTable.end()) {
                    processTable[pid]->pageTable[pageNumber].dirty = true;
                }
                
                return tlb[i].frameNumber * PAGE_SIZE + offset;
            }
        }
        
        tlbMisses++;
        output << "TLB Miss: Process " << pid << ", Page " << pageNumber << "\n";
        
        if (processTable.find(pid) == processTable.end()) {
            handleInterrupt(INVALID_ACCESS, pid, virtualAddr);
            return -1;
        }
        
        PCB* pcb = processTable[pid];
        
        if (pageNumber >= pcb->allocatedPages) {
            handleInterrupt(SEGMENTATION_FAULT, pid, virtualAddr);
            return -1;
        }
        
        if (!pcb->pageTable[pageNumber].valid) {
            handlePageFault(pid, pageNumber);
        }
        
        int frame = pcb->pageTable[pageNumber].frameNumber;
        pcb->pageTable[pageNumber].referenced = true;
        
        if (write) {
            pcb->pageTable[pageNumber].dirty = true;
        }
        
        // Update TLB (FIFO replacement)
        static int tlbIndex = 0;
        tlb[tlbIndex].pid = pid;
        tlb[tlbIndex].pageNumber = pageNumber;
        tlb[tlbIndex].frameNumber = frame;
        tlb[tlbIndex].valid = true;
        tlbIndex = (tlbIndex + 1) % TLB_SIZE;
        
        return frame * PAGE_SIZE + offset;
    }
    
    // Handle interrupts
    void handleInterrupt(InterruptType type, int pid, int addr) {
        output << "\n=== INTERRUPT HANDLER ===\n";
        
        switch (type) {
            case PAGE_FAULT:
                output << "Type: PAGE FAULT\n";
                output << "Process: " << pid << ", Address: " << addr << "\n";
                break;
                
            case INVALID_ACCESS:
                output << "Type: INVALID ACCESS\n";
                output << "Process: " << pid << " does not exist\n";
                break;
                
            case SEGMENTATION_FAULT:
                output << "Type: SEGMENTATION FAULT\n";
                output << "Process: " << pid << ", Invalid address: " << addr << "\n";
                if (processTable.find(pid) != processTable.end()) {
                    processTable[pid]->state = TERMINATED;
                }
                break;
                
            case TIMER_INTERRUPT:
                output << "Type: TIMER INTERRUPT\n";
                output << "Context switch triggered\n";
                break;
        }
        
        output << "=========================\n\n";
    }
    
    // Terminate process
    void terminateProcess(int pid) {
        if (processTable.find(pid) == processTable.end()) {
            output << "Error: Process " << pid << " not found\n";
            return;
        }
        
        PCB* pcb = processTable[pid];
        
        // Free all allocated frames
        for (int i = 0; i < pcb->pageTable.size(); i++) {
            if (pcb->pageTable[i].valid) {
                freeFrame(pcb->pageTable[i].frameNumber);
            }
        }
        
        // Clear TLB entries
        for (auto& entry : tlb) {
            if (entry.valid && entry.pid == pid) {
                entry.valid = false;
            }
        }
        
        pcb->state = TERMINATED;
        output << "Process " << pid << " terminated. Page faults: " << pcb->pageFaults << "\n";
        
        delete pcb;
        processTable.erase(pid);
    }
    
    // Print statistics
    void printStatistics() {
        output << "\n=== SYSTEM STATISTICS ===\n";
        output << "TLB Hits: " << tlbHits << "\n";
        output << "TLB Misses: " << tlbMisses << "\n";
        
        if (tlbHits + tlbMisses > 0) {
            double hitRate = (double)tlbHits / (tlbHits + tlbMisses) * 100;
            output << "TLB Hit Rate: " << fixed << setprecision(2) << hitRate << "%\n";
        }
        
        output << "Free Frames: " << freeFrames.size() << "/" << PHYSICAL_MEMORY_SIZE << "\n";
        output << "Active Processes: " << processTable.size() << "\n";
        output << "=========================\n";
    }
    
    // Print memory map
    void printMemoryMap() {
        output << "\n=== MEMORY MAP ===\n";
        for (auto& pair : processTable) {
            PCB* pcb = pair.second;
            output << "Process " << pcb->pid << " (State: ";
            
            switch (pcb->state) {
                case NEW: output << "NEW"; break;
                case READY: output << "READY"; break;
                case RUNNING: output << "RUNNING"; break;
                case WAITING: output << "WAITING"; break;
                case TERMINATED: output << "TERMINATED"; break;
            }
            
            output << ")\n";
            output << "  Page Faults: " << pcb->pageFaults << "\n";
            output << "  Valid Pages: ";
            
            int validCount = 0;
            for (int i = 0; i < pcb->allocatedPages; i++) {
                if (pcb->pageTable[i].valid) {
                    output << i << "->" << pcb->pageTable[i].frameNumber << " ";
                    validCount++;
                }
            }
            
            if (validCount == 0) {
                output << "None";
            }
            output << "\n";
        }
        output << "==================\n";
    }
};

// Original main function for file-based execution
int main_original() {
    ifstream input("input_phase2.txt");
    ofstream output("output.txt");
    
    if (!input.is_open()) {
        cerr << "Error opening input file\n";
        return 1;
    }
    
    MMU mmu(output);
    
    output << "=== OS SIMULATOR - PHASE 2 ===\n";
    output << "Page Size: " << PAGE_SIZE << " bytes\n";
    output << "Physical Memory: " << PHYSICAL_MEMORY_SIZE << " frames\n";
    output << "Virtual Memory: " << VIRTUAL_MEMORY_SIZE << " pages per process\n\n";
    
    string line;
    int lineNum = 0;
    
    while (getline(input, line)) {
        lineNum++;
        
        if (line.empty() || line[0] == '#') {
            continue;  // Skip empty lines and comments
        }
        
        output << "Command [" << lineNum << "]: " << line << "\n";
        
        istringstream iss(line);
        string command;
        iss >> command;
        
        if (command == "CREATE") {
            int pid, pages;
            iss >> pid >> pages;
            mmu.createProcess(pid, pages);
        }
        else if (command == "ACCESS") {
            int pid, addr;
            iss >> pid >> addr;
            output << "Accessing virtual address " << addr << " of process " << pid << "\n";
            int physAddr = mmu.translateAddress(pid, addr, false);
            if (physAddr != -1) {
                output << "Physical address: " << physAddr << "\n";
            }
        }
        else if (command == "WRITE") {
            int pid, addr;
            iss >> pid >> addr;
            output << "Writing to virtual address " << addr << " of process " << pid << "\n";
            int physAddr = mmu.translateAddress(pid, addr, true);
            if (physAddr != -1) {
                output << "Physical address: " << physAddr << "\n";
            }
        }
        else if (command == "TERMINATE") {
            int pid;
            iss >> pid;
            mmu.terminateProcess(pid);
        }
        else if (command == "STATS") {
            mmu.printStatistics();
        }
        else if (command == "MEMMAP") {
            mmu.printMemoryMap();
        }
        else {
            output << "Unknown command: " << command << "\n";
        }
        
        output << "\n";
    }
    
    output << "\n=== FINAL STATISTICS ===\n";
    mmu.printStatistics();
    mmu.printMemoryMap();
    
    input.close();
    output.close();
    
    cout << "Simulation complete. Check output.txt for results.\n";
    
    return 0;
}

// New main function for CLI execution with backend integration
int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments - run original behavior
        return main_original();
    }
    else if (argc == 3) {
        // CLI mode: phase2.exe input.txt output.txt
        ifstream inputFile(argv[1]);
        if (!inputFile.is_open()) {
            cerr << "Error: Cannot open input file " << argv[1] << endl;
            return 1;
        }
        
        string content((istreambuf_iterator<char>(inputFile)), 
                      istreambuf_iterator<char>());
        inputFile.close();
        
        MMU mmu;
        string result = mmu.executeCommands(content);
        
        ofstream outputFile(argv[2]);
        if (!outputFile.is_open()) {
            cerr << "Error: Cannot open output file " << argv[2] << endl;
            return 1;
        }
        
        outputFile << result;
        outputFile.close();
        
        return 0;
    }
    else {
        cerr << "Usage: " << argv[0] << " [input_file output_file]" << endl;
        cerr << "If no arguments provided, uses default input_phase2.txt" << endl;
        return 1;
    }
}