import logger from "./utils/logging";

// Configure logger based on environment or other factors
if (process.env.NODE_ENV === "development") {
  logger.setLevel("debug"); // Show all logs in development
} else {
  logger.setLevel("error"); // Only show errors in production
  // You could enable more logging in production if needed:
  // logger.enableProductionLogging(true);
}

export default logger;
