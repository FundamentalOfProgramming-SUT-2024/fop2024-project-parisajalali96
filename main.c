//
//  main.c
//  rogue (parisa's version)
//
//  Created by parisa on 12/22/24.
//Parisa Jalali
//403170933

#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#define MAX_SIZE 100
#define HEIGHT 30
#define WIDTH 120
#define ROOM_MIN_SIZE 5
#define ROOM_MAX_SIZE 13
#define ROOM_COUNT 6
#define CORRIDOR_VISIBLE 1
#define MAX_PILLAR 3
#define MAX_TRAP 3
#define MAX_FOOD 3
#define LOCKED_PASS_LEN 4
#define PASS_TIMEOUT 10
#define MAX_HEALTH 100
#define HEALTH_R 1
#define HUNGER_R 5
#define HEALTH_TIME 20
#define MAX_GOLD 1000

struct ROOM {
    int x, y, height, width, type;
};

struct scores {
    char name [50];
    int total_gold;
    int total_score;
    int total_games;
    time_t first_game;
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
    int fresh;
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
    int num_collect;
    int thrown;
};

struct potion {
    int x, y;
    int type;
    int state;
    char * name;
};

struct monster {
    int x, y;
    int health;
    char type;
    int num;
    int state;
    int movement_num;
    int movement_state;
    int level;
};
// structs

char map [HEIGHT][WIDTH];
bool visible[HEIGHT][WIDTH];
bool visited[HEIGHT][WIDTH];
bool trap_visible[HEIGHT][WIDTH];


struct scores ranks [MAX_SIZE];
int score_count = 0;

int hero_color = 10;
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
int hunger = 100;
struct food foods [100];
int food_count = 0;

struct picked_up pocket [100];
int pocket_count;

time_t last_health_update = 0;
time_t last_hunger_update = 0;


struct picked_up_food pocket_food [9];
struct gold golds[MAX_FOOD];
int gold_count = 0;
int gold = 0;
int score = 100;

struct weapon weapons[50];
struct weapon * weapon_in_hand;
int weapon_count = 0;
int wield_choice = 'm';

struct potion potions[50];
int potion_count = 0;
struct ROOM rooms[10][ROOM_COUNT];
int room_count[10] = {0};
int g_state = 0;

struct monster monsters[40];
int monster_count = 0;

char user_name [30] = "Guest_Player";
int hits = 0;
int potion_time_track = 0;
int food_time_out = 50;
int weapon_rate = 1;
int num_of_blocks = 1;
bool drank_potion = false;
bool ate_magic_food = false;

char last_direction [3];
bool long_range_weapon = false;

double difficulty_coeff = 1;
//global stuff

void generate_map ();
int determine_color(char, int, int);
void show_level();
void food_window();
void desplay_gold();
void end_game (char);
void health_bar (int);
void monster_move (int, int, struct monster *);
void render_map();
int monster_check (int , int , struct monster );
void monster_attack (struct monster );
void get_score (char*, int , int );
void play_menu ();
void load_hall();
void hall_of_fame();
void display_hits();
void elixir_of_everlife();
void dragon_blood ();
void storm_kiss ();
void lobby_art();
void main_menu();
void start_game_menu();
void show_pop_up (char * , int , char * );
//prototypes

void reset_game () {
    locked_door_count = 0;
    secret_door_count = 0;
    traps_count = 0;
    health = 100;
    hunger = 100;
    level = 1;
    food_count = 0;
    pocket_count = 0;
    last_health_update = 0;
    last_hunger_update = 0;
    gold_count = 0;
    gold = 0;
    score = 0;
    weapon_count = 0;
    wield_choice = 'm';
    potion_count = 0;
    for (int i = 0; i < 10; i++) {
        room_count[i] = 0;
    }
    g_state = 0;

    monster_count = 0;

    //strcpy(user_name,"Guest_Player");
    hits = 0;
    potion_time_track = 0;
    food_time_out = 50;
    weapon_rate = 1;
    num_of_blocks = 1;
    drank_potion = false;
    ate_magic_food = false;
    difficulty_coeff = 1;
}

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
        printw("You have entered full map mode.");
        attroff(COLOR_PAIR(5));
    } else if (strcmp(what_happened, "trap around") == 0) {
        attron(COLOR_PAIR(2));
        printw("There are %d traps around you.", maybe);
        attroff(COLOR_PAIR(2));
    } else if (strcmp(what_happened, "secret door around") == 0) {
        attron(COLOR_PAIR(2));
        printw("There are %d secret doors around you.", maybe);
        attroff(COLOR_PAIR(2));
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
        mvprintw(0, 0, "You picked up a bag of %s", type);
        mvprintw(1, 0, "earned %d more gold!",added_gold);
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
        else if (maybe == 2) strcpy(name, "Dragon's Blood");
        else strcpy(name, "Stormrider's Kiss");

        printw("You picked up The %s!", name);
    } else if (strcmp(what_happened, "ate food") == 0) {
        attron(COLOR_PAIR(9));
        if (maybe == 0) printw("You successfully consumed the food!");
        else if (maybe == 1) printw("The Legendry food doubles your damaging skills!");
        else if (maybe == 2) printw("The Magical food doubles your speed!");
        attroff(COLOR_PAIR(9));
            

    } else if (strcmp(what_happened, "took weapon") == 0) {
        attron(COLOR_PAIR(9));
        printw("You are now weilding the weapon!");
        attroff(COLOR_PAIR(9));
    } else if (strcmp(what_happened, "enter room") == 0) {
        if (maybe == 1) printw("You have entered an Enchant Room!");
        else if (maybe == 2) printw("You have entered The Treasure Room!");

    } else if (strcmp(what_happened, "picked up treasure") == 0) {
        attron(COLOR_PAIR(5));
        mvprintw(0, 0, "You open the ancient chest, and golden light floods the room.");
        mvprintw(1, 0, "The legendary treasure is yours!");
        attroff(COLOR_PAIR(5));
    } else if (strcmp(what_happened, "attack") == 0 ) {
        if (maybe == 0) {
            attron(COLOR_PAIR(2));
            printw("The Deamon attacks!");
            attroff(COLOR_PAIR(2));
        } else if (maybe == 1) {
            attron(COLOR_PAIR(2));
            printw("The Fire Breathing Monster scorches you with a blast of flames!");
            attroff(COLOR_PAIR(2));
        } else if (maybe == 2) {
            attron(COLOR_PAIR(2));
            printw("The Giant swings it's fist at you!");
            attroff(COLOR_PAIR(2));
        } else if (maybe == 3) {
            attron(COLOR_PAIR(2));
            printw("The Snake hisses and attacks!");
            attroff(COLOR_PAIR(2));
        } else if (maybe == 4) {
            attron(COLOR_PAIR(2));
            printw("The Undeed damages you greatly!");
            attroff(COLOR_PAIR(2));
        }
    } else if (strcmp(what_happened, "monster dead") == 0) {
        int type = monsters[maybe].num;
        if (type == 0) {
            attron(COLOR_PAIR(9));
            printw("You beat The Deamon!");
            attroff(COLOR_PAIR(9));
        } else if (type == 1) {
            attron(COLOR_PAIR(9));
            printw("The Fire Breathing Monster hisses as it's flames die out!");
            attroff(COLOR_PAIR(9));
        } else if (type == 2) {
            attron(COLOR_PAIR(9));
            printw("You defeat The Giant!");
            attroff(COLOR_PAIR(9));
        } else if (type == 3) {
            attron(COLOR_PAIR(9));
            printw("The Snake falls by you weapon!");
            attroff(COLOR_PAIR(9));
        } else if (type == 4) {
            attron(COLOR_PAIR(9));
            printw("The Undeed collapses into a pile of bones!");
            attroff(COLOR_PAIR(9));
        }
    } else if (strcmp(what_happened, "player attack") == 0) {
        int type = monsters[maybe].num;
        if (type == 0) {
            attron(COLOR_PAIR(9));
            printw("You strike The Deamon!");
            attroff(COLOR_PAIR(9));
        } else if (type == 1) {
            attron(COLOR_PAIR(9));
            printw("You hit the Fire Breathing Monster!");
            attroff(COLOR_PAIR(9));
        } else if (type == 2) {
            attron(COLOR_PAIR(9));
            printw("You do a great number on The Giant!");
            attroff(COLOR_PAIR(9));
        } else if (type == 3) {
            attron(COLOR_PAIR(9));
            printw("You attack the snake!");
            attroff(COLOR_PAIR(9));
        } else if (type == 4) {
            attron(COLOR_PAIR(9));
            printw("You attack the Undeed!");
            attroff(COLOR_PAIR(9));
        }
    } else if (strcmp(what_happened, "low health") == 0) {
        attron(COLOR_PAIR(9));
        printw("Your vision's blurry. Eat something!");
        attroff(COLOR_PAIR(9));
    } else if (strcmp(what_happened, "no monster") == 0) {
        printw("There are no monsters around you.");
    } else if (strcmp(what_happened, "no weapon") == 0) {
        printw("You're not wielding any weapon!");
    } else if (strcmp(what_happened, "weapon drop") == 0) {
        char weapon [20];
        if (maybe == 0) strcpy(weapon, "Dagger");
        else if (maybe == 1) strcpy(weapon, "Magic Wand");
        else strcpy(weapon, "Normal Arrow");
        attron(COLOR_PAIR(2));
        printw("You dropped the %s!", weapon);
        attroff(COLOR_PAIR(2));

    } else if (strcmp(what_happened, "monster frozen") == 0) {
        char name [30];
        if (monsters[maybe].type == 'D') strcpy(name, "Deamon");
        else if (monsters[maybe].type == 'F') strcpy(name, "Fire Breathing Monster");
        else if (monsters[maybe].type == 'G') strcpy(name, "Giant");
        else if (monsters[maybe].type == 'S') strcpy(name, "Snake");
        else if (monsters[maybe].type == 'U') strcpy(name, "Undeed");
        attron(COLOR_PAIR(2));
        printw("The spell paralyzes The %s!", name);
        attroff(COLOR_PAIR(2));
    } else if (strcmp(what_happened, "weapon in bag") == 0) {
        char weapon [20];
        if (maybe == 'd') strcpy(weapon, "Dagger");
        else if (maybe == '~') strcpy(weapon, "Magic Wand");
        else if (maybe == '!') strcpy(weapon, "Sword");
        else if (maybe == 'm') strcpy(weapon, "Mace");
        else strcpy(weapon, "Normal Arrow");
        
        printw("You put the %s back into it's sheath!", weapon);
    } else if (strcmp(what_happened, "put in bag") == 0) {
        attron(COLOR_PAIR(2));
        mvprintw(0, 0, "You're already wielding a weapon!");
        mvprintw(1, 0, "Put the weapon in your bag!");
        attroff(COLOR_PAIR(2));
    } else if (strcmp(what_happened, "took potion") == 0) {
        char name [30];
        if (maybe == 0) strcpy(name, "The Elixir of Everlife");
        else if (maybe == 1) strcpy(name, "The Stormrider's Kiss");
        else if (maybe == 2) strcpy(name, "The Dragon's Blood");
        attron(COLOR_PAIR(9));
        mvprintw(0, 0, "You drank %s!", name);
        if (maybe == 0) mvprintw(1, 0, "Your health begins to recover!");
        else if (maybe == 1) mvprintw(1, 0, "You move like lightning!");
        else mvprintw(1, 0, "Your weapons are now twice as deadly!");
        attroff(COLOR_PAIR(9));
    } else if (strcmp(what_happened, "potion time over") == 0) {
        printw("The potion's effects begin to wear off!");
    } else if (strcmp(what_happened, "ate spoiled food") == 0) {
        attron(COLOR_PAIR(2));
        printw("The food was spoiled. You feel sick...");
        attroff(COLOR_PAIR(2));
    } else if (strcmp(what_happened, "no long range weapon") == 0) {
        attron(COLOR_PAIR(2));
        printw("Select a direction first!");
        attroff(COLOR_PAIR(2));
    } else if (strcmp(what_happened, "new level") == 0) {
        attron(COLOR_PAIR(9));
        printw("Welcome to level %d!", level);
        attroff(COLOR_PAIR(9));
    }
    move(0,0);
    getch();
    clrtoeol();
    refresh();
}

