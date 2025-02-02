#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// #include "STRUCTS.h"

#define UP_MARGIN 6
#define DOWN_MARGIN 6
#define MAX_ROOM_NUM 6
#define MIN_ROOM_WIDTH 6
#define MIN_ROOM_LENGTH 8
#define MIN_DISTANCE 10
#define MARGIN 2
#define MAX_ROOM_RATIO 1.6
#define MIN_ROOM_RATIO 0.6

#define VISIBILITY_RADIUS 2
#define MAX_LEVEL 2
#define MAX_OBJECT 50

#define MIN_NORMAL_FOOD 5
#define MAX_NORMAL_FOOD 9

#define MIN_GOURMET_FOOD 1
#define MAX_GOURMET_FOOD 3

#define MIN_MAGIC_FOOD 0
#define MAX_MAGIC_FOOD 2

#define MIN_POISON_FOOD 0
#define MAX_POISON_FOOD 2

#define MIN_COIN 6
#define MAX_COIN 10

#define MAX_COIN_POINT 5
#define MIN_COIN_POINT 1

#define MAX_FOOD 200
#define MAX_WEAPON 200
#define MAX_SPELL 200

#define MAX_BLACK_COIN 1
#define MIN_BLACK_COIN 0

#define MAX_HEALTH_SPELL 2
#define MIN_HEALTH_SPELL 1

#define MAX_SPEED_SPELL 1
#define MIN_SPEED_SPELL 0

#define MAX_DAMAGE_SPELL 1
#define MIN_DAMAGE_SPELL 0

const char *welcome_options[4] = {"Log In", "Create New Account", "Play as Guest", "Quit"};
const char *new_account_options[3] = {"Username: ", "Email: ", "Password: "};

typedef struct
{
    int y;
    int x;
} Point;

typedef enum
{
    Normal = 0,
    Enhant,
    Nightmare,
    Treasure
} RoomType;

typedef struct
{
    int room_exist;
    int discovered;

    Point corner;
    Point middle_door;
    Point *door;
    int *door_positions;
    /*    0
        1   2
          3
    */
    int door_num;
    int room_position;

    Point window[4];
    int window_num;

    Point pillar[2];
    int pillar_num;

    int length;
    int width;

    RoomType type;
} Room;

typedef enum
{
    Hero = 2,
    Trap = 3,
    Staircase = 4,
    SecretDoor = 5,
    PasswordDoor = 6,
    MasterKey = 7,
    NormalFood = 8,
    GourmetFood = 9,
    MagicFood = 10,
    PoisonFood = 11,
    Coin = 12,
    Mace = 13,
    Dagger = 14,
    MagicWand = 15,
    NormalArrow = 16,
    Sword = 17,
    HealthSpell = 18,
    SpeedSpell = 19,
    DamageSpell = 20,
    BlackCoin = 21,
    EmptyWeapon = 22,
} ObjectType;

typedef struct
{
    ObjectType type;
    Point location;
    int visible;
    int location_room;
    int food_step_count;
} Object;

typedef enum
{
    deamon = 0,
    fire_monster,
    giant,
    snake,
    undead
} EnemyType;

typedef struct
{
    Point location;
    int health;
    int damage;
    int speed;
    int follow;
    int visible;
    int location_room;
    int stunned;
    EnemyType type;
} Enemy;

typedef struct
{
    int level_num;

    Room room[9];
    int room_num;
    int discovered_room_num;

    Point between_doors_1[12];
    Point between_doors_2[12];

    int corridor_exist[12];

    Object *objects;
    int object_num;

    Enemy *enemies;
    int enemy_num;
} Level;

typedef struct
{
    Level levels[4];
} Map;

const char *face[11] = {"🦊", "🐼", "🐯", "🐺", "👰", "🧙", "🧛", "🧝", "🎅", "👮"};
const char *face_name[11] = {"Fox", "Panda", "Tiger", "Wolf", "Bride", "Wizard", "Vampire", "Elf", "Santa", "Police"};

typedef enum
{
    // 🦊🐼🐯🐺👰🥷🧙🧛🧝🎅👮
    Fox,
    Panda,
    Tiger,
    Wolf,
    Bride,
    Wizard,
    Vampire,
    Elf,
    Santa,
    Police,
    Astronaut,
} face_options;

typedef struct
{
    Point location;
    int location_room;
    int level_num;
    face_options face_option;
    int health;
    int damage;
    int hunger;
    int coins;
    int speed;
    int satiety;
    int health_progress;
    int satiety_progress;

    Object *food_inventory;
    Object *weapon_inventory;
    Object *spell_inventory;

    int food_num;
    int weapon_num;
    int spell_num;

    Object current_weapon;
    int currect_weapon_num;

    int dir_y;
    int dir_x;

    int last_room;
} Character;

typedef struct
{
    char username[26];
    char password[29];
    char email[26];
    int difficulty;
    int color_option;
    int play_music;
    int playlist;
    int game_num;
    int win_num;
    int total_score;
    int total_gold;
    Map map;
    Character hero;
} User;

typedef struct
{
    int users_num;
    User **users;
} Game;

const char *undertale[] = {
    "music/track1.mp3",
    "music/track2.mp3",
    "music/track3.mp3",
    NULL};

