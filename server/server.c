#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mysql/mysql.h>
#include <stdbool.h>

#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define WHITE  "\x1B[37m"

#define PORT 2016

extern int errno;

// declaratii pentru serverul de mySQL
char *server = "localhost";
char *user = "root";
char *pass = "elena";
char *database = "topMusic";
// end declaratii pentru serverul de mySQL

void raiseClientError(int client, int location) {
    printf("%sAN ERROR OCCURRED DURING COMMUNICATION WITH THE CLIENT [%d].\n", RED, location);
    close(client);
    return;
}


int login(int client) {

    char username[100], password[15];
    bzero(username, 100);
    bzero(password, 15);

    if(read(client, username, 100) <= 0) {
        raiseClientError(client, 1);
        return 0;
    }

    if(read(client, password, 15) <= 0) {
        raiseClientError(client, 2);
        return 0;
    }

    int user_id = -1;
    int user_type = -1;
    char user_id_char[20];
    bzero(user_id_char, 20);

    char query[500];
    bzero(query, 500);
    strcat(query, "select id from user where userName = '");
    strcat(query, username);
    strcat(query, "' and password = '");
    strcat(query, password);
    strcat(query, "';");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        user_id = atoi(row[0]);
        sprintf(user_id_char, "%d", user_id);
    }

    mysql_free_result(res);
    mysql_close(conn);

    if(write(client, &user_id, sizeof(user_id)) <= 0) {
        raiseClientError(client, 3);
        return 0;
    }

    if(user_id != -1) {

        bzero(query, 500);
        strcat(query, "select type from user where id = ");
        strcat(query, user_id_char);
        strcat(query, ";");

        conn = mysql_init(NULL);

        if (!mysql_real_connect(conn, server, user, pass, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }


        if(mysql_query(conn, query)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }

        res = mysql_use_result(conn);

        while ((row = mysql_fetch_row(res)) != NULL)
            user_type = atoi(row[0]);

        mysql_free_result(res);
        mysql_close(conn);

    }

    if(write(client, &user_type, sizeof(user_type)) <= 0) {
        raiseClientError(client, 4);
        return 0;
    }

    if(user_id != -1) {
        return user_id;
    } else {
        login(client);
    }


}


int registerUser(int client) {
    char username[100], password[15], type[2];
    bzero(username, 100);
    bzero(password, 15);
    bzero(type, 2);

    if(read(client, username, 100) <= 0) {
        raiseClientError(client, 5);
        return 0;
    }

    if(read(client, password, 15) <= 0) {
        raiseClientError(client, 6);
        return 0;
    }

    if(read(client, type, 2) <= 0) {
        raiseClientError(client, 7);
        return 0;
    }

    int isAlreadyInBd = 0;

    char query[300];
    bzero(query, 300);
    strcat(query, "select count(*) from user where userName = '");
    strcat(query, username);
    strcat(query, "';");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 8);
        return 0;
    }
    if(isAlreadyInBd != 0) {
        registerUser(client);
    }


    bzero(query, 300);
    strcat(query, "insert into user values(NULL, '");
    strcat(query, username);
    strcat(query, "', '");
    strcat(query, password);
    strcat(query, "', ");
    strcat(query, type);
    strcat(query, ", ");
    strcat(query, "0");
    strcat(query, ");");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_close(conn);

}

void exit(int client) {
    close(client);
}

int findUserAccess (int user_id) {


    int access;
    char query[500];
    char user_id_char[20];
    bzero(user_id_char, 20);
    sprintf(user_id_char, "%d", user_id);

    bzero(query, 500);
    strcat(query, "select isBlocked from user where id = ");
    strcat(query, user_id_char);
    strcat(query, " ;");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user,
                            pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        access = atoi(row[0]);

    mysql_free_result(res);

    return access;
}

