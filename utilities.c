
#include "utilities.h"

extern struct auth_user *auth_user_first;
extern struct auth_user *auth_user_last;

extern struct Hang *Hang_first;
extern struct Hang *Hang_last;

extern struct Leaderboard *Leaderboard_first;
extern struct Leaderboard *Leaderboard_last;

extern char **nickname;

extern char **word1;
extern int won;
extern int played;
extern size_t number_of_rooms;



//adds user to list
void add_auth_user (auth_user *first, auth_user *last,char * nickname){
    struct auth_user * a_user;
    a_user = (struct auth_user*)malloc(sizeof(struct auth_user));
    a_user->nickname = nickname;
    a_user->logged_on = true;
    a_user->next = NULL;

    if (first == NULL) {
        first = a_user;
        last = a_user;
    } else {
        last->next = a_user;
        last = a_user;
    }

}

void add_user(auth_user *first,auth_user * last,auth_user *user){
    if (first == NULL){
        printf("i am adding to the first element \n");
        first = user;
        last = user;
    } else{
        last->next = user;
        last = user;
    }
}

//search for user and checks password return 0 or 1
struct auth_user* searchn(struct auth_user *ptr, char* nickname){
    printf("\n i am inside searchn functioin\n");
    while(ptr != NULL){
        printf("\n username is %s\n", ptr->nickname);
        if(strcmp(ptr->nickname, nickname)== 0){
            return ptr;
        }
        ptr = ptr->next;
    }
    return ptr;
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
//add game played and nickname to list
void add_board (char * nickname,int won1, int played1 ){
    struct Leaderboard * board;
    played = played + played1;
    won = won + won1;
    board = (struct Leaderboard*)malloc(sizeof(struct Leaderboard));
    board->players = 0;
    board->next = NULL;

    if (Leaderboard_first == NULL) {
        Leaderboard_first = board;
        Leaderboard_last = board;
    } else {
        Leaderboard_last->next = board;
        Leaderboard_last = board;
    }

}


Leaderboard* create_new_room(uint8_t id){
    if(Leaderboard_first != NULL){
        Leaderboard *new = malloc(sizeof(Leaderboard));
        new->id = id;
        new->next = NULL;
        Leaderboard_last->next = new;
        Leaderboard_last = new;
        new->players = 0;
        return new;
    } else{
        Leaderboard_first = malloc(sizeof(Leaderboard));
        Leaderboard_last = Leaderboard_first;
        Leaderboard_first->id = 0;
        return Leaderboard_first;
    }
}

Leaderboard* search_room(uint8_t id) {
    Leaderboard *ptr = Leaderboard_first;
    while(ptr != NULL){
        if(ptr->id == id)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}



void add_to_room(uint8_t id, auth_user * user){
    Leaderboard *ptr = Leaderboard_first;
    while(!(ptr == NULL || ptr->id == id)){
        ptr = ptr->next;
    }
    add_user(ptr->first, ptr->last, user);
}