const char *squid_game[] = {
    "music/track4.mp3",
    "music/track5.mp3",
    NULL};

const char *taylor_swift[] = {
    "test/music/Coldplay-FixYou.mp3",
    NULL};

const char **playlists[] = {undertale, squid_game, taylor_swift};
const char playlist_name[][13] = {"Undertale", "Squid Game", "Taylor Swift"};

void draw_border();

void title_screen();

void random_map();
void random_level(int level_num);
void print_level(int level_num);
void print_room(int level_num, int room_num);

void draw_welcome_screen_border();
int welcome_screen();

void draw_new_account_screen();
void new_account_screen();

// void save_user_data();
bool username_unique();
bool email_unique();
bool email_valid();
bool password_long_enough();
bool password_valid();

void load_users();
int username_found(const char *username);
int email_found(const char *email);
bool password_correct(int user_index, const char *password);
void data_from_username(const char *username, int *difficulty, int *color_option);
void data_from_email(const char *email, int *difficulty, int *color_option);
void continue_game_screen();

void print_user_taken();
void print_email_taken();
void print_wrong_email_format();
void print_password_short();
void print_wrong_password_format();

void draw_sign_in_screen(int option, int show_password);
void continue_game_screen();

// int username_found(char username[26]);
// int email_found(char email[29]);
// bool password_correct(int user_index, char password[26]);

void print_user_not_found();
void print_email_not_found();
void print_wrong_password();

void draw_user_options_menu();
int user_options_menu();

void draw_user_settings_menu();
void user_settings_menu();

void create_corridors(int level_num);
void break_corridors(Point door_1, Point door_2, int corridor_num, int level_num);
void print_corridors(int level_num);

Point middle_door(Room room);
void generate_middle_doors(Room *room);

void print_test(int level_num);

void initialize_hero(int level_num);
void print_hero();
void handle_move_command(int level_num, int input);
int valid_point(int level_num, Point destination);

void make_cells_visible(int level_num);
void initialize_visibility_grid();
void free_visibility_grid();

Point random_location(int level_num);

void create_objects(int level_num);
void print_objects(int level_num);
void initialize_map();

void draw_border();

int handle_location(int level_num);
void remove_object(int level_num, int object_index);

int find_room(int level_num, Point location);

void print_info(int level_num);
void print_message(char *message, char *guide);
void spawn_object(int level_num, ObjectType type);

void show_food_inventory();
void show_weapon_inventory();
void show_spell_inventory();

void print_entire_map(int level_num);
void print_entire_room(int level_num, int room_num);
void print_entire_corridors(int level_num);

Point random_location_in_room(int level_num, int room_num);

void create_objects(int level_num);
void initialize_map();
void room_to_file(FILE *user_map, int level_num, int room_num);
void point_to_file(FILE *user_map, Point point);
void level_to_file(char *file_name, int level_num);
void load_level_from_file(char *file_name, int level_num);

int find_level_num_from_file(char *file_name, int level_num);

int *three_extra_rooms();
int *extra_corridors();

int run_game_level(int level_num);

// Function to clear and draw the scoreboard menu
void draw_score_board_menu();

// Function to draw the users' scores on the scoreboard for a specific page
void draw_users_score_board(int page, int page_select);

// Function to handle the scoreboard menu logic
void score_board_menu();

// Comparator function for sorting users by total_score (descending)
int compare_users(const void *a, const void *b);

// Function to save a single user's data to a CSV file
int save_user_to_csv(const char *filename, const User *user);

// Function to load users from a CSV file, keeping only the latest entries and sorting by score
int load_users_from_csv(const char *filename, User ***users, int *num_users);

char *generate_password();
void golden_freddy_appear();

void create_enemies(int level_num);
void print_enemies(int level_num);

void make_enemies_visible(int level_num);
void handle_enemies_movement(int level_num);
int in_range(Point p1, Point p2, int distance);
int valid_point_enemy(int level_num, Point destination);

Point random_location_enemy(int level_num);
void draw_message_border();

void play_playlist(const char **playlist);
void draw_inventory_border();
void draw_info_border();

void remove_enemy(int level_num, int enemy_index);

void remove_food_from_inventory(int food_index);
void remove_weapon_from_inventory(int weapon_index);
void remove_spell_from_inventory(int spell_index);
void show_current_weapon();

// When the player picks up a new weapon
void add_current_weapon_to_inventory();

// When the player opens the weapon inventory
void show_weapon_inventory();

// When the player selects a weapon to equip
void equip_weapon(int selected_index);

const char *weapon_name(ObjectType type);

int attack_with_mace(int level_num);
int attack_with_dagger(int level_num, int direction_y, int direction_x);
int attack_with_magic_wand(int level_num, int direction_y, int direction_x);
int attack_with_normal_arrow(int level_num, int direction_y, int direction_x);
int attack_with_gun(int level_num, int direction_y, int direction_x);

void enemies_attack_hero(int level_num);

void update_food_status();

void treasure_level();
void create_treasure_room_objects();

void determine_room_types(int level_num);

void show_death_screen();
void show_win_screen();

int find_user_index_from_username(const char *username);
int find_user_index_from_email(const char *email);

#endif