#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

//enum used by game_play to pick a mode
typedef enum {
    _,
    PLAY_HANGMAN,
    SHOW_LEADERBOARD,
    QUIT
} Menu;

#define ARRAY_SIZE 30

#define MAX 256

#define ERROR -1

// function declerations
void Welcome();

void Game_menu();
void recieve_available_room(int socket_id);

void Board(int sock, char* username);

int menu_input(int sock, char* username);



//global socket id
int sockfd;
//checkking if game has been played
int local = 1;

//The client side of the hangman game
int game(int sock, char* username){

	char letters[MAX];
	int gletters;
	uint16_t converter;
	char word1[MAX];
	char word2[MAX];
	int GameOver = 0;
	int winner = 0;
    char category[MAX];
    
    int nsend;
    if ((nsend= recv(sock, category, MAX, 0)) <= 0  )
        return ERROR;
    category[nsend] = '\0';
    printf("\nThe category is : %s\n", category);

	while(1){

		//checks if game is over and breaks
		if(recv(sock, &converter, sizeof(uint16_t), 0) <= 0){
			return ERROR;
		}
		GameOver = ntohs(converter);

		//check if game has won and breaks
		if(recv(sock, &converter, sizeof(uint16_t), 0) <= 0){
			return ERROR;
		}
		winner = ntohs(converter);

		printf("\n=======================================================\n");

		//gets a letter from server
		if(recv(sock, letters, MAX, 0) <= 0){
			return ERROR;
		}
		printf("\nGuessed letters: %s", letters);

		//gets number of letters
		if(recv(sock, &converter, sizeof(uint16_t), 0) <= 0){
			return ERROR;
		}
		gletters = ntohs(converter);
		printf("\n\nNumber of guesses left: %d\n", gletters);

		//word1
		if(recv(sock, word1, MAX, 0) <= 0){
			return ERROR;
		}

    /* could not firgue out how to get the space between the words
        so just run the loop twice, will fix later
    */

		printf("\nWord: ");
		for(int i = 0 ; i < strlen(word1); i++){
			printf("%c ", word1[i]);
		}

		printf("\n\n");

		//breaks the loop if won
		if(winner){
			printf("Game over\n");
			printf("\n\nWell done %s! You won this round of Hangman!\n\n", username);
			break;
		}

		//break the loop if game over
		if(GameOver){
			printf("Game Over\n");
			printf("\n\nBad luck %s! You have run out of guesses. The hangman got you!\n\n", username);
			break;
		}



		//keep playing the game
		char send_letter;
		printf("Enter your guess - ");
		scanf("%s", &send_letter);

		//send guessed letter to server
		if (send(sock, &send_letter, sizeof(char), 0) <= 0){
			return ERROR;
		}
	}

	return 0;

}

//Game menu client side
void Game_play(int sock, char* username) {

    int input;
    do { //the game menu - after the login screen
				Game_menu();
        switch ((input = menu_input(sock,username))) {
            case PLAY_HANGMAN:
                game(sockfd,username);
                local = 0;
                break;
            case SHOW_LEADERBOARD:
                // print_list_board();
                Board(sockfd,username);
                break;
        }
    } while (input != QUIT); //quit if 3 is pressed
}

//Prints the game menu to screen
void Game_menu() {
    puts("\n\n"
                 "Please enter a selection\n"
                 "<1> Play Hangman\n"
                 "<2> Show Leaderboard\n"
                 "<3> Quit\n\n");
}

//get input frmm user and sents it too server
int menu_input(int sock, char* username) {
		uint16_t converter ;
		int input = 0;
    char in[1];
    do {
        printf("Select option 1-3 ->");
        scanf("%s", in);
        input = atoi(in);
    } while (input < 1 || input > 3);
		converter  = htons(input);
		if (send(sock, &converter , sizeof(uint16_t), 0) <= 0){
            // error in handling input so quit
			return QUIT;
		}
		input = ntohs(converter );;
    return input;
}

//Welcome message
void Welcome(){
	printf("============================================\n\n\n");
	printf("Welcome to the Online Hangman Gaming System\n\n");
	printf("============================================\n\n");
	printf("You are required to login with your registered username and Password \n\n");
}

//Scoreboard gets values from server and displays them
void Board(int sock, char* username){
  printf("=================================================================================\n\n\n");
  int16_t size_of_buffer;
  if( recv(sock, &size_of_buffer, sizeof(int16_t), 0) < 0)
      printf("Error in recieving ");
  size_of_buffer = htons(size_of_buffer);
  /*printf("\nsize of buffer is %hu\n", size_of_buffer);*/
  char buf[size_of_buffer];
  int nread;
  if( (nread = recv(sock, &buf, size_of_buffer, 0)) < 0)
      printf("Error in recieving ");
  buf[nread] = '\0';
  printf("\n%s", buf);
  printf("\n\n\n");
  printf("================================================================================\n\n");

}


//client singal stop for CTRL+C
void stop(int sigtype) {
    close(sockfd);
    exit(1);
}
//main loop stuff
int main(int argc, char *argv[]) {
    signal(SIGINT, stop);

    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    if (argc != 3) {
        fprintf(stderr,"usage: client_hostname port_number\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
        herror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(atoi(argv[2]));    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, \
                sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Waiting for server response...\n");
    Welcome();
    char nickname[20];
    char auth[20];
    int8_t str_len;
    while(true){
        printf("=========================================================\n");
        printf("\nPlease input your nickname ----> ");
        scanf("%s",nickname);
        if(send(sockfd,nickname,20,0) == -1){
            printf("Error sending username\n");
        }
        uint16_t room_number = 0;
        recieve_available_room(sockfd);
        printf("If you want to create new room send a room id bigger than the numberr of rooms. \n");
        printf("Enter the room number you want to join -----> ");
        scanf("%hu", &room_number);
        uint16_t converter = ntohs(room_number);
        if(send(sockfd, &converter, sizeof(uint16_t), 0) == -1)
            printf("error in sending room number");
        if((str_len = recv(sockfd,auth,20,0)) == -1){
            printf("Error receiving authentication token\n");
        }
        auth[str_len] = '\0';

        // int8_t test;
        if(strcmp(auth, "Auth") != 0){
            printf("nickname is taken please choose new nickname\n");
        }
        else break; // if nickname is not taken continue
    }
    size_t number_of_players_online;
    recv(sockfd, &number_of_players_online, sizeof(size_t), 0);
    printf("Waiting for other players to join !!\n");
    while(number_of_players_online == 1){
        recv(sockfd, &number_of_players_online, sizeof(size_t), 0);
        sleep(1);
    }
    Game_play(sockfd,nickname);


    printf("Disconnected\n");
    close(sockfd);

    return 0;
}

void recieve_available_room(int socket_id){
    int16_t number_of_rooms;
    if( recv(socket_id, &number_of_rooms, sizeof(int16_t), 0) < 0)
        printf("Error in recieving ");
    number_of_rooms = htons(number_of_rooms);
    printf("total number of rooms availabel %d \n", number_of_rooms);
    for(size_t i = 0 ; i < number_of_rooms; ++i){
        printf("Room id %lu\n", i);
        printf("Players are : \n");
        int16_t size_of_buffer;
        int nread;
        if( recv(socket_id, &size_of_buffer, sizeof(int16_t), 0) < 0)
            printf("Error in recieving ");
        size_of_buffer = htons(size_of_buffer);
        char buf[size_of_buffer];
        if( (nread = recv(socket_id, &buf, size_of_buffer , 0)) < 0)
            printf("Error in recieving ");
        buf[nread] = '\0';
        printf("\n%s\n", buf);
    }
}
