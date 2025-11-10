import React, { useState, useEffect } from 'react';
import FileUpload from './FileUpload';
import { executePhase1, checkHealth } from '../services/api';

const Phase1Simulator = () => {
  const [fileContent, setFileContent] = useState('');
  const [output, setOutput] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [fileName, setFileName] = useState('');
  const [backendStatus, setBackendStatus] = useState('checking');

  // Check backend health on component mount
  useEffect(() => {
    const checkBackend = async () => {
      const isHealthy = await checkHealth();
      setBackendStatus(isHealthy ? 'connected' : 'disconnected');
    };
    checkBackend();
  }, []);

  const handleFileUpload = async (file) => {
    setLoading(true);
    setError('');
    setFileName(file.name);
    
    try {
      const content = await readFileContent(file);
      setFileContent(content);
    } catch (err) {
      setError('Error reading file: ' + err.message);
    } finally {
      setLoading(false);
    }
  };

  const readFileContent = (file) => {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = (e) => resolve(e.target.result);
      reader.onerror = (e) => reject(e);
      reader.readAsText(file);
    });
  };

  const handleRunSimulation = async () => {
    if (!fileContent) {
      setError('Please upload a file first');
      return;
    }

    if (backendStatus !== 'connected') {
      setError('Backend server is not connected. Please start the backend server first.');
      return;
    }

    setLoading(true);
    setError('');
    
    try {
      const result = await executePhase1(fileContent);
      setOutput(result.output);
    } catch (err) {
      setError('Simulation error: ' + err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div>
      <h2>Phase 1 - Virtual Machine Simulator</h2>
      
      {/* Backend Status Indicator */}
      <div style={{ 
        padding: '10px', 
        marginBottom: '20px', 
        borderRadius: '5px',
        backgroundColor: backendStatus === 'connected' ? '#d4edda' : '#f8d7da',
        color: backendStatus === 'connected' ? '#155724' : '#721c24',
        border: `1px solid ${backendStatus === 'connected' ? '#c3e6cb' : '#f5c6cb'}`
      }}>
        <strong>Backend Status:</strong> {backendStatus === 'connected' ? '✅ Connected' : '❌ Disconnected'}
        {backendStatus !== 'connected' && (
          <div style={{ fontSize: '0.9em', marginTop: '5px' }}>
            Please make sure the backend server is running on http://localhost:5000
          </div>
        )}
      </div>
      
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
            disabled={loading || backendStatus !== 'connected'}
          >
            {loading ? 'Running Simulation...' : 'Run Simulation'}
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
          Executing Virtual Machine Simulation via Backend...
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

export default Phase1Simulator;