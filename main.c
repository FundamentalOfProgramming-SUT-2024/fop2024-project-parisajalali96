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
#define MAX_FOOD 3
#define LOCKED_PASS_LEN 4
#define PASS_TIMEOUT 30
#define MAX_HEALTH 100
#define HEALTH_R 1
#define HEALTH_TIME 20
#define MAX_GOLD 1000
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

struct food {
    int x, y;
    char * name;
    int color;
    int state;
};

struct gold {
    int x, y;
    int type;
    int state;
};

struct picked_up {
    int x, y;
    char * name;
    int state;
};

struct picked_up_food {
    int count;
    char * name;
};

struct weapon {
    int x, y;
    char symbol;
    int state;
};

struct potion {
    int x, y;
    int type;
    int state;
    char * name;
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

int health = 100;
struct food foods [100];
int food_count = 0;

struct picked_up pocket [100];
int pocket_count;

time_t last_health_update = 0;

struct picked_up_food pocket_food [9];
struct gold golds[MAX_FOOD];
int gold_count = 0;
int gold = 100;
int score = 0;

struct weapon weapons[50];
int weapon_count = 0;

struct potion potions[50];
int potion_count = 0;
//global stuff

void generate_map ();
int determine_color(char, int, int);
void show_level();
void food_window();
void desplay_gold();
//prototypes
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


void messages(char *what_happened, int maybe) {
    //clear();
    move(0, 0);

    if (strcmp(what_happened, "key broke") == 0) {
        printw("The Master Key breaks.\n");
    } else if (strcmp(what_happened, "fix key") == 0) {
        printw("Would you like to mend two broken Master Keys to enter? (y/n)\n");
    } else if (strcmp(what_happened, "key fixed") == 0) {
        printw("Two Master Keys have been mended. You can enter.\n");
    } else if (strcmp(what_happened, "picked up key") == 0) {
        attron(COLOR_PAIR(5));
        printw("You picked up a Master Key!\n");
        attroff(COLOR_PAIR(5));
    } else if (strcmp(what_happened, "cheat code M") == 0) {
        attron(COLOR_PAIR(5));
        printw("You have entered full map mode. Press any key to continue.");
        attroff(COLOR_PAIR(5));
    } else if (strcmp(what_happened, "trap around") == 0) {
        attron(COLOR_PAIR(8));
        printw("There are %d traps around you.", maybe);
        attroff(COLOR_PAIR(8));
    } else if (strcmp(what_happened, "secret door around") == 0) {
        attron(COLOR_PAIR(8));
        printw("There are %d secret doors around you.", maybe);
        attroff(COLOR_PAIR(8));
    } else if (strcmp(what_happened, "picked up food") == 0) {
        attron(COLOR_PAIR(9));
        printw("You picked up some %s!", foods[maybe].name);
        attroff(COLOR_PAIR(9));
    } else if (strcmp(what_happened, "picked up gold") == 0) {
        char type [10];
        int added_gold;
        if (maybe == 3) {
            strcpy(type, "Skygold");
            added_gold = 100;
        } else {
            strcpy(type, "Stargold");
            added_gold = 50;
        }
        gold += added_gold;
        desplay_gold();
        attron(COLOR_PAIR(5));
        mvprintw(0, 0, "You picked up a bag of %s and earned %d more gold!", type, added_gold);
        attroff(COLOR_PAIR(5));
    } else if (strcmp(what_happened, "picked up weapon") == 0) {
        char weapon_name [20];
        
        if(maybe == 1) strcpy(weapon_name, "Mace");
        else if (maybe == 2) strcpy(weapon_name, "Daggger");
        else if (maybe == 3) strcpy(weapon_name, "Magic Wand");
        else if (maybe == 4) strcpy(weapon_name, "Normal Arrow");
        else strcpy(weapon_name, "Sword");
   
        printw("You picked up a %s!", weapon_name);
    } else if (strcmp(what_happened, "picked up potion") == 0) {
        char name [20];
        if (maybe == 0) strcpy(name, "Elixir of Everlife");
        else if (maybe == 1) strcpy(name, "Dragon's Blood");
        else strcpy(name, "Stormrider's Kiss");

        printw("You picked up The %s!", name);
    } else if (strcmp(what_happened, "ate food") == 0) {
        attron(COLOR_PAIR(9));
        printw("You successfully consumed the food!");
        attroff(COLOR_PAIR(9));

    } else if (strcmp(what_happened, "took weapon") == 0) {
        attron(COLOR_PAIR(9));
        printw("You are now weilding the weapon!");
        attroff(COLOR_PAIR(9));
    } else if (strcmp(what_happened, "took potion") == 0) {
        
    }

    refresh();
    getch();
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
    init_pair(10, COLOR_WHITE, COLOR_BLACK);
    
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
    for ( int j = 0; j < HEIGHT; j ++) {
        for ( int i = 0; i < WIDTH; i ++) {
            int color = determine_color(map[j][i], i, j);
            attron(COLOR_PAIR(color));
            mvaddch(j, i, map[j][i]);
            attroff(COLOR_PAIR(color));
        }
    }
    refresh();
    messages("cheat code M", 0);
    getch();

}

void cheat_code_s (int ny, int nx) {
    bool trap = false;
    int trap_num = 0;
    bool secret_door = false;
    int door_num = 0;
    for ( int j = ny -1; j <= ny + 1; j ++) {
        for ( int i = nx -1; i <= nx + 1; i ++ ) {
            if (i!= nx || j!= ny) {
                if (map[j][i] == '?') {
                    secret_door = true;
                    door_num ++;
                }
                else if (map[j][i] == '^') {
                    trap = true;
                    trap_num++;
                }
            }
        }
    }
    
    if (trap) messages("trap around", trap_num);
    if (secret_door) messages("secret door around", door_num);
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

void pick_up (int y, int x) {
    if (map[y][x] == '*') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "master key";
        messages("picked up key", 0);
        pocket_count++;
    } else if (map[y][x] == '%') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "food";
        int food_index = 0;
        for ( int i = 0; i < food_count; i ++) {
            if (foods[i].x == x && foods[i].y == y) {
                food_index = i;
                foods[i].state = 0;

            }
        }
        messages("picked up food", food_index);
        pocket_count++;
    } else if (map[y][x] == '$') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "gold";
        int gold_type = 0;
        for ( int i = 0; i < gold_count; i ++) {
            if (golds[i].x == x && golds[i].y == y) {
                gold_type = golds[i].type;
                golds[i].state = 0;

            }
        }
        messages("picked up gold", gold_type);
        pocket_count++;
    } else if (map[y][x] == 'm' || map[y][x] == 'd' || map[y][x] == '~' || map[y][x] == 'a' || map[y][x] == '!') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "weapon";
        char symbol = 'm';
        int type;
        for ( int i = 0; i < weapon_count; i ++) {
            if (weapons[i].x == x && weapons[i].y == y) {
                symbol = weapons[i].symbol;
                weapons[i].state = 0;
            }
        }
        if (symbol == 'm') type = 1;
        else if (symbol == 'd') type = 2;
        else if (symbol == '~') type = 3;
        else if (symbol == 'a') type = 4;
        else type = 5;

        messages("picked up weapon", type);
        pocket_count++;
    } else if (map[y][x] == 'p') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "potions";
        int type = 0;
        for ( int i = 0; i < potion_count; i ++) {
            if (potions[i].x == x && potions[i].y == y) {
                type = potions[i].type;
                potions[i].state = 0;
            }
        }
        messages("picked up potion", type);
        pocket_count++;
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

