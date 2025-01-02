#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#define MAX_ROOM_NUM 8
#define MAX_ROOM_LENGTH 60
#define MIN_ROOM_LENGTH 10
#define MIN_DISTANCE 10
#define MARGIN 5
#define MAX_ROOM_RATIO 1.6
#define MIN_ROOM_RATIO 0.6

const char *welcome_options[4] = {"Continue Game", "Create New Account", "Play as Guest", "Quit"};
const char *new_account_options[3] = {"Username: ", "Email: ", "Password: "};

typedef struct
{
    int y;
    int x;
} Point;

typedef struct
{
    Point upper_left_corner;
    Point door;

    Point window[4];
    int window_num;

    Point pillar[2];
    int pillar_num;

    int length;
    int width;
} Room;

typedef struct
{
    Room room[MAX_ROOM_NUM];
    int room_num;

    Point staircase;
} Level;

typedef struct
{
    Level levels[4];
} Map;

typedef struct
{
    char username[50];
    char password[50];
    char email[50];
    Map map;
} User;

#endif