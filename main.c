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
#define CORRIDOR_VISIBLE 1
#define MAX_PILLAR 3
#define MAX_TRAP 3
#define LOCKED_PASS_LEN 4
#define PASS_TIMEOUT 30
//403170933
struct ROOM {
    int x, y, height, width;
};

struct scores {
    char name [50];
    int total_gold;
    int total_score;
    int total_games;
    time_t lastgame;
};

struct locked_door {
    int x, y;
    int state;
};

struct secret_door {
    int x, y;
    int state;
};

struct trap {
    int x, y;
    int state;
};

// structs

char map [HEIGHT][WIDTH];
bool visible[HEIGHT][WIDTH];
bool visited[HEIGHT][WIDTH];
bool trap_visible[HEIGHT][WIDTH];


struct scores ranks [MAX_SIZE];
int score_count = 0;

int hero_color = 1;
char password[LOCKED_PASS_LEN + 1];
time_t password_show_time = 0;

struct locked_door locked [50];
int locked_door_count = 0;

struct secret_door hiddens [50];
int secret_door_count = 0;

struct trap traps [50];
int traps_count = 0;

int level = 1;
bool master_key [5] = {false};
bool first_key [5] = {true};
bool master_keys_broken [5] = {false};
//global stuff

void generate_map ();

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


void messages(char *what_happened) {
    // Clear the line before printing the new message
    clear();              // Clears the screen or use clrtoeol() for just clearing the line
    move(0, 0);           // Move cursor to the top-left corner (optional, if needed)

    if (strcmp(what_happened, "key broke") == 0) {
        printw("The Master Key breaks.\n");
    } else if (strcmp(what_happened, "fix key") == 0) {
        printw("Would you like to mend two broken Master Keys to enter? (y/n)\n");
    } else if (strcmp(what_happened, "key fixed") == 0) {
        printw("Two Master Keys have been mended. You can enter.\n");
    } else if (strcmp(what_happened, "master key found") == 0) {
        attron(COLOR_PAIR(5));
        printw("You picked up a Master Key!\n");
        attroff(COLOR_PAIR(5));
    }

    refresh();  // Refresh the screen to show the updated message
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
    init_pair(9, COLOR_GREEN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(8, COLOR_WHITE, COLOR_RED);
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

void cheat_code_M () {
    
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

void door_fix(struct ROOM room) {

    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x] == '+') {
            if (map[y][room.x - 1] != '#') {
                map[y][room.x] = '|';
            }
        } else {
            if (map[y][room.x - 1] == '#') {
                map[y][room.x] = '+';
            }
        }
    }

    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x + room.width - 1] == '+') {
            if (map[y][room.x + room.width] != '#') {
                map[y][room.x + room.width - 1] = '|';
            }
        } else {
            if (map[y][room.x + room.width] == '#') {
                map[y][room.x + room.width - 1] = '+';
            }
        }
    }

    for (int x = room.x; x < room.x + room.width; x++) {
        if (map[room.y][x] == '+') {
            if (map[room.y - 1][x] != '#') {
                map[room.y][x] = '-';
            }
        } else {
            if (map[room.y - 1][x] == '#') {
                map[room.y][x] = '+';
            }
        }
    }

    for (int x = room.x; x < room.x + room.width; x++) {
        if (map[room.y + room.height - 1][x] == '+') {
            if (map[room.y + room.height][x] != '#') {
                map[room.y + room.height - 1][x] = '-';
            }
        } else {
            if (map[room.y + room.height][x] == '#') {
                map[room.y + room.height - 1][x] = '+';
            }
        }
    }
}

void add_pillar (struct ROOM room) {
    int pillar_count = 0;
    
    for (int y = room.y; y < room.y + room.height; y++) {
        if (pillar_count >= MAX_PILLAR) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            if (pillar_count >= MAX_PILLAR) break;
            if (rand () % 30 == 0 && map[y][x] == '.' && (map[y][x+ 1] != '+' && map[y][x-1] != '+' && map[y+1][x] != '+' && map[y-1][x] != '+')) {
                map[y][x] = 'O';
                pillar_count++;
            }
        }
    }
}

void add_trap (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
        if (traps_count >= MAX_TRAP) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            if (traps_count >= MAX_TRAP) break;
            if (rand () % 30 == 0 && map[y][x] == '.') {
                traps[traps_count].x = x;
                traps[traps_count].y = y;
                traps[traps_count].state = 0;
                traps_count++;
            }
        }
    }
}