void disableVote (int client) {
    char query[3000];
    bzero(query, 3000);
    strcat(query, "SELECT id, userName, type, isBlocked FROM user where type = 1;");
    char normalUsers[3000];
    bzero(normalUsers, 3000);

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {

        strcat(normalUsers, "ID = ");
        strcat(normalUsers, row[0]);
        strcat(normalUsers, " || USERNAME = ");
        strcat(normalUsers, row[1]);
        strcat(normalUsers, " || TYPE: ");
        strcat(normalUsers, row[2]);
        strcat(normalUsers, " || BLOCKED: ");
        strcat(normalUsers, row[3]);
        strcat(normalUsers, "\n\n");
    }
    strcat(normalUsers, "\0");

    mysql_free_result(res);

    if (write(client, normalUsers, 3000) <= 0) {
        raiseClientError(client, 11);
        return;
    }

    int userId;

    if (read(client, &userId, sizeof(userId)) <= 0) {
        raiseClientError(client, 12);
        return;
    }

    char userIdChar[20];
    bzero(userIdChar, 20);
    sprintf(userIdChar, "%d", userId);

    // verific daca id-ul introdus exista cu adevarat in baza de date

    bzero(query, 3000);
    strcat(query, "SELECT count(*) FROM user WHERE id = ");
    strcat(query, userIdChar);
    strcat(query, ";");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    int isACorrectId; 

    while ((row = mysql_fetch_row(res)) != NULL)
        isACorrectId = atoi(row[0]);

    mysql_free_result(res);

    if(write(client, &isACorrectId, sizeof(isACorrectId)) <= 0) {
        raiseClientError(client, 13);
        return;
    }
    if(isACorrectId == 0) {
        disableVote(client);
        return;
    }

    bzero(query, 3000);
    strcat(query, "UPDATE user SET isBlocked = 1 WHERE id = ");
    strcat(query, userIdChar);
    strcat(query, " ;");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_close(conn);
    return;

}

void addGenre (int client) {

    char name[100];
    bzero(name, 100);

    if(read(client, name, 100) <= 0) {
        raiseClientError(client, 9);
        return;
    }

    int isAlreadyInBd = 0;

    char query[100];
    bzero(query, 100);
    strcat(query, "select count(*) from genre where name = '");
    strcat(query, name);
    strcat(query, "';");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user,
                            pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 10);
        return;
    }

    if(isAlreadyInBd != 0) {
        addGenre(client);
        return;
    }


    bzero(query, 100);
    strcat(query, "insert into genre values(NULL, '");
    strcat(query, name);
    strcat(query, "');");



    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_close(conn);

}