void difficulty() {
    int ch;
    int choice = 0;
    char menu_name[50] = {"** CHOOSE DIFFICULTY **"};
    char *options[] = {"Hard", "Medium", "Easy", "Exit"};
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_width = 40;
    int win_height = 12;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    WINDOW *menu_win = newwin(win_height, win_width, start_y, start_x);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        wattron(menu_win, COLOR_PAIR(9));
        mvwprintw(menu_win, 1, (win_width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win, COLOR_PAIR(9));
        for (int i = 0; i < 4; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(9));
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win, COLOR_PAIR(9));
            } else {
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
            }
        }
        
        wrefresh(menu_win);
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 3) choice++;
        else if (ch == '\n') {
            if (choice == 0) {
                getch();
                refresh();
                break;
            } else if (choice == 1) {//hard
                difficulty_coeff = 0.5;
                getch();
                refresh();
                break;
            } else if (choice == 2) {//medium
                difficulty_coeff = 1;
                getch();
                refresh();
                break;
            } else if (choice == 3) {//easy
                //clear();
                difficulty_coeff = 2;
                refresh();
                break;
            }
        }
    }
    delwin(menu_win);
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
    init_pair(11, COLOR_BLACK, COLOR_YELLOW);
    init_pair (12, COLOR_BLACK, COLOR_GREEN);
    
    
}

