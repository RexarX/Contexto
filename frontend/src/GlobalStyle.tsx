import { createGlobalStyle } from "styled-components";
import { darkSber } from "@salutejs/plasma-tokens/themes";
import { text, gradient } from "@salutejs/plasma-tokens";

export const GlobalStyle = createGlobalStyle`
  html, body, #root {
    color: ${text};
    background: ${gradient};
    height: 100%;
    margin: 0;
    padding: 0;
    overflow-x: hidden;
  }

  body {
    margin: 0;
    padding: 0;
  }

  #root {
    display: flex;
    flex-direction: column;
    min-height: 100vh;
  }

  * {
    box-sizing: border-box;
    font-family: 'SB Sans Display', sans-serif;
    -webkit-tap-highlight-color: transparent;
  }

  :root {
    ${darkSber}
  }

  /* Mobile optimizations */
  input, button {
    font-size: 16px !important; /* Prevent zoom on focus in iOS */
  }

  @media (max-width: 768px) {
    /* Better button/input sizes for touch */
    button,
    [role="button"],
    input[type="submit"],
    input[type="button"] {
      min-height: 44px;
      min-width: 44px;
    }
  }
`;
