// tests/setup.ts
import '@testing-library/jest-dom';
import { expect, afterEach, vi } from 'vitest';
import { cleanup } from '@testing-library/react';
import * as matchers from '@testing-library/jest-dom/matchers';
import 'styled-components';

// Extend Vitest's expect method with testing-library methods
expect.extend(matchers);

// Clear any mocks and cleanup after each test
afterEach(() => {
  cleanup();
  vi.clearAllMocks();
});

// Mock styled-components to prevent serialization issues
vi.mock('styled-components', async () => {
  const actual = await vi.importActual('styled-components');
  return {
    ...(actual as any),
    createGlobalStyle: vi.fn(),
    css: vi.fn(),
    keyframes: vi.fn(),
    ThemeProvider: ({ children }: { children: React.ReactNode }) => children,
  };
});

// Reset the debug output after each test
afterEach(() => {
  if (document.body.querySelector('#debug-info')) {
    document.body.querySelector('#debug-info')?.remove();
  }
});