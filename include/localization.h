#ifndef GUARD_LOCALIZATION_H
#define GUARD_LOCALIZATION_H

// Preserve upstream MESSAGE fixtures in native tests. Normal and release ROMs
// select Czech before the game's charmap preprocessor encodes the string.
#if TESTING
#define LOCALIZED_TEXT(english, czech) english
#else
#define LOCALIZED_TEXT(english, czech) czech
#endif

#endif // GUARD_LOCALIZATION_H
