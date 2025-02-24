import React, { useState, useEffect } from 'react';
import styled from 'styled-components';

const AppContainer = styled.div`
  width: 100vw;
  height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: #1a1a1a;
  color: white;
`;

const WelcomeText = styled.h1`
  font-size: 48px;
  margin-bottom: 20px;
  text-align: center;
`;

const InputContainer = styled.div`
  margin: 20px;
  
  input {
    padding: 10px;
    font-size: 24px;
    background: #333;
    border: none;
    color: white;
    border-radius: 8px;
    width: 300px;
    text-align: center;

    &:focus {
      outline: 2px solid #666;
    }
  }
`;

const StatusText = styled.p`
  color: ${props => props.color || '#666'};
  margin-top: 10px;
  text-align: center;
`;

const DebugInfo = styled.div`
  position: fixed;
  bottom: 10px;
  left: 10px;
  color: #666;
  font-size: 12px;
  background: rgba(0, 0, 0, 0.8);
  padding: 10px;
  border-radius: 4px;
  max-width: 500px;
  word-wrap: break-word;
  white-space: pre-wrap;
`;

const App: React.FC = () => {
  const [message, setMessage] = useState<string>('Welcome to App!');
  const [name, setName] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [error, setError] = useState<string>('');
  const [debugInfo, setDebugInfo] = useState<string>('');

  const checkBackendHealth = async () => {
    try {
      const response = await fetch('/api/ping');
      setDebugInfo(`Backend health check: ${response.ok ? 'OK' : 'Failed'}`);
      return response.ok;
    } catch (error) {
      setDebugInfo(`Backend health check failed: ${error instanceof Error ? error.message : 'Unknown error'}`);
      return false;
    }
  };

  const fetchWelcome = async (inputName?: string) => {
    setIsLoading(true);
    setError('');

    try {
      // First check if backend is healthy
      const isHealthy = await checkBackendHealth();
      if (!isHealthy) {
        throw new Error('Backend is not responding');
      }

      const url = `/api/hello${inputName ? `?name=${encodeURIComponent(inputName)}` : ''}`;
      setDebugInfo(prev => `${prev}\nMaking request to: ${url}`);
      
      const response = await fetch(url);
      setDebugInfo(prev => `${prev}\nResponse status: ${response.status}`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const text = await response.text();
      setDebugInfo(prev => `${prev}\nRaw response: ${text}`);
      
      try {
        const data = JSON.parse(text);
        setDebugInfo(prev => `${prev}\nParsed data: ${JSON.stringify(data)}`);
        setMessage(data.message);
      } catch (parseError) {
        throw new Error(`Failed to parse JSON: ${text}`);
      }
    } catch (error) {
      console.error('Error fetching welcome message:', error);
      const errorMessage = error instanceof Error ? error.message : 'Unknown error';
      setError(`Failed to fetch welcome message: ${errorMessage}`);
      setDebugInfo(prev => `${prev}\nError: ${errorMessage}`);
      setMessage('Welcome to App!');
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    fetchWelcome();
  }, []);

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter' && !isLoading) {
      fetchWelcome(name);
    }
  };

  const handleSubmit = () => {
    if (!isLoading) {
      fetchWelcome(name);
    }
  };

  return (
    <AppContainer>
      <WelcomeText>{message}</WelcomeText>
      <InputContainer>
        <input
          type="text"
          value={name}
          onChange={(e) => setName(e.target.value)}
          onKeyDown={handleKeyDown}
          placeholder="Enter your name"
          disabled={isLoading}
          autoFocus
        />
      </InputContainer>
      <button 
        onClick={handleSubmit}
        disabled={isLoading}
        style={{
          padding: '10px 20px',
          fontSize: '18px',
          backgroundColor: '#444',
          color: 'white',
          border: 'none',
          borderRadius: '4px',
          cursor: isLoading ? 'not-allowed' : 'pointer',
          marginTop: '10px'
        }}
      >
        {isLoading ? 'Loading...' : 'Submit'}
      </button>
      {isLoading && <StatusText color="#666">Loading...</StatusText>}
      {error && <StatusText color="#ff4444">{error}</StatusText>}
      {debugInfo && <DebugInfo>{debugInfo}</DebugInfo>}
    </AppContainer>
  );
};

export default App;