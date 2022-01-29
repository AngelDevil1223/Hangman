#include <stdio.h>
#include <stdlib.h>
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

void Welcome();

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
		//word2
		if(recv(sock, word2, MAX, 0) <= 0 ){
			return ERROR;
		}

    /* could not firgue out how to get the space between the words
        so just run the loop twice, will fix later
    */

		printf("\nWord: ");
		for(int i = 0 ; i < strlen(word1); i++){
			printf("%c ", word1[i]);
		}

		for(int i = 0; i < strlen(word2); i++){
			printf(" %c", word2[i]);
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
			return;
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
void Board(int sock, char*username){
  uint16_t converter ;
  if(local == 1){
    printf("=================================================================================\n\n\n");
  	printf("There is no information currently stored in the Leader board. Try Again Later\n\n");
  	printf("================================================================================\n\n");

    return;
  }
  int played = 0;
  if(recv(sock, &converter , sizeof(uint16_t), 0) <= 0){
    return;
  }
  played = ntohs(converter );
  //won games
  if(recv(sock, &converter , sizeof(uint16_t), 0) <= 0){
    return;
  }
  int won = 0;
  won = ntohs(converter );
  printf("=================================================================================\n\n\n");
  printf("Player  - %s\nNumber of games won  - %d\nNumber of games played  - %d\n",username,won,played);
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

	uint16_t converter ;

    Welcome();
    char username[20];
  	char password[20];
  	char auth[20];
  	int8_t str_len;
  	printf("=========================================================\n");
  	printf("\nPlease input your Username ----> ");
  	scanf("%s",username);
  	printf("Please input your Password      ----> ");
  	scanf("%s",password);
  	if(send(sockfd,username,20,0) == -1){
  		printf("Error sending username\n");
  	}
  	if(send(sockfd,password,20,0) == -1){
  		printf("Error sending password\n");
  	}
    if((str_len = recv(sockfd,auth,20,0)) == -1){
  		printf("Error receiving authentication token\n");
  	}
  	auth[str_len] = '\0';

  	// int8_t test;
  	if(strcmp(auth, "Auth") != 0){
  		printf("Password or username is incorrect: Closing Client\n");
  		exit(EXIT_FAILURE); // close the client with unsuccessful logon
    }
		Game_play(sockfd,username);


	printf("Disconnected\n");
	close(sockfd);

	return 0;
}
