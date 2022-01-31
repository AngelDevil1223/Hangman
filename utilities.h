
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
#include<pthread.h>
#include <stdbool.h>

//struct
struct Hang {
    char *word1;
    struct Hang * next;
};

typedef struct auth_user auth_user;

struct auth_user {
    char * nickname;
    struct sockaddr_in address;
    int sockkfd;
    bool logged_on;
    size_t won;
    size_t played;
    size_t first_game_score;
    int score;
    struct auth_user * next;
};

typedef struct Leaderboard Leaderboard;
struct Leaderboard {
    uint8_t id;
    auth_user * first;
    auth_user * last;
    size_t players;
    struct Leaderboard * next;
};


//read the auth file
void read_file();

//adds user to list with nickname
void add_auth_user (auth_user *first, auth_user * last,char * nickname) ;

// add new user in the list with auth_user object
void add_user( auth_user *first,auth_user *last,auth_user *user);

// remove user from a auth_user list
void remove_user(auth_user *ptr,char *nickname);

//search for nickname if found send the ptr of the auth_user back
//else sends NULL pointer
struct auth_user* searchn(struct auth_user *ptr, char* nickname) ;

// create new room or leaderboard and returns its id
Leaderboard* create_new_room(uint8_t id);

Leaderboard* search_room(uint8_t id);

// adds the user to the leaderboard with the id
void add_to_room(uint8_t id, auth_user * user);


//read hangman file
void read_file_word(void);
//add words to list
void add_words(char * word1);

//print a random word from list
char *printRandom(struct Hang *head);

//add game played and nickname to list
void add_board (char * nickname,int won, int played );
