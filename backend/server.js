const express = require('express');
const cors = require('cors');
const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 5000;

// Middleware
app.use(cors());
app.use(express.json());

// Utility function to execute C++ programs
const executeCppProgram = (program, inputContent) => {
  return new Promise((resolve, reject) => {
    // Create temporary files
    const inputFile = `temp_input_${Date.now()}.txt`;
    const outputFile = `temp_output_${Date.now()}.txt`;
    
    console.log(`Executing ${program} with input file: ${inputFile}`);
    
    // Write input to temporary file
    fs.writeFileSync(inputFile, inputContent);
    
    // Execute C++ program
    const command = process.platform === 'win32' ? `${program}.exe` : `./${program}`;
    exec(`${command} ${inputFile} ${outputFile}`, (error, stdout, stderr) => {
      if (error) {
        console.error('Execution error:', error);
        // Cleanup on error
        if (fs.existsSync(inputFile)) fs.unlinkSync(inputFile);
        if (fs.existsSync(outputFile)) fs.unlinkSync(outputFile);
        reject(`Execution error: ${error.message}`);
        return;
      }
      
      if (stderr) {
        console.error('Stderr:', stderr);
      }
      
      // Read output file
      if (fs.existsSync(outputFile)) {
        const output = fs.readFileSync(outputFile, 'utf8');
        console.log(`Execution successful. Output length: ${output.length} characters`);
        
        // Cleanup temporary files
        fs.unlinkSync(inputFile);
        fs.unlinkSync(outputFile);
        
        resolve(output);
      } else {
        console.error('Output file not found:', outputFile);
        reject('Output file not found');
      }
    });
  });
};

// Health check endpoint
app.get('/api/health', (req, res) => {
  res.json({ 
    status: 'OK', 
    message: 'OS Simulator Backend is running',
    timestamp: new Date().toISOString()
  });
});

// Check if C++ executables exist
app.get('/api/check-executables', (req, res) => {
  const phase1Exists = fs.existsSync('phase1.exe');
  const phase2Exists = fs.existsSync('phase2.exe');
  
  res.json({
    phase1: phase1Exists ? 'Found' : 'Missing',
    phase2: phase2Exists ? 'Found' : 'Missing'
  });
});

// Phase 1 endpoint
app.post('/api/phase1', async (req, res) => {
  try {
    const { fileContent } = req.body;
    
    if (!fileContent) {
      return res.status(400).json({ error: 'File content is required' });
    }
    
    console.log('Received Phase 1 request, content length:', fileContent.length);
    const output = await executeCppProgram('phase1', fileContent);
    res.json({ success: true, output });
    
  } catch (error) {
    console.error('Phase 1 error:', error);
    res.status(500).json({ error: error.toString() });
  }
});

// Phase 2 endpoint
app.post('/api/phase2', async (req, res) => {
  try {
    const { fileContent } = req.body;
    
    if (!fileContent) {
      return res.status(400).json({ error: 'File content is required' });
    }
    
    console.log('Received Phase 2 request, content length:', fileContent.length);
    const output = await executeCppProgram('phase2', fileContent);
    res.json({ success: true, output });
    
  } catch (error) {
    console.error('Phase 2 error:', error);
    res.status(500).json({ error: error.toString() });
  }
});

// Start server
app.listen(PORT, () => {
  console.log(`🚀 Backend server running on http://localhost:${PORT}`);
  console.log('📊 Available endpoints:');
  console.log('  GET  /api/health');
  console.log('  GET  /api/check-executables');
  console.log('  POST /api/phase1');
  console.log('  POST /api/phase2');
  console.log('');
  console.log('⚠️  Make sure to compile C++ programs first: npm run compile');
});