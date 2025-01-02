//
//  main.c
//  rogue (parisa's version)
//
//  Created by parisa on 12/22/24.
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#define MAX_SIZE 100
#define HEIGHT 25
#define WIDTH 50
#define ROOM_MIN_SIZE 4
#define ROOM_MAX_SIZE 10
#define ROOM_COUNT 6
#define CORRIDOR_VISIBLE 4
struct ROOM {
    int x, y, height, width;
};

char map [HEIGHT][WIDTH];
bool visible[HEIGHT][WIDTH];
bool visited[HEIGHT][WIDTH];

struct scores {
    char name [50];
    int total_gold;
    int total_score;
    int total_games;
    time_t lastgame;
};

struct scores ranks [MAX_SIZE];
int score_count = 0;

int hero_color = 1;

void pick_one (int highlight, char* menu_name, char * options[], int n) {
    attron(COLOR_PAIR(1));
    printw("%s: \n", menu_name);
    attroff(COLOR_PAIR(1));

    for (int i = 0; i < n; i++) {
        if (i == highlight) {
            attron(A_REVERSE);
            printw("> %s\n", options[i]);
            attroff(A_REVERSE);
        } else {
            printw("  %s\n", options[i]);
        }
    }

}

void difficulty () {
    int ch;
    int choice = 0;
    char menu_name [50] = {"Choose difficulty"};
    char * options[] = {"Hard", "Medium", "Hard", "Exit"};
    
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 3);
        
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // hard
                getch();
                refresh();
                break;
            } else if ( choice == 1) { //medium
                getch();
                refresh();
                break;
            } else if ( choice == 2) {//easy
                getch();
                refresh();
                break;
            }
        }
    }
}

void init_colors() {
    start_color();
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
}

void customize_menu () {
    int ch;
    int choice = 0;
    char menu_name [50]= {"Customize"};
    char * options[] = {"PINK", "YELLOW", "BLUE", "RED", "CYAN"};
    
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 5);
        
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // pink
                hero_color = 6;
                break;
            } else if (choice == 1) {//yellow
                hero_color = 5;
                break;
            } else if (choice == 2) {//blue
                hero_color = 4;
                break;
            } else if (choice == 1) {//red
                hero_color = 2;
                break;
            } else if (choice == 1) {//cyan
                hero_color = 3;
                break;
            }
        }
    }
}

bool room_overlap (struct ROOM r1, struct ROOM r2) {
    return !(r1.x + r1.width < r2.x || r1.x > r2.x + r2.width || r1.y + r1.height < r2.y || r1.y > r2.y + r2.height);
}

void init_map () {
    
    for ( int i = 0; i < HEIGHT; i ++) {
        for (int j = 0; j < WIDTH; j ++) {
            map[i][j] = ' ';
            visible[i][j] = false;
        }
    }
}

void add_room (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
        for (int x = room.x; x < room.x + room.width; x++) {
            if (y == room.y || y == room.y + room.height - 1) {
                map[y][x] = '-';
            } else if (x == room.x || x == room.x + room.width - 1) {
                map[y][x] = '|';
            } else {
                map[y][x] = '.';
            }
        }
    }
}

void corridor (int x1, int y1, int x2, int y2) {
    int start_x = x1;
    int start_y = y1;
    bool door_placed = false;
    
    if (rand() % 2) {
        while (x1 != x2) {
            if (!door_placed && ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            x1 += (x2 > x1) ? 1 : -1;
        }
        
        while (y1 != y2) {
            if (!door_placed && ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            y1 += (y2 > y1) ? 1 : -1;
        }
    } else {
        
        while (y1 != y2) {
            if (!door_placed && ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            y1 += (y2 > y1) ? 1 : -1;
        }
        
        while (x1 != x2) {
            if (!door_placed && ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            x1 += (x2 > x1) ? 1 : -1;
        }
    }
}

void reveal_corridor (int px, int py) {
    
    for (int dy = -CORRIDOR_VISIBLE; dy <= CORRIDOR_VISIBLE; dy++) {
        for (int dx = -CORRIDOR_VISIBLE; dx <= CORRIDOR_VISIBLE; dx++) {
            int nx= dx + px;
            int ny= dy + py;
            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                if (map[ny][nx] == '#' || map[ny][nx] == '+') {
                    visible[ny][nx] = true;
                }
            }
        }
    }
}

void reveal_room(struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
        for (int x = room.x; x < room.x + room.width; x++) {
            visible[y][x] = true;
        }
    }
}

void player_in_room (int px, int py, struct ROOM rooms[], int room_count) {
    for (int i = 0; i < room_count; i++) {
        struct ROOM r = rooms[i];
        if (px >= r.x && px < r.x + r.width &&
            py >= r.y && py < r.y + r.height) {
            reveal_room(r);
        }
    }
    reveal_corridor(px, py);
}

void render_map() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if(visible[i][j]) {
                mvaddch(i, j, map[i][j]);
            } else {
                mvaddch(i, j, ' ');
            }
        }
    }
}

