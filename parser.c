#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

char* readGameFile(char* fname) {

	// Extract the contents of the save file into a string
	FILE *f = fopen(fname, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* gameData = malloc(fsize + 1);
	fread(gameData, fsize, 1, f);
	fclose(f);

	return gameData;

}

struct _gameData* parseGameData(char* gameDataString) {
	// Pointer to a _gameData struct to hold all of the
	// game instance configuration data we will return.
	struct _gameData* gameData = (struct _gameData*) malloc(sizeof(struct _gameData));

	// Parse the game data string into a json_object
	struct json_object* gameDataObj = json_tokener_parse(gameDataString);

	// Create a json_object for each of the top level json objects
	struct json_object* categoriesObj;
	struct json_object* levelsObj;
	struct json_object* questionsObj;
	struct json_object* answersObj;

	int categories;
	int levels;

	// Create variables using data from the json_object:
	// (int) 	categories 	--> Number of categories
	categoriesObj 		= json_object_object_get(gameDataObj, "categories");
	categories 		= json_object_get_int(categoriesObj);
	gameData->categories 	= categories;

	// (int) 	levels		--> Number of point levels
	levelsObj 		= json_object_object_get(gameDataObj, "levels");
	levels 			= json_object_get_int(levelsObj);
	gameData->levels	= levels;

	/* We need to iterate through the two array objects to extract their
	 contents into two double arrays of strings (a.k.a. two triple arrays
	 of chars). One for the questions, and one for the answers. */

	// Create double arrays of char*s of appropriate sizes to store the
	// questions and answers
	char* questions[levels][categories];
	char* answers[levels][categories];

	// First get the array objects
	questionsObj = json_object_object_get(gameDataObj, "questions");
	answersObj = json_object_object_get(gameDataObj, "answers");

	// Extract questions from the questionsObj
	
	// json_objects to hold nested arrays containing questions of the same point level
	// and individual questions
	struct json_object* sameLevelQuestions;
	struct json_object* singleQuestionObject;
 
	for(int i = 0; i < levels; i++) {
		sameLevelQuestions = json_object_array_get_idx(questionsObj, i);

		for (int j = 0; j < categories; j++) {
			singleQuestionObject = json_object_array_get_idx(sameLevelQuestions, j);
			questions[i][j] = json_object_get_string(singleQuestionObject);	
		}
	}
	
	// Extract answers from the answersObj
	
	// json_objects to hold nested arrays containing answers of the same point level
	// and individual answers
	struct json_object* sameLevelAnswers;
	struct json_object* singleAnswerObject;
 
	for(int i = 0; i < levels; i++) {
		sameLevelAnswers = json_object_array_get_idx(answersObj, i);

		for (int j = 0; j < categories; j++) {
			singleAnswerObject = json_object_array_get_idx(sameLevelAnswers, j);
			answers[i][j] = json_object_get_string(singleAnswerObject);	
		}
	}

	gameData->questions = questions;
/*
	//printf(typeof questions);
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 2; j++) {
			printf(questions[0][0]);
		}
	}*/
	gameData->answers = answers;

	return gameData;
} 


struct _gameData* readAndParseGameFile(char* fname) {

	char* gameDataString = readGameFile(fname);
	return parseGameData(gameDataString);

}

/*int main() {
	char* gameDataString = readGameFile("game.json");
	struct _gameData* gameData;
	gameData = parseGameData(gameDataString);
	//printf("Categories: %d\n", gameData->categories);
	//printf("Levels: %d\n", gameData->levels);
	
	for(int i = 0; i < gameData->levels; i++) {
		for(int j = 0; j < gameData->categories; j++) {
			printf(gameData->questions[j+(i*gameData->levels)]);
		}
		printf("\n");
	}
	free(gameData);
	return 0;
}*/
