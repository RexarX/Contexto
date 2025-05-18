type LogLevel = "debug" | "info" | "log" | "warn" | "error" | "none";

interface LoggerConfig {
  level: LogLevel;
  enabledInProduction: boolean;
}

// Default configuration
const config: LoggerConfig = {
  level: "log",
  enabledInProduction: false,
};

// Helper to determine if we're in production mode
const isProduction = (): boolean => {
  // Check for both Vite and standard NODE_ENV indicators
  return (
    import.meta.env?.PROD === true || process.env.NODE_ENV === "production"
  );
};

// Determine if a particular log level should be displayed
const shouldLog = (level: LogLevel): boolean => {
  if (isProduction() && !config.enabledInProduction) {
    return level === "error"; // Only errors in production by default
  }

  const levels: LogLevel[] = ["debug", "info", "log", "warn", "error"];
  const configLevelIndex = levels.indexOf(config.level);
  const requestedLevelIndex = levels.indexOf(level);

  return requestedLevelIndex >= configLevelIndex;
};

const logger = {
  debug: (...args: any[]): void => {
    if (shouldLog("debug")) {
      console.debug(...args);
    }
  },

  info: (...args: any[]): void => {
    if (shouldLog("info")) {
      console.info(...args);
    }
  },

  log: (...args: any[]): void => {
    if (shouldLog("log")) {
      console.log(...args);
    }
  },

  warn: (...args: any[]): void => {
    if (shouldLog("warn")) {
      console.warn(...args);
    }
  },

  error: (...args: any[]): void => {
    if (shouldLog("error") || isProduction()) {
      console.error(...args);
    }
  },

  // Configuration methods
  setLevel: (level: LogLevel): void => {
    config.level = level;
  },

  enableProductionLogging: (enabled: boolean): void => {
    config.enabledInProduction = enabled;
  },
};

export default logger;
