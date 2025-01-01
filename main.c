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
#include <time.h>
#define MAX_SIZE 100

struct scores {
    char name [50];
    int total_gold;
    int total_score;
    int total_games;
    time_t lastgame;
};

struct scores ranks [MAX_SIZE];
int score_count = 0;

bool valid_pass(char* password) {
    if (strlen(password) < 7) {
        return false;
    }
    bool upper = false, lower = false, num = false;
    for (int i = 0; password[i]; i++) {
        if (islower(password[i])) lower = true;
        if (isupper(password[i])) upper = true;
        if (isdigit(password[i])) num = true;
    }
    return upper && lower && num;
}

bool valid_email(char* email) {
    int at_count = 0, dot_count = 0;
    char *at = NULL, *dot = NULL;
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') {
            at_count++;
            at = &email[i];
        } else if (email[i] == '.') {
            dot_count++;
            dot = &email[i];
        }
    }
    return (at_count == 1 && dot && at < dot && *(at + 1) != '.' && *(dot + 1) != '\0');
}

void save_info(char* name, char* password, char* email) {
    FILE *file = fopen("user_data.txt", "a");
    if (file) {
        fprintf(file, "Username: %s\nPassword: %s\nEmail: %s\n", name, password, email);
        fclose(file);
    } else {
        printw("Error saving data!\n");
        refresh();
    }
}

bool unique_username(char* username) {
    FILE *file = fopen("user_data.txt", "r");
    if (!file) return true;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Username: ", 10) == 0) {
            char stored_user[50];
            sscanf(line, "Username: %s", stored_user);
            if (strcmp(stored_user, username) == 0) {
                fclose(file);
                return false;
            }
        }
    }
    fclose(file);
    return true;
}

bool validate_username(char* username, char* password) {
    FILE *file = fopen("user_data.txt", "r");
    if (!file) return false;

    char line[100], stored_user[50], stored_pass[50];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Username: ", 10) == 0) {
            sscanf(line, "Username: %s", stored_user);
            if (strcmp(stored_user, username) == 0) {
                fgets(line, sizeof(line), file);
                sscanf(line, "Password: %s", stored_pass);
                fclose(file);
                return strcmp(stored_pass, password) == 0;
            }
        }
    }
    fclose(file);
    return false;
}

void get_info(char* prompt, char* dest, int max_length, int pass) {
    int ch, i = 0;
    echo();
    if (pass) noecho();

    printw("%s", prompt);
    printw("\n");

    refresh();

    while ((ch = getch()) != '\n' && i < max_length - 1) {
        dest[i++] = (char)ch;
        refresh();
        if (pass) printw("*");
        refresh();
    }
    dest[i] = '\0';
    noecho();
}

void forgot_pass (char* username) {
    char line [50], stored_user [50], stored_pass [50];
    FILE * file = fopen("user_data.txt", "r");
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Username: ", 10) == 0) {
            sscanf(line, "Username: %s", stored_user);
            if (strcmp(stored_user, username) == 0) {
                fgets(line, sizeof(line), file);
                if (strncmp(line, "Password: ", 10) == 0) {
                    sscanf(line, "Password: %s", stored_pass);
                    printw("Your password is %s\n", stored_pass);
                    break;
                }
            }
        }
    }
}

void play_menu_choices (int highlight) {
    attron(COLOR_PAIR(1));
    printw("Play Menu: \n");
    attroff(COLOR_PAIR(1));

    char* options[] = {"Login", "Register", "Forgot Password", "Exit"};
    for (int i = 0; i < 4; i++) {
        if (i == highlight) {
            attron(A_REVERSE);
            printw("> %s\n", options[i]);
            attroff(A_REVERSE);
        } else {
            printw("  %s\n", options[i]);
        }
    }
}