void add_stairs (struct ROOM room) {
    bool stairs_placed = false;
    for (int y = room.y; y < room.y + room.height; y++) {
        if (stairs_placed) return;
        for (int x = room.x; x < room.x + room.width; x++) {
            if (stairs_placed) return;
            if (rand () % 20 == 0 && map[y][x] == '.') {
                attron(COLOR_PAIR(9));
                map[y][x] = '<';
                attroff(COLOR_PAIR(9));
                stairs_placed = true;
                refresh();

            }
        }
    }
    
    if (!stairs_placed) {
        int center_x = room.x + room.width / 2;
        int center_y = room.y + room.height / 2;
        map[center_y][center_x] = '<';
    }
}

void reveal_door (int ny, int nx) {
    int which_door = 0;
    for ( int i = 0; i < secret_door_count; i ++) {
        if(hiddens[i].x == nx && hiddens[i].y == ny) {
            which_door = i;
            hiddens[i].state = 1;
        }
    }
    if (hiddens[which_door].state) {
        attron(COLOR_PAIR(2));
        map[ny][nx] = '?';
        attroff(COLOR_PAIR(2));
        refresh();
    }
}

void reveal_trap (int ny, int nx) {
    int which_trap = 0;
    for ( int i = 0; i < traps_count; i ++) {
        if(traps[i].x == nx && traps[i].y == ny) {
            which_trap = i;
            traps[i].state = 1;
        }
    }
    
    if (traps[which_trap].state) {
        attron(COLOR_PAIR(2));
        map[ny][nx] = '^';
        attroff(COLOR_PAIR(2));
        refresh();
    }
    
}

void add_hidden_door (struct ROOM room ) {

    
    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x] == '+') {
            hiddens[secret_door_count].x = room.x;
            hiddens[secret_door_count].y = y;
            hiddens[secret_door_count].state = 0;
            secret_door_count++;
            attron(COLOR_PAIR(5));
            map[y][room.x] = '|';
            attroff(COLOR_PAIR(5));
            refresh();
        }
    }
    
    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x + room.width - 1] == '+') {
            hiddens[secret_door_count].x= room.x + room.width - 1;
            hiddens[secret_door_count].y = y;
            hiddens[secret_door_count].state = 0;
            secret_door_count++;
            attron(COLOR_PAIR(5));
            map[y][room.x + room.width - 1] = '|';
            attroff(COLOR_PAIR(5));
            refresh();
        }
    }
    
    for (int x = room.x; x < room.x + room.width; x++) {
        if (map[room.y][x] == '+') {
            hiddens[secret_door_count].x= x;
            hiddens[secret_door_count].y= room.y;
            hiddens[secret_door_count].state = 0;
            secret_door_count++;
            attron(COLOR_PAIR(5));
            map[room.y][x] = '-';
            attroff(COLOR_PAIR(5));
            refresh();
        }
    }
        
        for (int x = room.x; x < room.x + room.width; x++) {
            if (map[room.y + room.height - 1][x] == '+') {
                hiddens[secret_door_count].x= x;
                hiddens[secret_door_count].y= room.y + room.height - 1;
                hiddens[secret_door_count].state = 0;
                secret_door_count++;
                attron(COLOR_PAIR(5));
                map[room.y + room.height - 1][x] = '-';
                attroff(COLOR_PAIR(5));
                refresh();
            }
        }
    
    
}

void add_master_key (struct ROOM room) {
    bool key_placed = false;
    for (int y = room.y; y < room.y + room.height; y++) {
        if (key_placed) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            if (key_placed) break;
            if (rand () % 20 == 0 && map[y][x] == '.') {
                map[y][x] = '*';
                key_placed = true;
                
            }
        }
    }
    
    if (!key_placed) {
        int center_x = room.x + room.width / 3;
        int center_y = room.y + room.height / 3;
        map[center_y][center_x] = '*';
    }
    
}
        
