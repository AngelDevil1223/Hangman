#define _GNU_SOURCE
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "utilities.h"


#define MAX 256 /* max number of bytes we can get at once */
#define BACKLOG 10
#define MAX_CLIENT 100


#define MAX_HANGMEN_WORDS 288
#define ERROR -1
#define DEFAULT_PORT 12345


//function decleratios

void* SendScore(int socket_id, uint8_t leaderboard_id);
int menu_input(int socket_id, char* nickname);
void Game_play(auth_user *user, uint8_t leaderboard_id);
int guesses_min(int words_length);
int hangman(int socket_id, char* nickname);
void *connection_handler(void *user);
void list_available_room(int socket_id);
void sleep_and_lock(int socket_id, size_t * player_number);




pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct auth_user *auth_user_first = NULL;
struct auth_user *auth_user_last = NULL;

struct Hang *Hang_first = NULL;
struct Hang *Hang_last = NULL;

pthread_mutex_t board_mutex = PTHREAD_MUTEX_INITIALIZER;
struct Leaderboard *Leaderboard_first =  NULL;
struct Leaderboard *Leaderboard_last = NULL;
size_t number_of_rooms = 0;

typedef enum {
    _,
    PLAY_HANGMAN,
    SHOW_LEADERBOARD,
    QUIT
} Menu;

char **users;
char **passwords;

char **word1;

int won = 0;
int played = 0;
int local = 1;
//listen socket
int sockfd;

//running the server side game
int hangman(int socket_id, char* username){


    uint16_t converter;
    //gets random word from linked list
    char *results = printRandom(Hang_first);

    char str[200];
    char* word = results;
    char *ptr;

    //splits the word where the , is
    strcpy (str, word);
    strtok_r (str, ",", &ptr);

    //the two words will try and make it better later
    char* word1 = str;
    char* word2 = ptr;

    //prints it out for debugging
    printf("Word1 is %s and word2 is %s \n", word1, word2);
    printf("Word1 length is %zu and word2 length is %zu \n", strlen(word1), strlen(word2));

    //so many error
    char* client_guessed_letters = calloc(MAX, sizeof(char));

    //number of guesses left
    int num_guesses = guesses_min(strlen(word1) + strlen(word2) + 10);

    char cword1[MAX];
    char cword2[MAX];

    for(int i = 0; i < strlen(word1); i++){
        cword1[i] = 95;
    }
    cword1[strlen(word1)] = '\0';

    for(int i = 0; i < strlen(word2); i++){
        cword2[i] = 95;
    }
    cword2[strlen(word2)] = '\0';

    //initialize client guessed single letter
    char recv_letter;

    int GameOver = 0;
    int winner = 0;


    while(1){

        //gameover man
        if(num_guesses <= 0){
            GameOver = 1;
        }

        //checking if words are the same will fix later bad bad code
        if(strcmp(word1, cword1) == 0 && strcmp(word2, cword2) == 0){
            winner = 1;
        }

        //Game to server
        converter = htons(GameOver);
        if(send(socket_id, &converter, sizeof(uint16_t), 0) <= 0){
            return ERROR;
        }
        //winner to server
        converter = htons(winner);
        if(send(socket_id, &converter, sizeof(uint16_t), 0) <= 0){
            return ERROR;
        }

        //send guessed letterd
        if(send(socket_id, client_guessed_letters, MAX, 0) <= 0){
            return ERROR;
        }

        //how many
        converter = htons(num_guesses);
        if(send(socket_id, &converter, sizeof(uint16_t), 0) <= 0){
            return ERROR;
        }

        //send words need to get rid of double loop
        if(send(socket_id, cword1, MAX, 0) <= 0){
            return ERROR;
        }
        if(send(socket_id, cword2, MAX, 0) <= 0){
            return ERROR;
        }

        //winning adds score to scoreboard and breaks
        if(winner){
            printf("\nPlayer %s win the game!\n", username);
            return 1;
        }

        //game over add score to board and break
        if(GameOver){
            printf("\nPlayer %s lose the game!\n", username);
            // flag for loosing
            return -2;
        }

        //looping thru
        if(recv(socket_id, &recv_letter, sizeof(char), 0) <= 0){
            return ERROR;
        }


        num_guesses--;

        //add in
        if(strchr(client_guessed_letters, recv_letter) == NULL){
            strcat(client_guessed_letters, &recv_letter);
        }

        //put in guesses need to get rid of this double loop
        for(int i = 0; i < strlen(word1); i++){
            if (word1[i] == recv_letter){
                cword1[i] = word1[i];
            }
        }

        for(int i = 0; i < strlen(word2); i++){
            if(word2[i] == recv_letter){
                cword2[i] = word2[i];
            }
        }

    }
    return 0;
}


