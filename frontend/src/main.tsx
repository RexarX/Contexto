import React from "react";
import { createRoot } from "react-dom/client";
import { App } from "./App";
import "./index.css";
import { DeviceThemeProvider } from "@salutejs/plasma-ui";
import { GlobalStyle } from "./GlobalStyle";

const root = createRoot(document.getElementById("root") as HTMLElement);
root.render(
  <React.StrictMode>
    <DeviceThemeProvider>
      <GlobalStyle />
      <App />
    </DeviceThemeProvider>
  </React.StrictMode>,
);
