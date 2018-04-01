#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HUMAN 1
#define COMPUTER 2
#define VALID_LINE_SIZE 70
#define ALL 1
#define PRELIMINARY 2


/**
 * A struct representing the state of the board and its properties:
 *   - the height of the board
 *   - the width of the board
 *   - a 2D character array representing the state of the board
 */
struct GameProperties {
    int height;
    int width;
    char** gameGrid;
};


/** 
 * A struct for storing the variables needed to generate computer player
 *  moves:
 *   - I_r, the initial row
 *   - I_c, the initial column
 *   - F, a multiplication factor
 *   - M, a counter of moves generated
 *   - r = I_r
 *   - c = I_c
 *   - B = I_r * G_w + I_c
 *   - the row of the next move generated
 *   - the column of the next move generated
 */
struct MoveAlgorithm {
    int ir;
    int ic;
    int f;
    int m;
    int r;
    int c;
    int b;
    int nextX;
    int nextY;
};


/**
 * A struct representing a player and their properties:
 *   - the type of player, either human or computer
 *   - the players' token, either X or O
 *   - what move the player is up to
 *   - a struct collecting the variables needed to generate computer player
 *   moves
 */
struct Player {
    int type;
    char token;
    int move;
    struct MoveAlgorithm* variables;
};


/**
 * Frees the memory allocated previously with malloc.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - allocated, a representation of whether only the PRELIMINARY series
 *    of malloc reservations have occurred or whether ALL malloc reservations
 *    have occurred
 */
void free_allocated_memory(struct GameProperties* game,
        struct Player** players, int allocated) {

    if (allocated == ALL) {
        for (int i = 0; i < 2; i++) {
            free(players[i]->variables);
        }
        for (int i = 0; i < 2; i++) {
            free(players[i]);
        }
        free(players);
        for (int i = 0; i < game->height; ++i) {
            free(game->gameGrid[i]);
        }
        free(game->gameGrid);
        free(game);

    } else if (allocated == PRELIMINARY) {
        for (int i = 0; i < 2; i++) {
            free(players[i]);
        }
        free(players);
        free(game);
    }
}


/**
 * Exits the game, with the specified exit status and a message.
 *   - exitStatus, the exit status to exit with
 */
void exit_program(int exitStatus) {
    switch(exitStatus) {
        case 1:
            fprintf(stderr, "Usage: nogo p1type p2type "
                    "[height width | filename]\n");
            exit(1);
            break;
        case 2:
            fprintf(stderr, "Invalid player type\n");
            exit(2);
            break;
        case 3:
            fprintf(stderr, "Invalid board dimension\n");
            exit(3);
            break;
        case 4:
            fprintf(stderr, "Unable to open file\n");
            exit(4);
            break;
        case 5:
            fprintf(stderr, "Incorrect file contents\n");
            exit(5);
            break;
        case 6:
            fprintf(stderr, "End of input from user\n");
            exit(6);
            break;
    }
}


/**
 * Prints a horizontal border at the top and bottom of the game board.
 *   - game, a struct of the game state 
 */
void display_horizontal_border(struct GameProperties* game) {   
    for (int j = 0; j < game->width; j++) {
        putchar('-');
    }      
}


/**
 * Prints the game board, including borders.
 *   - game, a struct of the game state 
 */
void display_grid(struct GameProperties* game) {   
    putchar('/');
    display_horizontal_border(game);
    printf("\\\n");
    
    /* Print game board tokens with side borders */
    for (int i = 0; i < game->height; ++i) {   
        printf("%c", '|');    
        for (int j = 0; j < game->width; j++) {
            printf("%c", game->gameGrid[i][j]);        
        }      
        printf("%c\n", '|');    
    }
    putchar('\\');
    display_horizontal_border(game);
    printf("/\n");
}