void corridor (int x1, int y1, int x2, int y2) {
    bool door_placed = false;
    
    if (rand() % 2) {
        while (x1 != x2) {
            if (( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                //door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            x1 += (x2 > x1) ? 1 : -1;
        }
        
        while (y1 != y2) {
            if (( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                //door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            y1 += (y2 > y1) ? 1 : -1;
        }
    } else {
        
        while (y1 != y2) {
            if (!door_placed || ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            y1 += (y2 > y1) ? 1 : -1;
        }
        
        while (x1 != x2) {
            if (!door_placed || ( map[y1][x1] == '|' || map[y1][x1] == '-'))  {
                map[y1][x1] = '+';
                door_placed = true;
            }  else if ( map[y1][x1] == ' ')  {
                map[y1][x1] = '#';
            }
            x1 += (x2 > x1) ? 1 : -1;
        }
    }
}

void generate_pass (char *password) {
    for (int i = 0; i < LOCKED_PASS_LEN; i++) {
        password[i] = '0' + rand() % 10;
    }
    password[LOCKED_PASS_LEN] = '\0';
}

void show_password(int px, int py) {
    if (password_show_time == 0) {
        generate_pass(password);
        password_show_time = time(NULL);
    }

    int win_width = 30;
    int win_height = 8;
    WINDOW *password_win = newwin(win_height, win_width, py - 2, px + 2);
    
    wbkgd(password_win, COLOR_PAIR(7));
    box(password_win, 0, 0);

    int time_passed = (int)difftime(time(NULL), password_show_time);

    char msg1[] = "SHH! Don't tell anyone!";
    int msg1_len = strlen(msg1);
    int start_col1 = (win_width - msg1_len) / 2;

    char msg2[] = "Password: ";
    int msg2_len = strlen(msg2);
    int start_col2 = (win_width - msg2_len - strlen(password)) / 2;

    while (time_passed < PASS_TIMEOUT) {
        mvwprintw(password_win, 3, start_col1, "%s", msg1);

        mvwprintw(password_win, 4, start_col2, "%s%s", msg2, password);
        
        time_passed = (int)difftime(time(NULL), password_show_time);

        napms(100);
        wrefresh(password_win);
    }

    password_show_time = 0;
    delwin(password_win);
}



int lock_pass_input(int px, int py) {
    int which_door = 0;
    for (int c = 0; c < locked_door_count; c ++ ) {
        if (locked[c].x == px && locked[c].y == py) {
            which_door = c;
        }
    }
    
    char is_pass[5];
    int win_width = 30;
    int win_height = 8;
    WINDOW *password_win = newwin(win_height, win_width, py - 2, px + 2);
    
    wbkgd(password_win, COLOR_PAIR(7));
    box(password_win, 0, 0);
    
    echo();
    char msg1 [] = "Knock Knock! Who's there?";
    int msg1_len = strlen(msg1);
    int start_col1 = (win_width - msg1_len) / 2;

    char msg2 [] = "The password, please!: ";
    int msg2_len = strlen(msg2
                          );
    int start_col2 = (win_width - msg2_len) / 2;
    mvwprintw(password_win, 3, start_col1, "%s", msg1);
    mvwprintw(password_win, 4, start_col2, "%s", msg2);

    wrefresh(password_win);
    
    mvwgetstr(password_win, 5, start_col2, is_pass);
    noecho();

    size_t len = strlen(is_pass);
    if (len > 0 && is_pass[len - 1] == '\n') {
        is_pass[len - 1] = '\0';
    }


    if (strcmp(password, is_pass) == 0 ) {
        locked[which_door].state = 1;
        char msg3 [] = "Aha! The door swings open for you!";
        int msg3_len = strlen(msg3);
        int start_col3 = (win_width - msg3_len) / 2;
        
        mvwprintw(password_win, 3, start_col3, "%s", msg3);
        wrefresh(password_win);
        delwin(password_win);
        attron(COLOR_PAIR(2));
        mvaddch(py, px, '@');
        map[py][px] = '@';
        attroff(COLOR_PAIR(2));
        refresh();
    } else {
        char msg4 [] = "Wrong password. Nice try, though!";
        int msg4_len = strlen(msg4);
        int start_col4 = (win_width - msg4_len) / 2;
        
        mvwprintw(password_win, 3, start_col4, "%s", msg4);
        wrefresh(password_win);
        delwin(password_win);
    }
    return which_door;
}

void locked_door (struct ROOM room) {
    int door_x [100], door_y [100];
    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x] == '+' || map[y][room.x] == '?') {
            locked[locked_door_count].x = room.x;
            locked[locked_door_count].y = y;
            locked[locked_door_count].state = 0;
            locked_door_count++;
            attron(COLOR_PAIR(2));
            map[y][room.x] = '@';
            attroff(COLOR_PAIR(2));
           // refresh();
        }
    }
    
    for (int y = room.y; y < room.y + room.height; y++) {
        if (map[y][room.x + room.width - 1] == '+' || map[y][room.x + room.width - 1] == '?') {
            locked[locked_door_count].x  = room.x + room.width - 1;
            locked[locked_door_count].y = y;
            locked[locked_door_count].state = 0;
            locked_door_count++;
            attron(COLOR_PAIR(2));
            map[y][room.x + room.width - 1] = '@';
            attroff(COLOR_PAIR(2));
            //refresh();
        }
    }
    
    for (int x = room.x; x < room.x + room.width; x++) {
        if (map[room.y][x] == '+' || map[room.y][x] == '?') {
            locked[locked_door_count].x = x;
            locked[locked_door_count].y = room.y;
            locked[locked_door_count].state = 0;
            locked_door_count++;
            attron(COLOR_PAIR(2));
            map[room.y][x] = '@';
            attroff(COLOR_PAIR(2));
            //refresh();
        }
    }
    
    for (int x = room.x; x < room.x + room.width; x++) {
        if (map[room.y + room.height - 1][x] == '+' || map[room.y + room.height - 1][x] == '?') {
            locked[locked_door_count].x  = x;
            locked[locked_door_count].y = room.y + room.height - 1;
            locked[locked_door_count].state = 0;
            locked_door_count++;
            attron(COLOR_PAIR(2));
            map[room.y + room.height - 1][x] = '@';
            attroff(COLOR_PAIR(2));
            //refresh();
        }
    }

    
    int hint_x, hint_y;
    do {
        hint_x = room.x + 1 + rand() % (room.width - 2);
        hint_y = room.y + 1 + rand() % (room.height - 2);
    } while (map[hint_y][hint_x] != '.');
    
    attron(COLOR_PAIR(5));
    map[hint_y][hint_x] = '&';
    attroff(COLOR_PAIR(5));
    refresh();
    
    
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

void new_level () {
    init_map();
    generate_map();
    level++;
}

void stair_activated (char stair) {
    if ( stair == '<') {
        new_level();
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
            add_pillar(new_room);
            add_trap(new_room);
            
            if (room_count > 0) {
                corridor (rooms[room_count - 1].x + (rooms[room_count - 1].width - 1) / 2, rooms[room_count - 1].y + (rooms[room_count - 1].height -1) / 2, new_room.x + (new_room.width  -1)/ 2, new_room.y + (new_room.height -1) / 2);
                door_fix(new_room);
              // if ( rand () % 6 == 0) locked_door(new_room);
                if ( rand () % 6 == 0) add_hidden_door(new_room);
                
            }
            rooms[room_count++] = new_room;
            
        }
    }
    
    int room_with_stairs = rand () %6;
    int room_with_key = rand () %6;
    add_stairs(rooms[room_with_stairs]);
    add_master_key(rooms[room_with_key]);
    if (rand() % 4 == 0) locked_door(rooms[0]);
    int px = rooms[0].x + 1, py = rooms[0].y + 1;
    player_in_room(px, py, rooms, room_count);
    
    int ch;
    
    while (1) {
        curs_set(0);
        clear();
        render_map();
        init_colors();
        if (hero_color == 1) attron(COLOR_WHITE);
        else attron(COLOR_PAIR(hero_color));
        mvaddch(py, px, '@');
        if (hero_color == 1) attroff(COLOR_WHITE);
        else attroff(COLOR_PAIR(hero_color));
        refresh();
        
        ch = getch();
        if (ch == 'q') break;
        int nx = px, ny = py;
        
        if (ch == KEY_UP) ny--;
        else if (ch == KEY_DOWN) ny++;
        else if (ch == KEY_LEFT) nx--;
        else if (ch == KEY_RIGHT) nx++;
        
        if ( map[ny][nx] == '#' || map[ny][nx] == '+' || map[ny][nx] == '^' || map[ny][nx] == '?') {
            px = nx;
            py = ny;
            player_in_room(px, py, rooms, room_count);
        } else if (map[ny][nx] == '<') {
            char enter = getch();
            stair_activated(enter);
            px = nx;
            py = ny;
        }
        else if (map[ny][nx] == '@') {
            bool master = false;
            if (master_key[level] && master_keys_broken[level] == false) {
                px = nx;
                py = ny;
                master_key[level] = false;
                master = true;
            } else {
                if (master_keys_broken[level] == true) {
                    messages("key broke");
                    for ( int i = 0; i < level; i ++) {
                        if (master_keys_broken[i]) {
                            messages("fix key");
                            char forge = getch();
                            if (forge == 'y') {
                                master_keys_broken[level] = false;
                                messages("key fixed");
                                master = true;
                            }
                        }
                    }
                    
                }
            }
            if (master) {
                px = nx;
                py = ny;
            } else {
                int which_door = lock_pass_input(nx, ny);
                if (locked[which_door].state) {
                    px = nx;
                    py = ny;
                }
            }
        } else if (map[ny][nx] == '&') {
            show_password(nx, ny);
        } else if (map[ny][nx] == '*') {
           messages("master key found");
           // int break_prob = rand() % 10;
            if (1) {
                master_keys_broken [level] = true;
                
            }
            px = nx;
            py = ny;
            
            if (first_key[level]) {
                master_key[level] = true;
                first_key[level] = false;
            }
        } else if(map[ny][nx] == '|' || map[ny][nx] == '-') {
            reveal_door(ny, nx);
        } else if (map[ny][nx] == '.') {
            
            for ( int i = 0; i < traps_count; i ++) {
                if(traps[i].x == nx && traps[i].y == ny) {
                    reveal_trap(ny, nx);
                }
            }
            px = nx;
            py = ny;
        }
    }
    curs_set(1);
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
    char * options [] = { "Play as a Guest" ,"Login", "Register", "Forgot Password", "Exit"};
    while (1) {
        clear();
        pick_one(choice, menu_name, options, 5);
        refresh();
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) { //guest
                clear();
                printw("You are no playing as a guest!\n");
                printw("Press any key to continue.\n");
                getch();
                start_game_menu();
            } else if (choice == 1) { // Login
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
            } else if (choice == 2) { // Register
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
            } else if ( choice == 3) { // forgot pass
                get_info("Enter your username: ", username, 50, 0);
                refresh();
                printw("Do you pinky promise it's you, %s? :(", username);
                getch();
                forgot_pass(username);
            } else if (choice == 4) { //exit
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

#include <ncurses.h>
#include <string.h>

void load_welcome_page() {
    clear();
    int height = 15, width = 55;
    int start_y =0;
    int start_x = 0;

    attron(COLOR_PAIR(2));
    mvprintw(start_y + 1, start_x + 8, "                           /   \\");
    mvprintw(start_y + 2, start_x + 8, "_                  )      ((   ))     (");
    mvprintw(start_y + 3, start_x + 8, "(@)               /|\\      ))_((     /|\\                   _");
    mvprintw(start_y + 4, start_x + 8, "|-|`\\            / | \\    (/\\|/\\)   / | \\                (@)");
    mvprintw(start_y + 5, start_x + 8, "| |-------------/--|-voV---\\`|'/--Vov-|--\\--------------|-|");
    mvprintw(start_y + 6, start_x + 8, "|-|                '^`     (o o)     '^`                  | |");
    mvprintw(start_y + 7, start_x + 8, "| |                        `\\Y/'                         |-|");
    mvprintw(start_y + 8, start_x + 8, "|-|                                                       | |");
    mvprintw(start_y + 9, start_x + 8, "| |                                                       |-|");
    mvprintw(start_y + 10, start_x + 8,"|_|_______________________________________________________| |");
    mvprintw(start_y + 11, start_x + 8, "(@)       l   /\\ /         ( (       \\ /\\   l         `\\|-|");
    mvprintw(start_y + 12, start_x + 8, "         l /   V           \\ \\        V  \\ l            (@)");
    mvprintw(start_y + 13, start_x + 8, "         l/                _) )_           \\I");
    mvprintw(start_y + 14, start_x + 8, "                          `\\ /'");
    mvprintw(start_y + 15, start_x + 8, "                             `");
    attroff(COLOR_PAIR(2));
    refresh();
    
    attron(COLOR_PAIR(8));
    char welcome[] = "Welcome to the Dungeon of Doom!";
    int welcome_len = strlen(welcome);
    int start = (width - welcome_len) / 2;
    mvprintw(start_y + 8, 23, "%s", welcome);

    attroff(COLOR_PAIR(8));
    refresh();
    getch();
}


int main() {
    initscr();
    setlocale(LC_ALL, "en_US.UTF-8");
    srand(time(NULL));
    raw();
    keypad(stdscr, TRUE);
    noecho();

    if (has_colors()) {
        start_color();
        init_colors();
    }
    
    load_welcome_page();
    main_menu();
    
    getch();
    endwin();
    return 0;
}
