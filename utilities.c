
#include "utilities.h"

extern struct auth_user *auth_user_first;
extern struct auth_user *auth_user_last;

extern struct Hang *Hang_first;
extern struct Hang *Hang_last;

extern struct Leaderboard *Leaderboard_first;
extern struct Leaderboard *Leaderboard_last;

extern char **users;
extern char **passwords;

extern char **word1;
extern int won;
extern int played;


//read the auth file
void read_file() {
    FILE *fp;
    // struct List *temp;
    char *fuser, *fpass, buf[100];
    void **pair;
    //attempt to load the text file
    fp = fopen("auth.txt", "r");
    if (fp == NULL) {
        puts("Unable to read Auth file");
        exit(EXIT_FAILURE);
    }

    fgets(buf, sizeof buf, fp);
    while (fgets(buf, sizeof buf, fp) != NULL) { //get each line
        //allocate the memory for the user pass
        fuser = malloc(10 * sizeof(char));
        fpass = malloc(10 * sizeof(char));

        pair = malloc(2*sizeof(char*)); //the container

        sscanf(buf, "%s %s", fuser, fpass);
        pair[0] = fuser;
        pair[1] = fpass;
        add_auth_user(pair[0],pair[1]);

    }

    //close connection
    fclose(fp);
}

//adds user to list
void add_auth_user (char * username, char * password){
    struct auth_user * a_user;
    a_user = (struct auth_user*)malloc(sizeof(struct auth_user));
    a_user->username = username;
    a_user->password = password;
    a_user->next = NULL;

    if (auth_user_first == NULL) {
        auth_user_first = a_user;
        auth_user_last = a_user;
    } else {
        auth_user_last->next = a_user;
        auth_user_last = a_user;
    }

}

//search for user and checks password return 0 or 1
int8_t searchn(struct auth_user *ptr, char* username, char* password){
    while (ptr!=NULL && strcmp(ptr->username, username) != 0){
        ptr = ptr->next;
    }

    if (ptr != NULL && strcmp(ptr->username, username)==0 && ptr != NULL && strcmp(ptr->password, password)==0 ) {
        // printf("%s found! \n", username);
        // printf("%s \n", ptr->username);
        // printf("%s \n", ptr->password);
        // printf("\n");
        return 1;
    }
    return 0;
}

//read hangman file
void read_file_word() {
    FILE *fp;
    char *words1, buf[100];
    void **pair;
    //attempt to load the text file
    fp = fopen("hang.txt", "r");
    if (fp == NULL) {
        puts("Unable to read Auth file");
        exit(EXIT_FAILURE);
    }

    fgets(buf, sizeof buf, fp);
    while (fgets(buf, sizeof buf, fp) != NULL) { //get each line
        //allocate the memory for the user pass
        words1 = malloc(10 * sizeof(char));

        pair = malloc(2*sizeof(char*)); //the container

        sscanf(buf, "%s", words1);
        pair[0] = words1;
        add_words(pair[0]);

    }

    //close connection
    fclose(fp);
}

//add words to list
void add_words(char * word1){
    struct Hang * hangman;
    hangman = (struct Hang*)malloc(sizeof(struct Hang));
    hangman->word1 = word1;
    hangman->next = NULL;

    if (Hang_first == NULL) {
        Hang_first = hangman;
        Hang_last = hangman;
    } else {
        Hang_last->next = hangman;
        Hang_last = hangman;
    }

}

//print a random word from list
char *printRandom(struct Hang *head)
{
    // IF list is empty
    if (head == NULL)
       return(0);

    // Use a different seed value so that we don't get
    // same result each time we run this program
    srand(time(NULL));

    // Initialize result as first node

    char *result = head->word1;

    // Iterate from the (k+1)th element to nth element
    struct Hang *current = head;
    int n;
    for (n=2; current!=NULL; n++)
    {
        // change result with probability 1/n
        if (rand() % n == 0)
           result = current->word1;

        // Move to next node
        current = current->next;
    }
    return result;

}
//add game played and username to list
void add_board (char * username,int won1, int played1 ){
    struct Leaderboard * board;
    played = played + played1;
    won = won + won1;
    board = (struct Leaderboard*)malloc(sizeof(struct Leaderboard));
    board->username = username;
    board->won = won;
    board->played = played;
    board->next = NULL;

    if (Leaderboard_first == NULL) {
        Leaderboard_first = board;
        Leaderboard_last = board;
    } else {
        Leaderboard_last->next = board;
        Leaderboard_last = board;
    }

}

//prints user debugging
void print_list(void)
{
    struct auth_user *ptr = auth_user_first;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%s] [%s] \n",ptr->username, ptr->password);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");


}
//prints words debugging
void print_list_word(void)
{
    struct Hang *ptr = Hang_first;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%s]  \n",ptr->word1);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");


}
//prints scoreboard
void print_list_board(void)
{
    struct Leaderboard *ptr = Leaderboard_first;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%s] [%d] [%d] \n",ptr->username, ptr->won, ptr->played);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");


}