void play_menu () {
    char username[50], password[50], email[50];
    int ch;
    int choice = 0;
    
    while (1) {
        clear();
        play_menu_choices(choice);
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // Login
                clear();
                get_info("Enter username: ", username, 50, 0);

                get_info("Enter password: ", password, 50, 1);
                
                if (validate_username(username, password)) {
                    printw("\nLogin successful!");
                } else {
                    printw("\nInvalid password.");
                }
                refresh();
                getch();
            } else if (choice == 1) { // Register
                while (1) {
                    clear();
                    get_info("Enter username: ", username, 50, 0);
                    printw("\n");
                    if (!unique_username(username)) {
                        printw("Username already exists!\n");
                        refresh();
                        getch();
                        continue;
                    }
                    break;
                }
                while (1) {
                    get_info("Enter password: ", password, 50, 1);
                    if (!valid_pass(password)) {
                        printw("Invalid password!\n");
                        refresh();
                        getch();
                        continue;
                    }
                    break;
                }
                while (1) {
                    get_info("Enter email: ", email, 50, 0);
                    if (!valid_email(email)) {
                        printw("Invalid email!\n");
                        refresh();
                        getch();
                        continue;
                    }
                    break;
                }
                save_info(username, password, email);
                printw("Registration successful!\n");
                refresh();
                getch();
            } else if ( choice == 2) { // forgot pass
                get_info("Enter your username: ", username, 50, 0);
                refresh();
                printw("Do you pinky promise it's you, %s? :(", username);
                getch();
                forgot_pass(username);
            } else if (choice == 3) { //exit
                clear();
                refresh();
                break;
            }
        }
        
    }
}

void main_menu_choice (int highlight) {
    
    attron(COLOR_PAIR(1));
    printw("Main Menu: \n");
    attroff(COLOR_PAIR(1));

    char* options[] = {"Play", "Hall of Fame"};
    for (int i = 0; i < 2; i++) {
        if (i == highlight) {
            attron(A_REVERSE);
            printw("> %s\n", options[i]);
            attroff(A_REVERSE);
        } else {
            printw("  %s\n", options[i]);
        }
    }
}
void load_hall () {
    FILE * file = fopen("hall_of_fame.txt", "r");
    if(!file) return;
    
    score_count = 0;
    while(fscanf(file, "%s %d %d %d %ld", ranks[score_count].name, &ranks[score_count].total_score, &ranks[score_count].total_gold, &ranks[score_count].total_games, &ranks[score_count].lastgame) == 5 ) {
        score_count++;
    }
    fclose(file);
}

void save_scores () {
    FILE * file = fopen("hall_of_fame.txt", "w");
    if (!file) return;
    
    for ( int i = 0; i < score_count; i ++) {
        fprintf(file , "%s %d %d %d %ld", ranks[i].name, ranks[i].total_score, ranks[i].total_gold, ranks[i].total_games, ranks[i].lastgame);
    }
    fclose(file);
}

void get_score (char* name, int score, int gold) {
    time_t current_time = time(NULL);
    int found = 0;
    for ( int i = 0; i < score_count; i ++) {
        if (strcmp(ranks[i].name, name)== 0) {
            ranks[i].total_score += score;
            ranks[i].total_gold += gold;
            ranks[i].total_games ++;
            ranks[i].lastgame = current_time;
            found = 1;
            break;
        }
    }
    
    if( !found && score_count < MAX_SIZE) {
        strcpy(ranks[score_count].name, name);
        ranks[score_count].total_score = score;
        ranks[score_count].total_gold = gold;
        ranks[score_count].total_games = 1;
        ranks[score_count].lastgame = current_time;
        score_count ++;
    }
    
}

void hall_of_fame () {
    printw("HALL OF FAME\n");
    printw("Rank Name      Score  Gold  Games Played  Experience\n");
    
    for ( int i = 0; i < score_count; i ++) {
        char time [20];
        struct tm *tm_info = localtime(&ranks[i].lastgame);
        strftime(time, 20, "%Y-%m-%d", tm_info);
        
        printw("%2d   %-10s %5d %5d %5d %s", i + 1, ranks[i].name,
               ranks[i].total_score, ranks[i].total_gold, ranks[i].total_games, time);
    }
    printw("Press any key to exit.");
    refresh();
    getch();
}

void main_menu (){
    char username[50], password[50], email[50];
    int ch, choice = 0;
    
    while (1) {
        clear();
        main_menu_choice(choice);
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        if (ch == KEY_DOWN && choice < 2) choice++;
        
        if (ch == '\n') {
            if (choice == 0) { // Play
                play_menu();
            } else if (choice == 1) { // Hall of Fame
                while (1) {
                    clear();
                    load_hall();
                    hall_of_fame();
                    break;
                }
            }
        }
    }
}

void load_welcome_page () {
    clear();
    printw("Welcome to the Dungeon of Doom!\n");
    refresh();
    
}
int main() {

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);
    }
    load_welcome_page();
    main_menu();
    
    getch();
    endwin();
    return 0;
}
