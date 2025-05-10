import { createGlobalStyle } from "styled-components";
import { darkSber } from "@salutejs/plasma-tokens/themes";
import { text, background } from "@salutejs/plasma-tokens";

export const GlobalStyle = createGlobalStyle`
  html {
    color: ${text};
    background-color: ${background};
    height: 100%;
  }

  body {
      margin: 0;
      padding: 0;
    }

    * {
      box-sizing: border-box;
      font-family: 'SB Sans Display', sans-serif;
    }

  :root {
    ${darkSber}
  }
`;