void customize_menu() {
    int ch;
    int choice = 0;
    char menu_name[50] = {"** CUSTOMIZE HERO **"};
    char *options[] = {"PINK", "YELLOW", "BLUE", "RED", "CYAN"};
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_width = 40;
    int win_height = 12;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    WINDOW *menu_win = newwin(win_height, win_width, start_y, start_x);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        wattron(menu_win, COLOR_PAIR(2));
        mvwprintw(menu_win, 1, (win_width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win, COLOR_PAIR(2));

        for (int i = 0; i < 5; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(2));
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win, COLOR_PAIR(2));
            } else {
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
            }
        }
        
        wrefresh(menu_win);
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) {
                hero_color = 6;
                break;
            } else if (choice == 1) {
                hero_color = 5;
                break;
            } else if (choice == 2) {
                hero_color = 4;
                break;
            } else if (choice == 3) {
                hero_color = 2;
                break;
            } else if (choice == 4) {
                hero_color = 3;
                break;
            }
        }
    }
    delwin(menu_win);
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
    
    for (int y = room.y + 1; y < room.y + room.height - 1; y ++) {
        for (int x = room.x + 1 ; x < room.x + room.width - 1; x ++) {
            if (map[y][x] == '+') map[y][x] = '.';
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

void add_monster (struct ROOM room) {
    int prob;
    if (level == 1) prob = difficulty_coeff *50;
    else if (level == 2) prob = difficulty_coeff * 30;
    else if (level == 3) prob = difficulty_coeff * 25;
    else prob = difficulty_coeff * 20;

    for (int y = room.y; y < room.y + room.height; y++) {
        //if (traps_count >= max) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            int type = rand () % 10;
            char symbol = 'D';
            if (type == 0 || type == 2 || type == 3) {
                symbol = 'D';//D
                monsters[monster_count].health = 5;
                monsters[monster_count].num = 0;

            }
            else if (type == 4 || type == 5) {
                symbol = 'F';//F
                monsters[monster_count].health = 10;
                monsters[monster_count].num = 1;

            }
            else if (type == 6 || type == 7) {
                symbol = 'G';//G
                monsters[monster_count].health = 15;
                monsters[monster_count].num = 2;

            }
            else if (type == 8) {
                symbol = 'S'; //S
                monsters[monster_count].health = 20;
                monsters[monster_count].num = 3;
            }
            else if (type == 9) {
                symbol = 'U'; //U
                monsters[monster_count].health = 30;
                monsters[monster_count].num = 4;
            }
            if (rand () % prob == 0 && map[y][x] == '.') {
                monsters[monster_count].level = level;
                monsters[monster_count].x = x;
                monsters[monster_count].y = y;
                monsters[monster_count].type = symbol;
                map[y][x] = symbol;
                monsters[monster_count].state = 0;
                monsters[monster_count].movement_num = 0;
                monsters[monster_count].movement_state = 0;
                monster_count++;
            }
        }
    }
}

void add_trap (struct ROOM room) {
    int type = room.type;
    int prob, max;
    if (type == 2) {
        prob = 5;
        max = 15;
    } else {
        prob = 50;
        max = 3;
    }
    for (int y = room.y; y < room.y + room.height; y++) {
        //if (traps_count >= max) break;
        for (int x = room.x; x < room.x + room.width; x++) {
           // if (traps_count >= max) break;
            if (rand () % prob == 0 && map[y][x] == '.') {
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
    int which_door = -1;
    for ( int i = 0; i < secret_door_count; i ++) {
        if(hiddens[i].x == nx && hiddens[i].y == ny) {
            which_door = i;
            hiddens[i].state = 1;
        }
    }
    if (which_door != -1 && hiddens[which_door].state) {
        attron(COLOR_PAIR(2));
        map[ny][nx] = '?';
        attroff(COLOR_PAIR(2));
        refresh();
    }
    render_map();
}

void lose_health (int value) {
    health -= value;
    health_bar(health);
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
    
    lose_health(5);
    
    
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
    if (x1 < 2) x1 = 2;
    if (y1 < 2) y1 = 2;
    if (x2 < 2) x2 = 2;
    if (y2 < 2) y2 = 2;
    
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

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_width = 30;
    int win_height = 8;

    int start_x = (cols - win_width) / 2;
    int start_y = (rows - win_height) / 2;

    WINDOW *password_win = newwin(win_height, win_width, start_y, start_x);

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
    werase(password_win);
    delwin(password_win);
}



int lock_pass_input(int px, int py) {
    int which_door = 0;
    for (int c = 0; c < locked_door_count; c++) {
        if (locked[c].x == px && locked[c].y == py) {
            which_door = c;
        }
    }
    if (locked[which_door].state == 1) return which_door;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_width = 30;
    int win_height = 8;

    int start_x = (cols - win_width) / 2;
    int start_y = (rows - win_height) / 2;

    char is_pass[5];
    WINDOW *password_win = newwin(win_height, win_width, start_y, start_x);
    wbkgd(password_win, COLOR_PAIR(7));
    box(password_win, 0, 0);

    echo();
    char msg1[] = "Knock Knock! Who's there?";
    int msg1_len = strlen(msg1);
    int start_col1 = (win_width - msg1_len) / 2;

    char msg2[] = "The password, please!: ";
    int msg2_len = strlen(msg2);
    int start_col2 = (win_width - msg2_len) / 2;
    mvwprintw(password_win, 3, start_col1, "%s", msg1);
    mvwprintw(password_win, 4, start_col2, "%s", msg2);
    mvwgetstr(password_win, 5, start_col2, is_pass);
    wrefresh(password_win);
    noecho();

    size_t len = strlen(is_pass);
    if (len > 0 && is_pass[len - 1] == '\n') {
        is_pass[len - 1] = '\0';
    }

    if (strcmp(password, is_pass) == 0) {
        locked[which_door].state = 1;
        messages("door opened", 0);
        attron(COLOR_PAIR(2));
        mvaddch(py, px, '@');
        map[py][px] = '@';
        attroff(COLOR_PAIR(2));
        refresh();
    } else {
        WINDOW *warning_1 = newwin(win_height, win_width, start_y, start_x);
        wbkgd(warning_1, COLOR_PAIR(11));
        box(warning_1, 0, 0);
        char msg4[] = "WARNING! Try again!";
        int msg4_len = strlen(msg4);
        int start_col4 = (win_width - msg4_len) / 2;

        mvwprintw(warning_1, 3, start_col4, "%s", msg4);
        echo();
        mvwgetstr(warning_1, 5, start_col4, is_pass);
        noecho();

        size_t len = strlen(is_pass);
        if (len > 0 && is_pass[len - 1] == '\n') {
            is_pass[len - 1] = '\0';
        }
        wrefresh(warning_1);
        delwin(warning_1);

        if (strcmp(password, is_pass) == 0) {
            locked[which_door].state = 1;
            messages("door opened", 0);
        } else {
            WINDOW *warning_2 = newwin(win_height, win_width, start_y, start_x);
            wbkgd(warning_2, COLOR_PAIR(8));
            box(warning_2, 0, 0);
            char msg5[] = "WARNING! Last Try!";
            int msg5_len = strlen(msg5);
            int start_col5 = (win_width - msg5_len) / 2;

            mvwprintw(warning_2, 3, start_col5, "%s", msg5);
            echo();
            mvwgetstr(warning_2, 5, start_col5, is_pass);
            noecho();

            size_t len = strlen(is_pass);
            if (len > 0 && is_pass[len - 1] == '\n') {
                is_pass[len - 1] = '\0';
            }
            wrefresh(warning_2);
            delwin(warning_2);

            if (strcmp(password, is_pass) == 0) {
                locked[which_door].state = 1;
                messages("door opened", 0);
            } else {
                WINDOW *warning_3 = newwin(win_height, win_width, start_y, start_x);
                wbkgd(warning_3, COLOR_PAIR(8));
                box(warning_3, 0, 0);
                char msg6[] = "WARNING! Security Lockdown!";
                int msg6_len = strlen(msg6);
                int start_col6 = (win_width - msg6_len) / 2;

                mvwprintw(warning_3, 3, start_col6, "%s", msg6);
                wrefresh(warning_3);
                getch();
                delwin(warning_3);
            }

        }

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
            if (foods[i].state == -1 && foods[i].x == x && foods[i].y == y) {
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
            if (golds[i].state == -1 && golds[i].x == x && golds[i].y == y) {
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
        int num = 1;
        int type = 1;
        for ( int i = 0; i < weapon_count; i ++) {
            if ((weapons[i].state == -1 || weapons[i].thrown == 1) && weapons[i].x == x && weapons[i].y == y) {
                symbol = weapons[i].symbol;
                if (symbol == 'm') type = 1;
                else if (symbol == 'd') {
                    type = 2;
                    num = 10;
                }
                else if (symbol == '~') {
                    type = 3;
                    num = 8;
                }
                else if (symbol == 'a') {
                    type = 4;
                    num = 20;
                }
                else type = 5;
                
                if (weapons[i].num_collect < num) weapons[i].num_collect++;
               // weapons[i].num_collect ++;
                //if (num <  weapons[i].num_collect) weapons[i].num_collect++;
                //else weapons[i].num_collect += weapons[i].num_collect;
                weapons[i].state = 0;
            }
        }
        messages("picked up weapon", type);
        pocket_count++;
    } else if (map[y][x] == 'p') {
        map[y][x] = '.';
        pocket[pocket_count].x = x;
        pocket[pocket_count].y = y;
        pocket[pocket_count].name = "potion";
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
    if (room.type != 0) messages("enter room", room.type);
    
}

void monster_health_check (struct monster * monster) {
    if (monster->health <= 0) monster->state = 1;
    else monster->state = 0;
}

void calculate_score () {
    score = hits*50*0.5 + gold*0.5;
}
void player_attack (int mx, int my, char type) {
    if (potion_time_track == 0) weapon_rate = 1;
    int damage = weapon_rate*5;
    if (type == 'd') damage = weapon_rate*12;
    else if (type == '~') damage = weapon_rate*15;
    else if (type == 'a') damage = weapon_rate*5;
    else if (type == '!') damage = weapon_rate*10;

    for (int i = 0; i < monster_count; i ++) {
        if (monsters[i].state == 0 && monsters[i].level == level && monsters[i].x == mx && monsters[i].y == my) {
            if (type == '~') {
                monsters[i].movement_state = 1;
                messages("monster frozen", i);
            }
            monsters[i].health -= damage;
            messages("player attack", i);
            monster_health_check(&monsters[i]);
            if (monsters[i].state) {
                map[my][mx] = '.';
                messages("monster dead", i);
                if (weapon_in_hand->symbol != 'm' && weapon_in_hand->symbol != '!') weapon_in_hand->num_collect--;
                if (weapon_in_hand->num_collect == 0) weapon_in_hand->state = -1;
                hits ++;
                display_hits();
            }
        }
    }
}

struct ROOM which_room (int px, int py) {
    struct ROOM room;
    room.x = -1;
    room.y = -1;
    for (int i = 0; i < room_count[level]; i++) {
        if (px >= rooms[level][i].x && px <= rooms[level][i].x + rooms[level][i].width &&
            py >= rooms[level][i].y && py <= rooms[level][i].y + rooms[level][i].height) {
            room = rooms[level][i];
        }
    }
    return room;
}
void while_inside_room (int px, int py, struct ROOM room) {
    
    for ( int i = 0; i < monster_count; i ++) {
        if (monsters[i].level == level && monsters[i].state == 0 && (monsters[i].x >= room.x && monsters[i].x <= room.x + room.width) && (monsters[i].y >= room.y && monsters[i].y <= room.y + room.height) && room.type != 1) {
            if (monsters[i].type == 'G' || monsters[i].type == 'S' || monsters[i].type == 'U') {
                monster_move(px, py, &monsters[i]);
            }
            if (monster_check(px, py, monsters[i])) monster_attack(monsters[i]);

        }
    }
    //render_map();
    
}
 
void player_in_room (int px, int py, struct ROOM rooms[], int room_count) {
    struct ROOM room;
    for (int i = 0; i < room_count; i++) {
         room = rooms[i];
        if (px >= room.x && px < room.x + room.width &&
            py >= room.y && py < room.y + room.height) {
            reveal_room(room);
        }
    }
    reveal_corridor(px, py);
    
    //while_inside_room(px, py, room);
    
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
    } else if (tile == 'D' || tile == 'F' || tile == 'G' || tile == 'S' || tile == 'U') {
        return 2;
    }
    else if (tile == '@') {
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
        
    } else if (tile == '|' || tile == '-') {
        for ( int i = 0; i < room_count[level]; i ++) {
            if ((x >= rooms[level][i].x && x <= rooms[level][i].x + rooms[level][i].width ) && (y >= rooms[level][i].y && y <= rooms[level][i].y + rooms[level][i].height ) ) {
                if (rooms[level][i].type == 2) return 5;
                else if (rooms[level][i].type == 1) return 6;
                else return 10;
            }
        }
    }
    
    else if (tile == '$'){
        for ( int m = 0; m < gold_count; m ++) {
            if (golds[m].x == x && golds[m].y == y) {
                return golds[m].type;
            }
        }
    } else if (tile == 'T') {
        return 5;
    }
    else if (map[y][x] == 'm' || map[y][x] == 'd' || map[y][x] == '~' || map[y][x] == 'a' || map[y][x] == '!') {
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
    messages("new level", 0);
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

void hunger_update () {
    time_t current_time;
    time(&current_time);

    if (difftime(current_time, last_hunger_update) >= HEALTH_TIME ) {
        hunger -= HUNGER_R;
        
        if (hunger < 0) {
            hunger = 0;
        }
        last_hunger_update = current_time;
    }
}

void health_update (int maybe) {
    time_t current_time;
    time(&current_time);

    if (difftime(current_time, last_health_update) >= HEALTH_TIME && hunger >= 80 ) {
        int speed = 1;
        if (maybe == 1 && potion_time_track) speed = 2;
        health += speed*HEALTH_R;
        
        if (health < 0) {
            health = 0;
        }
        last_health_update = current_time;
    } else if (difftime(current_time, last_health_update) >= HEALTH_TIME && hunger < 80) {
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

void drop_weapon (int x, int y, struct weapon * weapon) {
    weapon->x = x;
    weapon->y = y;
    weapon->state = 0;
    weapon->thrown = 1;
    map[y][x] = weapon->symbol;
    weapon->num_collect--;
    if (weapon->num_collect == 0) weapon->state = -1;

}


void dagger_wand_arrow_attack(int px, int py, char *direction, int type) {
    int distance;
    char symbol;
    
    if (type == 0) {  // Dagger
        distance = 5;
        symbol = 'd';
    } else if (type == 1) {  // Wand
        distance = 10;
        symbol = '~';
    } else {  // Arrow
        distance = 5;
        symbol = 'a';
    }

    int dx = 0, dy = 0;
    
    if (strcmp(direction, "u") == 0) dy = -1;
    else if (strcmp(direction, "d") == 0) dy = +1;
    else if (strcmp(direction, "l") == 0) dx = -1;
    else if (strcmp(direction, "r") == 0) dx = +1;
    else if (strcmp(direction, "ur") == 0) { dx = +1; dy = -1; }
    else if (strcmp(direction, "ul") == 0) { dx = -1; dy = -1; }
    else if (strcmp(direction, "dl") == 0) { dx = -1; dy = +1; }
    else if (strcmp(direction, "dr") == 0) { dx = +1; dy = +1; }

    bool weapon_used = false;

    for (int i = 1; i <= distance; i++) {
        int new_x = px + i * dx;
        int new_y = py + i * dy;

        if (new_x < 0 || new_x >= WIDTH || new_y < 0 || new_y >= HEIGHT) {
            drop_weapon(px + (i - 1) * dx, py + (i - 1) * dy, weapon_in_hand);
            return;
        }

        if (map[new_y][new_x] == '|' || map[new_y][new_x] == '-' || map[new_y][new_x] == '#') {
            drop_weapon(px + (i - 1) * dx, py + (i - 1) * dy, weapon_in_hand);
            return;
        }

        for (int j = 0; j < monster_count; j++) {
            if (monsters[j].x == new_x && monsters[j].y == new_y) {
                player_attack(monsters[j].x, monsters[j].y, symbol);
                weapon_used = true;
                return;
            }
        }
    }

    if (!weapon_used) {
        drop_weapon(px + distance * dx, py + distance * dy, weapon_in_hand);
    }

    strncpy(last_direction, direction, sizeof(last_direction) - 1);
    last_direction[sizeof(last_direction) - 1] = '\0';

    long_range_weapon = true;
}

/*void a_command (int px, int py) {
    int type;
    if( weapon_in_hand->symbol == 'd') type = 0;
    else if( weapon_in_hand->symbol == '~') type = 1;
    else if( weapon_in_hand->symbol == 'a') type = 2;

    dagger_wand_arrow_attack(px, py, last_direction, type);
}*/
void display_hits () {
    attron(COLOR_PAIR(2));
    mvprintw(LINES -1, COLS/2 + 12 - 55, "Hits: %d", hits);
    attroff(COLOR_PAIR(2));
    refresh();
}

void desplay_gold () {
    attron(COLOR_PAIR(5));
    mvprintw(LINES -1, COLS/2 + 12, "Gold: %d", gold);
    attroff(COLOR_PAIR(5));
    refresh();
}

void monster_move(int px, int py, struct monster *m) {
    
    if (m->type == 'G' || m->type == 'U') if (m->movement_num > 5) return;
    if (m->movement_state == 1) return;
    int my = m->y;
    int mx = m->x;

    int nx = mx;
    int ny = my;

    if (mx < px && mx + 1 < WIDTH && map[my][mx + 1] == '.') {
        nx++;
    } else if (mx > px && mx - 1 >= 0 && map[my][mx - 1] == '.') {
        nx--;
    } else if (my < py && my + 1 < HEIGHT && map[my + 1][mx] == '.') {
        ny++;
    } else if (my > py && my - 1 >= 0 && map[my - 1][mx] == '.') {
        ny--;
    }

    if (nx == px && ny == py) {
        monster_attack(*m);
        return;
    }

    if (nx != mx || ny != my) {
        map[my][mx] = '.';
        m->x = nx;
        m->y = ny;
        m->movement_num ++;
        map[ny][nx] = m->type;
    }
    
    render_map();
}

struct monster monster_in_room (int px, int py ) {
    struct monster monster;
    monster.x = -1;
    monster.y = -1;

    for ( int i = 0; i < monster_count; i ++) {
        if (monsters[i].state == 0 && monsters[i].level == level && (monsters[i].x == px || monsters[i].x == px + 1 || monsters[i].x == px - 1) && ( monsters[i].y == py || monsters[i].y == py + 1 || monsters[i].y == py - 1)) {
            return monsters[i];
        }
    }
    return monster;
}
int monster_check (int x, int y, struct monster monster) {
    if (monster.state == 0 && (monster.x == x || monster.x == x + 1 || monster.x == x - 1) && ( monster.y == y || monster.y == y + 1 || monster.y == y - 1)) {
        return 1;
    } else return 0;
}

void monster_attack (struct monster monster)  {
    char type = monster.type;
    if (type == 'D') {
        messages("attack", 0);
        lose_health(5);
    }
    else if (type == 'F') {
        messages("attack", 1);
        lose_health(5);
    }
    else if (type == 'G') {
        messages("attack", 2);
        lose_health(5);
    }
    else if (type == 'S') {
        messages("attack", 3);
        lose_health(5);
    }
    else if (type == 'U') {
        messages("attack", 4);
        lose_health(5);
    }

}
void hunger_bar (int hunger) {
    if (hunger > 100) hunger = 100;
    if (hunger <= 0) end_game('l');
    //if (health > 0 && health <= 50) messages("low health", 0);
    int filled = (hunger * 20) / MAX_HEALTH;
    move(LINES - 1, COLS - 20 - 10 - 56);
    addch('[');
    for (int i = 0; i < 20; i++) {
        if (i < filled) {
            attron(COLOR_PAIR(2));
            addch('#');
            attroff(COLOR_PAIR(2));
            //refresh();
            
        } else {
            attron(COLOR_PAIR(2));
            addch('-');
            attroff(COLOR_PAIR(2));
            //refresh();
        }
    }
    addch(']');
    mvprintw(LINES -1, COLS - 39 - 56 , "Hunger: ");
    if (hunger <= 100 && hunger > 80) mvprintw(LINES - 1,COLS -8 - 56, "Full");
    else if (hunger <= 80 && hunger > 60) mvprintw(LINES - 1,COLS -8 - 56, "Satisfied");
    else if (hunger <= 60 && hunger > 40) mvprintw(LINES - 1,COLS -8 - 56, "Hungry");
    else if (hunger <= 40 && hunger > 20) mvprintw(LINES - 1,COLS -8 - 56, "Starving");
    else if (hunger <= 20 && hunger > 0) mvprintw(LINES - 1,COLS -8 - 56, "Dying");
    refresh();

}
void health_bar (int health) {
    if (health > 100) health = 100;
    if (health <= 0) end_game('l');
    //if (health > 0 && health <= 50) messages("low health", 0);
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
    mvprintw(LINES - 1, COLS -8, "%d%%", (health * 100) / MAX_HEALTH);
    refresh();

}

void food_choice (char * name ) {
    for ( int i = 0; i < food_count; i ++) {
        if (strcmp(foods[i].name, name) == 0 && foods[i].state == 0) {
            if (foods[i].fresh == 0 || foods[i].fresh == 1)  {
                int type = 0;
                if (foods[i].fresh == 1 && foods[i].color == 5) {
                    dragon_blood();
                    type = 1;
                } else if (foods[i].fresh == 1 && foods[i].color == 6) {
                    storm_kiss();
                    type = 2;
                    ate_magic_food = true;
                } foods[i].state =1;
                health += 5;
                hunger += 5;
                health_bar(health);
                hunger_bar(hunger);
                messages("ate food", type);
                break;
            } else {
                foods[i].state =1;
                health -= 5;
                hunger -= 5;
                health_bar(health);
                hunger_bar(hunger);
                messages("ate spoiled food", 0);
                break;
            }
        }
    }
    food_time_out = 50;
    //food_window();
}

void food_window () {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int win_h = 10, win_w = 50;
    int start_y = (max_y - win_h) / 2;
    int start_x = (max_x - win_w) / 2;

    WINDOW * food = newwin(win_h, win_w, start_y, start_x);
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
    wattron(food,COLOR_PAIR(6));
    int len = strlen("** FOOD **");
    mvwprintw(food, 1, (win_w - len)/2, "** FOOD **");
    wattroff(food,COLOR_PAIR(6));
    
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
        mvwprintw(food, identifier + 1, 1, "%d. %d Slightly Moldy Cheese",identifier, cheese);
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
        int x = (win_w - strlen(text)) / 2;
        mvwprintw(food, 4, x, "%s", text);
    }
    
    wrefresh(food);

    int choice = getch();
    choice = choice - '0';
    if (choice == ex_berry_id) {
        food_choice("Exploding Berries");
    }
    else if (choice == eth_berry_id) {
        food_choice("Ethereal Berries");
    }
    else if (choice == pie_id) {
        food_choice("Potionberry Pie");
    }
    else if (choice == amb_id) {
        food_choice("Ambrosia");
    }
    else if (choice == cheese_id) {
        food_choice("Slightly Moldy Cheese");
    }
    else if (choice == biscuit_id) {
        food_choice("Rock-hard Biscuit");
    }
    else if (choice == steak_id) {
        food_choice("Infernal Steak");
    }
    else if (choice == meat_id) {
        food_choice("Mystery Meat");
    }
    else if (choice == apple_id) {
        food_choice("Questionable Apple");
    }
    
    if (choice == 'q') return;
}


void weapon_in_bag() {
    weapon_in_hand->state = 0;
    wield_choice = '0';
}

void weapon_choice(char symbol) {
    if (weapon_in_hand->symbol == symbol) return;

    weapon_in_bag();

    for (int i = 0; i < weapon_count; i++) {
        if (weapons[i].symbol == symbol && weapons[i].state == 0) {
            weapons[i].state = 1;
            weapon_in_hand = &weapons[i];
            wield_choice = symbol;
            break;
        }
    }
}

void weapon_window() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_height = 12, win_width = 80;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    WINDOW *arsenal = newwin(win_height, win_width, start_y, start_x);
    wclear(arsenal);
    box(arsenal, 0, 0);

    int len = strlen("** ARSENAL **");
    mvwprintw(arsenal, 1, (win_width - len)/2, "** ARSENAL **");

    for (int i = 2; i < win_height - 1; i++) {
          mvwaddch(arsenal, i, 39, '|');
      }
    
    int dag = 0, wand = 0, arrow = 0, sword = 0;
    for (int i = 0; i < weapon_count; i++) {
        if (weapons[i].symbol == 'd' && weapons[i].state != -1) dag += weapons[i].num_collect;
        if (weapons[i].symbol == '~' && weapons[i].state != -1) wand += weapons[i].num_collect;
        if (weapons[i].symbol == 'a' && weapons[i].state != -1) arrow += weapons[i].num_collect;
        if (weapons[i].symbol == '!' && weapons[i].state != -1) sword += weapons[i].num_collect;
       // if (weapons[i].symbol == 'm' && weapons[i].state != -1) mace += weapons[i].num_collect;
    }

    int identifier = 1;
    int short_y = 3, long_y = 3;

    mvwprintw(arsenal, short_y, 5, "Short-Range Weapons:");
    mvwprintw(arsenal, long_y, 45, "Long-Range Weapons:");

    short_y += 2;
    long_y += 2;
    //int color = 10;
    
    int mac_id = 0, dag_id = 0, wand_id = 0, arrow_id = 0, sword_id = 0;

    if (1) {
        int color = 10;
        if ('m' == wield_choice) color = 6;
        wattron(arsenal, COLOR_PAIR(color));
        //mac_id = identifier;
        mvwprintw(arsenal, short_y++, 5, "[m] Mace (%d)", 1);
        wattroff(arsenal, COLOR_PAIR(color));
        wattron(arsenal, COLOR_PAIR(2));
        mvwprintw(arsenal, short_y++, 5, "damage = 5");
        wattroff(arsenal, COLOR_PAIR(2));

    }
    if (dag > 0) {
        int color = 10;
        if ('d' == wield_choice) color = 6;
        wattron(arsenal, COLOR_PAIR(color));
        //dag_id = identifier;
        mvwprintw(arsenal, long_y++, 45, "[d] Dagger (%d)",  dag);
        wattroff(arsenal, COLOR_PAIR(color));
        wattron(arsenal, COLOR_PAIR(2));
        mvwprintw(arsenal, long_y++, 45, "distance = 5, damage = 5");
        wattroff(arsenal, COLOR_PAIR(2));
    }
    if (sword > 0) {
        int color = 10;
        if ('!' == wield_choice) color = 6;
        wattron(arsenal, COLOR_PAIR(color));
        //sword_id = identifier;
        mvwprintw(arsenal, short_y++, 5, "[!] Sword (%d)", sword);
        wattroff(arsenal, COLOR_PAIR(color));
        wattron(arsenal, COLOR_PAIR(2));
        mvwprintw(arsenal, short_y++, 5, "damage = 10");
        wattroff(arsenal, COLOR_PAIR(2));

    }
    if (wand > 0) {
        int color = 10;
        if ('~' == wield_choice) color = 6;
        wattron(arsenal, COLOR_PAIR(color));
        //wand_id = identifier;
        mvwprintw(arsenal, long_y++, 45, "[~] Magic Wand (%d)", wand);
        wattroff(arsenal ,COLOR_PAIR(color));
        wattron(arsenal, COLOR_PAIR(2));
        mvwprintw(arsenal, long_y++, 45, "distance = 10, damage = 12");
        wattroff(arsenal, COLOR_PAIR(2));

    }
    if (arrow > 0) {
        int color = 10;
        if ('a' == wield_choice) color = 6;
        wattron(arsenal, COLOR_PAIR(color));
        //arrow_id = identifier;
        mvwprintw(arsenal, long_y++, 45, "[a] Normal Arrow (%d)", arrow);
        wattroff(arsenal, COLOR_PAIR(color));
        wattron(arsenal, COLOR_PAIR(2));
        mvwprintw(arsenal, long_y++, 45, "distance = 5, damage = 5");
        wattroff(arsenal, COLOR_PAIR(2));

    }
    
    wrefresh(arsenal);

    int choice = wgetch(arsenal);
    if (wield_choice != '0') {
        messages("put in bag", 0);
        delwin(arsenal);
        return;
    }
    if (choice == 'm' || choice == 'd' || choice == '!' || choice == '~' || choice == 'a') {
        weapon_choice(choice);
        wield_choice = choice;
        messages("took weapon", 0);
    }
    delwin(arsenal);
    refresh();
}

void potion_choice (int type) {

    
    for ( int i = 0; i < potion_count; i ++) {
        if (potions[i].type == type && potions[i].state == 0) {
            potions[i].state = 1;
            messages("took potion", type);
            drank_potion = true;
            break;
        }
    }
    if (type == 0) elixir_of_everlife();
    else if (type == 1) storm_kiss();
    else dragon_blood();
    potion_time_track = 10;
        
}
void potion_window () {
    int height = 12, width = 40;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW * potion = newwin(height, width, start_y, start_x);
    wclear(potion);
    box(potion, 0, 0);
    
    int len = strlen("** POTIONS **");
    mvwprintw(potion, 1, (width - len) / 2, "** POTIONS **");

    int elix = 0, drag = 0, kiss = 0;
    for (int i = 0; i < potion_count; i++) {
        if (potions[i].type == 0 && potions[i].state == 0) elix++;
        if (potions[i].type == 2 && potions[i].state == 0) drag++;
        if (potions[i].type == 1 && potions[i].state == 0) kiss++;
    }
    
    //int elix_id = 0, drag_id = 0, kiss_id = 0;
    int identifier = 1;
    if (elix != 0) {
        mvwprintw(potion, identifier++ , 1, "[e] %d Elixir of Everlife", elix);
    }
    if (drag != 0) {
        mvwprintw(potion, identifier++ + 1, 1, "[d] %d Dragon's Blood", drag);
    }
    if (kiss != 0) {
        mvwprintw(potion, identifier++ + 1, 1, "[s] %d Stormrider's Kiss", kiss);
    }
    if (elix == 0 && drag == 0 && kiss == 0) {
        char text[] = "You don't have any potions to drink!";
        int x = (width - strlen(text)) / 2;
        mvwprintw(potion, 4, x, "%s", text);
    }

    wrefresh(potion);
    int choice = wgetch(potion);

    if (choice == 'e') potion_choice(0);
    else if (choice == 'd') potion_choice(2);
    else if (choice == 's') potion_choice(1);
    delwin(potion);
}

void inventory_window() {
    int height = 10, width = 50;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;

    WINDOW *inventory = newwin(height, width, start_y, start_x);
    keypad(inventory, TRUE);
    wclear(inventory);
    box(inventory, 0, 0);

    const char *options[] = {"WEAPONS", "FOOD", "POTIONS"};
    int num_options = 3;
    int selected = 0;
    int key;

    while (1) {
        wclear(inventory);
        box(inventory, 0, 0);
        mvwprintw(inventory, 1, (width - strlen("** INVENTORY **")) / 2, "** INVENTORY **");
        wattron(inventory, COLOR_PAIR(6));
        mvwprintw(inventory, 2, (width - 32) / 2, "     /| ___________________");
        mvwprintw(inventory, 3, (width - 32) / 2, "O|===|* >__________________>");
        mvwprintw(inventory, 4, (width - 32) / 2, "     \\|");
        wattron(inventory, COLOR_PAIR(6));

        for (int i = 0, x = 5; i < num_options; i++, x += 15) {
            if (i == selected) {
                wattron(inventory, COLOR_PAIR(4));
                mvwprintw(inventory, 6, x, "[ %s ]", options[i]);
                wattroff(inventory, COLOR_PAIR(4));
            } else {
                wattron(inventory, COLOR_PAIR(10));
                mvwprintw(inventory, 6, x, " %s ", options[i]);
                wattroff(inventory, COLOR_PAIR(10));

            }
        }

        wrefresh(inventory);
        key = wgetch(inventory);

        if (key == KEY_RIGHT) {
            selected = (selected + 1) % num_options;
        } else if (key == KEY_LEFT) {
            selected = (selected - 1 + num_options) % num_options;
        } else if (key == '\n') {
            break;
        }
    }

    delwin(inventory);

    switch (selected) {
        case 0: {
            weapon_window();
            break;
        }
        case 1: {
            food_window();
            break;
        }
        case 2: {
            potion_window();
            break;
        }
    }
}

void p_command () {
    mvprintw(0, 0,"What potion would you like to drink? (press * for the list)");
    getch();
    potion_window();
}
void i_command () {
    //mvprintw(0, 0,"What weapon would you like to wield? (press * for the list)");
    //getch();
   // weapon_window();
    inventory_window();
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
                int type = rand() % 4;
                char symbol;
                int num;
               if (type == 0) {
                    symbol = 'd';
                   num = 10;
                } else if (type == 1) {
                    symbol = '~';
                    num = 8;
                } else if (type == 2) {
                    symbol = 'a';
                    num = 20;
                } else {
                    symbol = '!';
                    num = 1;
                }
                if (weapon_count == 0) {
                    symbol = 'm';
                    num = 1;
                }
                weapons[weapon_count].symbol = symbol;
                if (weapon_count) map[y][x] = symbol;
                weapons[weapon_count].x = x;
                weapons[weapon_count].y = y;
                weapons[weapon_count].state = -1;
                weapons[weapon_count].num_collect = num;
                weapon_count++;
            }
        }
    }
    if (level == 1) {
        weapon_in_hand = &weapons[0];
    }
}

void picked_tresure () {
    
    messages("picked up treasure", 0);
    end_game('w');
    
}

void add_treasure (struct ROOM room) {
    bool treasure = false;
    for (int y = room.y; y < room.y + room.height; y++) {
        if (treasure) return;
        for (int x = room.x; x < room.x + room.width; x++) {
            if (treasure) return;
            if (rand () % 20 == 0 && map[y][x] == '.') {
                attron(COLOR_PAIR(9));
                map[y][x] = 'T';
                attroff(COLOR_PAIR(9));
                treasure = true;
                refresh();

            }
        }
    }
    
    if (!treasure) {
        int center_x = room.x + room.width / 2;
        int center_y = room.y + room.height / 2;
        map[center_y][center_x] = 'T';
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

void elixir_of_everlife () {
   // health_update(1);
    health += 10;
    health_bar(health);
}

void dragon_blood () {
    weapon_rate = 2;
    potion_time_track = 10;
    //food_time_out = 50;
}

void storm_kiss () {
    num_of_blocks = 2;
    potion_time_track = 10;
    //food_time_out = 50;
}
void add_potion (struct ROOM room) {
    int prob;
    int room_type = room.type;
    if (room_type == 1) prob = 10;
    else prob = 50;
    for (int y = room.y; y < room.y + room.height; y++) {
       // if (food_count >= MAX_FOOD) break;
        for (int x = room.x; x < room.x + room.width; x++) {
            //if (food_count >= MAX_FOOD) break;
            if (rand () % prob == 0 && map[y][x] == '.') {
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

void spoil_normal () {
    for (int j = 0; j < food_count; j ++) {
        if (foods[j].fresh  == 1 && foods[j].state == 0) foods[j].fresh  = 0; //turn to normal
        else if (foods[j].fresh  == 0 && foods[j].state == 0) foods[j].fresh = -1; //rotten
    }
    food_time_out = 50;
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
                    foods[food_count].fresh = 1;
                } else if (color == 1) {
                    color = 2;
                    foods[food_count].fresh  = 0;
                } else {
                    color = 6;
                    foods[food_count].fresh  = 1;
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

void save_game (char * name, int px, int py) {
    char arg [35];
    strcpy(arg, name);
    strcat(arg, ".txt");
    FILE * save_game = fopen(arg, "w");
    
    fprintf(save_game, "Name: %s", user_name);
    fprintf(save_game, "Health: %d", health);
    fprintf(save_game, "Hunger: %d", hunger);
    fprintf(save_game, "Hits: %d", hits);
    fprintf(save_game, "Gold: %d", gold);
    fprintf(save_game, "Level: %d", level);
    fprintf(save_game, "Position: %d, %d", py, px);
    fprintf(save_game, "food_count: %d", food_count);
    fprintf(save_game, "locked_door_count: %d", locked_door_count);
    fprintf(save_game, "secret_door_count: %d", secret_door_count);
    fprintf(save_game, "traps_count: %d", traps_count);
    fprintf(save_game, "gold_count: %d", gold_count);
    fprintf(save_game, "monster_count: %d", monster_count);
    fprintf(save_game, "potion_count: %d", potion_count);
    fprintf(save_game, "weapon_count: %d", weapon_count);

    
    fprintf(save_game, "FOOD:");
    for ( int k = 0; k < food_count; k ++) {
        fprintf(save_game, "%s %d %d %d %d",foods[k].name, foods[k].state, foods[k].color, foods[k].x, foods[k].y );
    }
    
    fprintf(save_game, "LOCKED DOORS:");
    for ( int k = 0; k < locked_door_count; k ++) {
        fprintf(save_game, "%d %d %d",locked[k].state, locked[k].x, locked[k].y );
    }
    
    fprintf(save_game, "HIDDEN DOORS:");
    for ( int k = 0; k < secret_door_count; k ++) {
        fprintf(save_game, "%d %d %d",hiddens[k].state, hiddens[k].x, hiddens[k].y );
    }
    
    fprintf(save_game, "TRAPS:");
    for ( int k = 0; k < traps_count; k ++) {
        fprintf(save_game, "%d %d %d",traps[k].state, traps[k].x, traps[k].y );
    }
    
    fprintf(save_game, "GOLDS:");
    for ( int k = 0; k < gold_count; k ++) {
        fprintf(save_game, "%d %d %d %d",golds[k].type, golds[k].state, golds[k].x, golds[k].y );
    }
    
    fprintf(save_game, "MONSTERS:");
    for ( int k = 0; k < monster_count; k ++) {
        fprintf(save_game, "%c %d %d %d %d %d %d %d",monsters[k].type, monsters[k].state, monsters[k].num, monsters[k].health, monsters[k].movement_num, monsters[k].movement_state, monsters[k].x, monsters[k].y );
    }
    
    fprintf(save_game, "POTIONS:");
    for ( int n = 0; n < potion_count; n ++) {
        fprintf(save_game, "%s %d %d %d %d",potions[n].name, potions[n].state, potions[n].type, potions[n].x, potions[n].y );
    }
    
    fprintf(save_game, "WEAPONS:");
    for ( int m = 0; m < weapon_count; m ++) {
        fprintf(save_game, "%c %d %d %d %d",weapons[m].symbol, weapons[m].num_collect, weapons[m].state, weapons[m].x, weapons[m].y );
    }
    
    fprintf(save_game, "MAP:");

    for ( int j = 0; j < HEIGHT; j ++) {
        for ( int i = 0; i < WIDTH; i ++) {
            int visited = 0;
            if (visible[j][i]) visited = 1;
            else visited = 0;
            fprintf(save_game, "%c %d",map[j][i], visited );
        }
    }
    
}

void load_game(char * file_name, int *px, int *py) {
    FILE *save_file = fopen("prev_games.txt", "r");
    if (!save_file) {
        perror("No saved game found.");
        return;
    }

    fscanf(save_file, "Name: %s", user_name);
    fscanf(save_file, "Health: %d", &health);
    fscanf(save_file, "Hunger: %d", &hunger);
    fscanf(save_file, "Hits: %d", &hits);
    fscanf(save_file, "Gold: %d", &gold);
    fscanf(save_file, "Level: %d", &level);
    fscanf(save_file, "Position: %d, %d", py, px);
    fscanf(save_file, "food_count: %d", &food_count);
    fscanf(save_file, "locked_door_count: %d", &locked_door_count);
    fscanf(save_file, "secret_door_count: %d", &secret_door_count);
    fscanf(save_file, "traps_count: %d", &traps_count);
    fscanf(save_file, "gold_count: %d", &gold_count);
    fscanf(save_file, "monster_count: %d", &monster_count);
    fscanf(save_file, "potion_count: %d", &potion_count);
    fscanf(save_file, "weapon_count: %d", &weapon_count);

    fscanf(save_file, "FOOD:");
    for (int k = 0; k < food_count; k++) {
        fscanf(save_file, "%s %d %d %d %d", foods[k].name, &foods[k].state, &foods[k].color, &foods[k].x, &foods[k].y);
    }

    fscanf(save_file, "LOCKED DOORS:");
    for (int k = 0; k < locked_door_count; k++) {
        fscanf(save_file, "%d %d %d", &locked[k].state, &locked[k].x, &locked[k].y);
    }

    fscanf(save_file, "HIDDEN DOORS:");
    for (int k = 0; k < secret_door_count; k++) {
        fscanf(save_file, "%d %d %d", &hiddens[k].state, &hiddens[k].x, &hiddens[k].y);
    }

    fscanf(save_file, "TRAPS:");
    for (int k = 0; k < traps_count; k++) {
        fscanf(save_file, "%d %d %d", &traps[k].state, &traps[k].x, &traps[k].y);
    }

    fscanf(save_file, "GOLDS:");
    for (int k = 0; k < gold_count; k++) {
        fscanf(save_file, "%d %d %d %d", &golds[k].type, &golds[k].state, &golds[k].x, &golds[k].y);
    }

    fscanf(save_file, "MONSTERS:");
    for (int k = 0; k < monster_count; k++) {
        fscanf(save_file, "%c %d %d %d %d %d %d %d",
               &monsters[k].type, &monsters[k].state, &monsters[k].num, &monsters[k].health,
               &monsters[k].movement_num, &monsters[k].movement_state, &monsters[k].x, &monsters[k].y);
    }

    fscanf(save_file, "POTIONS:");
    for (int n = 0; n < potion_count; n++) {
        fscanf(save_file, "%s %d %d %d %d", potions[n].name, &potions[n].state, &potions[n].type, &potions[n].x, &potions[n].y);
    }

    fscanf(save_file, "WEAPONS:");
    for (int m = 0; m < weapon_count; m++) {
        fscanf(save_file, "%c %d %d %d %d", &weapons[m].symbol, &weapons[m].num_collect, &weapons[m].state, &weapons[m].x, &weapons[m].y);
    }

    fscanf(save_file, "MAP:");
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            char tile;
            int visited;
            fscanf(save_file, "%c %d", &tile, &visited);
            map[j][i] = tile;
            visible[j][i] = visited;
        }
    }

    fclose(save_file);
    //printf("Game loaded successfully!\n");
}

void get_game_name (char * save_name) {
    int height = 12, width = 40;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW *name_win = newwin(height, width, start_y, start_x);
    box(name_win, 0, 0);
    keypad(name_win, TRUE);
    curs_set(1);
    
    mvwprintw(name_win, 1, (width - strlen("Enter game's name:")) / 2 - 1, "Enter game's name:");
    wrefresh(name_win);
    
    int ch, index = 0;
    save_name[0] = '\0';
    
    while (1) {
        ch = wgetch(name_win);
        
        if (ch == '\n' && index > 0) {
            save_name[index] = '\0';
            break;
        } else if (ch == 27) {
            save_name[0] = '\0';
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (index > 0) {
                index--;
                save_name[index] = '\0';
            }
        } else if (index < 29 && ch >= 32 && ch <= 126) {
            save_name[index++] = (char)ch;
            save_name[index] = '\0';
        }
        
        mvwprintw(name_win, 4, (width - 30) / 2, "%-20s", save_name);
        wrefresh(name_win);
    }
    
    curs_set(0);
    delwin(name_win);
    show_pop_up("Your game was successfully saved!", 0, 0);
}

void lose_art () {
    int rows, cols;
      
    getmaxyx(stdscr, rows, cols);
    int start_y = 6;
    int start_x = 25;
    attron(COLOR_PAIR(10));
    mvprintw(start_y++, start_x, "                                 _____  _____");
    mvprintw(start_y++, start_x, "                                <     `/     |");
    mvprintw(start_y++, start_x, "                                 >          (");
    mvprintw(start_y++, start_x, "                                |   _     _  |");
    mvprintw(start_y++, start_x, "                                |  |_) | |_) |");
    mvprintw(start_y++, start_x, "                                |  | \\ | |   |");
    mvprintw(start_y++, start_x, "                 ______.________|            |__________  _____");
    mvprintw(start_y++, start_x, "               _/                                       \\|     |");
    attroff(COLOR_PAIR(10));
    attron(COLOR_PAIR(6));
    mvprintw(start_y++, start_x, "              |                   %-20.20s                    ", user_name);
    attroff(COLOR_PAIR(6));
    mvprintw(start_y++, start_x, "              |_____.-._________              ____/|___________|");
    mvprintw(start_y++, start_x, "                                |            |");
    mvprintw(start_y++, start_x, "                                |  01/06/85  |");
    mvprintw(start_y++, start_x, "                                |            |");
    mvprintw(start_y++, start_x, "                                |            |");
    mvprintw(start_y++, start_x, "                                |   _        <");
    mvprintw(start_y++, start_x, "                                |__/         |");
    mvprintw(start_y++, start_x, "                                 / `--.      |");
    attron(COLOR_PAIR(9));
    mvprintw(start_y++, start_x, "                                %|            |%");
    mvprintw(start_y++, start_x, "                            |/.%%|          -< @%%%");
    mvprintw(start_y++, start_x, "                             `\%`@|     v      |@@%@%%    ");
    mvprintw(start_y++, start_x, "                           .%%%@@@|%    |    % @@@%%@%%%%");
    mvprintw(start_y++, start_x, "                      _.%%%%%%@@@@@@%%_/%\\_%@@%%@@@@@@@%%%%%%");
    attroff(COLOR_PAIR(9));
    
    refresh();
}

void win_art () {
    int rows, cols;
      
    getmaxyx(stdscr, rows, cols);
    int start_y = 3;
    int start_x = 45;
    attron(COLOR_PAIR(2));
    mvprintw(start_y++, start_x, "                          ==(W{==========-      /===-");
    mvprintw(start_y++, start_x, "                            ||  (.--.)         /===-_---~~~~~~~~~------____");
    mvprintw(start_y++, start_x, "                            | \\_,|**|,__      |===-~___                _,-'`");
    mvprintw(start_y++, start_x, "               -==\\\\        `\\ ' `--'   ),    `//~\\\\   ~~~~`---.___.-~~");
    mvprintw(start_y++, start_x, "             ______-==|        /`\\_. .__/\\ \\    | |  \\\\           _-~`");
    mvprintw(start_y++, start_x, "       __--~~~  ,-/-==\\\\      (   | .  |~~~~|   | |   `\\        ,'");
    mvprintw(start_y++, start_x, "    _-~       /'    |  \\\\     )__/==0==-\\<>/   / /      \\      /");
    mvprintw(start_y++, start_x, "  .'        /       |   \\\\      /~\\___/~~\\/  /' /        \\   /'");
    mvprintw(start_y++, start_x, " /  ____  /         |    \\`\\.__/-~~   \\  |_/'  /          \\/'");
    mvprintw(start_y++, start_x, "/-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`");
    mvprintw(start_y++, start_x, "                  \\_|      /        _) | ;  ),   __--~~");
    mvprintw(start_y++, start_x, "                    '~~--_/      _-~/- |/ \\   '-~ \\");
    mvprintw(start_y++, start_x, "                   {\\__--_/}    / \\\\_>-|)<__\\      \\");
    mvprintw(start_y++, start_x, "                   /'   (_/  _-~  | |__>--<__|      |");
    mvprintw(start_y++, start_x, "                  |   _/) )-~     | |__>--<__|      |");
    mvprintw(start_y++, start_x, "                  / /~ ,_/       / /__>---<__/      |");
    mvprintw(start_y++, start_x, "                 o-o _//        /-~_>---<__-~      /");
    mvprintw(start_y++, start_x, "                 (^(~          /~_>---<__-      _-~");
    mvprintw(start_y++, start_x, "                ,/|           /__>--<__/     _-~");
    mvprintw(start_y++, start_x, "             ,//('(          |__>--<__|     /                  .----_");
    mvprintw(start_y++, start_x, "            ( ( '))          |__>--<__|    |                 /' _---_~\\");
    mvprintw(start_y++, start_x, "         `-)) )) (           |__>--<__|    |                   /'  /     ~\\`\\");
    mvprintw(start_y++, start_x, "        ,/,'//( (             \\__>--<__\\    \\            /'  //        ||");
    mvprintw(start_y++, start_x, "      ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'");
    mvprintw(start_y++, start_x, "    `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/");
    mvprintw(start_y++, start_x, "  ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~");
    mvprintw(start_y++, start_x, "   ;'( ')/ ,)(                              ~~~~~~~~~~");
    mvprintw(start_y++, start_x, "  ' ') '( (/");
    attroff(COLOR_PAIR(2));
    
    refresh();
}
void end_game(char state) {
    clear();
    refresh();

    int height = 12, width = 40;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    
    if (state == 'w') win_art();
    else lose_art();
    int x = 10;
    int z = 5;
    int color = 10;
    if (state == 'w') color = 9;
    else color = 2;
    attron(COLOR_PAIR(color) | A_BOLD);
    if (state == 'w') mvprintw(2+x, 12, "== YOU WIN! ==");
    else mvprintw(2, (cols - strlen("== YOU LOSE! =="))/2, "== YOU LOSE! ==");
    attroff(COLOR_PAIR(color) | A_BOLD);

    if (state == 'w') {
        attron(COLOR_PAIR(5));
        mvprintw(4 +x, 2 + z, "With the treasure in your hands,");
        mvprintw(5 +x, 2 +z, "the dungeon fades behind as you emerge victorious!");
        attroff(COLOR_PAIR(5));
    } else {
        attron(COLOR_PAIR(5));
        mvprintw(3, (cols - strlen("Darkness consumes you as you fall...")) / 2, "Darkness consumes you as you fall...");
        mvprintw(4, (cols - strlen("The dungeon claims another soul."))/2, "The dungeon claims another soul.");
        attroff(COLOR_PAIR(5));
    }

    //mvprintw(7+x, 2+z, "Press any key to enter Hall of Fame!");

    refresh();
    getch();

    calculate_score();
    get_score(user_name, score, gold);
    hall_of_fame();
}

void resume_save_window (int px, int py) {
    int height = 12, width = 40;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;

    WINDOW *resume = newwin(height, width, start_y, start_x);
    keypad(resume, TRUE);
    wclear(resume);
    box(resume, 0, 0);

    const char *options[] = {"Quit", "Save", "Exit"};
    int num_options = 3;
    int selected = 0;
    int key;

    while (1) {
        wclear(resume);
        box(resume, 0, 0);
        mvwprintw(resume, 1, (width - strlen("** RESUME MENU **")) / 2 + 1, "** RESUME MENU **");
        wattron(resume, COLOR_PAIR(6));
        mvwprintw(resume, 2, (width - 32) / 2, " \\\\\\\\\\_____________________\\\"-._");
        mvwprintw(resume, 3, (width - 32) / 2, " /////~~~~~~~~~~~~~~~~~~~~~/.-'");
        wattron(resume, COLOR_PAIR(6));

        for (int i = 0, x = 3; i < num_options; i++, x += 12) {
            if (i == selected) {
                wattron(resume, COLOR_PAIR(6));
                mvwprintw(resume, 6, x, "[ %s ]", options[i]);
                wattroff(resume, COLOR_PAIR(6));
            } else {
                wattron(resume, COLOR_PAIR(10));
                mvwprintw(resume, 6, x, " %s ", options[i]);
                wattroff(resume, COLOR_PAIR(10));

            }
        }

        wrefresh(resume);
        key = wgetch(resume);

        if (key == KEY_RIGHT) {
            selected = (selected + 1) % num_options;
        } else if (key == KEY_LEFT) {
            selected = (selected - 1 + num_options) % num_options;
        } else if (key == '\n') {
            break;
        }
    }

    delwin(resume);

    switch (selected) {//quit
        case 0: {
            clear();
            lobby_art();
            start_game_menu();
            break;
        }
        case 1: {//save
            char name_game [30];
            get_game_name(name_game);
            save_game(name_game, px, py);
            clear();
            lobby_art();
            start_game_menu();
            break;
        }
        case 2: {//exit
            break;
        }
    }
}

void cheat_g () {
    g_state = 1;
}

void fix_edges(struct ROOM room) {
    for (int y = room.y; y < room.y + room.height; y++) {
        if (room.x > 0 && map[y][room.x - 1] == '#') {
            map[y][room.x] = '+';
        }
    }

    for (int y = room.y; y < room.y + room.height; y++) {
        if (room.x + room.width < WIDTH && map[y][room.x + room.width] == '#') {
            map[y][room.x + room.width - 1] = '+';
        }
    }

    for (int x = room.x; x < room.x + room.width; x++) {
        if (room.y > 0 && map[room.y - 1][x] == '#') {
            map[room.y][x] = '+';
        }
    }

    for (int x = room.x; x < room.x + room.width; x++) {
        if (room.y + room.height < HEIGHT && map[room.y + room.height][x] == '#') {
            map[room.y + room.height - 1][x] = '+';
        }
    }
}

void generate_map (){
    clear();
    curs_set(0);
    bool treasure_room_place = false;
    init_map();
    while (room_count[level] < ROOM_COUNT) {
        int type = 0;
        struct ROOM new_room;
        new_room.width = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1);
        new_room.height = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1);
        new_room.x = rand() % (WIDTH - new_room.width - 1);
        new_room.y = rand() % (HEIGHT - new_room.height - 1);
        
        bool overlap = false;
        
        for ( int i = 0; i < room_count[level] ; i ++ ){
            if (room_overlap(new_room, rooms[level][i])) {
                overlap = true;
                break;
            }
        }
        
        if (!overlap) {
           if ( level < 4) {
                int prob = rand () % 6;
                if (prob == 0) new_room.type = 1;
                else new_room.type = 0;
            } else new_room.type = 0;
            door_fix(new_room);
            add_room(new_room);
            add_pillar(new_room);
            add_trap(new_room);
            add_food(new_room);
            add_gold(new_room);
            add_weapon(new_room);
            add_potion(new_room);
            if (new_room.type != 1) add_monster(new_room);
            
            if (room_count[level] > 0) {
                corridor(
                         rooms[level][room_count[level] - 1].x + rooms[level][room_count[level] - 1].width/ 2,
                    rooms[level][room_count[level] - 1].y + rooms[level][room_count[level] - 1].height/ 2,
                    new_room.x + (new_room.width - 2) / 2,
                    new_room.y + (new_room.height - 2) / 2
                );

                fix_edges(new_room);
              // if ( rand () % 6 == 0) locked_door(new_room);
                if ( rand () % 6 == 0) add_hidden_door(new_room);
                
            }
            rooms[level][room_count[level]++] = new_room;
        }
    }
    
    if (level == 4 && !treasure_room_place) {
        int room = rand () % 6;
        if (room == 0) room = 1;
        rooms[level][room].type = 2;
        treasure_room_place = true;
        add_treasure (rooms[level][room]);
    }

    
    int room_with_stairs = rand () %6;
    int room_with_key = rand () %6;
    add_stairs(rooms[level][room_with_stairs]);
    add_master_key(rooms[level][room_with_key]);
    if (rand() % 4 == 0) locked_door(rooms[level][0]);
    int px = rooms[level][0].x + 1, py = rooms[level][0].y + 1;
    player_in_room(px, py, rooms[level], room_count[level]);
    
    int ch;
    
    while (1) {
        //g_state = 0;
        curs_set(0);
        // clear();
        refresh();
        cbreak();
        noecho();
        health_bar(health);
        hunger_bar(hunger);
        show_level();
        desplay_gold();
        display_hits();
        render_map();
        init_colors();
        //player_in_room(px, py, rooms[level], room_count[level]);
        if (hero_color == 1) attron(COLOR_WHITE);
        else attron(COLOR_PAIR(hero_color));
        mvaddch(py, px, '@');
        if (hero_color == 1) attroff(COLOR_WHITE);
        else attroff(COLOR_PAIR(hero_color));
        refresh();
        
        ch = getch();
        if (ch == 'q') break;
        int nx = px, ny = py;
        if (potion_time_track > 0) potion_time_track--;
        if (food_time_out > 0) food_time_out--;
        if (food_time_out == 0) spoil_normal();
        if (potion_time_track <= 0 && (drank_potion || ate_magic_food)) {
            //printw("TIME OUT");
            num_of_blocks = 1;
            if (drank_potion) messages("potion time over", 0);
            drank_potion = false;
            ate_magic_food = false;
        }
        if (ch == KEY_UP || ch == 'J') ny -= num_of_blocks;
        else if (ch == KEY_DOWN || ch == 'K') ny+= num_of_blocks;
        else if (ch == KEY_LEFT || ch == 'H') nx-= num_of_blocks;
        else if (ch == KEY_RIGHT || ch == 'L') nx+= num_of_blocks;
        else if (ch == 'Y') {
            nx-= num_of_blocks;
            ny-= num_of_blocks;
        }
        else if (ch =='U') {
            ny-= num_of_blocks;
            nx+= num_of_blocks;
        } else if (ch =='B') {
            ny+= num_of_blocks;
            nx-= num_of_blocks;
        } else if (ch =='N') {
            nx+= num_of_blocks;
            ny+= num_of_blocks;
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
        } else if (ch == 'g') {
            cheat_g();
        } else if (ch == 'w') {
            messages("weapon in bag", weapon_in_hand->symbol);
            weapon_in_bag();
        } else if (ch == 'a') {
            //if (long_range_weapon) a_command(px, py);
           // else messages("no long range weapon", 0);
        }
        else if (ch == ' ') {
            if (weapon_in_hand) {
                if ((weapon_in_hand->symbol == 'd' || weapon_in_hand->symbol == '~' || weapon_in_hand->symbol == 'a')) {
                    long_range_weapon = true;
                    mvprintw(0, 0, "Choose a direction to throw the weapon!");
                    int direction = getch();
                    char dir[3] = {0};
                    int type = 0;
                    if (direction == KEY_UP) strcpy(dir, "u");
                    else if (direction == KEY_DOWN) strcpy(dir, "d");
                    else if (direction == KEY_LEFT) strcpy(dir, "l");
                    else if (direction == KEY_RIGHT) strcpy(dir, "r");
                    else if (direction == 'U') strcpy(dir, "ur");
                    else if (direction == 'B') strcpy(dir, "dl");
                    else if (direction == 'N') strcpy(dir, "dr");
                    else if (direction == 'Y') strcpy(dir, "ul");
                    
                    if (weapon_in_hand->symbol == 'd') type = 0;
                    else if (weapon_in_hand->symbol == '~') type = 1;
                    else if (weapon_in_hand->symbol == 'a') type = 2;
                    dagger_wand_arrow_attack(px, py, dir, type);
                } else {
                    long_range_weapon = false;
                    struct monster mon = monster_in_room(px, py);
                    if (mon.x != -1 && mon.y != -1) {
                        player_attack(mon.x, mon.y, weapon_in_hand->symbol);
                    } else messages("no monster", 0);
                }
            } else messages("no weapon", 0);
        
        } else if (ch == 'r') {
            resume_save_window(px, py);
        } else if (ch == 'f') {
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
            player_in_room(px, py, rooms[level], room_count[level]);
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
        /*} else if (map[ny][nx] == 'D' || map[ny][nx] == 'F' || map[ny][nx] == 'G' || map[ny][nx] == 'S' || map[ny][nx] == 'U') {
            int attack = getch();
            if (attack == ' ') player_attack(nx, ny);
            
            for (int i = 0; i < monster_count; i ++) {
                if (monsters[i].x == nx && monsters[i].y == ny) {
                    if (monsters[i].state == 1) map[ny][nx] = '.';
                }
            }
            */
        } else if (map[ny][nx] == '&') {
            show_password(nx, ny);
        } else if (map[ny][nx] == '*') {
            if (!g_state) {
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
                    g_state = 0;
                }
            } else {
                px = nx;
                py = ny;
            }
        } else if (map[ny][nx] == '%') {
            px = nx;
            py = ny;
            if (!g_state) pick_up(ny, nx);
            g_state = 0;
        } else if (map[ny][nx] == 'T') {
            px = nx;
            py = ny;
            if (!g_state) picked_tresure();
            g_state = 0;
        }
        else if (map[ny][nx] == '$') {
            px = nx;
            py = ny;
            if (!g_state) pick_up(ny, nx);
            g_state = 0;
        } else if (map[ny][nx] == 'm' || map[ny][nx] == 'd' || map[ny][nx] == '~' || map[ny][nx] == 'a' || map[ny][nx] == '!') {
            px = nx;
            py = ny;
            if (!g_state) pick_up(ny, nx);
            g_state = 0;
        } else if (map[ny][nx] == 'p') {
            px = nx;
            py = ny;
            if (!g_state) pick_up(ny, nx);
            g_state = 0;
            
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
            
            struct ROOM room_p = which_room(px, py);
            if (room_p.x != -1 && room_p.y != -1) {
                while_inside_room(px, py, room_p);
            }
        }
        
        
        health_update(0);
        hunger_update();
        refresh();
    }
    
    curs_set(1);
}


void setting_menu() {
    int ch;
    int choice = 0;
    char menu_name[50] = {"** GAME SETTINGS **"};
    char *options[] = {"Difficulty", "Customize", "Music", "Exit"};
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int win_width = 40;
    int win_height = 12;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    WINDOW *menu_win = newwin(win_height, win_width, start_y, start_x);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        
        wattron(menu_win, COLOR_PAIR(4));
        mvwprintw(menu_win, 1, (win_width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win, COLOR_PAIR(4));
        
        for (int i = 0; i < 4; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(4));
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win, COLOR_PAIR(4));
            } else {
                mvwprintw(menu_win, 3 + i, (win_width - strlen(options[i])) / 2, "%s", options[i]);
            }
        }
        
        wrefresh(menu_win);
        
        ch = getch();
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 3) choice++;
        else if (ch == '\n') {
            if (choice == 0) {
                //clear();
                difficulty();
                refresh();
            } else if (choice == 1) {
                //clear();
                customize_menu();
                refresh();
            } else if (choice == 2) {
                //clear();
                refresh();
            } else if (choice == 3) {
               // clear();
                refresh();
                break;
            }
        }
    }
    delwin(menu_win);
}

void show_profile () {
    char name[50];
    int total_score = 0, total_gold = 0, games_played = 0;
    char buffer[256];
    FILE *file = fopen("hall_of_fame.txt", "r");
    
    int found = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "%s %d %d %d", name, &total_score, &total_gold, &games_played);
        if (strcmp(name, user_name) == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);
    
    int height = 15, width = 50;
    int rows, cols;
    
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW *win = newwin(height, width, start_y, start_x);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(6));
    mvwprintw(win, 1, (50 - strlen("** PROFILE **"))/2, "** PROFILE **");
    wattroff(win, COLOR_PAIR(6));
    
    mvwprintw(win, 10, 5, "Player: %s", name);
    mvwprintw(win, 11, 5, "Total Score: %d", total_score);
    mvwprintw(win, 12, 5, "Total Gold: %d", total_gold);
    mvwprintw(win, 13, 5, "Games Played: %d", games_played);
    
    int height2 = 6, width2 = 40;
    int y2 = 2, x2 = (width - width2) / 2;
    
    WINDOW *profile = derwin(win, height2, width2, y2, x2);
    box(profile, 0, 0);
    
    char welcome[] = "Welcome back, Rogue!";
    int welcome_len = strlen(welcome);
    int x_posw = (width2 - welcome_len) / 2;
    
    mvwprintw(profile, 2, x_posw + 2, "%s", welcome);
    wattron(profile, COLOR_PAIR(hero_color));

    mvwprintw(profile, 1, 1, "  |\\_/|");
    mvwprintw(profile, 2, 1, "  `o.o'");
    mvwprintw(profile, 3, 1, "  =(_)=");
    mvwprintw(profile, 4, 1, "    U");

    wattroff(profile, COLOR_PAIR(hero_color));

    wrefresh(profile);
    wrefresh(win);
    
    wgetch(win);
    delwin(profile);
    delwin(win);
}


void start_game_menu() {
    int ch, choice = 0;
    const char *menu_name = "** GAME MENU **";
    const char *options[] = {"New Game", "Continue Previous Game", "Game Settings", "Profile", "Exit"};
    int num_options = sizeof(options) / sizeof(options[0]);
    int height = 12, width = 40;
    int rows, cols;
    
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW *menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        wattron(menu_win, COLOR_PAIR(3));
        mvwprintw(menu_win, 1, (width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win, COLOR_PAIR(3));

        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(3));
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win, COLOR_PAIR(3));
            } else {
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
            }
        }
        
        wrefresh(menu_win);
        ch = wgetch(menu_win);
        
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < num_options - 1) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // New Game
                reset_game();
                init_map();
                generate_map();
                getch();
                refresh();
            } else if (choice == 1) { // Continue Previous Game
                getch();
                refresh();
            } else if (choice == 2) { // Game Settings
                setting_menu();
                getch();
                refresh();
            } else if (choice == 3) { // Profile
                show_profile();
                getch();
                refresh();
            } else if (choice == 4) { // Exit
                play_menu();
                getch();
                refresh();
            }
        }
    }
    
    delwin(menu_win);
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

