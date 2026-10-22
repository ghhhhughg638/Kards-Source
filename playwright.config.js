import { defineConfig } from "@playwright/test";
export default defineConfig({
  testDir: "./tools/browser",
  timeout: 30000,
  use: { baseURL: "http://127.0.0.1:4184", viewport: { width: 1440, height: 950 }, launchOptions: { ...(process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH ? { executablePath: process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH } : {}) } },
  webServer: { command: "node tools/serve.mjs", url: "http://127.0.0.1:4184", reuseExistingServer: !process.env.CI },
});
