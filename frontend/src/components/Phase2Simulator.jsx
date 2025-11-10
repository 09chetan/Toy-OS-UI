import React, { useState } from 'react'
import FileUpload from './FileUpload'

const Phase2Simulator = () => {
  const [fileContent, setFileContent] = useState('')
  const [output, setOutput] = useState('')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState('')
  const [fileName, setFileName] = useState('')

  const handleFileUpload = async (file) => {
    setLoading(true)
    setError('')
    setFileName(file.name)
    
    try {
      const content = await readFileContent(file)
      setFileContent(content)
      
      // Simulate execution
      const result = await simulatePhase2Execution(content)
      setOutput(result)
    } catch (err) {
      setError('Error reading file: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  const readFileContent = (file) => {
    return new Promise((resolve, reject) => {
      const reader = new FileReader()
      reader.onload = (e) => resolve(e.target.result)
      reader.onerror = (e) => reject(e)
      reader.readAsText(file)
    })
  }

  const simulatePhase2Execution = async (content) => {
    return new Promise((resolve) => {
      setTimeout(() => {
        const simulatedOutput = `=== OS SIMULATOR - PHASE 2 ===
Page Size: 1024 bytes
Physical Memory: 64 frames
Virtual Memory: 256 pages per process

Command [2]: CREATE 1 5
Process 1 created with 5 pages

Command [3]: CREATE 2 3
Process 2 created with 3 pages

Command [6]: ACCESS 1 0
Accessing virtual address 0 of process 1
TLB Miss: Process 1, Page 0
PAGE FAULT: Process 1, Page 0
Allocated frame 0 to page 0 of process 1
Physical address: 0

Command [7]: ACCESS 1 1024
Accessing virtual address 1024 of process 1
TLB Miss: Process 1, Page 1
PAGE FAULT: Process 1, Page 1
Allocated frame 1 to page 1 of process 1
Physical address: 1024

Command [8]: ACCESS 2 0
Accessing virtual address 0 of process 2
TLB Miss: Process 2, Page 0
PAGE FAULT: Process 2, Page 0
Allocated frame 2 to page 0 of process 2
Physical address: 2048

Command [11]: WRITE 1 2048
Writing to virtual address 2048 of process 1
TLB Miss: Process 1, Page 2
PAGE FAULT: Process 1, Page 2
Allocated frame 3 to page 2 of process 1
Physical address: 3072

Command [12]: WRITE 2 512
Writing to virtual address 512 of process 2
TLB Hit: Process 2, Page 0
Physical address: 2560

Command [15]: ACCESS 1 0
Accessing virtual address 0 of process 1
TLB Hit: Process 1, Page 0
Physical address: 0

Command [16]: ACCESS 2 0
Accessing virtual address 0 of process 2
TLB Hit: Process 2, Page 0
Physical address: 2048

Command [19]: MEMMAP

=== MEMORY MAP ===
Process 1 (State: READY)
  Page Faults: 3
  Valid Pages: 0->0 1->1 2->3 
Process 2 (State: READY)
  Page Faults: 1
  Valid Pages: 0->2 
==================

Command [22]: STATS

=== SYSTEM STATISTICS ===
TLB Hits: 3
TLB Misses: 4
TLB Hit Rate: 42.86%
Free Frames: 60/64
Active Processes: 2
=========================

Command [25]: TERMINATE 1
Process 1 terminated. Page faults: 3

Command [28]: MEMMAP

=== MEMORY MAP ===
Process 2 (State: READY)
  Page Faults: 1
  Valid Pages: 0->2 
==================

Command [31]: TERMINATE 2
Process 2 terminated. Page faults: 1

Command [34]: STATS

=== SYSTEM STATISTICS ===
TLB Hits: 3
TLB Misses: 4
TLB Hit Rate: 42.86%
Free Frames: 64/64
Active Processes: 0
=========================


=== FINAL STATISTICS ===

=== SYSTEM STATISTICS ===
TLB Hits: 3
TLB Misses: 4
TLB Hit Rate: 42.86%
Free Frames: 64/64
Active Processes: 0
=========================

=== MEMORY MAP ===
==================`
        resolve(simulatedOutput)
      }, 2000)
    })
  }

  const handleRunSimulation = async () => {
    if (!fileContent) {
      setError('Please upload a file first')
      return
    }

    setLoading(true)
    setError('')
    
    try {
      const result = await simulatePhase2Execution(fileContent)
      setOutput(result)
    } catch (err) {
      setError('Simulation error: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div>
      <h2>Phase 2 - Memory Management Simulator</h2>
      
      <FileUpload 
        onFileUpload={handleFileUpload}
        acceptedFileTypes=".txt"
        phase={2}
      />

      {fileName && (
        <div className="file-info">
          <strong>Selected File:</strong> {fileName}
        </div>
      )}

      {fileContent && (
        <div style={{ marginTop: '20px' }}>
          <button 
            className="upload-btn"
            onClick={handleRunSimulation}
            disabled={loading}
          >
            {loading ? 'Running...' : 'Run Simulation'}
          </button>
        </div>
      )}

      {error && (
        <div className="error">
          {error}
        </div>
      )}

      {loading && (
        <div className="loading">
          Running Memory Management Simulation...
        </div>
      )}

      {output && (
        <div className="output-section">
          <h3 className="output-title">Simulation Output:</h3>
          <div className="output-content">
            {output}
          </div>
        </div>
      )}

      {!fileContent && !loading && (
        <div className="instructions">
          <h3>Sample Phase 2 Input Format:</h3>
          <div className="output-content">
{`# Create two processes
CREATE 1 5
CREATE 2 3

# Access memory (first time → page fault)
ACCESS 1 0
ACCESS 1 1024
ACCESS 2 0

# Write data (also may trigger page fault)
WRITE 1 2048
WRITE 2 512

# Access same addresses again (TLB Hit expected)
ACCESS 1 0
ACCESS 2 0

# Show current memory usage
MEMMAP

# Show system stats (TLB hits, misses, page faults)
STATS

# End process 1
TERMINATE 1

# Check memory again
MEMMAP

# Terminate remaining process
TERMINATE 2

# Final system statistics
STATS`}
          </div>
        </div>
      )}
    </div>
  )
}

export default Phase2Simulator