void setGenre(int client, int songId) {
    char genresToBeShownOnClient[3000];
    bzero(genresToBeShownOnClient, 3000);
    int selectedGenre;
    char selectedGenreChar[20];
    bzero(selectedGenreChar, 20);
    int isACorrectId;
    char songIdChar[20];
    bzero(songIdChar, 20);
    sprintf(songIdChar, "%d", songId);

    char query[300];
    bzero(query, 300);
    strcat(query, "SELECT id, name FROM genre;");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {

        strcat(genresToBeShownOnClient, "ID = ");
        strcat(genresToBeShownOnClient, row[0]);
        strcat(genresToBeShownOnClient, " || NAME = ");
        strcat(genresToBeShownOnClient, row[1]);
        strcat(genresToBeShownOnClient, "\n");
    }
    strcat(genresToBeShownOnClient, "\0");

    mysql_free_result(res);

    if (write(client, genresToBeShownOnClient, 3000) <= 0) {
        raiseClientError(client, 11);
        return;
    }

    // transmite catre client toate genurile muzicale disponibile, 
    // trebuie sa aleaga un gen anume si sa il transmita catre server,
    // deci trebuie sa fac read si sa o procesez.

    if (read(client, &selectedGenre, sizeof(selectedGenre)) <= 0) {
        raiseClientError(client, 12);
        return;
    }

    sprintf(selectedGenreChar, "%d", selectedGenre);

    // verific daca id-ul introdus exista cu adevarat in baza de date

    bzero(query, 300);
    strcat(query, "SELECT count(*) FROM genre WHERE id = ");
    strcat(query, selectedGenreChar);
    strcat(query, ";");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isACorrectId = atoi(row[0]);

    mysql_free_result(res);

    if(write(client, &isACorrectId, sizeof(isACorrectId)) <= 0) {
        raiseClientError(client, 13);
        return;
    }
    if(isACorrectId == 0) {
        setGenre(client, songId);
        return;
    }

    // daca ajunge aici, inseamna ca a introdus un id de gen bun, ceea ce inseamna 
    //ca trebuie sa verific daca acel gen este deja asociat acelei piese

    bzero(query, 300);
    strcat(query, "SELECT count(*) FROM songGenre WHERE genre_Fk = ");
    strcat(query, selectedGenreChar);
    strcat(query, " AND song_Fk = ");
    strcat(query, songIdChar);
    strcat(query, ";");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    int associationAlreadyExists;

    while ((row = mysql_fetch_row(res)) != NULL)
        associationAlreadyExists = atoi(row[0]);

    mysql_free_result(res);


    char* resultMessage;
    resultMessage = (char *) malloc(100 * sizeof(char));
    bzero(resultMessage, 100);

    if (associationAlreadyExists != 0) {
        resultMessage = "This association already exists. Do you want to add another one? (y / n): ";
    } else {
        bzero(query, 300);
        strcat(query, "INSERT INTO songGenre VALUES(NULL,");
        strcat(query, songIdChar);
        strcat(query, ", ");
        strcat(query, selectedGenreChar);
        strcat(query, ");");

        if (mysql_query(conn, query)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }
        resultMessage = "Association registered. Do you want to add another one? (y / n): ";
    }

    if (write(client, resultMessage, 100) <= 0) {
        raiseClientError(client, 14);
        return;
    }

    mysql_close(conn); // toate operatiile s-au terminat, deci inchid conexiunea cu baza de date

    // am trimis mesajul inapoi catre client. Oricum ar fi ambele mesaje posibile au ca si continuitate daca doresc sa 
    // mai adaug inca un gen, ceea ce inseamna ca trebuie sa primesc si un raspuns de la client.

    char response[5];
    bzero(response, 5);

    if(read(client, response, 5) <= 0) {
        raiseClientError(client, 15);
        return;
    }

    if(strcmp(response, "y") == 0) {
        setGenre(client, songId);
        return;
    }

    return;
}

void addSong (int client) {

    char name[255], description[255], externalLink[255];
    bzero(name, 255);
    bzero(description, 255);
    bzero(externalLink, 255);
    int isAlreadyInBd = 0;

    if(read(client, name, 255) <= 0) {
        raiseClientError(client, 16);
        return;
    }

    if(read(client, description, 255) <= 0) {
        raiseClientError(client, 17);
        return;
    }

    if(read(client, externalLink, 255) <= 0) {
        raiseClientError(client, 18);
        return;
    }

    char query[1000];
    bzero(query,1000);
    strcat(query, "select count(*) from song where name = '");
    strcat(query, name);
    strcat(query, "';");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 19);
        return;
    }
    if(isAlreadyInBd != 0) {
        addSong(client);
        return;
    }

    int songId;

    bzero(query, 1000);
    strcat(query, "insert into song values(NULL, '");
    strcat(query, name);
    strcat(query, "','");
    strcat(query, description);
    strcat(query, "','");
    strcat(query, externalLink);
    strcat(query, "');");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    // dupa ce se termina introducerea piesei, interoghez bd pentru a afla
    // id-ul noii piese, dupa care apelez setGenre.

    bzero(query, 1000);
    strcat(query, "SELECT id FROM song WHERE name = '");
    strcat(query, name);
    strcat(query, "';");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        songId = atoi(row[0]);

    mysql_free_result(res);

    mysql_close(conn);

    setGenre(client, songId);

}

