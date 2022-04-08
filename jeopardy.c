#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <json.h>

#include "parser.h"

#define NUMBER_OF_TEAMS 	8

#define GAME_MODE_MENU		0
#define GAME_MODE_GAME		1	
#define GAME_MODE_EDITOR	2

void loadCSS();
void activate();
struct _gameData* loadGameData(char* fname);
void assignPoints(GtkWidget *button, gpointer user_data);
void questionButtonClick(GtkWidget *button, gpointer user_data);
void answerButtonClick(GtkWidget *button, gpointer user_data);

int gameMode = GAME_MODE_GAME;

char teamNames [26][2] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
char categories [5][100] = {"Animals", "Sports", "Countries", "Places", "Jobs"};

struct _questionButtonClick {
	char *questionText;
	char *answerText;
	int points;
	int numberOfTeams;
	int* teamScoreCount;
	GtkWidget* window;
	GtkWidget* mainVBox;
	GtkWidget* questionGrid;
	GtkWidget** teamScores;
};

struct _answerButtonClick {
	char *answerText;
	GtkWidget* window;
	GtkWidget * questionScreenGrid;
};

struct _assignPoints {
	GtkWidget *questionButton;
	int winner;
	int points;
	int numberOfTeams;
	int* teamScoreCount;
	GtkWidget* window;
	GtkWidget* mainVBox;
	GtkWidget* questionGrid;
	GtkWidget** teamScores;
};

GtkStyleContext *context;

int main(int argc, char **argv) {

	GtkApplication *app;
	int status;

	app = gtk_application_new("jp.vivacocoa.hello", G_APPLICATION_FLAGS_NONE);

	g_signal_connect (app, "activate",
				G_CALLBACK(activate),	NULL);

	status = g_application_run (G_APPLICATION(app), argc, argv);
	g_object_unref (app);
	return status;
}

// Application Setup
void activate(GtkApplication *app, gpointer data) {

	// Window to hold the whole application
	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW(window), "Jeopardy");
	gtk_window_set_default_size(GTK_WINDOW(window), 1500, 1000);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 15);

	// Setup CSS
	GtkCssProvider *cssProvider = gtk_css_provider_new();

	gtk_css_provider_load_from_path(cssProvider, "jeopardy.css", NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
							GTK_STYLE_PROVIDER(cssProvider),
							GTK_STYLE_PROVIDER_PRIORITY_USER);


	startGame(window, 8);
	gtk_widget_show_all(window);
}

