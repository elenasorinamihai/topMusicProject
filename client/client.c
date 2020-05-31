#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;
int port;
struct UserInfo{
    int user_id;
    int user_type;
} userClientInfo;

#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define WHITE  "\x1B[37m"
#define NORMAL  "\x1B[0m"

void clearscr() {
    system("clear");
}

void raiseServerError(int sd) {
    printf("%sAN ERROR OCCURRED DURING COMMUNICATION WITH THE SERVER.", RED);
    printf("%s", NORMAL);
    exit(sd);
}

void login(int sd) {
    char username[100];
    bzero(username, 100);
    char *password;
    password = (char *) malloc(15 * sizeof(char));
    int numberOfReadCharacters;

    int user_id = -1;
    int user_type = -1;
    printf("Username: ");
    fflush(stdout);
    numberOfReadCharacters = read(0, username, 100);
    strcpy(&username[numberOfReadCharacters - 1], "\0");

    password = getpass("Password: ");

    if (write(sd, username, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (write(sd, password, 15) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &user_id, sizeof(user_id)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &user_type, sizeof(user_type)) <=0) {
        raiseServerError(sd);
        return;
    }

    if(user_id != -1) {
        clearscr();
        printf("Welcome, %s\n", username);
        userClientInfo.user_id = user_id;
        userClientInfo.user_type = user_type;
        return;
    }

    clearscr();
    printf("Wrong credentials for %s or this username doesn't exist. Please try again!\n", username);
    login(sd);
}

void registerUser(int sd) {
    char username[100], type[2];
    bzero(username, 100);
    bzero(type, 2);
    char *password, *verifyPassword;
    password = (char *) malloc(15 * sizeof(char));
    verifyPassword = (char *) malloc(15 * sizeof(char));
    int numberOfReadCharacters;
    int isAlreadyInBd = 0;

    printf("Username: ");
    fflush(stdout);
    numberOfReadCharacters = read(0, username, 100);
    strcpy(&username[numberOfReadCharacters - 1], "\0");

    printf("Type (admin -> 0 || sclav -> 1) (0 / 1): ");
    fflush(stdout);
    numberOfReadCharacters = read(0, type, 2);
    strcpy(&type[numberOfReadCharacters - 1], "\0");

    if (strcmp(type, "0") != 0 && strcmp(type, "1") != 0) {
        printf("%sType should be 1 or 0. Please enter a correct value!\n", RED);
        printf("%s", NORMAL);
        sleep(3);
        clearscr();
        registerUser(sd);
    }

    password = getpass("Password: ");
    strcpy(verifyPassword, password);
    password = getpass("Confirm your password: ");

    if(strcmp(password, verifyPassword) != 0) {
        printf("Passwords don't match.");
        sleep(2);
        registerUser(sd);
    }

    if (write(sd, username, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (write(sd, password, 15) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (write(sd, type, 2) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd != 0) {
        printf("Username %s already exists. Please try another one!", username);
        registerUser(sd);
        return;
    }

    clearscr();
    printf("%s has successfully registered. Please login!\n", username);

}

void exit(int sd) {
    close(sd);
}

void disableVote (int sd) {
    char normalUsers[3000];
    bzero(normalUsers, 3000);

    int numberOfReadCharacters;
    char userId[20];
    bzero(userId, 20);
    int intUserId;

    // citesc de la server toate genurile si le afisez

    if (read(sd, normalUsers, 3000) <= 0) {
        raiseServerError(sd);
        return;
    }
    printf("\n%s%s", GREEN, normalUsers);
    printf("%sType the id you want to disable from voting: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    fflush(stdin);

    numberOfReadCharacters = read(0, userId, 20);
    strcpy(&userId[numberOfReadCharacters - 1], "\0");
    intUserId = atoi(userId);

    if (write(sd, &intUserId, sizeof(intUserId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    // am transmis catre server genreId, acum trebuie sa citesc raspunsul de la server
    // (daca este corect sau nu)

    if (read(sd, &intUserId, sizeof(intUserId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (intUserId == 0) {
        printf("%sThe id is not correct. Please try again.\n", RED);
        disableVote(sd);
        return;
    }

    printf("User blocked!\n");
    return;

}

void addGenre (int sd) {
    int numberOfReadCharacters;
    char name[100];
    int isAlreadyInBd;

    printf("%sEnter music genre: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    numberOfReadCharacters = read(0, name, 100);
    strcpy(&name[numberOfReadCharacters - 1], "\0");

   if (write(sd, name, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd != 0) {
            printf("Genre %s already exists. Please try another one!\n", name);
            addGenre(sd);
            return;
    }
}

void setGenre(int sd) {
    char genresToBeShown[3000];
    bzero(genresToBeShown, 3000);

    int numberOfReadCharacters;
    char genreId[20];
    bzero(genreId, 20);
    int intGenreId;

    // citesc de la server toate genurile si le afisez

    if (read(sd, genresToBeShown, 3000) <= 0) {
        raiseServerError(sd);
        return;
    }

    printf("%s%s\n\n", GREEN, genresToBeShown);
    printf("%sType the id of the genre: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    fflush(stdin);

    numberOfReadCharacters = read(0, genreId, 20);
    strcpy(&genreId[numberOfReadCharacters - 1], "\0");
    intGenreId = atoi(genreId);

    if (write(sd, &intGenreId, sizeof(intGenreId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    // am transmis catre server genreId, acum trebuie sa citesc raspunsul de la server
    // (daca este corect sau nu)

    if (read(sd, &intGenreId, sizeof(intGenreId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (intGenreId == 0) {
        printf("%s\nThe id is not correct. Please try again.\n", RED);
        setGenre(sd);
        return;
    }

    // daca a ajuns pana aici astept raspunsul final de la server.

    char resultMessage[100];
    bzero(resultMessage, 100);

    if (read(sd, resultMessage, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    printf("%s%s", GREEN, resultMessage);
    printf("%s", NORMAL);
    fflush(stdout);
    fflush(stdin);

    char response[5];
    bzero(response, 5);
    numberOfReadCharacters = read(0, response, 3);
    strcpy(&response[numberOfReadCharacters - 1], "\0");

    if (write(sd, response, 5) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(strcmp(response, "y") == 0) {
        setGenre(sd);
        return;
    }

    return;
}

void addSong (int sd) {
    
    char name[255];
    bzero(name, 255);
    char description[255];
    bzero(description, 255);
    char externalLink[255];
    bzero(externalLink, 255);
    int isAlreadyInBd;
    int numberOfReadCharacters;

    printf("%sEnter the name of the song: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    numberOfReadCharacters = read(0, name, 255);
    strcpy(&name[numberOfReadCharacters-1], "\0");

    printf("%sEnter a description for the song: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    numberOfReadCharacters = read(0, description, 255);
    strcpy(&description[numberOfReadCharacters-1], "\0");

    printf("%sEnter an external link for the song: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    numberOfReadCharacters = read(0, externalLink, 255);
    strcpy(&externalLink[numberOfReadCharacters-1], "\0");


    if (write(sd, name, 255) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (write(sd, description, 255) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (write(sd, externalLink, 255) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd != 0) {
        printf("Song %s already exists. Please try another one!", name);
        addSong(sd);
        return;
    }

    clearscr();
    printf("%s have successfully registered. Please add a genre!\n", name);

    setGenre(sd);
}

void voteSong (int sd) {
    char option[50];
    bzero(option, 50);   
    int alreadyVoted;
    int isAlreadyInBd; 
    int numberOfReadCharacters;

    printf("%sSelect the song by entering it's ID number: ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    numberOfReadCharacters = read(0, option, 50);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 50) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd == 0) {
        printf("Song %s does not exist. Please try another one!\n", option);
        voteSong(sd);
        return;
    }

    if (read(sd, &alreadyVoted, sizeof(alreadyVoted)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(alreadyVoted == 1) {
        printf("You already voted for this song. Please try another one!\n");
        voteSong(sd);
        return;
    }

    printf("Your vote has been registered.\n");
    mainMenu(sd);

}

void addComment(int sd) {
    char option[50];
    bzero(option, 50); 
    char content[500];
    bzero(content, 500);
    char confirmation[10];
    bzero(confirmation, 10);
    int numberOfReadCharacters;
    int isAlreadyInBd;

    printf("%sSelect the song by entering it's ID number: ", MAGENTA);
    printf("%s", NORMAL);
    
    fflush(stdout);
    numberOfReadCharacters = read(0, option, 50);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 50) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd == 0) {
        printf("Song %s does not exist. Please try another one!\n", option);
        addComment(sd);
        return;
    } 

    printf("%sWrite your comment: ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    numberOfReadCharacters = read(0, content, 500);
    strcpy(&content[numberOfReadCharacters -1], "\0");

    if (write(sd, content, 500) <= 0) {
        raiseServerError(sd);
        return;
    }


    printf("%sYour comment has been added!\n", MAGENTA);

}

void showComments(int sd) {
    char results[10000];
    bzero(results,10000);
    char option[60];
    bzero(option, 60); 
    int numberOfReadCharacters;

    printf("%sSelect the song by entering it's ID number: ", MAGENTA);
    printf("%s", NORMAL);
    


    fflush(stdout);
    numberOfReadCharacters = read(0, option, 60);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if(write(sd, option, 60) <= 0) {
        raiseServerError(sd);
        return;
    }


    if(read(sd,results, 10000) <= 0){
        raiseServerError(sd);
        return;
    }

    printf("%s%s", YELLOW, results);

    printf("%sChoose an option: voteSong / addComment / changeSong / menu: ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    fflush(stdin);

    
    bzero(option, 60);


    fflush(stdout);
    numberOfReadCharacters = read(0, option, 60);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 60) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(strcmp(option, "voteSong") == 0) {
        voteSong(sd);
        
    } else if(strcmp(option, "addComment") == 0) {
        addComment(sd);

    } else if (strcmp(option, "changeSong") == 0) {
        showComments(sd);

    } else if (strcmp(option, "menu") == 0) {
        mainMenu(sd);

    } else {
        mainMenu(sd); // a introdus ceva gresit, revine la showSongs();
    }


}


void showSongs(int sd) {
    char results[10000];
    bzero(results, 10000);
    char access[2];

    if (read(sd, results, 10000) <= 0) {
        raiseServerError(sd);
        return;
    }

    printf("%s%s", YELLOW, results);

    printf("%sChoose an option: voteSong / addComment / comments / mainMenu : ", GREEN);
    printf("%s", NORMAL);


    fflush(stdout);
    fflush(stdin);

    char option[50];
    bzero(option, 50);

    int numberOfReadCharacters;

    fflush(stdout);
    numberOfReadCharacters = read(0, option, 50);
    strcpy(&option[numberOfReadCharacters -1], "\0");
     printf("%s%s\n\n", GREEN, option);

    if (write(sd, option, 50) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, access, 2) <= 0) {
        raiseServerError(sd);
        return;
    }



    if(strcmp(option, "voteSong") == 0 && (atoi(access) == 0)) {
        voteSong(sd);
        return;
    } else if(strcmp(option, "voteSong") == 0 && atoi(access) == 1) {
        printf("%sYou are not allowed to vote!\n", RED);
        mainMenu(sd);
        return;
    } else if (strcmp(option, "addComment") == 0) {
        addComment(sd);
        return;
    } else if (strcmp(option, "comments") == 0) {
        showComments(sd);
        return;
    } else if (strcmp(option, "mainMenu") == 0) {
        mainMenu(sd);
        return;
    } 
      else {
        showSongs(sd); 
        return;
    }
}

void showGenreTop (int sd) {
    char genresToBeShown[3000];
    bzero(genresToBeShown, 3000);

    int numberOfReadCharacters;
    char genreId[20];
    bzero(genreId, 20);
    int intGenreId;

    // citesc de la server toate genurile si le afisez

    if (read(sd, genresToBeShown, 3000) <= 0) {
        raiseServerError(sd);
        return;
    }

    printf("%s%s\n\n", GREEN, genresToBeShown);
    printf("%sType the id of the genre: ", MAGENTA);
    printf("%s", NORMAL);
    fflush(stdout);
    fflush(stdin);

    numberOfReadCharacters = read(0, genreId, 20);
    strcpy(&genreId[numberOfReadCharacters - 1], "\0");
    intGenreId = atoi(genreId);

    if (write(sd, &intGenreId, sizeof(intGenreId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    // am transmis catre server genreId, acum trebuie sa citesc raspunsul de la server
    // (daca este corect sau nu)

    if (read(sd, &intGenreId, sizeof(intGenreId)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (intGenreId == 0) {
        printf("%s\nThe id is not correct. Please try again.\n", RED);
        setGenre(sd);
        return;
    }
    char results[10000];
    bzero(results,10000);
    char option[100];
    bzero(option, 100);

    if(read(sd,results, 10000) <= 0){
        raiseServerError(sd);
        return;
    }

    printf("%s%s", YELLOW, results);

    printf("%sChoose an option: voteSong / addComment / changeSong / menu: ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    fflush(stdin);

    
    bzero(option, 100);


    fflush(stdout);
    numberOfReadCharacters = read(0, option, 100);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(strcmp(option, "voteSong") == 0) {
        voteSong(sd);
        
    } else if (strcmp(option, "showGenreTop") == 0) {
        showGenreTop(sd);

    } else if (strcmp(option, "menu") == 0) {
        mainMenu(sd);

    } else {
        mainMenu(sd); // a introdus ceva gresit, revine la showSongs();
    }

}


void showGeneralTop (int sd) {
    char results[10000];
    bzero(results,10000);
    char option[100];
    bzero(option, 100); 
    int numberOfReadCharacters;


    if(read(sd,results, 10000) <= 0){
        raiseServerError(sd);
        return;
    }

    printf("%s%s", YELLOW, results);

    printf("%sPress exit to exit and a key to go back ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    fflush(stdin);

    
    bzero(option, 100);


    fflush(stdout);
    numberOfReadCharacters = read(0, option, 100);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (strcmp(option, "exit") == 0) {
        exit(sd);

    } else {
        mainMenu(sd); // a introdus ceva gresit, revine la showSongs();
    }


}

void deleteSong(int sd) {
    char results[10000];
    bzero(results,10000);
    char option[100];
    bzero(option, 100);
    int numberOfReadCharacters;
    int isAlreadyInBd;

    if(read(sd,results, 10000) <= 0){
        raiseServerError(sd);
        return;
    }

    printf("%s%s", YELLOW, results);
    printf("%sChoose the song you want to delete by entering it's ID: ", MAGENTA);
    printf("%s", NORMAL);

    fflush(stdout);
    fflush(stdin);


    numberOfReadCharacters = read(0, option, 100);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 100) <= 0) {
        raiseServerError(sd);
        return;
    }

    if (read(sd, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseServerError(sd);
        return;
    }

    if(isAlreadyInBd == 0) {
        printf("Song %s does not exist. Please try another one!\n", option);
        voteSong(sd);
        return;
    }

    printf("Song deleted!\n");
    return;

}

int firstMenu(int sd) {

    clearscr();

    int numberOfReadCharacters;

    char response[5];
    bzero(response, 5);

    printf("%sDo you have an account? (y / n): ", YELLOW);
    printf("%s", NORMAL);
    fflush(stdout);
    numberOfReadCharacters = read(0, response, 5);
    strcpy(&response[numberOfReadCharacters -1], "\0");

    if (write(sd, response, 5) <= 0) {
        raiseServerError(sd);
        return 0;
    }

    if(strcmp(response, "y") == 0) {
        login(sd);
    } else {
        registerUser(sd);
        login(sd);
    }

}

int mainMenu(int sd) {
    sleep(3);
    clearscr();
    int user_id = userClientInfo.user_id;
    int user_type = userClientInfo.user_type;

    if (user_type == 1) {
        printf("%sMENU: (showSongs/ addSong / showGeneralTop/ showGenreTop/ exit) : ", CYAN);
    } else if(user_type == 0){
        printf("%sMENU: (showSongs/ addSong / showGeneralTop/ showGenreTop/ deleteSong/ disableVote/ addGenre/ exit) : ", CYAN);
    }


    printf("%s", NORMAL);
    char option[50];
    bzero(option, 50);

    int numberOfReadCharacters;

    fflush(stdout);
    numberOfReadCharacters = read(0, option, 50);
    strcpy(&option[numberOfReadCharacters -1], "\0");

    if (write(sd, option, 50) <= 0) {
        raiseServerError(sd);
        return 0;
    }

    if(strcmp(option, "showSongs") == 0) {
        showSongs(sd);
        mainMenu(sd);
    }

    else if(strcmp(option, "addSong") == 0) {
        addSong(sd);
        mainMenu(sd);
    }

    else if(strcmp(option, "showGeneralTop") == 0) {
        showGeneralTop(sd);
    }

    else if(strcmp(option, "showGenreTop") == 0) {
        showGenreTop(sd);
    }

    else if(strcmp(option, "deleteSong") == 0 && user_type == 0) {
        deleteSong(sd);
        mainMenu(sd);
    }

    else if(strcmp(option, "disableVote") == 0 && user_type == 0) {
        disableVote(sd);
        mainMenu(sd);
    }

    else if(strcmp(option, "addGenre") == 0 && user_type == 0) {
        addGenre(sd);
        mainMenu(sd);
    }
    //celelalte functionalitati care mai sunt (unele doar pentru administrator)

    else {
        exit(sd);
    }

}

int main (int argc, char *argv[])
{

    int sd;
    struct sockaddr_in server;

    if(argc != 3) {
        printf("%sSyntax: %s <server_address> <port>\n", RED, argv[0]);
        printf("%s", NORMAL);
        return -1;
    }

    port = atoi (argv[2]);

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("SOCKET ERROR");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if(connect(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        printf("%sAN ERROR OCCURRED DURING COMMUNICATION WITH THE SERVER.", RED);
        printf("%s", NORMAL);
        return errno;
    }

   
    firstMenu(sd);
    mainMenu(sd);
}