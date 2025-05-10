import React from "react";
import { GuessInput } from "../components/GuessInput";
import { WordsList } from "../components/WordsList";
import { Celebration } from "../components/Celebration";
import { VoiceFeedback } from "../components/VoiceFeedback";
import { GameState } from "../types";
import { Container, Header, Headline1, Body1, Card } from "@salutejs/plasma-ui";
import styled from "styled-components";

const StyledHeader = styled(Header)`
  text-align: center;
  display: block;
  margin-bottom: 1.5rem;
  width: 100%;
`;

const StyledHeadline = styled(Headline1)`
  margin-bottom: 0.5rem;
  text-align: center;
  width: 100%;
`;

interface ContextoGameProps {
  gameState: GameState;
  onGuess: (word: string) => void;
  onNewGame: () => void;
  feedbackMessage?: {
    text: string;
    type: "feedback" | "error" | "success";
  };
}

export const ContextoGame: React.FC<ContextoGameProps> = ({
  gameState,
  onGuess,
  onNewGame,
  feedbackMessage,
}) => {
  const { guessedWords, gameOver, targetWord } = gameState;

  return (
    <Container>
      <Card style={{ padding: "2rem" }}>
        <StyledHeader style={{ textAlign: "center", display: "block" }}>
          <StyledHeadline>Contexto</StyledHeadline>
        </StyledHeader>

        <WordsList
          words={guessedWords}
          gameOver={gameOver}
          targetWord={targetWord}
        />

        <div style={{ marginTop: "1rem" }}>
          <GuessInput onGuess={onGuess} disabled={gameOver} />

          {gameOver && (
            <div style={{ marginTop: "1rem" }}>
              <GuessInput.NewGameButton onClick={onNewGame}>
                Новая игра
              </GuessInput.NewGameButton>
            </div>
          )}
        </div>
      </Card>

      {feedbackMessage && (
        <VoiceFeedback
          message={feedbackMessage.text}
          type={feedbackMessage.type}
        />
      )}

      {gameOver && targetWord && <Celebration word={targetWord} />}
    </Container>
  );
};