void get_info(const char* prompt, char* dest, int max_length, int pass) {
    int ch, i = 0;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int win_width = 40;
    int win_height = 12;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);

    int prompt_len = strlen(prompt);
    mvwprintw(win, 1, (win_width - prompt_len) / 2, "%s", prompt);
    wrefresh(win);

    echo();
    if (pass) noecho();

    int input_x = 2;
    int input_y = 3;

    mvwaddch(win, input_y, input_x, ' ');
    wrefresh(win);

    while ((ch = wgetch(win)) != '\n' && i < max_length - 1) {
        if (ch == 127 || ch == KEY_BACKSPACE) {
            if (i > 0) {
                i--;
                mvwaddch(win, input_y, input_x + i, ' ');
                wrefresh(win);
            }
        } else {
            dest[i++] = (char)ch;
            mvwaddch(win, input_y, input_x + i - 1, pass ? '*' : ch);
            wrefresh(win);
        }
    }

    dest[i] = '\0';

    noecho();
    delwin(win);
}


void show_pop_up (char * pop_up, int gb, char * pass) {
    int color;
    if (gb == 0) color = 9;
    else color = 2;
    int width = 40;
    int height = 12;
    int start_x = (COLS - width) / 2;
    int start_y = (LINES - height) / 2;

    WINDOW *popup = newwin(height, width, start_y, start_x);
    box(popup, 0, 0);
    wattron(popup, (COLOR_PAIR(color) | A_BOLD));
    if (strcmp(pop_up, "Your password is") == 0)  {
        mvwprintw(popup, 5, 3, "%s %s", pop_up, pass);
    }
    else mvwprintw(popup, 5, (width - strlen(pop_up))/2, "%s", pop_up);
    wattroff(popup, (COLOR_PAIR(color) | A_BOLD));
    wrefresh(popup);
    getch();
    delwin(popup);
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
                    show_pop_up("Your password is", 0, stored_pass);
                    //printw("Your password is %s\n", stored_pass);
                    break;
                }
            }
        }
    }
}