/*wchar_t determine_char (char tile) {
    if (tile == 'f') return L'✦';
    else return tile;
}
*/

int determine_color (char tile, int x, int y) {
    if (tile == 'f') {
        for ( int i = 0; i < food_count; i ++) {
            if (foods[i].x == x && foods[i].y == y) {
                return foods[i].color;
            }
        }
    } else if (tile == '@') {
        for (int j = 0; j < locked_door_count; j ++) {
            if (locked[j].x == x && locked[j].y == y) {
                if (locked[j].state == 0) return 2;
                else return 9;
            }
        }
    } else if (tile == '?') {
        for ( int k = 0; k < secret_door_count; k ++) {
            if (hiddens[k].x == x && hiddens[k].y == y) {
                if (hiddens[k].state == 0) return 10;
                else return 5;
            }
        }
    } else if (tile == '<') {
        return 9;
    } else if (tile == '^') {
        for (int n = 0; n < traps_count; n ++) {
            if (traps[n].x == x && traps[n].y == y) {
                if (traps[n].state == 0) return 10;
                else return 6;
            }
        }
    } else if (tile == '%') {
        for ( int p = 0; p < food_count; p ++) {
            if (foods[p].x == x && foods[p].y == y) {
                return foods[p].color;
            }
        }
    } else if (tile == 'p') {
        return 6;
        
    }
    
    else if (tile == '$'){
        for ( int m = 0; m < gold_count; m ++) {
            if (golds[m].x == x && golds[m].y == y) {
                return golds[m].type;
            }
        }
    }else if (map[y][x] == 'm' || map[y][x] == 'd' || map[y][x] == '~' || map[y][x] == 'a' || map[y][x] == '!') {
        return 4;
    } else if (tile == '&') return 5;
    else if (tile == '*') return 5;
    return 10;
}


