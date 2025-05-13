// @ts-check
import { defineConfig } from "eslint/config";
import globals from "globals";
import js from "@eslint/js";
import tseslint from "typescript-eslint";

export default defineConfig([
  {
    files: ["**/*.{js,mjs,cjs,ts}"],
    languageOptions: {
      globals: globals.node,
      parserOptions: {
        projectService: true,
        tsconfigRootDir: import.meta.dirname,
      },
    },
    plugins: { js },
    extends: ["js/recommended"],
  },
  // @ts-expect-error https://github.com/typescript-eslint/typescript-eslint/issues/10935
  tseslint.configs.recommendedTypeChecked,
  {
    rules: {
      "@typescript-eslint/no-namespace": ["error", { allowDeclarations: true }],
    },
  },
  { ignores: ["dist/", "build/"] },
]);