void play_menu() {
    char username[50], password[50], email[50];
    int ch;
    int choice = 0;
    char menu_name[50] = {"** PLAY MENU **"};
    char *options[] = {"Play as a Guest", "Login", "Register", "Forgot Password", "Exit"};
    int height = 12, width = 40;
    int rows, cols;
    
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW *menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        wattron(menu_win, (COLOR_PAIR(6) | A_BOLD));
        mvwprintw(menu_win, 1, (width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win,  (COLOR_PAIR(6) | A_BOLD));

        for (int i = 0; i < 5; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(6));
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win,  COLOR_PAIR(6));
            } else {
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
            }
        }

        wrefresh(menu_win);
        ch = wgetch(menu_win);
        
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < 4) choice++;
        else if (ch == '\n') {
            if (choice == 0) {
               // clear();
                start_game_menu();
            } else if (choice == 1) {
                //clear();
                get_info("Enter username: ", username, 50, 0);
                //strcpy(user_name, username);
                get_info("Enter password: ", password, 50, 1);
                
                if (validate_username(username, password)) {
                    show_pop_up("Login successful!", 0, 0);
                    lobby_art();
                    refresh();
                    //printw("\nLogin successful!");
                    strcpy(user_name, username);
                    start_game_menu();
                } else {
                    show_pop_up("Invalid password!", 1, 0);
                    refresh();
                    //printw("\nInvalid password.");
                }
                refresh();
                getch();
            } else if (choice == 2) {
                while (1) {
                    get_info("Enter username: ", username, 50, 0);
                    //printw("\n");
                    if (!unique_username(username)) {
                        show_pop_up("Username already exists!", 1, 0);
                        //printw("Username already exists!\n");
                        refresh();
                        //getch();
                        continue;
                    }
                    break;
                }
                while (1) {
                    get_info("Enter password: ", password, 50, 1);
                    if (!valid_pass(password)) {
                        show_pop_up("Invalid password!", 1, 0);
                        //printw("Invalid password!\n");
                        refresh();
                        continue;
                    }
                    break;
                }
                while (1) {
                    get_info("Enter email: ", email, 50, 0);
                    if (!valid_email(email)) {
                        show_pop_up("Invalid email!", 1, 0);
                        refresh();
                        continue;
                    }
                    break;
                }
                save_info(username, password, email);
                show_pop_up("Registration successful!", 0, 0);
                refresh();
            } else if (choice == 3) {
                get_info("Enter your username: ", username, 50, 0);
                show_pop_up("Do you pinky promise it's you? :D", 0, 0);
                //printw("Do you pinky promise it's you, %s? :D", username);
                forgot_pass(username);
                refresh();
                break;
            } else if (choice == 4) {
                refresh();
                main_menu();
            }
        }
    }

    delwin(menu_win);
}


