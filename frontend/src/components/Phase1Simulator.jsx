import React, { useState } from 'react'
import FileUpload from './FileUpload'

const Phase1Simulator = () => {
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
      
      // Simulate execution (in real app, this would call your C++ backend)
      const result = await simulatePhase1Execution(content)
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

  const simulatePhase1Execution = async (content) => {
    // Simulate API call to backend
    return new Promise((resolve) => {
      setTimeout(() => {
        // This is where you would integrate with your actual C++ backend
        // For now, we'll simulate the output
        const simulatedOutput = `Virtual Machine Simulation Started...
Processing file content...

Input Program:
${content}

Execution Trace:
New Job started
Program Card loading
Data card loading
Read function called
Write function called
HELLO WORLD
Terminate called

END of Job

Simulation completed successfully!`
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
      // In a real implementation, this would call your backend API
      const result = await simulatePhase1Execution(fileContent)
      setOutput(result)
    } catch (err) {
      setError('Simulation error: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div>
      <h2>Phase 1 - Virtual Machine Simulator</h2>
      
      <FileUpload 
        onFileUpload={handleFileUpload}
        acceptedFileTypes=".txt"
        phase={1}
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
          Running Virtual Machine Simulation...
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
          <h3>Sample Phase 1 Input Format:</h3>
          <div className="output-content">
{`$AMJ000100050001
GD20 PD20 H
$DTA
HELLO WORLD
$END0001`}
          </div>
        </div>
      )}
    </div>
  )
}

export default Phase1Simulator