void render_map() {
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            if(visible[j][i]) {
               // wchar_t display_char = determine_char(map[j][i]);
                int color = determine_color(map[j][i], i, j);
                attron(COLOR_PAIR(color));
                mvaddch(j, i, map[j][i]);
                attroff(COLOR_PAIR(color));
            } else {
                mvaddch(j, i, ' ');
            }
        }
    }
    refresh();
}

void new_level () {
    level++;
    init_map();
    generate_map();
}

void stair_activated (char stair) {
    if ( stair == '>') {
        new_level();
    }
}

char * food_name (int color) {
    char * name [] = {"Ambrosia", "Slightly Moldy Cheese", "Rock-hard Biscuit", "Questionable Apple", "Mystery Meat", "Infernal Steak", "Ethereal Berries", "Exploding Berries", "Potionberry Pie"};
    if (color == 5) {
        int which = rand () % 3;
        return name[which];
    } else if (color == 2) {
        int which = 3 + rand () % 3;
        return name[which];
    } else {
        int which = 6 + rand () % 3;
        return name[which];
    }
}

void health_update () {
    time_t current_time;
    time(&current_time);

    if (difftime(current_time, last_health_update) >= HEALTH_TIME) {
        health -= HEALTH_R;
        
        if (health < 0) {
            health = 0;
        }
        last_health_update = current_time;
    }
}


void show_level () {
    attron(COLOR_PAIR(2));
    mvprintw(LINES - 1, 0, "Level: %d", level);
    attroff(COLOR_PAIR(2));
    refresh();
}

void desplay_gold () {
    attron(COLOR_PAIR(5));
    mvprintw(LINES -1, COLS/2, "Gold: %d", gold);
    attroff(COLOR_PAIR(5));
    refresh();
}

void health_bar (int health) {
    int filled = (health * 20) / MAX_HEALTH;
    move(LINES - 1, COLS - 20 - 10);
    addch('[');
    for (int i = 0; i < 20; i++) {
        if (i < filled) {
            attron(COLOR_PAIR(9));
            addch('#');
            attroff(COLOR_PAIR(9));
            refresh();
            
        } else {
            attron(COLOR_PAIR(9));
            addch('-');
            attroff(COLOR_PAIR(9));
            refresh();
        }
    }
    addch(']');
    mvprintw(LINES -1, COLS - 39, "Health: ");
    mvprintw(LINES - 1, COLS - 20 - 10 + 20 + 2, "%d%%", (health * 100) / MAX_HEALTH);
    refresh();

}

void food_choice (char * name ) {
    for ( int i = 0; i < food_count; i ++) {
        if (strcmp(foods[i].name, name) == 0 && foods[i].state == 0) {
            foods[i].state =1;
            health += 5;
            health_bar(health);
            break;
        }
    }
    //food_window();
}