void load_hall () {
    FILE * file = fopen("hall_of_fame.txt", "r");
    if(!file) return;
    
    score_count = 0;
    while(fscanf(file, "%s %d %d %d %ld", ranks[score_count].name, &ranks[score_count].total_score, &ranks[score_count].total_gold, &ranks[score_count].total_games, &ranks[score_count].first_game) == 5) {
        score_count++;
    }
    fclose(file);
}

void save_scores () {
    FILE *file = fopen("hall_of_fame.txt", "r");
    struct scores temp_ranks[20];
    int temp_count = 0;
    int found[20] = {0};
    
    if (file) {
        while (fscanf(file, "%s %d %d %d %ld", temp_ranks[temp_count].name,
                      &temp_ranks[temp_count].total_score,
                      &temp_ranks[temp_count].total_gold,
                      &temp_ranks[temp_count].total_games,
                      &temp_ranks[temp_count].first_game) == 5) {
            temp_count++;
        }
        fclose(file);
    }
    
    for (int i = 0; i < score_count; i++) {
        int updated = 0;
        for (int j = 0; j < temp_count; j++) {
            if (strcmp(ranks[i].name, temp_ranks[j].name) == 0) {
                temp_ranks[j] = ranks[i];
                found[j] = 1;
                updated = 1;
                break;
            }
        }
        if (!updated && temp_count < 20) {
            temp_ranks[temp_count++] = ranks[i];
        }
    }
    
    file = fopen("hall_of_fame.txt", "w");
    if (!file) return;
    
    for (int i = 0; i < temp_count; i++) {
        fprintf(file, "%s %d %d %d %ld\n", temp_ranks[i].name,
                temp_ranks[i].total_score, temp_ranks[i].total_gold,
                temp_ranks[i].total_games, temp_ranks[i].first_game);
    }
    
    fclose(file);

}

