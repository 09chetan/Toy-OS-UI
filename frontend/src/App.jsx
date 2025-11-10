import React, { useState } from 'react'
import Phase1Simulator from './components/Phase1Simulator'
import Phase2Simulator from './components/Phase2Simulator'
import './App.css'

function App() {
  const [activePhase, setActivePhase] = useState(1)

  return (
    <div className="container">
      <div className="header">
        <h1>Toy OS </h1>
        <p>Virtual Machine & Memory Management Simulator</p>
      </div>

      <div className="phase-selector">
        <button 
          className={`phase-btn ${activePhase === 1 ? 'active' : ''}`}
          onClick={() => setActivePhase(1)}
        >
          Phase 1 - Virtual Machine
        </button>
        <button 
          className={`phase-btn ${activePhase === 2 ? 'active' : ''}`}
          onClick={() => setActivePhase(2)}
        >
          Phase 2 - Memory Management
        </button>
      </div>

      <div className="simulator-container">
        {activePhase === 1 ? <Phase1Simulator /> : <Phase2Simulator />}
      </div>
    </div>
  )
}

export default App