/**
 * When given a point, this function will determine whether the string it
 * belongs to has any '.' adjacent to it.
 *  - game, a struct of the game state
 *  - anyLiberties, a boolean where the result of this function is stored
 *  - consideredPoints, the points in the string that have been checked
 *  so far for adjacent '.'
 *  - row, the row of the original point of interest
 *  - col, the column of the original point of interest
 *  (Note: uses the token of (row, col) to determine the string's token)
 */
void adjacent_space_check(struct GameProperties* game, bool* anyLiberties, 
        char** consideredPoints, int row, int col) {

    if (*anyLiberties == true) {
        return;
    }
    /* Add point to array of considered ('Y') points */
    consideredPoints[row][col] = 'Y'; 
    
    /* Check if an adjacent point is free */
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (*anyLiberties == true) {
                return;
            }
            /* This combination corresponds to adjacent points*/            
            if (((i - j) == 1) || ((j - i) == 1)) {  
                if ((i + row) < 0 || (j + col) < 0) {
                    continue;
                } else if ((i + row) > (game->height - 1) 
                        || (j + col) > (game->width - 1)) {
                    continue;
                } else if (game->gameGrid[i + row][j + col] == '.') {
                    *anyLiberties = true;
                    return;

                /* Repeat search for any adjacent members of the string */
                } else if (game->gameGrid[i + row][j + col] == 
                        game->gameGrid[row][col]) { 
                    if (consideredPoints[i + row][j + col] == 'N') { 
                        //'N' for no, point has not been checked
                        adjacent_space_check(game, anyLiberties, 
                                consideredPoints, (i + row), (j + col));
                    }
                }
            }
        }  
    }
}


/**
 * Checks the string that the point (row, col) belongs to for any liberties.
 * Returns true if at least one liberty exists, otherwise false.
 *  - game, a struct of the game state
 *  - row, the row of the point
 *  - col, the column of the point
 */
bool any_liberties(struct GameProperties* game, int row, int col) {
    bool anyLiberties = false;
    char** consideredPoints = malloc(sizeof(char*) * game->height);
    for (int i = 0; i < game->height; ++i) {
        consideredPoints[i] = malloc(sizeof(char) * game->width);
    }

    for (int i = 0; i < game->height; ++i) {
        for (int j = 0; j < game->width; j++) {
            consideredPoints[i][j] = 'N';
            // initialising each point as not ('N') having been considered
        }
    }
    adjacent_space_check(game, &anyLiberties, consideredPoints, row, col);

    for (int i = 0; i < game->height; ++i) {
        free(consideredPoints[i]);
    }
    free(consideredPoints);

    return anyLiberties; 
} 


/**
 * Checks whether the inactive player has just lost.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - inactive, the inactive player: 0 if it is player O or 1 if it is 
 *   player X
 */
void check_game_over(struct GameProperties* game, struct Player** players, 
        int inactive) {
    for (int i = 0; i < game->height; ++i) {
        for (int j = 0; j < game->width; j++) {
            if (game->gameGrid[i][j] == players[inactive]->token) {
                if (any_liberties(game, i, j) == false) {
                    printf("Player %c wins\n", players[1 - inactive]->token);
                    exit(0);
                }
            }
        }
    }
}


/**
 * Determines whether a move is allowed. 
 * Returns true if move is allowed, otherwise false. 
 *  - game, a struct of the game state
 *  - row, the row of the point
 *  - col, the column of the point
 */
bool valid_move(struct GameProperties* game, int row, int col) {
    if ((row > -1) && (col > -1) && (row < game->height) 
            && (col < game->width)) {
        return (game->gameGrid[row][col] == '.');
    }
    return false;
}


/** 
 * Saves the game state.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - filepath, the filepath of the save file
 *   - active, the active player: 0 if it is player O or 1 if it is 
 *   player X
 */
