import React from "react";
import { Button, TextField } from "@salutejs/plasma-ui";
import styled from "styled-components";

interface GuessInputProps {
  onGuess: (word: string) => void;
  onGiveUp: () => void;
  disabled?: boolean;
  layout: "desktop" | "mobile";
}

interface GuessInputState {
  guess: string;
}

// Desktop-specific components
const DesktopForm = styled.form`
  display: flex;
  gap: 1rem;
  width: 100%;
  margin: 0 auto;
`;

const DesktopInputWrapper = styled.div`
  flex: 1;
`;

// Mobile-specific components
const MobileForm = styled.form`
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
  width: 100%;
  margin: 0 auto;
`;

const MobileInputWrapper = styled.div`
  width: 100%;
`;

const MobileButtonsWrapper = styled.div`
  display: flex;
  gap: 0.5rem;

  button {
    flex: 1;
    height: 44px; // More touch-friendly
  }
`;

// Common components - fixed without using layout prop
const DesktopNewGameButton = styled(Button)`
  width: 100%;
  margin-bottom: 16px;
  height: 48px;
  font-size: 16px;
`;

const MobileNewGameButton = styled(Button)`
  width: 100%;
  margin-bottom: 16px;
  height: 44px;
  font-size: 15px;
`;

interface NewGameButtonProps {
  children: React.ReactNode;
  onClick: () => void;
  layout?: "desktop" | "mobile";
}

export class GuessInput extends React.Component<
  GuessInputProps,
  GuessInputState
> {
  static NewGameButton: React.FC<NewGameButtonProps> = ({
    children,
    onClick,
    layout = "desktop",
  }) =>
    layout === "desktop" ? (
      <DesktopNewGameButton view="success" onClick={onClick}>
        {children}
      </DesktopNewGameButton>
    ) : (
      <MobileNewGameButton view="success" onClick={onClick}>
        {children}
      </MobileNewGameButton>
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
    const { layout, disabled } = this.props;

    if (layout === "desktop") {
      return (
        <DesktopForm onSubmit={this.handleSubmit}>
          <DesktopInputWrapper>
            <TextField
              placeholder="Введите слово"
              value={this.state.guess}
              onChange={this.handleChange}
              disabled={disabled}
              required
              autoFocus
              style={{ width: "100%" }}
            />
          </DesktopInputWrapper>

          <Button
            type="submit"
            view="primary"
            disabled={disabled}
            style={{ height: "48px", minWidth: "120px" }}
          >
            Угадать
          </Button>

          {!disabled && (
            <Button
              view="warning"
              onClick={this.handleGiveUp}
              style={{ height: "48px", minWidth: "120px" }}
            >
              Сдаться
            </Button>
          )}
        </DesktopForm>
      );
    } else {
      return (
        <MobileForm onSubmit={this.handleSubmit}>
          <MobileInputWrapper>
            <TextField
              placeholder="Введите слово"
              value={this.state.guess}
              onChange={this.handleChange}
              disabled={disabled}
              required
              autoFocus
              style={{ width: "100%" }}
            />
          </MobileInputWrapper>

          <MobileButtonsWrapper>
            <Button type="submit" view="primary" disabled={disabled} size="s">
              Угадать
            </Button>

            {!disabled && (
              <Button view="warning" onClick={this.handleGiveUp} size="s">
                Сдаться
              </Button>
            )}
          </MobileButtonsWrapper>
        </MobileForm>
      );
    }
  }
}