void food_window () {//damn what is this
    WINDOW * food = newwin(10, 40, 0, 0);
    wclear(food);
    box(food, 0, 0);

    int ex_berry = 0, eth_berry = 0, pie = 0, amb = 0, cheese = 0, biscuit = 0, steak = 0, apple = 0, meat = 0;
    for ( int i = 0; i < food_count; i ++) {
        if (strcmp(foods[i].name, "Exploding Berries") ==0 && foods[i].state == 0) ex_berry++;
        else if (strcmp(foods[i].name, "Ethereal Berries") ==0 && foods[i].state == 0) eth_berry++;
        else if (strcmp(foods[i].name, "Potionberry Pie") ==0 && foods[i].state == 0) pie++;
        else if (strcmp(foods[i].name, "Ambrosia") ==0 && foods[i].state == 0) amb++;
        else if (strcmp(foods[i].name, "Slightly Moldy Cheese") ==0 && foods[i].state == 0) cheese++;
        else if (strcmp(foods[i].name, "Rock-hard Biscuit") ==0 && foods[i].state == 0) biscuit++;
        else if (strcmp(foods[i].name, "Infernal Steak") ==0 && foods[i].state == 0) steak++;
        else if (strcmp(foods[i].name, "Mystery Meat") ==0 && foods[i].state == 0) meat++;
        else if (strcmp(foods[i].name, "Questionable Apple") ==0 && foods[i].state == 0) apple++;
    }
    
    int ex_berry_id = 0, eth_berry_id = 0, pie_id = 0, amb_id = 0, cheese_id = 0, biscuit_id = 0, steak_id = 0, apple_id = 0, meat_id = 0;
    int identifier = 1;
    if (ex_berry != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Exploding Berries", identifier, ex_berry);
        ex_berry_id = identifier;
        identifier++;
    }
    if (eth_berry != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Ethereal Berries", identifier, eth_berry);
        eth_berry_id = identifier;
        identifier ++;
    }
    if (pie != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Potionberry Pie",identifier, pie);
        pie_id = identifier;
        identifier ++;
    }
    if (amb != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Ambrosia",identifier, amb);
        amb_id = identifier;
        identifier ++;
    }
    if (cheese != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Slighlty Moldy Cheese",identifier, cheese);
        cheese_id = identifier;
        identifier ++;
    }
    if (biscuit != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Rock-hard Biscuit ",identifier, biscuit);
        biscuit_id = identifier;
        identifier ++;
    }
    if (steak != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Infernal Steak",identifier, steak);
        steak_id = identifier;
        identifier ++;
    }
    if (apple != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Questionable Apple",identifier, apple);
        apple_id = identifier;
        identifier ++;
    }
    if (meat != 0) {
        mvwprintw(food, identifier + 1, 1, "%d. %d Mystery Meat",identifier, meat);
        meat_id = identifier;
        identifier ++;
    }
    
    if (ex_berry == 0 && eth_berry == 0 && pie == 0 && amb == 0 && cheese == 0 && biscuit == 0 && steak == 0 && apple == 0 && meat == 0 ) {
        char text [50] = "You don't have any food to consume!";
        int x = (40 - strlen(text)) / 2;
        mvwprintw(food, 4, x, "%s", text);
    }
    
    wrefresh(food);

    
    int choice = getch();
    choice = choice - '0';
    if (choice == ex_berry_id) food_choice("Exploding Berries");
    else if (choice == eth_berry_id) food_choice("Ethereal Berries");
    else if (choice == pie_id) food_choice("Potionberry Pie");
    else if (choice == amb_id) food_choice("Ambrosia");
    else if (choice == cheese_id) food_choice("Slightly Moldy Cheese");
    else if (choice == biscuit_id) food_choice("Rock-hard Biscuit");
    else if (choice == steak_id) food_choice("Infernal Steak");
    else if (choice == meat_id) food_choice("Mystery Meat");
    else if (choice == apple_id) food_choice("Questionable Apple");
    if ( choice != '\n') {
        messages("ate food", 0);
    }

}

void weapon_choice (char symbol) {
    for ( int i = 0; i < weapon_count; i ++) {
        if (weapons[i].symbol == symbol && weapons[i].state == 0) {
            weapons[i].state =1;
            break;
        }
    }
}

