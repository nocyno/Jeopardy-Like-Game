#ifndef parser.h
#define parser.h
#include <json.h>

// Struct for holding configuration data for
// a particular game instance
struct _gameData {
	int 	categories;	// Number of categories
	int 	levels;		// Number of point levels
	char*** questions;	// Array of strings for all board questions
	char*** answers;	// Array of strings for all board answers
};

struct _gameData* readAndParseGameFile(char* fname);

#endif
