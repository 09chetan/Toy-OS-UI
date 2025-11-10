# OS Simulator

A comprehensive web-based application that simulates fundamental operating system concepts including Virtual Machine execution and Memory Management with Paging.

![OS Simulator](https://img.shields.io/badge/React-18.x-blue) ![C++](https://img.shields.io/badge/C++-17-green) ![License](https://img.shields.io/badge/license-MIT-blue)

## 📋 Overview

The OS Simulator provides an intuitive interface to visualize and interact with core operating system concepts:

- **Phase 1**: Virtual Machine Simulator with instruction processing and I/O operations
- **Phase 2**: Memory Management Unit with paging, TLB, and page replacement algorithms

## ✨ Features

### Phase 1 - Virtual Machine Simulator
- ✅ Instruction Processing (GD, PD, H, LR, SR, CR, BT)
- ✅ 100x4 Memory Grid Simulation
- ✅ Register Operations (R register and toggle)
- ✅ I/O Operations Simulation
- ✅ Job Control with $AMJ, $DTA, $END cards

### Phase 2 - Memory Management Unit
- ✅ Paging System (1KB page size, 64 physical frames)
- ✅ 4-Entry TLB with FIFO replacement
- ✅ FIFO Page Replacement Algorithm
- ✅ Process Management (CREATE, TERMINATE)
- ✅ Virtual to Physical Address Translation
- ✅ Real-time Statistics (TLB hits/misses, page faults)

### User Interface
- 🎨 Modern gradient design
- 📁 Drag-and-drop file upload
- 📊 Real-time output with syntax highlighting
- 🔄 Interactive phase switching
- ⚠️ Comprehensive error handling
- 📝 Built-in sample templates

## 🚀 Getting Started

### Prerequisites

- Node.js (v16 or higher)
- C++ Compiler (GCC/G++ or MinGW on Windows)
- npm or yarn package manager

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/os-simulator.git
cd os-simulator
```

2. **Setup Backend (C++ Core)**
```bash
cd backend

# Windows
compile.bat

# Linux/Mac
g++ -o phase1 phase1.cpp
g++ -o phase2 phase2.cpp
```

3. **Setup Frontend (React UI)**
```bash
cd frontend
npm install
npm run dev
```

4. **Access the application**
```
Open http://localhost:3000 in your browser
```

## 📁 Project Structure

```
os-simulator-ui/
├── backend/                 # C++ Simulation Core
│   ├── phase1.cpp          # Virtual Machine Implementation
│   ├── phase2.cpp          # Memory Management Implementation
│   ├── compile.bat         # Windows Compilation Script
│   └── run.bat             # Execution Helper
├── frontend/               # React User Interface
│   ├── public/
│   │   └── index.html
│   ├── src/
│   │   ├── components/
│   │   │   ├── Phase1Simulator.jsx
│   │   │   ├── Phase2Simulator.jsx
│   │   │   └── FileUpload.jsx
│   │   ├── App.jsx
│   │   ├── App.css
│   │   ├── index.css
│   │   └── main.jsx
│   ├── package.json
│   └── vite.config.js
└── README.md
```

## 📖 Usage

### Phase 1: Virtual Machine

**Input Format:**
```
$AMJ000100050001
GD20 PD20 H
$DTA
HELLO WORLD
$END0001
```

**Instructions:**
- `GD` - Get Data (Read)
- `PD` - Put Data (Write)
- `H` - Halt
- `LR` - Load Register
- `SR` - Store Register
- `CR` - Compare Register
- `BT` - Branch if True

**Control Cards:**
- `$AMJ` - Start of Job
- `$DTA` - Start of Data
- `$END` - End of Job

### Phase 2: Memory Management

**Input Format:**
```
CREATE 1 5
CREATE 2 3
ACCESS 1 0
ACCESS 1 1024
WRITE 1 2048
MEMMAP
STATS
TERMINATE 1
```

**Commands:**
- `CREATE <pid> <pages>` - Create process with specified pages
- `ACCESS <pid> <address>` - Read from virtual address
- `WRITE <pid> <address>` - Write to virtual address
- `TERMINATE <pid>` - Terminate process
- `MEMMAP` - Display memory map
- `STATS` - Show system statistics

## 🔧 Technical Details

### Phase 1 Architecture
```
Virtual Machine Components:
- Memory[100][4]    → 400-byte main memory
- IR[4]             → Instruction Register
- R[4]              → General Purpose Register
- IC                → Instruction Counter
- C                 → Toggle Register
- SI                → System Interrupt
- Buffer[40]        → I/O Buffer
```

### Phase 2 Architecture
```
Memory Management Components:
- Physical Memory: 64 frames × 1024 bytes
- Virtual Memory: 256 pages per process
- TLB: 4 entries with FIFO replacement
- Page Table: Per-process with valid/dirty/referenced bits
- FIFO Queue: For page replacement
```

## 📊 Example Output

### Phase 1 Output
```
Virtual Machine Simulation Started...
New Job started
Program Card loading
Data card loading
Read function called
Write function called
HELLO WORLD
Terminate called
END of Job
```

### Phase 2 Output
```
=== SYSTEM STATISTICS ===
TLB Hits: 3
TLB Misses: 4
TLB Hit Rate: 42.86%
Free Frames: 60/64
Active Processes: 2
=========================
```

## 🐛 Troubleshooting

### Common Issues

**C++ Compilation Errors:**
- Ensure GCC/G++ is installed and in PATH
- Check for syntax errors in C++ files

**React Dependencies:**
- Delete `node_modules` and run `npm install` again
- Ensure Node.js version compatibility

**File Upload Issues:**
- Use correct file format (.txt)
- Follow the sample input structure
- Check file encoding (UTF-8 recommended)

**Port Already in Use:**
```bash
npm run dev -- --port 3001
```