void weapon_window () {
    WINDOW * arsenal = newwin(10, 40, 0, 0);
    wclear(arsenal);
    box(arsenal, 0, 0);

    int mace = 0, dag = 0, wand = 0, arrow = 0, sword = 0;
    for ( int i = 0; i < weapon_count; i ++) {
        if (weapons[i].symbol == 'm' && weapons[i].state == 0) mace++;
        if (weapons[i].symbol == 'd' && weapons[i].state == 0) dag++;
        if (weapons[i].symbol == '~' && weapons[i].state == 0) wand++;
        if (weapons[i].symbol == 'a' && weapons[i].state == 0) arrow++;
        if (weapons[i].symbol == '!' && weapons[i].state == 0) sword++;
    }
    
    int mace_id = 0, dag_id = 0, wand_id = 0, arrow_id = 0, sword_id = 0;
    int identifier = 1;
    if (mace != 0) {
        mvwprintw(arsenal, identifier + 1, 1, "%d. %d Maces", identifier, mace);
        mace_id = identifier;
        identifier++;
    }
    if (dag != 0) {
        mvwprintw(arsenal, identifier + 1, 1, "%d. %d Dagger", identifier, dag);
        dag_id = identifier;
        identifier ++;
    }
    if (wand != 0) {
        mvwprintw(arsenal, identifier + 1, 1, "%d. %d Magic Wand",identifier, wand);
        wand_id = identifier;
        identifier ++;
    }
    if (arrow != 0) {
        mvwprintw(arsenal, identifier + 1, 1, "%d. %d Normal Arrow",identifier, arrow);
        arrow_id = identifier;
        identifier ++;
    }
    if (sword != 0) {
        mvwprintw(arsenal, identifier + 1, 1, "%d. %d Sword",identifier, sword);
        sword_id = identifier;
        identifier ++;
    }
    
    if (mace == 0 && dag == 0 && arrow == 0 && sword == 0 && wand == 0) {
        char text [50] = "You don't have any weapons to wield!";
        int x = (40 - strlen(text)) / 2;
        mvwprintw(arsenal, 5, x, "%s", text);
    }
    
    wrefresh(arsenal);

    
    int choice = getch();
    choice = choice - '0';
    if (choice == mace_id) weapon_choice('m');
    else if (choice == dag_id) weapon_choice('d');
    else if (choice == arrow_id) weapon_choice('a');
    else if (choice == wand_id) weapon_choice('~');
    else if (choice == sword_id) weapon_choice('!');
  
    messages("took weapon", 0);

}
void potion_choice (int type) {
    
    for ( int i = 0; i < potion_count; i ++) {
        if (potions[i].type == type && potions[i].state == 0) {
            potions[i].state =1;
            break;
        }
    }
}
void potion_window () {
    WINDOW * potion = newwin(10, 40, 0, 0);
    wclear(potion);
    box(potion, 0, 0);

    int elix = 0, drag = 0, kiss = 0;
    for ( int i = 0; i < potion_count; i ++) {
        if (potions[i].type == 0 && potions[i].state == 0) elix++;
        if (potions[i].type == 1 && potions[i].state == 0) drag++;
        if (potions[i].type == 2 && potions[i].state == 0) kiss++;
    }
    
    int elix_id = 0, drag_id = 0, kiss_id = 0;
    int identifier = 1;
    if (elix != 0) {
        mvwprintw(potion, identifier + 1, 1, "%d. %d Elixir of Everlife", identifier, elix);
        elix_id = identifier;
        identifier++;
    }
    if (drag != 0) {
        mvwprintw(potion, identifier + 1, 1, "%d. %d Dragon's Blood", identifier, drag);
        drag_id = identifier;
        identifier++;
    }
    if (kiss != 0) {
        mvwprintw(potion, identifier + 1, 1, "%d. %d Stormrider's Kiss", identifier, kiss);
        kiss_id = identifier;
        identifier++;
    }
    if (elix == 0 && drag == 0 && kiss == 0) {
        char text [50] = "You don't have any potions to drink!";
        int x = (40 - strlen(text)) / 2;
        mvwprintw(potion, 5, x, "%s", text);
    }
    wrefresh(potion);

    
    int choice = getch();
    choice = choice - '0';
    if (choice == elix_id) potion_choice(0);
    else if (choice == drag_id) potion_choice(1);
    else if (choice == kiss_id) potion_choice(2);
  
    messages("took potion", 0);

}
void p_command () {
    mvprintw(0, 0,"What potion would you like to drink? (press * for the list)");
    getch();
    potion_window();
}
void i_command () {
    mvprintw(0, 0,"What weapon would you like to wield? (press * for the list)");
    getch();
    weapon_window();
}
void E_command () {
    mvprintw(0, 0,"What food would you like to eat? (press * for the list)");
    getch();
    food_window();
}