void save(struct GameProperties* game, struct Player** players, 
        char* filepath, int active) {

    /* Strips 'w' from save file path string */
    char filepathCorrected[VALID_LINE_SIZE];
    int count = 0;
    while(true) {
        if ((filepath[count + 1] == '\0') 
                || (filepath[count + 1] == '\n')) {
            break;
        } else if (count == VALID_LINE_SIZE - 1) {
            break;
        }
        filepathCorrected[count] = filepath[count + 1];
        count++;
    }
    filepathCorrected[count] = '\0';
    
    FILE* file = fopen(filepathCorrected, "w");
    if (file == NULL) {
        fprintf(stderr, "Unable to save game\n");
    }

    fprintf(file, "%d %d %d %d %d %d %d %d %d\n", game->height, game->width, 
            active, players[0]->variables->nextX, 
            players[0]->variables->nextY, players[0]->variables->m, 
            players[1]->variables->nextX, players[1]->variables->nextY, 
            players[1]->variables->m);

    for (int i = 0; i < game->height; ++i) {   
        for (int j = 0; j < game->width; j++) {
            fprintf(file, "%c", game->gameGrid[i][j]);       
        }      
        fprintf(file, "\n");
    }
    fflush(file);       
    fclose(file);
}


/**
 * Increments the active computer players' next move to be performed.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - active, the active player: 0 if it is player O or 1 if it is 
 *   player X
 */
void increment_next_move(struct GameProperties* game, 
        struct Player** players, int active) {

    ++players[active]->variables->m;

    int n = (players[active]->variables->b + players[active]->variables->m 
            / 5 * players[active]->variables->f) % 1000003;

    switch(players[active]->variables->m % 5) {
        case 0:
            players[active]->variables->r = n / game->width;
            players[active]->variables->c = n % game->width;
            break;
        case 1:
            players[active]->variables->r += 1;
            players[active]->variables->c += 1;
            break;
        case 2:
            players[active]->variables->r += 2;
            players[active]->variables->c += 1;
            break;
        case 3:
            players[active]->variables->r += 1;
            players[active]->variables->c += 0;
            break;
        case 4:
            players[active]->variables->r += 0;
            players[active]->variables->c += 1;
            break;
    }
    players[active]->variables->nextX = 
            players[active]->variables->r % game->height;
    players[active]->variables->nextY = 
            players[active]->variables->c % game->width;
}


/**
 * Prompts player for move until a valid move is supplied, and then makes the
 *  move.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - active, the active player: 0 if it is player O or 1 if it is 
 *   player X
 *   - x, the row of the of proposed move
 *   - y, the column of the proposed move
 */
void get_player_move(struct GameProperties* game, struct Player** players, 
        int active, int* x, int* y) {

    while(true) {
        printf("Player %c> ", players[active]->token);
        
        char inputString[VALID_LINE_SIZE + 2]; 
        //allows for '/n' and null terminator characters 
        
        if ((fgets(inputString, VALID_LINE_SIZE + 2, stdin) == NULL)) { 
            free_allocated_memory(game, players, ALL);
            exit_program(6); 
        }
        /* Empties stdin */
        if (inputString[strlen(inputString) - 1] != '\n') {
            int remainingInput;
            while(((remainingInput = fgetc(stdin)) != EOF) 
                    && (remainingInput != '\n')) {
            }
            continue; 
        }
        if (inputString[0] == 'w') {
            save(game, players, inputString, active);
            continue;

        } else if (sscanf(inputString, "%d %d", x, y) == 2) {
            if (valid_move(game, *x, *y) == true) {
                break;
            }
        }
    }
}


/**
 *  Runs the game until a player has lost.
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 */
void run_game(struct GameProperties* game, struct Player** players) {
    int active = players[0]->move;

    while (true) {
        display_grid(game);
        int x, y;
        check_game_over(game, players, 1 - active); 
        check_game_over(game, players, active);
        
        if (players[1]->move < players[0]->move) {
            active = 1;
        } else {
            active = 0;
        }   

        /* Calculating the x (row) and y (col) of the valid move to make */
        if (players[active]->type == COMPUTER) {
            while (game->gameGrid[players[active]->variables->nextX]
                    [players[active]->variables->nextY] != '.') {
                increment_next_move(game, players, active);    
            }
            x = players[active]->variables->nextX;
            y = players[active]->variables->nextY;
            increment_next_move(game, players, active);    
            printf("Player %c: %d %d\n", players[active]->token, x, y);
        } else {
            get_player_move(game, players, active, &x, &y);
        }
        players[active]->move++; 
        game->gameGrid[x][y] = players[active]->token;
    }
}


