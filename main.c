//
//  main.c
//  rogue (parisa's version)
//
//  Created by parisa on 12/22/24.
//
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool valid_pass (char* password) {
    
    if (strlen(password) < 7) {
        return false;
    }
    bool upper = false;
    bool lower = false;
    bool num = false;
    
    for ( int i = 0; i < strlen(password); i ++) {
        if (islower(password[i])) lower = true;
        if (isupper(password[i])) upper = true;
        if (isnumber(password[i])) num = true;
    }
    return upper && lower && num;
}

bool valid_email (char* email) {
    char* at = strchr(email, '@');
    char* dot = strchr(email, '.');

    if ( !at || !dot || at > dot) return false;
    
    if (at == email || dot == email || at + 1 == dot || *(dot + 1) == '\0') return false;
    
    return true;
}

void save_info (char* name, char* password, char* email) {
    FILE *file = fopen("user_data.txt", "a");
    
    if ( file) {
        fprintf(file, "Username: %s\n", name);
        fprintf(file, "Password: %s\n", password);
        fprintf(file, "Email: %s\n", email);
        fclose(file);
    } else {
        printf("error saving data!\n");
        refresh();
        
    }
}

bool unique_username( char *username) {
    FILE *file = fopen("user_data.txt", "r");
    if (file == NULL) {
        return true;
    }
    
    char line[50];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Username: ", 10) == 0) {
            if (strncmp(line + 10, username, strlen(username)) == 0) {
                fclose(file);
                return false;
            }
        }
    }
    
    fclose(file);
    return true;
}

void get_info (char * prompt, char * dest, int pass) {
    int ch, i = 0;
    echo();
    if (pass) {
        noecho();
    }

    printw("%s", prompt);
    refresh();
    
    while ((ch = getch()) != '\n' ) {
        
        dest[i ++] = (char)ch;
        if(pass) {
            printw("*");
        } else {
            //printw("%c", ch);
        }
        refresh();
    }
    dest[i] = '\0';
    noecho();
}

int main(int argc, const char * argv[]) {

    char username [10], password [20], email[50];
    
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
   // WINDOW * win = newwin(20,20, 10,20);
    
    //box(win, 0, 0);
    
    mvprintw(10,10 , "Welcome to the Dungeon of Doom!\n");
    refresh();
    
    while (1) {
        get_info("Enter your username: ", username, 0);
        printw("\n");
        if(unique_username(username)) break;
        else {
            printw("Username already exists! Please enter a new one :D\n");
            refresh();
        }
    }
    
    while (1) {
        get_info("Enter your password (7 or more characters): ", password, 1);
        printw("\n");

        if(valid_pass(password)) break;
        else {
            printw("Password is not valid! Try again :D\n");
            refresh();
        }
    }
    
    while (1) {
        get_info("Enter a valid Email address: ", email, 0);
        printw("\n");

        if(valid_email(email)) break;
        else {
            printw("Email is not valid! Try again :D\n");
            
            refresh();
        }
    }
    
    save_info(username, password, email);
    printw("Entered successfully!\n");
    refresh();
    getchar();
    
    endwin();
    return 0;
}