void add_weapon (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
       // if (food_count >= MAX_FOOD) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            //if (food_count >= MAX_FOOD) break;
            if (rand () % 50 == 0 && map[y][x] == '.') {
                int type = rand() % 5;
                char symbol;
                if (type == 0) {
                    symbol = 'm';
                } else if (type == 1) {
                    symbol = 'd';
                } else if (type == 2) {
                    symbol = '~';
                } else if (type == 3) {
                    symbol = 'a';
                } else {
                    symbol = '!';
                }
                weapons[weapon_count].symbol = symbol;
                map[y][x] = symbol;
                weapons[weapon_count].x = x;
                weapons[weapon_count].y = y;
                weapons[weapon_count].state = -1;
                weapon_count++;
            }
        }
    }
}

void add_gold (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
       // if (food_count >= MAX_FOOD) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            //if (food_count >= MAX_FOOD) break;
            if (rand () % 50 == 0 && map[y][x] == '.') {
                int color = rand() % 10;
                if ((color % 3) == 0) {
                    color = 3;
              } else {
                    color = 5;
                }
                golds[gold_count].type = color;
                map[y][x] = '$';
                golds[gold_count].x = x;
                golds[gold_count].y = y;
                golds[gold_count].state = -1;
                gold_count++;
            }
        }
    }
}