void Game_play(auth_user *user, uint8_t leaderboard_id) {
    int input;
    do { //the game menu - after the login screen
        switch ((input = menu_input(user->sockkfd
                        ,user->nickname))) {
            case PLAY_HANGMAN:
                user->played += 1;
                //plays the game
                int flag = hangman(user->sockkfd
                        ,user->nickname);
                if(flag == 1){
                    user->won += 1;
                    user->score += 1;
                } else if (flag == -2){
                    user->score += -2;
                }
                if (user->played == 1){
                    user->first_game_score = flag;
                }
                //for displaying nothing on client side
                local = 0;
                break;
            case SHOW_LEADERBOARD:
                //sends score
                SendScore(user->sockkfd, leaderboard_id);
                //prints linked list
                break;
        }
    } while (input != QUIT); //quit if 3 is pressed
}

//gets client input
int menu_input(int socket_id, char* nickname) {
    uint16_t converter;
    int input = 0;
    if (recv(socket_id, &converter, sizeof(uint16_t), 0) <= 0){
        return -1;
    }
    input = ntohs(converter);
    //debugging
    printf("User menu option : %d \n", input);

    return input;
}

int compare_user(const void *a, const void *b){
    return (((auth_user*)a)->score - ((auth_user*) b)->score);
}

void* SendScore(int socket_id, uint8_t id){
    Leaderboard *ptr = search_room(id);
    // here we are copying all the element in the leaderboard
    // into an array and then using qsort on it 
    // and then sending the players in decending order
    int number_of_players = 0;
    int i = 0;
    auth_user *user = ptr->first;
    while(user != NULL){
        ++number_of_players;
        user = user->next;
    }
    user = ptr->first;
    auth_user arr[number_of_players];
    while(user != NULL){
        asprintf(&arr[i].nickname, "%s",user->nickname);
        arr[i].won = user->won;
        arr[i].played = user->played;
        arr[i].score = user->score;
        arr[i].first_game_score = user->first_game_score;
        user = user->next;
        ++i;
    }
    --i;
    qsort(arr, number_of_players, sizeof(auth_user),compare_user);
    char *buf = NULL;
    // for storing the size of the string
    size_t offset = 0;
    user = ptr->first;
    printf("printing the sorted leaderboard\n");
    for(; i >= 0; i--){
        char *newbuf;
        if(arr[i].played >= 20){
            arr[i].score -= arr[i].first_game_score;
        }
        asprintf(&newbuf, "Player  - %s\nNumber of games won  - %lu\nNumber of games played  - %lu\nScore - %d\n", 
                arr[i].nickname,
                arr[i].won,
                arr[i].played,
                arr[i].score);
        printf("sending this : %s",newbuf);
        // since we are adding \t and \n
        offset += strlen(newbuf);
        // apending newbuf to the old buf
        if(buf == NULL)
            buf = newbuf;
        else
            strcat(buf, newbuf);
    }
    // sending the lenght of the string here
    int16_t stuff = htons(offset);
    if(send(socket_id, &stuff, sizeof(int16_t), 0) <= 0){
        perror("error in sending numbers of room");
    }
    send(socket_id, buf, offset, 0);
    return NULL;
}

int guesses_min(int words_length){
    if (words_length <= 26){
        return words_length;
    } else {
        return 26;
    }
}

void list_available_room(int socket_id){
    Leaderboard *ptr = Leaderboard_first;
    // sending the total number of rooms to the client
    int16_t stuff = htons(number_of_rooms);
    if(send(socket_id, &stuff, sizeof(int16_t), 0) <= 0){
        perror("error in sending numbers of room");
    }
    printf("total number of rooms  : %lun", number_of_rooms);
    // here we are iterating over all the available room
    // and sending the client everything
    // since there can be many rooms and players
    // that's why we are doing this and not using a fixed buffer
    while(ptr != NULL){
        // buffer to store all the player
        char *buf = NULL;
        // for storing the size of the string
        size_t offset = 0;
        auth_user *user = ptr->first;
        while(user != NULL){
            char *newbuf;
            asprintf(&newbuf, "%s \n", user->nickname);
            printf("sending this : %s",newbuf);
            // since we are adding \t and \n
            offset += strlen(newbuf);
            // apending newbuf to the old buf
            if(buf == NULL)
                buf = newbuf;
            else
                strcat(buf, newbuf);
            user = user->next;
        }
        // sending the lenght of the string here
        int16_t stuff = htons(offset);
        if(send(socket_id, &stuff, sizeof(int16_t), 0) <= 0){
            perror("error in sending numbers of room");
        }
        send(socket_id, buf, offset, 0);
        ptr = ptr->next;
    }
}


