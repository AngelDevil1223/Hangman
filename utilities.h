
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

//struct
struct Hang {
    char *word1;
    struct Hang * next;
};

struct auth_user {
    char * username;
    char * password;
    struct auth_user * next;
};

struct Leaderboard {
    char * username;
    int won;
    int played;
    struct Leaderboard * next;
};


//read the auth file
void read_file();

//adds user to list
void add_auth_user (char * username, char * password);

//search for user and checks password return 0 or 1
int8_t searchn(struct auth_user *ptr, char* username, char* password);

//read hangman file
void read_file_word(void);
//add words to list
void add_words(char * word1);

//print a random word from list
char *printRandom(struct Hang *head);

//add game played and username to list
void add_board (char * username,int won, int played );

//prints user debugging
void print_list(void);

//prints words debugging
void print_list_word(void);


void print_list_board(void);
