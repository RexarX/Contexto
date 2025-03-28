import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { resolve } from 'path';

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      '@': resolve(__dirname, 'src'),
    },
  },
  server: {
    port: 3000,
    open: true,
  },
  define: {
    'import.meta.env': {
      VITE_API_URL: process.env.VITE_API_URL,
      VITE_APP_NAME: process.env.VITE_APP_NAME,
    },
  },
});