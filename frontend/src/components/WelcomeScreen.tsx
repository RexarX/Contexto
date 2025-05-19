import React from "react";
import { Button, Card, Headline1, Headline2, Body1 } from "@salutejs/plasma-ui";
import styled from "styled-components";

interface WelcomeScreenProps {
  onStartGame: () => void;
}

const WelcomeCard = styled(Card)`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 2rem;
  background: rgba(25, 25, 25, 0.75);
  border-radius: 16px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  max-width: 800px;
  margin: 2rem auto;
  text-align: center;
`;

const Title = styled(Headline1)`
  margin-bottom: 1rem;
`;

const Subtitle = styled(Headline2)`
  color: rgba(255, 255, 255, 0.8);
  margin-bottom: 1.5rem;
  font-size: 1.5rem;
`;

const Description = styled(Body1)`
  margin-bottom: 2rem;
  max-width: 600px;
  line-height: 1.6;
  color: rgba(255, 255, 255, 0.7);
`;

const StartButton = styled(Button)`
  padding: 1rem 3rem;
  font-size: 1.2rem;
  margin-top: 1rem;
`;

export const WelcomeScreen: React.FC<WelcomeScreenProps> = ({
  onStartGame,
}) => {
  return (
    <WelcomeCard>
      <Title>Contexto</Title>
      <Subtitle>Игра в угадывание слов</Subtitle>
      <Description>
        В игре Contexto нужно угадать секретное слово. Ваши слова отсортированы
        по близости к загаданному. Чем ближе слово к началу списка, тем оно
        ближе к секретному слову.
      </Description>
      <StartButton view="primary" onClick={onStartGame}>
        Начать игру
      </StartButton>
    </WelcomeCard>
  );
};