void get_score (char* name, int score, int gold) {
    load_hall();
    time_t current_time = time(NULL);
    int found = 0;
    for ( int i = 0; i < score_count; i ++) {
        if (strcmp(ranks[i].name, name)== 0) {
            ranks[i].total_score += score;
            ranks[i].total_gold += gold;
            ranks[i].total_games ++;
            found = 1;
            break;
        }
    }
    
    if( !found && score_count < MAX_SIZE) {
        strcpy(ranks[score_count].name, name);
        ranks[score_count].total_score = score;
        ranks[score_count].total_gold = gold;
        ranks[score_count].total_games = 1;
        ranks[score_count].first_game = current_time;
        score_count ++;
    }
    
    save_scores();
    
}


int compare_scores(const void *a, const void *b) {
    return ((struct scores*)b)->total_score - ((struct scores*)a)->total_score;
}

void hall_of_fame() {
    clear();
    lobby_art();
    qsort(ranks, score_count, sizeof(struct scores), compare_scores);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int win_height = 20;
    int win_width = 70;
    int start_y = (rows - win_height) / 2;
    int start_x = (cols - win_width) / 2;

    int each_page = win_height - 8;
    int pages = (score_count + each_page - 1) / each_page;
    int cur_page = 0;

    WINDOW *hall = newwin(win_height, win_width, start_y, start_x);

    int ch;
    do {
        werase(hall);
        box(hall, 0, 0);
        mvwprintw(hall, 1, (win_width - 11) / 2 - 3, "** HALL OF FAME **");
        wattron(hall, (COLOR_PAIR(6) | A_BOLD));
        mvwprintw(hall, 3, 2, "RANK       NAME           SCORE    GOLD    GAMES      EXPERIENCE");
        wattroff(hall, (COLOR_PAIR(6) | A_BOLD));
        
        attron(COLOR_PAIR(5));
        const char *message = "== Honor, glory, and a few questionable decisions led these legends here == ";
        int message_len = strlen(message);
        int message_pos = (cols - message_len) / 2;

        mvprintw(1, message_pos, "%s", message);
        attroff(COLOR_PAIR(5));
           
        time_t current_time = time(NULL);

        int start_index = cur_page * each_page;
        int end_index = (start_index + each_page < score_count) ? (start_index + each_page) : score_count;

        for (int i = start_index; i < end_index; i++) {
            int color = (i == 0) ? 5 : (i == 1) ? 2 : (i == 2) ? 4 : 10;
            char title[30] = "      ";
            if (i >= 0 && i <= 2) strcpy(title, "LEGEND");

            if (strcmp(ranks[i].name, user_name) == 0) {
                wattron(hall, (COLOR_PAIR(color) | A_BOLD));
            } else {
                wattron(hall, COLOR_PAIR(color));
            }

            double seconds_passed = difftime(current_time, ranks[i].first_game);
            int days_passed = seconds_passed / (60 * 60 * 24);

            const char *rank_symbol = (i == 0) ? "*" : (i == 1) ? "**" : (i == 2) ? "***" : "   ";
            mvwprintw(hall, 5 + (i - start_index), 2, "%s %-3s %d. %-10s %6d %6d %6d    %6d days",
                      title, rank_symbol, i + 1, ranks[i].name,
                      ranks[i].total_score, ranks[i].total_gold, ranks[i].total_games, days_passed);

            if (strcmp(ranks[i].name, user_name) == 0) {
                wattroff(hall, (COLOR_PAIR(color) | A_BOLD));
            } else {
                wattroff(hall, COLOR_PAIR(color));
            }
        }

        if (pages > 1) {
            mvwprintw(hall, win_height - 2, win_width / 2 - 5, "Page %d of %d", cur_page + 1, pages);
        }

        wrefresh(hall);

        ch = getch();
        if (ch == KEY_RIGHT && cur_page < pages - 1) {
            cur_page++;
        } else if (ch == KEY_LEFT && cur_page > 0) {
            cur_page--;
        }
    } while (ch != 'q' && ch != 'Q');

    delwin(hall);
    clear();
    start_game_menu();
}

