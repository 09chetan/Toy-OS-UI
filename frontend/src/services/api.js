const API_BASE_URL = 'http://localhost:5000/api';

// Generic API call function
const apiCall = async (endpoint, data) => {
  try {
    const response = await fetch(`${API_BASE_URL}${endpoint}`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(data),
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
    }

    return await response.json();
  } catch (error) {
    console.error('API call failed:', error);
    throw error;
  }
};

// Health check
export const checkHealth = async () => {
  try {
    const response = await fetch(`${API_BASE_URL}/health`);
    return response.ok;
  } catch (error) {
    return false;
  }
};

// Phase 1 API
export const executePhase1 = async (fileContent) => {
  return await apiCall('/phase1', { fileContent });
};

// Phase 2 API
export const executePhase2 = async (fileContent) => {
  return await apiCall('/phase2', { fileContent });
};