import React from 'react'

const FileUpload = ({ onFileUpload, acceptedFileTypes, phase }) => {
  const handleFileChange = (event) => {
    const file = event.target.files[0]
    if (file) {
      onFileUpload(file)
    }
  }

  return (
    <div className="upload-section">
      <h3>Upload Input File for Phase {phase}</h3>
      
      <div className="instructions">
        <h3>Instructions:</h3>
        {phase === 1 ? (
          <ul>
            <li>Upload a text file with Virtual Machine instructions</li>
            <li>Format: $AMJ, program cards, $DTA, data, $END</li>
            <li>Example: GD20, PD20, H instructions</li>
          </ul>
        ) : (
          <ul>
            <li>Upload a text file with memory management commands</li>
            <li>Supported commands: CREATE, ACCESS, WRITE, TERMINATE, STATS, MEMMAP</li>
            <li>Comments start with #</li>
          </ul>
        )}
      </div>

      <div className="file-input">
        <input
          type="file"
          accept={acceptedFileTypes}
          onChange={handleFileChange}
        />
      </div>
    </div>
  )
}

export default FileUpload