void main_menu() {
    int ch, choice = 0;
    const char *menu_name = "** MAIN MENU **";
    const char *options[] = {"Play", "Hall of Fame", "Exit"};
    int num_options = sizeof(options) / sizeof(options[0]);
    int height = 12, width = 40;
    int rows, cols;
    
    getmaxyx(stdscr, rows, cols);
    int start_y = (rows - height) / 2;
    int start_x = (cols - width) / 2;
    
    WINDOW *menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    
    while (1) {
        wclear(menu_win);
        box(menu_win, 0, 0);
        wattron(menu_win, COLOR_PAIR(5));
        mvwprintw(menu_win, 1, (width - strlen(menu_name)) / 2, "%s", menu_name);
        wattroff(menu_win, COLOR_PAIR(5));

        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                wattron(menu_win, COLOR_PAIR(5));
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
                wattroff(menu_win, COLOR_PAIR(5));
                
            } else {
                mvwprintw(menu_win, 3 + i, (width - strlen(options[i])) / 2, "%s", options[i]);
                
            }
        }
        
        wrefresh(menu_win);
        ch = wgetch(menu_win);
        
        if (ch == KEY_UP && choice > 0) choice--;
        else if (ch == KEY_DOWN && choice < num_options - 1) choice++;
        else if (ch == '\n') {
            if (choice == 0) { // Play
                play_menu();
                
            } else if (choice == 1) { // Hall of Fame
                clear();
                load_hall();
                hall_of_fame();
                getch();
                
            } else if (choice == 2) { // Exit
                break;
                
                
            }
        }
    }
    
    delwin(menu_win);
    endwin();
}

void lobby_art () {
    int rows, cols;
      
    getmaxyx(stdscr, rows, cols);
    int start_y = 3;
    int start_x = 53;
    attron(COLOR_PAIR(6));
    mvprintw(start_y++, start_x, "                                                      )         ");
    mvprintw(start_y++, start_x, "                                                        (            ");
    mvprintw(start_y++, start_x, "                                                      '    }      ");
    mvprintw(start_y++, start_x, "                                                    (    '      ");
    mvprintw(start_y++, start_x, "                                                   '      (   ");
    mvprintw(start_y++, start_x, "                                                    )  |    ) ");
    mvprintw(start_y++, start_x, "                                                  '   /|\\    `");
    mvprintw(start_y++, start_x, "                                                 )   / | \\  ` )   ");
    mvprintw(start_y++, start_x, "                                                {    | | |  {   ");
    mvprintw(start_y++, start_x, "                                               }     | | |  .");
    mvprintw(start_y++, start_x, "                                                '    | | |    )");
    mvprintw(start_y++, start_x, "                                               (    /| | |\\    .");
    mvprintw(start_y++, start_x, "                                                .  / | | | \\  (");
    mvprintw(start_y++, start_x, "                                              }    \\ \\ | / /  .        ");
    mvprintw(start_y++, start_x, "                                               (    \\ `-' /    }");
    mvprintw(start_y++, start_x, "                                               '    / ,-. \\    ' ");
    mvprintw(start_y++, start_x, "                                                }  / / | \\ \\  }");
    mvprintw(start_y++, start_x, "                                               '   \\ | | | /   } ");
    mvprintw(start_y++, start_x, "                                                  )  | | |  )");
    mvprintw(start_y++, start_x, "                                                  .  | | |  '");
    mvprintw(start_y++, start_x, "                                                     J | L");
    mvprintw(start_y++, start_x, "                                               /|    J_|_L    |\\");
    mvprintw(start_y++, start_x, "                                               \\ \\___/ o \\___/ /");
    mvprintw(start_y++, start_x, "                                                \\_____ _ _____/");
    mvprintw(start_y++, start_x, "                                                      |-|");
    mvprintw(start_y++, start_x, "                                                      |-|");
    mvprintw(start_y++, start_x, "                                                      |-|");
    mvprintw(start_y++, start_x, "                                                     ,'-'.");
    mvprintw(start_y++, start_x, "                                                     '---'");
    attroff(COLOR_PAIR(6));
    start_x = 5;
    start_y = 3;
    attron(COLOR_PAIR(3));
    mvprintw(start_y++, start_x, "              )         ");
    mvprintw(start_y++, start_x, "                (            ");
    mvprintw(start_y++, start_x, "              '    }      ");
    mvprintw(start_y++, start_x, "            (    '      ");
    mvprintw(start_y++, start_x, "           '      (   ");
    mvprintw(start_y++, start_x, "            )  |    ) ");
    mvprintw(start_y++, start_x, "          '   /|\\    `");
    mvprintw(start_y++, start_x, "         )   / | \\  ` )   ");
    mvprintw(start_y++, start_x, "        {    | | |  {   ");
    mvprintw(start_y++, start_x, "       }     | | |  .");
    mvprintw(start_y++, start_x, "        '    | | |    )");
    mvprintw(start_y++, start_x, "       (    /| | |\\    .");
    mvprintw(start_y++, start_x, "        .  / | | | \\  (");
    mvprintw(start_y++, start_x, "    }    \\ \\ | / /  .        ");
    mvprintw(start_y++, start_x, "       (    \\ `-' /    }");
    mvprintw(start_y++, start_x, "       '    / ,-. \\    ' ");
    mvprintw(start_y++, start_x, "        }  / / | \\ \\  }");
    mvprintw(start_y++, start_x, "       '   \\ | | | /   } ");
    mvprintw(start_y++, start_x, "          )  | | |  )");
    mvprintw(start_y++, start_x, "          .  | | |  '");
    mvprintw(start_y++, start_x, "             J | L");
    mvprintw(start_y++, start_x, "       /|    J_|_L    |\\");
    mvprintw(start_y++, start_x, "       \\ \\___/ o \\___/ /");
    mvprintw(start_y++, start_x, "        \\_____ _ _____/");
    mvprintw(start_y++, start_x, "              |-|");
    mvprintw(start_y++, start_x, "              |-|");
    mvprintw(start_y++, start_x, "              |-|");
    mvprintw(start_y++, start_x, "             ,'-'.");
    mvprintw(start_y++, start_x, "             '---'");
    attroff(COLOR_PAIR(3));
    refresh();
   // getch();
    //clear();
}
void load_welcome_page() {
    curs_set(0);
    clear();
    int height = 15, width = 55;
    int start_y =0;
    int start_x = 0;

    attron(COLOR_PAIR(2));
    int x = 25;
    int y = 10;
    mvprintw(start_y + 1 + y, start_x + 8 +x, "                           /   \\");
    mvprintw(start_y + 2 + y, start_x + 8 +x, "_                  )      ((   ))     (");
    mvprintw(start_y + 3 + y, start_x + 8 +x, "(@)               /|\\      ))_((     /|\\                   _");
    mvprintw(start_y + 4 + y, start_x + 8 +x, "|-|`\\            / | \\    (/\\|/\\)   / | \\                (@)");
    mvprintw(start_y + 5 + y, start_x + 8 +x, "| |-------------/--|-voV---\\`|'/--Vov-|--\\--------------|-|");
    mvprintw(start_y + 6 + y, start_x + 8 +x, "|-|                '^`     (o o)     '^`                | |");
    mvprintw(start_y + 7 + y, start_x + 8 +x, "| |                        `\\Y/'                        |-|");
    mvprintw(start_y + 8 + y, start_x + 8 +x, "|-|                                                     | |");
    mvprintw(start_y + 9 + y, start_x + 8 +x, "| |                                                     |-|");
    mvprintw(start_y + 10 + y, start_x + 8 +x,"|_|_____________________________________________________| |");
    mvprintw(start_y + 11 + y, start_x + 8 +x, "(@)       l   /\\ /         ( (       \\ /\\   l         `\\|-|");
    mvprintw(start_y + 12 + y, start_x + 8 +x, "         l /   V           \\ \\        V  \\ l            (@)");
    mvprintw(start_y + 13 + y, start_x + 8 +x, "         l/                _) )_           \\I");
    mvprintw(start_y + 14 + y, start_x + 8 +x, "                          `\\ /'");
    mvprintw(start_y + 15 + y, start_x + 8 +x, "                             `");
    attroff(COLOR_PAIR(2));
    refresh();
    
    attron(COLOR_PAIR(5));
    char welcome[] = "Welcome to the Dungeon of Doom!";
    int welcome_len = strlen(welcome);
    int start = (width - welcome_len) / 2;
    mvprintw(start_y + 8 +y, 23 +x, "%s", welcome);

    attroff(COLOR_PAIR(5));
    refresh();
    getch();
    clear();
    refresh();

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
    clear();
    lobby_art();
    main_menu();
    getch();
    endwin();
    return 0;
}