void voteSong (int client, int user_id, int user_type) {
    char option[50];
    bzero(option, 50);
    int alreadyVoted = 0; 
    int isAlreadyInBd = 0; 

    if (read(client, option, 50) <= 0) {
        raiseClientError(client, 578);
        return;
    }

    char query[1000];
    bzero(query,1000);
    strcat(query, "select count(*) from song where id = ");
    strcat(query, option);
    strcat(query, ";");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 614);
        return;
    }
    if(isAlreadyInBd == 0) {
        voteSong(client, user_id, user_type);
        return;
    }
    char user_id_char[20];
    bzero(user_id_char, 20);;
    sprintf(user_id_char, "%d", user_id);

    bzero(query,1000);
    strcat(query, "select count(*) from vote where user_Fk = ");
    strcat(query, user_id_char);
    strcat(query, " and song_Fk = ");
    strcat(query, option);
    strcat(query, ";");

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        alreadyVoted = atoi(row[0]);

    mysql_free_result(res);

    if(write(client, &alreadyVoted, sizeof(alreadyVoted)) <= 0) {
        raiseClientError(client, 653);
        return;
    }

    if(alreadyVoted == 1) {
        voteSong(client, user_id, user_type);
        return;
    } 
    
    bzero(query, 1000);
    strcat(query, "insert into vote values(NULL, ");
    strcat(query, option);
    strcat(query, ",");
    strcat(query, user_id_char);
    strcat(query, ", 1 , 0 );");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_close(conn);
    mainMenu(client, user_id, user_type);


}

void addComment(int client, int user_id, int user_type) {
    char content[500];
    bzero(content, 500);
    char option[50];
    bzero(option, 50);
    int isAlreadyInBd = 0;

    if (read(client, option, 50) <= 0) {
        raiseClientError(client, 578);
        return;
    }


    char query[1000];
    bzero(query,1000);
    strcat(query, "select count(*) from song where id = ");
    strcat(query, option);
    strcat(query, ";");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 725);
        return;
    }
    if(isAlreadyInBd == 0) {
        addComment(client, user_id, user_type);
        return;
    }

    if(read(client, content, 500) <= 0) {
        raiseClientError(client, 734);
        return;
    }

    printf("%sSERVER LINIA 629: ", MAGENTA);
 
    
    char user_id_char[20];
    bzero(user_id_char, 20);
    sprintf(user_id_char, "%d", user_id);

    
    bzero(query, 1000);
    strcat(query, "insert into comment values(NULL, ");
    strcat(query, option);
    strcat(query, ",");
    strcat(query, user_id_char);
    strcat(query, ",'");
    strcat(query, content);
    strcat(query, "');");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_close(conn);

}


void showComments(int client, int user_id, int user_type) {
    
    char option[60];
    bzero(option, 60);
    char results[10000];
    bzero(results, 10000);
    bool songExists = false;

    if (read(client, option, 60) <= 0) {
        raiseClientError(client, 25);
        return;
    }

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char query[300];
    bzero(query, 300);

    strcat(query, "SELECT count(*) FROM song WHERE id = ");
    strcat(query, option);
    strcat(query, ";");

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                    user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);
    
    while ((row = mysql_fetch_row(res)) != NULL) {

        if(atoi(row[0]) != 0) {
            songExists = true;
         }
    }

    mysql_free_result(res);
    mysql_close(conn);

    if (!songExists) {
        strcat(results, "\nThis song doesn't exists.\n"); 
    } else {
        bzero(query, 300);
        strcat(query, "SELECT u.userName, s.name, s.id, c.content FROM user u JOIN comment c ON c.user_Fk=u.id JOIN song s ON c.song_Fk=s.id WHERE s.id= ");
        strcat(query, option);
        strcat(query, " ;");

        conn = mysql_init(NULL);

        if (!mysql_real_connect(conn, server,
                                user, pass, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }    

        if (mysql_query(conn, query)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }

        res = mysql_use_result(conn);
        bool hasValues = false;

        while ((row = mysql_fetch_row(res)) != NULL) {
            hasValues = true;
            strcat(results, "\nUser : ");
            strcat(results, row[0]);
            strcat(results, "\nSong : ");
            strcat(results, row[1]);
            strcat(results, "\nId : ");
            strcat(results, row[2]);
            strcat(results, "\nComment : ");
            strcat(results, row[3]);

            strcat(results, "\n.................................................");
            strcat(results, "\n");
        }

        if(!hasValues) {
            strcat(results, "\nThis song has no comments.\n");
        }


        mysql_free_result(res);
        mysql_close(conn);

    }

    strcat(results, "\0");

    if(write(client, results, 10000) <= 0) {
        raiseClientError(client, 20);
        return;
    }

    bzero(option, 60);

    if (read(client, option, 60) <= 0) {
        raiseClientError(client, 25);
        return;
    }


    if(strcmp(option, "voteSong") == 0) {
        voteSong(client, user_id, user_type);
        return;
    }else if(strcmp(option, "addComment") == 0) {
        addComment(client, user_id, user_type);
        return;
    } else if (strcmp(option, "changeSong") == 0) {
        showComments(client, user_id, user_type);
        return;

    } else if (strcmp(option, "menu") == 0) {
        mainMenu(client);
        return;

    } else {
        mainMenu(client);
        return;
    }
    return;

}

