import React from "react";
import { GuessInput } from "../components/GuessInput";
import { WordsList } from "../components/WordsList";
import { Celebration } from "../components/Celebration";
import { VoiceFeedback } from "../components/VoiceFeedback";
import { GameState } from "../types";
import { Container, Header, Headline1, Card } from "@salutejs/plasma-ui";
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

const GameContainer = styled(Container)`
  min-height: 85vh; // Slightly reduced from 90vh to prevent overflowing
  display: flex;
  flex-direction: column;
  padding-bottom: 80px; // Add extra padding at the bottom to avoid overlay with assistant menu
`;

const GameCard = styled(Card)`
  padding: 2rem;
  flex-grow: 1;
  display: flex;
  flex-direction: column;
  background: rgba(25, 25, 25, 0.85);
  border-radius: 16px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(8px);
  border: 1px solid rgba(255, 255, 255, 0.1);
`;

const ContentArea = styled.div`
  flex-grow: 1;
  display: flex;
  flex-direction: column;
`;

const BottomArea = styled.div`
  margin-top: 1rem;
  margin-bottom: 1rem;
`;

interface ContextoGameProps {
  gameState: GameState;
  onGuess: (word: string) => void;
  onNewGame: () => void;
  onGiveUp: () => void;
  feedbackMessage?: {
    text: string;
    type: "feedback" | "error" | "success";
  };
}

export const ContextoGame: React.FC<ContextoGameProps> = ({
  gameState,
  onGuess,
  onNewGame,
  onGiveUp,
  feedbackMessage,
}) => {
  const { guessedWords, gameOver, targetWord, userGaveUp } = gameState;

  return (
    <GameContainer>
      <GameCard>
        <StyledHeader>
          <StyledHeadline>Contexto</StyledHeadline>
        </StyledHeader>

        <ContentArea>
          <WordsList
            words={guessedWords}
            gameOver={gameOver}
            targetWord={targetWord}
          />

          <BottomArea>
            <GuessInput
              onGuess={onGuess}
              onGiveUp={onGiveUp}
              disabled={gameOver}
            />

            {gameOver && (
              <div style={{ marginTop: "1rem" }}>
                <GuessInput.NewGameButton onClick={onNewGame}>
                  Новая игра
                </GuessInput.NewGameButton>
              </div>
            )}
          </BottomArea>
        </ContentArea>
      </GameCard>

      {feedbackMessage && (
        <VoiceFeedback
          message={feedbackMessage.text}
          type={feedbackMessage.type}
        />
      )}

      {gameOver && targetWord && !userGaveUp && (
        <Celebration word={targetWord} />
      )}
    </GameContainer>
  );
};