void generate_map (){
    struct ROOM rooms [ROOM_COUNT];
    int room_count = 0;
    
    init_map();
    while (room_count < ROOM_COUNT) {
        struct ROOM new_room;
        new_room.width = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1);
        new_room.height = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1);
        new_room.x = rand() % (WIDTH - new_room.width - 1);
        new_room.y = rand() % (HEIGHT - new_room.height - 1);
        
        bool overlap = false;
        
        for ( int i = 0; i < room_count ; i ++ ){
            if (room_overlap(new_room, rooms[i])) {
                overlap = true;
                break;
            }
        }
        
        if (!overlap) {
            add_room(new_room);
            
            if (room_count > 0) {
                corridor (rooms[room_count - 1].x + rooms[room_count - 1].width / 2, rooms[room_count - 1].y + rooms[room_count - 1].height / 2, new_room.x + new_room.width / 2, new_room.y + new_room.height / 2);
            }
            rooms[room_count++] = new_room;
        }
    }
    
    int px = rooms[0].x + 1, py = rooms[0].y + 1;
    player_in_room(px, py, rooms, room_count);
    
    int ch;
    
    while (1) {
        clear();
        render_map();
        init_colors();
        attron(COLOR_PAIR(hero_color));
        mvaddch(py, px, '@');
        attroff(COLOR_PAIR(hero_color));
        refresh();
        
        ch = getch();
        if (ch == 'q') break;
        int nx = px, ny = py;
        
        if (ch == KEY_UP) ny--;
        else if (ch == KEY_DOWN) ny++;
        else if (ch == KEY_LEFT) nx--;
        else if (ch == KEY_RIGHT) nx++;
        
        if (map[ny][nx] == '.' || map[ny][nx] == '#' || map[ny][nx] == '+') {
            px = nx;
            py = ny;
            player_in_room(px, py, rooms, room_count);
        }
    }
           
}
           


void setting_menu (){
    int ch;
    int choice = 0;
    char menu_name [50] = {"Setting"};
    char * options[] = {"Difficulty", "Customize", "Music", "Exit"};
    
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 4);
        
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { //difficulty
                clear();
                difficulty();
                refresh();
            } else if (choice == 1) { // customize
                clear();
                customize_menu();
                
                refresh();
            } else if (choice == 2) { //Music
                clear();
                refresh();

            } else if (choice == 3) { //Exit
                clear();
                refresh();
                break;
            }
        }
    }
}
        
void start_game_menu () {
    int ch;
    int choice = 0;
    char * options [] = {"New Game", "Continue Previous Game", "Game Settings", "Exit"};
    char menu_name[50] = {"Game Menu"};
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 4);
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // New Game
                init_map();
                generate_map();
                //render_map();
                getch();
                refresh();
            } else if (choice == 1) {// Prev Game (make the menu later)
                getch();
                refresh();
            } else if (choice == 2) {//Settings
                setting_menu();
                getch();
                refresh();
            } else if (choice == 3) {// Exit
                getch();
                refresh();
            }
        }
        
    }
}

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

void play_menu () {
    char username[50], password[50], email[50];
    int ch;
    int choice = 0;
    char menu_name [50] = {"Play Menu"};
    char * options [] = { "Login", "Register", "Forgot Password", "Exit"};
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 4);
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
                    start_game_menu();
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
    char menu_name [50] = {"Main Menu"};
    char * options [] = { "Play", "Hall of Fame"};
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 2);
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
    getch();
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    srand(time(NULL));
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    
    if (has_colors()) {
        init_colors();
    }
    
    load_welcome_page();
    main_menu();
    
    getch();
    endwin();
    return 0;
}