void add_potion (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
       // if (food_count >= MAX_FOOD) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            //if (food_count >= MAX_FOOD) break;
            if (rand () % 50 == 0 && map[y][x] == '.') {
                int type = rand() % 3;
    
                map[y][x] = 'p';
                potions[potion_count].x = x;
                potions[potion_count].y = y;
                potions[potion_count].type = type;
                potions[potion_count].state = -1;
                potion_count++;
            }
        }
    }
}
void add_food (struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
       // if (food_count >= MAX_FOOD) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            //if (food_count >= MAX_FOOD) break;
            if (rand () % 50 == 0 && map[y][x] == '.') {
                int color = rand() % 3;
                if (color == 0) {
                    color = 5;
                } else if (color == 1) {
                    color = 2;
                } else {
                    color = 6;
                }
                foods[food_count].color = color;
                map[y][x] = '%';
                foods[food_count].x = x;
                foods[food_count].y = y;
                foods[food_count].name = food_name(color);
                foods[food_count].state = -1;
                food_count++;
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
            add_pillar(new_room);
            add_trap(new_room);
            add_food(new_room);
            add_gold(new_room);
            add_weapon(new_room);
            add_potion(new_room);
            
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
        //clear();
        refresh();
        health_bar(health);
        show_level();
        desplay_gold();
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
        
        if (ch == KEY_UP || ch == 'J') ny--;
        else if (ch == KEY_DOWN || ch == 'K') ny++;
        else if (ch == KEY_LEFT || ch == 'H') nx--;
        else if (ch == KEY_RIGHT || ch == 'L') nx++;
        else if (ch == 'Y') {
            nx--;
            ny--;
        }
        else if (ch =='U') {
            ny--;
            nx++;
        } else if (ch =='B') {
            ny++;
            nx--;
        } else if (ch =='N') {
            nx++;
            ny++;
        }
        else if (ch == 'M') {
            cheat_code_M();
        }
        else if (ch == 's') {
            cheat_code_s(ny, nx);
        } else if (ch == 'E') {
            E_command();
        } else if (ch == 'i') {
            i_command();
        } else if (ch == 'p') {
            p_command();
        }
        else if (ch == 'f') {
            int condition [2] = {0};
            int direction = getch();
            if (direction == KEY_UP || direction == 'J') condition[0] =-1;
            else if (direction == KEY_DOWN || direction == 'K') condition[0]=1;
            else if (direction == KEY_LEFT || direction == 'H') condition[1]=-1;
            else if (direction == KEY_RIGHT || direction == 'L') condition[1]=1;
            else if (direction =='Y') {
                condition[0] = -1;
                condition[1] = -1;
            } else if (direction =='U') {
                condition[0] = -1;
                condition[1] = 1;
            }  else if (direction =='B') {
                condition[0] = 1;
                condition[1] = -1;
            }  else if (direction =='N') {
                condition[0] = 1;
                condition[1] = 1;
            }
            
            while (map[ny][nx] == '.') {
                nx +=condition[1];
                ny += condition[0];
                if (map[ny][nx] == '.') {
                    px = nx;
                    py = ny;
                }
            }
        }
        
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
                    messages("key broke", 0);
                    for ( int i = 0; i < level; i ++) {
                        if (master_keys_broken[i]) {
                            messages("fix key", 0);
                            char forge = getch();
                            if (forge == 'y') {
                                master_keys_broken[level] = false;
                                messages("key fixed", 0);
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
            int break_prob = rand() % 10;
            if (break_prob == 0) {
                master_keys_broken [level] = true;
                
            }
            px = nx;
            py = ny;
            pick_up(ny, nx);
            
            if (first_key[level]) {
                messages("master key found", 0);
            }
            if (first_key[level]) {
                master_key[level] = true;
                first_key[level] = false;
            }
        } else if (map[ny][nx] == '%') {
            px = nx;
            py = ny;
            pick_up(ny, nx);
            
        }
        else if (map[ny][nx] == '$') {
            px = nx;
            py = ny;
            pick_up(ny, nx);
            
        } else if (map[ny][nx] == 'm' || map[ny][nx] == 'd' || map[ny][nx] == '~' || map[ny][nx] == 'a' || map[ny][nx] == '!') {
            px = nx;
            py = ny;
            pick_up(ny, nx);
            
        } else if (map[ny][nx] == 'p') {
            px = nx;
            py = ny;
            pick_up(ny, nx);
            
            
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
        
        
        health_update();
        refresh();
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


void load_welcome_page() {
    curs_set(0);
    clear();
    int height = 15, width = 55;
    int start_y =0;
    int start_x = 0;

    attron(COLOR_PAIR(2));
    int x = 10;
    mvprintw(start_y + 1 + x, start_x + 8 +x, "                           /   \\");
    mvprintw(start_y + 2 +x, start_x + 8 +x, "_                  )      ((   ))     (");
    mvprintw(start_y + 3 +x, start_x + 8 +x, "(@)               /|\\      ))_((     /|\\                   _");
    mvprintw(start_y + 4 +x, start_x + 8 +x, "|-|`\\            / | \\    (/\\|/\\)   / | \\                (@)");
    mvprintw(start_y + 5 +x, start_x + 8 +x, "| |-------------/--|-voV---\\`|'/--Vov-|--\\--------------|-|");
    mvprintw(start_y + 6 +x, start_x + 8 +x, "|-|                '^`     (o o)     '^`                  | |");
    mvprintw(start_y + 7 +x, start_x + 8 +x, "| |                        `\\Y/'                         |-|");
    mvprintw(start_y + 8 +x, start_x + 8 +x, "|-|                                                       | |");
    mvprintw(start_y + 9 +x, start_x + 8 +x, "| |                                                       |-|");
    mvprintw(start_y + 10 +x, start_x + 8 +x,"|_|_______________________________________________________| |");
    mvprintw(start_y + 11 +x, start_x + 8 +x, "(@)       l   /\\ /         ( (       \\ /\\   l         `\\|-|");
    mvprintw(start_y + 12 +x, start_x + 8 +x, "         l /   V           \\ \\        V  \\ l            (@)");
    mvprintw(start_y + 13 +x, start_x + 8 +x, "         l/                _) )_           \\I");
    mvprintw(start_y + 14 +x, start_x + 8 +x, "                          `\\ /'");
    mvprintw(start_y + 15 +x, start_x + 8 +x, "                             `");
    attroff(COLOR_PAIR(2));
    refresh();
    
    attron(COLOR_PAIR(8));
    char welcome[] = "Welcome to the Dungeon of Doom!";
    int welcome_len = strlen(welcome);
    int start = (width - welcome_len) / 2;
    mvprintw(start_y + 8 +x, 23 +x, "%s", welcome);

    attroff(COLOR_PAIR(8));
    refresh();
    getch();
    curs_set(1);

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
