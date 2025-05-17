import React from "react";
import { Button, TextField } from "@salutejs/plasma-ui";
import styled from "styled-components";

interface GuessInputProps {
  onGuess: (word: string) => void;
  onGiveUp: () => void; // Add this new prop
  disabled?: boolean;
}

interface GuessInputState {
  guess: string;
}

const Form = styled.form`
  display: flex;
  gap: 0.5rem;
  width: 100%;
  margin: 0 auto;
`;

const NewGameBtn = styled(Button)`
  width: 100%;
  margin-bottom: 16px; // Add margin to ensure visibility above assistant menu
`;

interface NewGameButtonProps {
  children: React.ReactNode;
  onClick: () => void;
}

export class GuessInput extends React.Component<
  GuessInputProps,
  GuessInputState
> {
  static NewGameButton: React.FC<NewGameButtonProps> = ({
    children,
    onClick,
  }) => (
    <NewGameBtn view="success" onClick={onClick}>
      {children}
    </NewGameBtn>
  );

  constructor(props: GuessInputProps) {
    super(props);
    this.state = {
      guess: "",
    };
  }

  handleSubmit = (event: React.FormEvent<HTMLFormElement>): void => {
    event.preventDefault();
    if (this.state.guess.trim() && !this.props.disabled) {
      this.props.onGuess(this.state.guess);
      this.setState({ guess: "" });
    }
  };

  handleChange = (event: React.ChangeEvent<HTMLInputElement>): void => {
    this.setState({ guess: event.target.value });
  };

  handleGiveUp = (): void => {
    this.props.onGiveUp();
  };

  render(): React.ReactNode {
    return (
      <>
        <Form onSubmit={this.handleSubmit}>
          <TextField
            placeholder="Введите слово"
            value={this.state.guess}
            onChange={this.handleChange}
            disabled={this.props.disabled}
            required
            autoFocus
          />
          <Button type="submit" view="primary" disabled={this.props.disabled}>
            Угадать
          </Button>
          {!this.props.disabled && (
            <Button view="warning" onClick={this.handleGiveUp}>
              Сдаться
            </Button>
          )}
        </Form>
      </>
    );
  }
}