/**
 * Initialises the player properties and values used to generate the computer 
 * player moves
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - argv, the commandline arguments used to launch the game
 */
void initialise_player(struct GameProperties* game, struct Player** players, 
        char** argv) {

    /*Initialise player O variables*/
    players[0]->token = 'O';
    if (argv[1][0] == 'h') {          
        players[0]->type = HUMAN;
    } else if (argv[1][0] == 'c') {
        players[0]->type = COMPUTER;
    }
    players[0]->move = 0;
    
    players[0]->variables = malloc(sizeof(struct MoveAlgorithm));
    players[0]->variables->ir = 1;        
    players[0]->variables->ic = 4;        
    players[0]->variables->f = 29;        
    players[0]->variables->m = 0;        
    players[0]->variables->r = 1;        
    players[0]->variables->c = 4;        
    players[0]->variables->b = players[0]->variables->ir * game->width 
            + players[0]->variables->ic;        
    players[0]->variables->nextX = 1 % game->height;        
    players[0]->variables->nextY = 4 % game->width;        

    /*Initialise player X variables*/
    players[1]->token = 'X';   

    if (argv[2][0] == 'h') {          
        players[1]->type = HUMAN;
    } else if (argv[2][0] == 'c') {
        players[1]->type = COMPUTER;
    }
    players[1]->move = 0;

    players[1]->variables = malloc(sizeof(struct MoveAlgorithm));
    players[1]->variables->ir = 2;        
    players[1]->variables->ic = 10;        
    players[1]->variables->f = 17;        
    players[1]->variables->m = 0;        
    players[1]->variables->r = 2;        
    players[1]->variables->c = 10;        
    players[1]->variables->b = players[1]->variables->ir * game->width 
            + players[1]->variables->ic;        
    players[1]->variables->nextX = 2 % game->height;        
    players[1]->variables->nextY = 10 % game->width;       
}


/**
 * Exits the program if the load file is not of the correct format.
 *   - loadFile, the file being loaded 
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 */
void validate_load_file(FILE* loadFile, struct GameProperties* game, 
        struct Player** players) {

    if (loadFile == NULL) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(4);
    }    
    /* Verify first line has correct format */
    int arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9;
    if (9 != fscanf(loadFile, "%d %d %d %d %d %d %d %d %d", &arg1, &arg2, 
            &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9)) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(5);
    }
    if ((char)fgetc(loadFile) != '\n') {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(5);
    }
    /* Verify arguments are valid values */
    if (arg1 < 4 || arg1 > 1000) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(5);
    }
    if (arg2 < 4 || arg2 > 1000) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(5);
    }
    if (arg3 != 0 && arg3 != 1) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(5);
    } 
    /* Verify game grid is valid and consistent */
    for (int i = 0; i < arg1; ++i) {
        for (int j = 0; j < arg2; j++) {
            char token = (char)fgetc(loadFile);
            if (token != '.' && token != 'X' && token != 'O') {
                free_allocated_memory(game, players, PRELIMINARY);
                exit_program(5);
            }
        }
        char token = (char)fgetc(loadFile);
        if (token != '\n') {
            free_allocated_memory(game, players, PRELIMINARY);
            exit_program(5);
        }
    }
    rewind(loadFile);
}


/**
 * Loads saved data and continues the saved game.
 *   - loadFile, the file being loaded 
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 *   - argv, the commandline arguments used to launch the game
 */