void startGame(GtkWidget* window, int numberOfTeams) {

	struct _gameData* gameData = readAndParseGameFile("game.json");

	int numberOfLevels = gameData->levels;
	int numberOfCategories = gameData->categories;

	char* questions[numberOfLevels][numberOfCategories];
	char* answers[numberOfLevels][numberOfCategories];

	for(int i = 0; i < numberOfLevels; i++) {
		for(int j = 0; j < numberOfCategories; j++) {
			//printf("TEST");
			questions[i][j] = gameData->questions[(i*numberOfLevels) + j];
			answers[i][j] = gameData->answers[(i*numberOfLevels) + j];
		}
	}

	// Setup HBox to display all of the team scores
	GtkWidget *scoreHBox;
	static GtkWidget *teamScores[26];
	static int teamScoreCount[26];

	gchar* scoreLabel;
	for(int i = 0; i < numberOfTeams; i++) {
		scoreLabel = g_strdup_printf("Team %s: 0", teamNames[i]);

		GtkWidget *teamScoreLabel = gtk_label_new("");
		gtk_label_set_text(teamScoreLabel, scoreLabel);
		gtk_widget_set_hexpand(teamScoreLabel, TRUE);
		gtk_widget_set_vexpand(teamScoreLabel, TRUE);
		gtk_label_set_xalign(teamScoreLabel, 0.5);
		context = gtk_widget_get_style_context(teamScoreLabel);
		gtk_style_context_add_class(context, "team_score_label");
		teamScores[i] = teamScoreLabel;
	}

	// VBox to contain the questions and score counters
	GtkWidget* mainVBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_container_add(GTK_CONTAINER(window), mainVBox);

	// Grid to contain the category labels and question buttons
	GtkWidget* questionGrid = gtk_grid_new();
	gtk_grid_set_column_spacing (GTK_GRID(questionGrid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(questionGrid), 5);

	// Are to contain the score counters
	scoreHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

	// Put the question/category area and score area in the correct VBox
	gtk_box_pack_start(GTK_CONTAINER(mainVBox), questionGrid, 1, 1, 0);
	gtk_box_pack_end(GTK_CONTAINER(mainVBox), scoreHBox, 1, 1, 0);

	// Put the score counters in the HBox designated for the score counters
	for(int j = 0; j < numberOfTeams; j++) {
		gtk_box_pack_start(GTK_CONTAINER(scoreHBox), teamScores[j], 1, 1, 0);
	}
	
	// Add the category labels to questionGrid
	GtkWidget *categoryLabel;
	for(int col = 0; col < numberOfCategories; col++) {
		categoryLabel = gtk_label_new(categories[col]);
		gtk_widget_set_hexpand(categoryLabel, TRUE);
		gtk_widget_set_vexpand(categoryLabel, TRUE);
		gtk_label_set_justify(GTK_LABEL(categoryLabel), GTK_JUSTIFY_CENTER);
		context = gtk_widget_get_style_context(categoryLabel);
		gtk_style_context_add_class(context, "category_label");
		gtk_grid_attach(GTK_GRID(questionGrid), categoryLabel, col, 0, 1, 1);
	}
	
	// Add the question buttons to questionGrid
	GtkWidget *questionButton;
	int pointValues[5] = {100, 200, 300, 400, 500};
	for(int row = 0; row < numberOfLevels; row++) {
		for(int col = 0; col < numberOfCategories; col++) {
			
			gchar *pointText;
			pointText = g_strdup_printf("%d", pointValues[row]);
			questionButton = gtk_button_new_with_label(pointText);
			gtk_widget_set_hexpand(questionButton, TRUE);
			gtk_widget_set_vexpand(questionButton, TRUE);

			// Setup data struct to pass to question button callback
			struct _questionButtonClick* questionData = (struct _questionButtonClick*) malloc(sizeof(struct _questionButtonClick));
			questionData->questionText = questions[row][col];
			questionData->answerText = answers[row][col];
			questionData->points = pointValues[row];
			questionData->numberOfTeams = numberOfTeams;
			questionData->window = window;
			questionData->mainVBox = mainVBox;
			questionData->questionGrid = questionGrid;
			questionData->teamScores = teamScores;
			questionData->teamScoreCount = teamScoreCount;

			g_signal_connect(G_OBJECT(questionButton), "clicked",
						G_CALLBACK(questionButtonClick), (gpointer) questionData);

			context = gtk_widget_get_style_context(questionButton);
			gtk_style_context_add_class(context, "question_button");
			gtk_grid_attach(GTK_GRID(questionGrid), questionButton, col, row+1, 1, 1);
		}
	}

}

void questionButtonClick(GtkWidget *button, gpointer user_data) {

	// Unpack passed in data
	struct _questionButtonClick* questionData = (struct _questionButtonClick*) user_data;

	char *questionText = questionData->questionText;
	char *answerText = questionData->answerText;
	int points = questionData->points;
	int numberOfTeams = questionData->numberOfTeams;
	int* teamScoreCount = questionData->teamScoreCount;
	GtkWidget* window = questionData->window;
	GtkWidget* mainVBox = questionData->mainVBox;
	GtkWidget* questionGrid = questionData->questionGrid;
	GtkWidget** teamScores = questionData->teamScores;

	// GtkWidgets for the question screen
	GtkWidget *questionScreenGrid;
	GtkWidget *questionScreenVBox;
	GtkWidget *pointAssignmentHBox;
	GtkWidget *questionTextLabel;
	GtkWidget *showAnswerButton;
	GtkWidget *teamAButton;
	GtkWidget *noneButton;
	GtkWidget *teamBButton;
	GtkWidget *teamButtons[numberOfTeams + 1];
	
	// Set up the GtkWidgets for the question screen
	questionScreenGrid = gtk_grid_new();
	gtk_grid_set_column_spacing (GTK_GRID(questionScreenGrid), 5);
	gtk_grid_set_row_spacing(GTK_GRID(questionScreenGrid), 5);

	questionScreenVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

	questionTextLabel = gtk_label_new(questionText);
	gtk_widget_set_hexpand(questionTextLabel, TRUE);
	gtk_widget_set_vexpand(questionTextLabel, TRUE);
	gtk_label_set_line_wrap(questionTextLabel, TRUE);
	context = gtk_widget_get_style_context(questionTextLabel);

	gtk_style_context_add_class(context, "question_label");

	struct _answerButtonClick* answerData = (struct _answerButtonClick*) malloc(sizeof(struct _answerButtonClick));
	answerData->answerText = answerText;
	answerData->questionScreenGrid = questionScreenGrid;
	answerData->window = window;

	showAnswerButton = gtk_button_new_with_label("SHOW ANSWER");
	gtk_widget_set_hexpand(showAnswerButton, TRUE);
	gtk_widget_set_vexpand(showAnswerButton, TRUE);

	context = gtk_widget_get_style_context(showAnswerButton);

	gtk_style_context_add_class(context, "answer_button");
	g_signal_connect(G_OBJECT(showAnswerButton), "clicked",
				G_CALLBACK(answerButtonClick), (gpointer) answerData);

	pointAssignmentHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_widget_set_hexpand(pointAssignmentHBox, TRUE);
	gtk_widget_set_vexpand(pointAssignmentHBox, TRUE);
	gtk_grid_attach(GTK_GRID(questionScreenGrid), questionTextLabel, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(questionScreenGrid), showAnswerButton, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(questionScreenGrid), pointAssignmentHBox, 0, 2, 1, 1);

	struct _pointAssignmentData* pointAssignmentDataArray[numberOfTeams + 1];

	struct _assignPoints* pointAssignmentDataNone = (struct _assignPoints*) malloc(sizeof(struct _assignPoints));
	pointAssignmentDataNone->questionButton = button;
	pointAssignmentDataNone->points = points;
	pointAssignmentDataNone->winner = numberOfTeams;
	pointAssignmentDataNone->numberOfTeams = numberOfTeams;
	pointAssignmentDataNone->window = window;
	pointAssignmentDataNone->mainVBox = mainVBox;
	pointAssignmentDataNone->questionGrid = questionGrid;
	pointAssignmentDataNone->teamScores = teamScores;
	pointAssignmentDataNone->teamScoreCount = teamScoreCount;

	pointAssignmentDataArray[numberOfTeams] = pointAssignmentDataNone;

	for(int i = 0; i < numberOfTeams; i++) {
		struct _assignPoints* pointAssignmentData = (struct _assignPoints*) malloc(sizeof(struct _assignPoints));
		pointAssignmentData->questionButton = button;
		pointAssignmentData->points = points;
		pointAssignmentData->winner = i;
		pointAssignmentData->numberOfTeams = numberOfTeams;
		pointAssignmentData->window = window;
		pointAssignmentData->mainVBox = mainVBox;
		pointAssignmentData->questionGrid = questionGrid;
		pointAssignmentData->teamScores = teamScores;
		pointAssignmentData->teamScoreCount = teamScoreCount;
		pointAssignmentDataArray[i] = pointAssignmentData;
	}

	noneButton = gtk_button_new_with_label("Nobody");
	context = gtk_widget_get_style_context(noneButton);
	gtk_style_context_add_class(context, "team_button");

	g_signal_connect(G_OBJECT(noneButton), "clicked",
				G_CALLBACK(assignPoints), (gpointer) pointAssignmentDataNone);

	GtkWidget *teamButton;
	gchar *scoreButtonLabel;
	for(int j = 0; j < numberOfTeams + 1; j++) {
		scoreButtonLabel = g_strdup_printf("Team %s", teamNames[j]);

		teamButton = gtk_button_new_with_label(scoreButtonLabel);	
		context = gtk_widget_get_style_context(teamButton);
		gtk_style_context_add_class(context, "team_button");

		g_signal_connect(G_OBJECT(teamButton), "clicked", G_CALLBACK(assignPoints), (gpointer) pointAssignmentDataArray[j]);
		teamButtons[j] = teamButton;	
	}

	for(int k = 0; k < numberOfTeams; k++) {
		gtk_box_pack_start(GTK_CONTAINER(pointAssignmentHBox), teamButtons[k], 1, 1, 0);
	}
		
	gtk_box_pack_start(GTK_CONTAINER(pointAssignmentHBox), noneButton, 1, 1, 0);

	g_object_ref(G_OBJECT(mainVBox));
	gtk_container_remove(GTK_CONTAINER(window), mainVBox);
	gtk_container_add(GTK_CONTAINER(window), questionScreenGrid);
	gtk_widget_show_all(window);

}

void answerButtonClick(GtkWidget *button, gpointer user_data) {
	GtkWidget* answerLabel;
	struct _answerButtonClick* answerData = (struct _answerButtonClick*) user_data;

	GtkWidget *questionScreenGrid = answerData->questionScreenGrid;
	char *answerText = answerData->answerText;
	GtkWidget* window = answerData->window;

	answerLabel = gtk_label_new(answerText);
	gtk_widget_set_hexpand(answerLabel, TRUE);
	gtk_widget_set_vexpand(answerLabel, TRUE);
	gtk_label_set_justify(GTK_LABEL(answerLabel), GTK_JUSTIFY_CENTER);
	context = gtk_widget_get_style_context(answerLabel);
	gtk_style_context_add_class(context, "answer_label");


	gtk_container_remove(GTK_CONTAINER(questionScreenGrid), button);
	gtk_grid_attach(GTK_GRID(questionScreenGrid), answerLabel, 0, 1, 1, 1);
	gtk_widget_show_all(window);
}

void assignPoints(GtkWidget *button, gpointer user_data) {

	struct _assignPoints* pointAssignmentData = (struct _assignPoints*) user_data;

	int numberOfTeams = pointAssignmentData->numberOfTeams;
	int* teamScoreCount = pointAssignmentData->teamScoreCount;
	GtkWidget* questionButton = pointAssignmentData->questionButton;
	GtkWidget* window = pointAssignmentData->window;
	GtkWidget* mainVBox = pointAssignmentData->mainVBox;
	GtkWidget* questionGrid = pointAssignmentData->questionGrid;
	GtkWidget** teamScores = pointAssignmentData->teamScores;

	int winner = pointAssignmentData->winner;
	int points = pointAssignmentData->points;

	GList *children, *iter;

	children = gtk_container_get_children(GTK_CONTAINER(window));
	for(iter = children; iter != NULL; iter = g_list_next(iter)) {
		gtk_container_remove(GTK_CONTAINER(window), GTK_WIDGET(iter->data));
	}
	g_list_free(children);

	gtk_container_add(GTK_CONTAINER(window), mainVBox);

	if(winner != numberOfTeams) {
		teamScoreCount[winner] += points;
		gchar *scoreLabel;
		teamScoreCount[winner];
		scoreLabel = g_strdup_printf("Team %s: %d", teamNames[winner], teamScoreCount[winner]);
		gtk_label_set_text(teamScores[winner], scoreLabel);
		gtk_container_remove(GTK_CONTAINER(questionGrid), questionButton);
	}

	gtk_widget_show_all(window);
}

struct _gameData* loadGameData(char* fname) {

	struct _gameData* gameData = readAndParseGameFile("game.json");

	int numberOfLevels = gameData->levels;
	int numberOfCategories = gameData->categories;

	int levels = numberOfLevels;
	int categories = numberOfCategories;

	return gameData;
}
