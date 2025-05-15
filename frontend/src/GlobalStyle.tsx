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
  }

  :root {
    ${darkSber}
  }
`;