void showSongs(int client, int user_id, int user_type) {
    char results[10000];
    bzero(results, 10000);

    char query[300];
    bzero(query, 300);
    strcat(query, "SELECT id, name, description, externalLink FROM song;");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    MYSQL *conn2;
    MYSQL_RES *res2;
    MYSQL_ROW row2;

    MYSQL *conn3;
    MYSQL_RES *res3;
    MYSQL_ROW row3;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {

        strcat(results, "ID = ");
        strcat(results, row[0]);
        strcat(results, "\nNAME = ");
        strcat(results, row[1]);
        strcat(results, "\nDESCRIPTION = ");
        strcat(results, row[2]);
        strcat(results, "\nEXTERNAL LINK = ");
        strcat(results, row[3]);
        strcat(results, "\nGENRES = ");


        // iau toate inregistrarile songGenre

        char query2[300];
        bzero(query2, 300);

        strcat(query2, "SELECT genre_Fk FROM songGenre WHERE song_Fk = ");
        strcat(query2, row[0]);
        strcat(query2, ";");

        conn2 = mysql_init(NULL);

        if (!mysql_real_connect(conn2, server,
                            user, pass, database, 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(conn2));
            exit(1);
         }

        if (mysql_query(conn2, query2)) {
            fprintf(stderr, "%s\n", mysql_error(conn2));
            exit(1);
        }

        res2 = mysql_use_result(conn2);

        while ((row2 = mysql_fetch_row(res2)) != NULL) {
            char query3[300];
            bzero(query3, 300);

            strcat(query3, "SELECT name FROM genre WHERE id = ");
            strcat(query3, row2[0]);
            strcat(query3, ";");

            conn3 = mysql_init(NULL);

            if (!mysql_real_connect(conn3, server,
                                user, pass, database, 0, NULL, 0)) {
                fprintf(stderr, "%s\n", mysql_error(conn3));
                exit(1);
             }

            if (mysql_query(conn3, query3)) {
                fprintf(stderr, "%s\n", mysql_error(conn3));
                exit(1);
            }

            res3 = mysql_use_result(conn3);
            while ((row3 = mysql_fetch_row(res3)) != NULL) {
                strcat(results, " ");
                strcat(results, row3[0]);
            }
            mysql_free_result(res3);
            mysql_close(conn3);
        }

        mysql_free_result(res2);
        mysql_close(conn2);

        strcat(results, "\n\n##################################");
        strcat(results, "\n");
    }
    strcat(results, "\0");

    mysql_free_result(res);
    mysql_close(conn);


    if(write(client, results, 10000) <= 0) {
        raiseClientError(client, 10000);
        return;
    }

    char option[50];
    bzero(option, 50);

    if (read(client, option, 50) <= 0) {
        raiseClientError(client, 737);
        return;
    }

    int user_access;
    user_access=findUserAccess(user_id);
    char userAccessChar[2];
    bzero(userAccessChar,2);
    sprintf(userAccessChar, "%d", user_access);

    if(write(client, userAccessChar, 2) <= 0) {
        raiseClientError(client, 10000);
        return;
    }

    if(strcmp(option, "voteSong") == 0 && user_access == 0) {
        voteSong(client, user_id, user_type);
        return;
    } else if(strcmp(option, "voteSong") == 0 && user_access == 1) {
        mainMenu(client, user_id, user_type);
        return;
    } else if (strcmp(option, "addComment") == 0) {
        addComment(client, user_id, user_type);
        return;
    } else if (strcmp(option, "comments") == 0) {
        showComments(client, user_id, user_type);
        return;
    } else if (strcmp(option, "mainMenu") == 0) {
        mainMenu(client, user_id, user_type);
        return;
    } else {
        showSongs(client, user_id, user_type);
        return;
    }
    return;
}


