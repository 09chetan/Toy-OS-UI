#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// Constants and structures remain the same as your original code
const int PAGE_SIZE = 1024;
const int PHYSICAL_MEMORY_SIZE = 64;
const int VIRTUAL_MEMORY_SIZE = 256;
const int TLB_SIZE = 4;

enum InterruptType {
    PAGE_FAULT,
    INVALID_ACCESS,
    SEGMENTATION_FAULT,
    TIMER_INTERRUPT
};

enum ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

struct PageTableEntry {
    int frameNumber;
    bool valid;
    bool dirty;
    bool referenced;
    
    PageTableEntry() : frameNumber(-1), valid(false), dirty(false), referenced(false) {}
};

struct TLBEntry {
    int pid;
    int pageNumber;
    int frameNumber;
    bool valid;
    
    TLBEntry() : pid(-1), pageNumber(-1), frameNumber(-1), valid(false) {}
};

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

class MMU {
private:
    vector<bool> physicalMemory;
    queue<int> freeFrames;
    vector<TLBEntry> tlb;
    int tlbHits;
    int tlbMisses;
    map<int, PCB*> processTable;
    stringstream output;
    queue<pair<int, int>> fifoQueue;
    
public:
    MMU() : tlbHits(0), tlbMisses(0) {
        physicalMemory.resize(PHYSICAL_MEMORY_SIZE, false);
        tlb.resize(TLB_SIZE);
        
        for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
            freeFrames.push(i);
        }
    }
    
    string executeCommands(const string& input) {
        output.str("");
        output.clear();
        
        output << "=== OS SIMULATOR - PHASE 2 ===\n";
        output << "Page Size: " << PAGE_SIZE << " bytes\n";
        output << "Physical Memory: " << PHYSICAL_MEMORY_SIZE << " frames\n";
        output << "Virtual Memory: " << VIRTUAL_MEMORY_SIZE << " pages per process\n\n";
        
        istringstream inputStream(input);
        string line;
        int lineNum = 0;
        
        while (getline(inputStream, line)) {
            lineNum++;
            
            if (line.empty() || line[0] == '#') {
                continue;
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
    
    int allocateFrame() {
        if (!freeFrames.empty()) {
            int frame = freeFrames.front();
            freeFrames.pop();
            physicalMemory[frame] = true;
            return frame;
        }
        return -1;
    }
    
    void freeFrame(int frame) {
        if (frame >= 0 && frame < PHYSICAL_MEMORY_SIZE) {
            physicalMemory[frame] = false;
            freeFrames.push(frame);
        }
    }
    
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
    
    int translateAddress(int pid, int virtualAddr, bool write = false) {
        int pageNumber = virtualAddr / PAGE_SIZE;
        int offset = virtualAddr % PAGE_SIZE;
        
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
        
        static int tlbIndex = 0;
        tlb[tlbIndex].pid = pid;
        tlb[tlbIndex].pageNumber = pageNumber;
        tlb[tlbIndex].frameNumber = frame;
        tlb[tlbIndex].valid = true;
        tlbIndex = (tlbIndex + 1) % TLB_SIZE;
        
        return frame * PAGE_SIZE + offset;
    }
    
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
    
    void terminateProcess(int pid) {
        if (processTable.find(pid) == processTable.end()) {
            output << "Error: Process " << pid << " not found\n";
            return;
        }
        
        PCB* pcb = processTable[pid];
        
        for (int i = 0; i < pcb->pageTable.size(); i++) {
            if (pcb->pageTable[i].valid) {
                freeFrame(pcb->pageTable[i].frameNumber);
            }
        }
        
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

// For direct execution
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: phase2 <input_file> <output_file>" << endl;
        return 1;
    }
    
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    
    if (!input.is_open()) {
        cout << "Error opening input file" << endl;
        return 1;
    }
    
    string content((istreambuf_iterator<char>(input)), 
                   istreambuf_iterator<char>());
    
    MMU mmu;
    string result = mmu.executeCommands(content);
    output << result;
    
    input.close();
    output.close();
    return 0;
}