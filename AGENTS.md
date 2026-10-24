# Working agreements

- Always run `npm format` after modifying files.
- Always run `npm lint` after modifying files.
- Always run `npm test` after modifying JavaScript files.
- Prefer `npm` when installing dependencies.
- Ask for confirmation before adding new production dependencies.

# Save compatibility during development

This project is in active prototype development. New or updated features may
break older save files. Do not preserve obsolete species identifiers, add
compatibility aliases or save migrations, or constrain a feature to keep old
saves working unless the user explicitly requests compatibility. Update the
current game data and tests together, and use a new save for validation when
needed. Compatibility within the current build (saving and loading, storage,
breeding, trading, and multiplayer) must still work.