void *connection_handler(void *arg){
    auth_user *user = (auth_user*)arg;
    uint8_t room_id;
    //get client input
    recv(user->sockkfd, user->nickname, 20,0);
    printf("username is %s\n", user->nickname);
    list_available_room(user->sockkfd);
    uint16_t converter;
    if(recv(user->sockkfd, &converter, sizeof(uint16_t), 0) <= 0)
        perror("error in recieving room code");
    room_id = (uint8_t)htons(converter);
    Leaderboard * room = search_room(room_id);
    if(room == NULL){
        room_id = create_new_room();
        room_id = 0;
        number_of_rooms++;
        room = search_room(room_id);
        room->players = 0;
    }
    // checks if found out or not or the nickname is taken
    struct auth_user *ind = searchn(room->first, user->nickname);
    // login stuff
    if (ind !=  NULL){
        if(!ind->logged_on){
            // nickname is present but the user is not present
            // so we can login 
            ind->logged_on = true;
            if(send(user->sockkfd, "Auth", 20, 0) == -1) perror("send");
            ind->sockkfd = user->sockkfd;
            ind->address = user->address;
            room->players += 1;
            send(user->sockkfd, &room->players, sizeof(size_t), 0);
            sleep_and_lock(user->sockkfd, &room->players);
            while(true){
                // game time
                Game_play(ind, room_id);
            }
        }
        else {
            // this means that the nickname is taken
            // this is not getting executed
            if(send(user->sockkfd, "UnAuth", 20, 0) == -1) perror("send");
            // recursing until new user is given
            return connection_handler((void*) user);
        }
    }
    else {
        // the user doesn't exist 
        printf("the user with %s doesn't exist\n", user->nickname);
        if(send(user->sockkfd, "Auth", 20, 0) == -1) perror("send");
        // adding the username to database
        user->logged_on = true;
        user->score = 0;
        user->played = 0;

        add_user(room->first, room->last, user);
        if(room->first == NULL){
            room->first = user;
            room->last = user;
            user->next = NULL;
            printf("\n room->fist is  null\n");
        }else{
            room->last->next = user;
            room->last = user;
        }
        printf("\n Total Number of People in Room is %lu\n", room->players);
        room->players += 1;
        printf("total number of players in the room is %lu\n", room->players);
        send(user->sockkfd, &room->players, sizeof(size_t), 0);
        sleep_and_lock(user->sockkfd, &room->players);
        while(true){
            Game_play(user, room_id);
        }
        room->players -= 1;
    }
    close(user->sockkfd);
    pthread_detach(pthread_self());
    return NULL;
}

void sleep_and_lock(int socket_id, size_t * player_number){
    // if there is only 1 player sleep and lock the thread
    // the thread will wake every 1 second and send the number 
    // of player to the client if more than 1 the loop will break
    while(*player_number == 1){
        sleep(1);
        if(send(socket_id, player_number, sizeof(size_t),0) == -1)
            perror("Error in sending message to client");
    }
}

int main(int argc, char *argv[]) {


    int new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;

    /* Get port number for server to listen on */
    if (argc > 2) {
        fprintf(stderr,"usage: client port_number\n");
        exit(1);
    }

    //default port
    int port_num;
    if (argc == 1){
        port_num = DEFAULT_PORT;
    } else {
        port_num = atoi(argv[1]);
    }


    /* generate the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }


    int reuseaddr=1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr))==-1) {
        perror("setsockopt");
    }

    /* generate the end point */
    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(port_num);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    /* bzero(&(my_addr.sin_zero), 8);   ZJL*/     /* zero the rest of the struct */

    /* bind the socket to the end point */
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) \
            == -1) {
        perror("bind");
        exit(1);
    }

    /* start listnening */
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    //read all the hangman words in
    read_file_word();

    printf("Server starts listnening ...\n");

    /* repeat: accept, send, close the connection */
    /* for every accepted connection, use a sepetate process or thread to serve it */
    while(1) {  /* main accept() loop */
        pthread_t tid;
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                        &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        auth_user *user = malloc(sizeof(auth_user));
        user->nickname = malloc(20 * sizeof(char));
        user->address = their_addr;
        user->sockkfd = new_fd;
        user->next = NULL;
        user->won = 0;
        user->played = 0;
        user->score = 0;
        user->first_game_score = 0;
        pthread_create(&tid, NULL, &connection_handler, (void*) user);
    }
    return EXIT_SUCCESS;
}
