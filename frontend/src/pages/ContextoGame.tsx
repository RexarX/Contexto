import React from "react";
import { GuessInput } from "../components/GuessInput";
import { WordsList } from "../components/WordsList";
import { Celebration } from "../components/Celebration";
import { VoiceFeedback } from "../components/VoiceFeedback";
import { GameState } from "../types";
import { Container, Header, Headline1, Card } from "@salutejs/plasma-ui";
import styled from "styled-components";

// Common styled components
const StyledHeadline = styled(Headline1)`
  text-align: center;
  margin-bottom: 0.5rem;
  width: 100%;
`;

// Desktop Layout Components
const DesktopContainer = styled(Container)`
  display: flex;
  flex-direction: column;
  max-width: 1000px;
  min-height: calc(100vh - 80px);
  margin: 0 auto;
  padding: 2rem;
  padding-bottom: 180px;

  @media (max-width: 768px) {
    display: none; // Hide on mobile
  }
`;

const DesktopGameCard = styled(Card)`
  padding: 2rem;
  flex-grow: 1;
  display: flex;
  flex-direction: column;
  background: rgba(25, 25, 25, 0.75);
  border-radius: 16px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
`;

const DesktopHeader = styled(Header)`
  text-align: center;
  display: block;
  margin-bottom: 1.5rem;
  width: 100%;
`;

const DesktopContent = styled.div`
  flex-grow: 1;
  display: flex;
  flex-direction: column;
`;

const DesktopBottomArea = styled.div`
  margin-top: 1.5rem;
`;

// Mobile Layout Components
const MobileContainer = styled(Container)`
  display: flex;
  flex-direction: column;
  min-height: calc(100vh - 100px);
  margin: 0 auto;
  padding: 0.75rem;
  padding-bottom: 180px;

  @media (min-width: 769px) {
    display: none; // Hide on desktop
  }
`;

const MobileGameCard = styled(Card)`
  padding: 1rem;
  flex-grow: 1;
  display: flex;
  flex-direction: column;
  background: rgba(25, 25, 25, 0.75);
  border-radius: 12px;
  box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
`;

const MobileHeader = styled(Header)`
  text-align: center;
  display: block;
  margin-bottom: 0.5rem;
`;

const MobileHeadline = styled(Headline1)`
  font-size: 28px;
  text-align: center;
  margin-bottom: 0.5rem;
  width: 100%;
`;

const MobileContent = styled.div`
  flex-grow: 1;
  display: flex;
  flex-direction: column;
`;

const MobileBottomArea = styled.div`
  margin-top: 0.75rem;
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

  // Common elements
  const feedbackElement = feedbackMessage && (
    <VoiceFeedback message={feedbackMessage.text} type={feedbackMessage.type} />
  );

  const celebrationElement = gameOver && targetWord && !userGaveUp && (
    <Celebration word={targetWord} />
  );

  // Render both layouts, and let CSS handle which one is displayed
  return (
    <>
      {/* Desktop Layout */}
      <DesktopContainer>
        <DesktopGameCard>
          <DesktopHeader>
            <StyledHeadline>Контексто</StyledHeadline>
          </DesktopHeader>

          <DesktopContent>
            <WordsList
              words={guessedWords}
              gameOver={gameOver}
              targetWord={targetWord}
              layout="desktop"
            />

            <DesktopBottomArea>
              <GuessInput
                onGuess={onGuess}
                onGiveUp={onGiveUp}
                disabled={gameOver}
                layout="desktop"
              />

              {gameOver && (
                <div style={{ marginTop: "1rem" }}>
                  <GuessInput.NewGameButton onClick={onNewGame}>
                    Новая игра
                  </GuessInput.NewGameButton>
                </div>
              )}
            </DesktopBottomArea>
          </DesktopContent>
        </DesktopGameCard>

        {feedbackElement}
        {celebrationElement}
      </DesktopContainer>

      {/* Mobile Layout */}
      <MobileContainer>
        <MobileGameCard>
          <MobileHeader>
            <MobileHeadline>Контексто</MobileHeadline>
          </MobileHeader>

          <MobileContent>
            <WordsList
              words={guessedWords}
              gameOver={gameOver}
              targetWord={targetWord}
              layout="mobile"
            />

            <MobileBottomArea>
              <GuessInput
                onGuess={onGuess}
                onGiveUp={onGiveUp}
                disabled={gameOver}
                layout="mobile"
              />

              {gameOver && (
                <div style={{ marginTop: "0.75rem" }}>
                  <GuessInput.NewGameButton onClick={onNewGame}>
                    Новая игра
                  </GuessInput.NewGameButton>
                </div>
              )}
            </MobileBottomArea>
          </MobileContent>
        </MobileGameCard>

        {feedbackElement}
        {celebrationElement}
      </MobileContainer>
    </>
  );
};
