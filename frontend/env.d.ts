/// <reference types="vite/client" />

interface ImportMetaEnv {
  readonly VITE_APP_TOKEN: string;
  readonly VITE_APP_SMARTAPP: string;
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}