int findUserType (int user_id) {


    int type;
    char query[500];
    char user_id_char[20];
    bzero(user_id_char, 20);
    sprintf(user_id_char, "%d", user_id);

    bzero(query, 500);
    strcat(query, "select type from user where id = ");
    strcat(query, user_id_char);
    strcat(query, " ;");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user,
                            pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        type = atoi(row[0]);

    mysql_free_result(res);

    return type;
}


void showGenreTop(int client, int user_id, int user_type) {
    char genresToBeShownOnClient[3000];
    bzero(genresToBeShownOnClient, 3000);
    int selectedGenre;
    char selectedGenreChar[20];
    bzero(selectedGenreChar, 20);
    char results[10000];
    bzero(results, 10000);
    int isACorrectId;
 
    char query[300];
    bzero(query, 300);
    strcat(query, "SELECT id, name FROM genre;");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {

        strcat(genresToBeShownOnClient, "ID = ");
        strcat(genresToBeShownOnClient, row[0]);
        strcat(genresToBeShownOnClient, " || NAME = ");
        strcat(genresToBeShownOnClient, row[1]);
        strcat(genresToBeShownOnClient, "\n");
    }
    strcat(genresToBeShownOnClient, "\0");

    mysql_free_result(res);

    if (write(client, genresToBeShownOnClient, 3000) <= 0) {
        raiseClientError(client, 11);
        return;
    }

    // transmite catre client toate genurile muzicale disponibile, 
    // trebuie sa aleaga un gen anume si sa il transmita catre server,
    // deci trebuie sa fac read si sa o procesez.

    if (read(client, &selectedGenre, sizeof(selectedGenre)) <= 0) {
        raiseClientError(client, 12);
        return;
    }

    sprintf(selectedGenreChar, "%d", selectedGenre);

    // verific daca id-ul introdus exista cu adevarat in baza de date

    bzero(query, 300);
    strcat(query, "SELECT count(*) FROM genre WHERE id = ");
    strcat(query, selectedGenreChar);
    strcat(query, ";");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isACorrectId = atoi(row[0]);

    mysql_free_result(res);

    if(write(client, &isACorrectId, sizeof(isACorrectId)) <= 0) {
        raiseClientError(client, 13);
        return;
    }

    bzero(query, 1000);
    strcat(query, "SELECT song.id, song.name, (select count(*) from vote  where song_fk = song.id and upVotes = 1) AS `upVotes`, (select count(*) from vote where song_fk = song.id and downVotes = 1) as `downVotes` from song where exists( select * from songGenre where songGenre.song_Fk=song.id and songGenre.genre_Fk = ");
    strcat(query, selectedGenreChar);
    strcat(query, ") order by (`upVotes` - `downVotes`) DESC, name ASC;");

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
         fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }    

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

     res = mysql_use_result(conn);
     bool hasValues = false;
     
    int i = 1;
    
     
    while ((row = mysql_fetch_row(res)) != NULL) {
        hasValues = true;
        char iChar[20];
        sprintf(iChar, "%d", i);
        int votes = atoi(row[2])-atoi(row[3]);
        char votesChar[100];
        bzero(votesChar,100);
        sprintf(votesChar, "%d", votes);

        strcat(results, iChar);
        strcat(results, ". ");
        strcat(results, row[1]);
        strcat(results, " (");
        strcat(results, row[0]);
        strcat(results, ")     ");
        strcat(results, "Votes: ");
        strcat(results, votesChar);
        strcat(results, "\n");
        i=i+1;
    }


        if(!hasValues) {
            strcat(results, "\nThere are no songs in the database.\n");
        }


        mysql_free_result(res);
        mysql_close(conn);



    strcat(results, "\0");

    if(write(client, results, 10000) <= 0) {
        raiseClientError(client, 20);
        return;
    }
    char option[100];
    bzero(option, 100);

    if (read(client, option, 100) <= 0) {
        raiseClientError(client, 25);
        return;
    }

    

    if(strcmp(option, "voteSong") == 0) {
        voteSong(client, user_id, user_type);
        return;
    } else if (strcmp(option, "showGenreTop") == 0) {
        showGenreTop(client, user_id, user_type);
        return;

    } else if (strcmp(option, "menu") == 0) {
        mainMenu(client);
        return;

    } else {
        mainMenu(client);
        return;
    }
    return;


    return;
}