void load_saved_data(FILE* loadFile, struct GameProperties* game, 
        struct Player** players, char** argv) {

    int height, width, nextPlayer, nextRowO, nextColO, movesO, nextRowX, 
            nextColX, movesX;

    fscanf(loadFile, "%d %d %d %d %d %d %d %d %d", &height, &width, 
            &nextPlayer, &nextRowO, &nextColO, &movesO, &nextRowX, 
            &nextColX, &movesX); 

    game->height = height;      
    game->width = width;
    
    /* Load game grid */
    game->gameGrid = malloc(sizeof(char*) * game->height);
    for (int i = 0; i < game->height; ++i) {
        game->gameGrid[i] = malloc(sizeof(char) * game->width);
    }
    for (int i = 0; i < game->height; ++i) {
        for (int j = 0; j < game->width; j++) {
            while (true) {
                char token = (char)fgetc(loadFile);
                if (token == '.' || token == 'X' || token == 'O') {
                    game->gameGrid[i][j] = token;
                    break;
                }
            }
        }
    }

    initialise_player(game, players, argv);
    players[0]->move = nextPlayer;
    
    players[0]->variables->nextX = nextRowO;       
    players[0]->variables->r = nextRowO;
    players[0]->variables->nextY = nextColO;
    players[0]->variables->c = nextColO;
    players[0]->variables->m = movesO;
    
    players[1]->variables->nextX = nextRowX;
    players[1]->variables->r = nextRowX;
    players[1]->variables->nextY = nextColX;
    players[1]->variables->c = nextColX;
    players[1]->variables->m = movesX;
}    


/**
 * Verifies the commandline arguments are valid.
 *   - argc, the number of commandline arguments
 *   - argv, the commandline arguments used to launch the game
 *   - game, a struct of the game state
 *   - players, an array of players' properties
 */
void validate_arguments(int argc, char** argv, struct GameProperties* game, 
        struct Player** players) {

    if ((argc > 5) || (argc < 4)) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(1);
    } else if (((strcmp(argv[1], "h")) != 0) 
            && ((strcmp(argv[1], "c")) != 0)) {
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(2);
    } else if (((strcmp(argv[2], "h")) != 0) 
            && ((strcmp(argv[2], "c")) != 0)) { 
        free_allocated_memory(game, players, PRELIMINARY);
        exit_program(2);
    }

    if (argc == 5) {
        int height = atoi(argv[3]);
        int width = atoi(argv[4]);
        
        if ((height < 4) || (height > 1000)) {
            free_allocated_memory(game, players, PRELIMINARY);
            exit_program(3);
        } else if ((width < 4) || (width > 1000)) {
            free_allocated_memory(game, players, PRELIMINARY);
            exit_program(3);
        }
    }
}


/**
 * Initialises the game grid tokens to '.'
 *   - game, a struct of the game state
 *   - argv, the commandline arguments used to launch the game
 */
void initialise_grid(struct GameProperties* game, char** argv) {
    game->height = atoi(argv[3]);      
    game->width = atoi(argv[4]);

    game->gameGrid = malloc(sizeof(char*) * game->height);
    for (int i = 0; i < game->height; ++i) {
        game->gameGrid[i] = malloc(sizeof(char) * game->width);
    }
    for (int i = 0; i < game->height; ++i) {
        for (int j = 0; j < game->width; j++) {
            game->gameGrid[i][j] = '.';
        }
    }
}


int main(int argc, char** argv) {
    struct GameProperties* game = malloc(sizeof(struct GameProperties));
	
    struct Player** players = malloc(sizeof(struct Player*) * 2);    
    for (int i = 0; i < 2; ++i) { 
        players[i] = malloc(sizeof(struct Player)); 
    }
    
    validate_arguments(argc, argv, game, players);
    if (argc == 5) {
        initialise_grid(game, argv);
        initialise_player(game, players, argv);
    } else if (argc == 4) {
        FILE* loadFile = fopen(argv[3], "r");
        validate_load_file(loadFile, game, players);
        load_saved_data(loadFile, game, players, argv);
        fclose(loadFile);
    }

    run_game(game, players);
    return 0;
}
