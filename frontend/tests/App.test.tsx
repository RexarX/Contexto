import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { describe, it, expect, beforeEach, vi } from 'vitest';
import App from '../src/App';

// Mock fetch globally
const mockFetch = vi.fn();
global.fetch = mockFetch;

// Helper function to create mock responses
const createMockResponse = (ok: boolean, status: number, data?: any) => ({
  ok,
  status,
  text: () => Promise.resolve(data ? JSON.stringify(data) : ''),
});

describe('App Component', () => {
  beforeEach(() => {
    vi.clearAllMocks();
    // Mock successful health check by default
    mockFetch.mockImplementationOnce(() => 
      Promise.resolve(createMockResponse(true, 200))
    );
  });

  it('renders initial welcome message', async () => {
    render(<App />);
    
    // Initial message should be visible
    expect(await screen.findByRole('heading', { level: 1 }))
      .toHaveTextContent('Welcome to App!');
  });

  it('renders input field and submit button in initial state', async () => {
    render(<App />);
    
    // Input should be enabled and empty
    const input = await screen.findByPlaceholderText('Enter your name');
    expect(input).toBeEnabled();
    expect(input).toHaveValue('');

    // Button should be enabled and show "Submit"
    const button = await screen.findByRole('button');
    expect(button).toBeEnabled();
    expect(button).toHaveTextContent('Submit');
  });

  it('handles successful name submission', async () => {
    mockFetch
      .mockImplementationOnce(() => Promise.resolve(createMockResponse(true, 200))) // health check
      .mockImplementationOnce(() => 
        Promise.resolve(createMockResponse(true, 200, { message: 'Welcome, John!' }))
      );

    render(<App />);
    
    // Find and interact with elements
    const input = await screen.findByPlaceholderText('Enter your name');
    const button = await screen.findByRole('button');

    // Type name and submit
    fireEvent.change(input, { target: { value: 'John' } });
    fireEvent.click(button);

    // Verify loading state
    expect(button).toBeDisabled();
    expect(button).toHaveTextContent('Loading...');

    // Verify success state
    expect(await screen.findByText('Welcome, John!')).toBeInTheDocument();
    expect(button).toBeEnabled();
    expect(button).toHaveTextContent('Submit');
  });

  it('handles API error response', async () => {
    mockFetch
      .mockImplementationOnce(() => Promise.resolve(createMockResponse(true, 200))) // health check
      .mockImplementationOnce(() => 
        Promise.resolve(createMockResponse(false, 500))
      );

    render(<App />);
    
    const input = await screen.findByPlaceholderText('Enter your name');
    const button = await screen.findByRole('button');

    fireEvent.change(input, { target: { value: 'John' } });
    fireEvent.click(button);

    // Verify error state
    expect(await screen.findByText(/Failed to fetch welcome message/)).toBeInTheDocument();
    expect(button).toBeEnabled();
    expect(button).toHaveTextContent('Submit');
  });

  it('handles keyboard submission with Enter key', async () => {
    mockFetch
      .mockImplementationOnce(() => Promise.resolve(createMockResponse(true, 200))) // health check
      .mockImplementationOnce(() => 
        Promise.resolve(createMockResponse(true, 200, { message: 'Welcome, Jane!' }))
      );

    render(<App />);
    
    const input = await screen.findByPlaceholderText('Enter your name');
    
    // Type name and press Enter
    fireEvent.change(input, { target: { value: 'Jane' } });
    fireEvent.keyDown(input, { key: 'Enter' });

    // Verify success state
    expect(await screen.findByText('Welcome, Jane!')).toBeInTheDocument();
  });
});