void showGeneralTop (int client, int user_id, int user_type) {
    char option[100];
    bzero(option, 100);
    char results[10000];
    bzero(results, 10000);
    bool songExists = false;
    char query[1000];
    bzero(query, 1000);

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    bzero(query, 1000);
    strcat(query, "SELECT song.id, song.name, (select count(*) from vote where song_fk = song.id and upVotes = 1) AS `upVotes`, (select count(*) from vote where song_fk = song.id and downVotes = 1) as `downVotes` from song order by (`upVotes` - `downVotes`) DESC, name ASC;");

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
         fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }    

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

     res = mysql_use_result(conn);
     bool hasValues = false;
     
    int i = 1;
    
     
    while ((row = mysql_fetch_row(res)) != NULL) {
        hasValues = true;
        char iChar[20];
        sprintf(iChar, "%d", i);
        int votes = atoi(row[2])-atoi(row[3]);
        char votesChar[100];
        bzero(votesChar,100);
        sprintf(votesChar, "%d", votes);

        strcat(results, iChar);
        strcat(results, ". ");
        strcat(results, row[1]);
        strcat(results, " (");
        strcat(results, row[0]);
        strcat(results, ")     ");
        strcat(results, "Votes: ");
        strcat(results, votesChar);
        strcat(results, "\n");
        i=i+1;
    }


        if(!hasValues) {
            strcat(results, "\nThere are no songs in the database.\n");
        }


        mysql_free_result(res);
        mysql_close(conn);



    strcat(results, "\0");

    if(write(client, results, 10000) <= 0) {
        raiseClientError(client, 20);
        return;
    }

    bzero(option, 100);

    if (read(client, option, 100) <= 0) {
        raiseClientError(client, 25);
        return;
    }



    if (strcmp(option, "exit") == 0) {
        exit(client);
        return;

    } else {
        mainMenu(client);
        return;
    }
    return;

}

