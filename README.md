# OS Simulator

A comprehensive web-based application that simulates fundamental operating system concepts with a modern React frontend and C++ backend.

![OS Simulator](https://img.shields.io/badge/React-18.2.0-blue) ![Node.js](https://img.shields.io/badge/Node.js-16%2B-green) ![C++](https://img.shields.io/badge/C%2B%2B-11-orange)

## 📋 Overview

The OS Simulator provides interactive simulations of two core operating system concepts:

- **Phase 1**: Virtual Machine Simulator - Execute VM instructions with memory and I/O operations
- **Phase 2**: Memory Management Unit - Paging system with TLB and page replacement algorithms

## 🎯 Features

### Phase 1 - Virtual Machine Simulator
- ✅ Instruction processing (GD, PD, H, LR, SR, CR, BT)
- ✅ 100x4 memory grid simulation
- ✅ Register operations and toggle handling
- ✅ I/O operations simulation
- ✅ Job control with $AMJ, $DTA, $END cards

### Phase 2 - Memory Management Unit
- ✅ Paging system (1KB pages, 64 physical frames)
- ✅ 4-entry TLB with FIFO replacement
- ✅ FIFO page replacement algorithm
- ✅ Process management (CREATE, TERMINATE)
- ✅ Virtual to physical address translation
- ✅ Statistics tracking (TLB hits/misses, page faults)

### User Interface
- 🎨 Modern gradient design
- 🔌 Real-time backend integration
- 📁 Drag-and-drop file upload
- 📊 Real-time output display
- 🔄 Interactive phase switching
- 🟢 Backend status monitoring
- ⚠️ Comprehensive error handling

## 🏗️ Project Structure

```
os-simulator-ui/
├── backend/                 # C++ Core + Node.js API
│   ├── phase1.cpp          # Virtual Machine
│   ├── phase2.cpp          # Memory Management
│   ├── server.js           # Express API Server
│   ├── package.json
│   ├── compile.bat
│   └── start-backend.bat
├── frontend/               # React UI
│   ├── src/
│   │   ├── components/
│   │   │   ├── Phase1Simulator.jsx
│   │   │   ├── Phase2Simulator.jsx
│   │   │   └── FileUpload.jsx
│   │   ├── services/
│   │   │   └── api.js
│   │   ├── App.jsx
│   │   └── main.jsx
│   ├── package.json
│   └── vite.config.js
└── README.md
```

## 🚀 Quick Start

### Prerequisites

- **Node.js** (v16 or higher) - [Download](https://nodejs.org/)
- **C++ Compiler** (GCC/G++/MinGW)
- **npm** (comes with Node.js)

### Installation

#### 1️⃣ Backend Setup

```bash
# Navigate to backend
cd backend

# Install dependencies
npm install

# Compile C++ programs
npm run compile
# OR on Windows:
.\compile.bat

# Start backend server
npm start
```

**Expected Output:**
```
🚀 Backend server running on http://localhost:5000
📊 Available endpoints:
  GET  /api/health
  GET  /api/check-executables
  POST /api/phase1
  POST /api/phase2
```

#### 2️⃣ Frontend Setup

Open a new terminal:

```bash
# Navigate to frontend
cd frontend

# Install dependencies
npm install

# Start development server
npm run dev
```

**Expected Output:**
```
VITE v4.4.0  ready in XXX ms
➜  Local:   http://localhost:3000/
```

#### 3️⃣ Access Application

Open your browser and navigate to: **http://localhost:3000**

## 📁 Input File Formats

### Phase 1 - Virtual Machine

```text
$AMJ000100050001
GD20 PD20 H
$DTA
HELLO WORLD
$END0001
```

**Control Cards:**
- `$AMJ` - Start of Job
- `$DTA` - Start of Data
- `$END` - End of Job

**Instructions:**
- `GD` - Get Data (Read)
- `PD` - Put Data (Write)
- `H` - Halt
- `LR` - Load Register
- `SR` - Store Register
- `CR` - Compare Register
- `BT` - Branch if True

### Phase 2 - Memory Management

```text
# Create processes
CREATE 1 5
CREATE 2 3

# Access memory
ACCESS 1 0
ACCESS 1 1024
ACCESS 2 0

# Write operations
WRITE 1 2048
WRITE 2 512

# Show statistics
MEMMAP
STATS

# Terminate processes
TERMINATE 1
TERMINATE 2
```

**Commands:**
- `CREATE <pid> <pages>` - Create process
- `ACCESS <pid> <address>` - Read from virtual address
- `WRITE <pid> <address>` - Write to virtual address
- `TERMINATE <pid>` - Terminate process
- `MEMMAP` - Display memory map
- `STATS` - Show system statistics

## 🎮 Usage Guide

### Running Phase 1
1. Select **Phase 1** from the phase selector
2. Check backend status (should show ✅ Connected)
3. Upload input file with VM instructions
4. Click **Run Simulation**
5. View output in the results panel

### Running Phase 2
1. Select **Phase 2** from the phase selector
2. Check backend status (should show ✅ Connected)
3. Upload input file with memory commands
4. Click **Run Simulation**
5. Monitor TLB hits, page faults, and memory allocation

## 🔍 Expected Output

### Phase 1 Example
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

### Phase 2 Example
```
=== SYSTEM STATISTICS ===
TLB Hits: 3
TLB Misses: 4
TLB Hit Rate: 42.86%
Free Frames: 60/64
Active Processes: 2
=========================

=== MEMORY MAP ===
Process 1 (State: READY)
  Page Faults: 3
  Valid Pages: 0->0 1->1 2->3 
Process 2 (State: READY)
  Page Faults: 1
  Valid Pages: 0->2 
==================
```

## 🐛 Troubleshooting

### Backend Connection Failed
- Ensure backend is running on port 5000
- Check `npm start` output in backend terminal
- Verify no other service is using port 5000

### C++ Compilation Errors
- Verify GCC/G++ is installed: `g++ --version`
- Run `npm run compile` in backend directory
- Check if `phase1.exe` and `phase2.exe` exist

### Frontend Won't Start
- Clear node_modules: `rm -rf node_modules && npm install`
- Check if port 3000 is available
- Try different port: `npm run dev -- --port 3001`

### Quick Diagnostics

**Check Backend Health:**
```bash
# PowerShell
Invoke-RestMethod -Uri "http://localhost:5000/api/health"

# Or open in browser
http://localhost:5000/api/health
```

**Check Executables:**
```bash
http://localhost:5000/api/check-executables
```

## 🔧 Technical Architecture

```
React Frontend (Port 3000)
        ↓
  REST API Calls
        ↓
Express Server (Port 5000)
        ↓
 Child Process Execution
        ↓
C++ Executables (phase1.exe, phase2.exe)
        ↓
    File I/O
```

### API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/health` | Backend health check |
| GET | `/api/check-executables` | Verify C++ programs |
| POST | `/api/phase1` | Execute Phase 1 simulation |
| POST | `/api/phase2` | Execute Phase 2 simulation |

## 🛠️ Development

### Backend
- Express.js server bridges React and C++
- Temporary files for process I/O
- Real-time output streaming

### Frontend
- React 18 with Vite
- Modern UI with gradient design
- File upload validation
- Error handling and loading states

## 📝 Scripts

### Backend
```bash
npm start          # Start server
npm run compile    # Compile C++ programs
```

### Frontend
```bash
npm run dev        # Start development server
npm run build      # Build for production
npm run preview    # Preview production build
```

## 📄 License

This project is created for educational purposes.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## 📧 Support

For issues or questions, please open an issue in the repository.

---

**Built with ❤️ for Operating Systems Education**
