import React, { useEffect, useState } from "react";
import { Word } from "../types";
import { Card, Badge, Body1, Headline3, Cell } from "@salutejs/plasma-ui";
import styled from "styled-components";

interface WordsListProps {
  words: Word[];
  gameOver: boolean;
  targetWord?: string;
}

const WordsContainer = styled(Card)`
  margin-bottom: 1.5rem;
  padding: 1rem;
  min-height: 300px;
`;

const TargetWordCard = styled(Card)`
  padding: 1.5rem;
  margin-bottom: 1rem;
  text-align: center;
  background-color: rgba(46, 204, 113, 0.1);
`;

const WordsListContainer = styled.div`
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
`;

const WordItem = styled(Cell)<{ isNew: boolean }>`
  animation: ${(props: { isNew: boolean }) =>
    props.isNew ? "highlight 1.5s ease-out" : "none"};

  @keyframes highlight {
    0% {
      background-color: rgba(52, 152, 219, 0.2);
      transform: translateX(-5px);
    }
    50% {
      background-color: rgba(52, 152, 219, 0.1);
      transform: translateX(0);
    }
    100% {
      background-color: transparent;
    }
  }
`;

const HintText = styled(Body1)`
  text-align: center;
  color: rgba(255, 255, 255, 0.6);
  padding: 3rem 0;
  font-style: italic;
`;

const RankBadge = styled(Badge)<{ customView?: string }>`
  background-color: ${(props) => {
    switch (props.customView) {
      case "accent":
        return "#27ae60"; // Green for very close matches
      case "default":
        return "#3498db"; // Blue for medium matches
      case "low":
        return "#95a5a6"; // Gray for distant matches
      default:
        return ""; // Use default Badge color
    }
  }};
  min-width: 24px;
  text-align: center;
  display: flex;
  align-items: center;
  justify-content: center;
`;

export const WordsList: React.FC<WordsListProps> = ({
  words,
  gameOver,
  targetWord,
}) => {
  const [lastAddedWordId, setLastAddedWordId] = useState<string | null>(null);

  useEffect(() => {
    // When a new word is added, mark it as the latest
    if (words.length > 0) {
      const latestWord = words[words.length - 1];
      setLastAddedWordId(latestWord.id);

      // Reset the animation after it completes
      const timer = setTimeout(() => {
        setLastAddedWordId(null);
      }, 1500);

      return () => clearTimeout(timer);
    }
  }, [words.length]);

  return (
    <WordsContainer>
      {gameOver && targetWord && (
        <TargetWordCard>
          <Headline3>
            Секретное слово:{" "}
            <span style={{ color: "#2ecc71", fontWeight: "bold" }}>
              {targetWord}
            </span>
          </Headline3>
        </TargetWordCard>
      )}

      <WordsListContainer>
        {words.length === 0 ? (
          <HintText>Попробуйте угадать секретное слово!</HintText>
        ) : (
          words.map((word) => {
            const rankValue = word.rank <= 0 ? "?" : String(word.rank);

            return (
              <WordItem
                key={word.id}
                isNew={word.id === lastAddedWordId}
                content={word.text}
                contentLeft={
                  <RankBadge
                    view={
                      word.rank <= 0
                        ? "primary"
                        : word.rank <= 5
                          ? "warning"
                          : word.rank <= 20
                            ? "primary"
                            : "secondary"
                    }
                    size="s"
                    text={rankValue}
                  />
                }
              />
            );
          })
        )}
      </WordsListContainer>
    </WordsContainer>
  );
};