void deleteSong(int client, int user_id, int user_type) {
    char option[100];
    bzero(option, 100);
    char results[10000];
    bzero(results, 10000);
    bool songExists = false;
    char query[1000];
    bzero(query, 1000);
    int isAlreadyInBd = 0;

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    bzero(query, 1000);
    strcat(query, "SELECT song.id, song.name from song order by song.name ASC;");

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
         fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }    

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

     res = mysql_use_result(conn);
     bool hasValues = false;
     
    int i = 1;
    
     
    while ((row = mysql_fetch_row(res)) != NULL) {
        hasValues = true;
        char iChar[20];
        sprintf(iChar, "%d", i);

        strcat(results, iChar);
        strcat(results, ". ");
        strcat(results, row[1]);
        strcat(results, " (");
        strcat(results, row[0]);
        strcat(results, ")     ");
        strcat(results, "\n");
        i=i+1;
    }


    if(!hasValues) {
         strcat(results, "\nThere are no songs in the database.\n");
    }


    mysql_free_result(res);
    mysql_close(conn);



    strcat(results, "\0");

    if(write(client, results, 10000) <= 0) {
        raiseClientError(client, 20);
        return;
    }

    bzero(option, 100);

    if (read(client, option, 100) <= 0) {
        raiseClientError(client, 25);
        return;
    }

    bzero(query,1000);
    strcat(query, "select count(*) from song where id = ");
    strcat(query, option);
    strcat(query, ";");


    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server,
                            user, pass, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
        isAlreadyInBd = atoi(row[0]);

    mysql_free_result(res);


    if(write(client, &isAlreadyInBd, sizeof(isAlreadyInBd)) <= 0) {
        raiseClientError(client, 614);
        return;
    }
    if(isAlreadyInBd == 0) {
        voteSong(client, user_id, user_type);
        return;
    }

    bzero(query, 1000);
    strcat(query, "delete from vote where song_fk = ");
    strcat(query, option);
    strcat(query, " ;");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    bzero(query, 1000);
    strcat(query, "delete from comment where song_fk = ");
    strcat(query, option);
    strcat(query, " ;");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    bzero(query, 1000);
    strcat(query, "delete from songGenre where song_fk = ");
    strcat(query, option);
    strcat(query, " ;");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    bzero(query, 1000);
    strcat(query, "delete from song where id = ");
    strcat(query, option);
    strcat(query, " ;");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
 
    mysql_close(conn);
    return;
}

int firstMenu(int client) {
    char response[5];
    bzero(response, 5);

    if(read(client, response, 5) <= 0) {
        raiseClientError(client, 21);
        return 0;
    }

    if(strcmp(response, "y") == 0) {
        login(client);
    } else {
        registerUser(client);
        login(client);
    }


}

int mainMenu(int client, int user_id, int user_type) {

    char option[50];
    bzero(option, 50);


    if(read(client, option, 50) <= 0) {
        raiseClientError(client, 22);
        return 0;
    }


    if(strcmp(option, "showSongs") == 0) {
        showSongs(client,user_id,user_type);
        mainMenu(client,user_id,user_type);

    }

    else if(strcmp(option, "addSong") == 0) {
        addSong(client);
        mainMenu(client,user_id,user_type);

    }

    else if(strcmp(option, "showGeneralTop") == 0) {
        showGeneralTop(client, user_id, user_type);
    }

    else if(strcmp(option, "showGenreTop") == 0) {
        showGenreTop(client, user_id, user_type);
    }

    else if(strcmp(option, "deleteSong") == 0 && user_type == 0) {
        deleteSong(client, user_id, user_type);
        mainMenu(client, user_id, user_type);
    }

    else if(strcmp(option, "disableVote") == 0 && user_type == 0) {
        disableVote(client);
        mainMenu(client, user_id, user_type);
    }

    else if(strcmp(option, "addGenre") == 0 && user_type == 0) {

        addGenre(client);
        mainMenu(client,user_id,user_type);
    }

    else {
        exit(client);
    }



}

int main () {

    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("%sSOCKET ERROR.\n", RED);
        return errno;
    }

    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        printf("%sBIND ERROR.\n", RED);
        return errno;
    }

    if (listen (sd, 5) == -1) {
        printf("%sLISTEN ERROR.\n", RED);
        return errno;
    }

    printf ("%sLISTENING ON PORT %d...\n", GREEN, PORT);
    fflush (stdout);

    while (1) {
        int client;
        int length = sizeof (from);

        int pid;

        client = accept (sd, (struct sockaddr *) &from, &length);
        if(pid = fork() ==0) {

            if (client < 0) {
                printf("%sAN ERROR OCCURRED DURING CONNECTING WITH THE CLIENT.\n", RED);
                continue;
            } else {
                printf("%sONE CLIENT IS CONNECTED\n", BLUE);
            }

            int user_id;
            int user_type;
            user_id = firstMenu(client);
            user_type=findUserType(user_id);

            mainMenu(client, user_id, user_type);
        }

    }
}