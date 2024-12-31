#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#define MAX_ROOM_NUM 8
#define MAX_ROOM_LENGTH 80
#define MIN_ROOM_LENGTH 10
#define MIN_DISTANCE 10
#define MARGIN 5
#define MAX_ROOM_RATIO 1.6
#define MIN_ROOM_RATIO 0.6

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

#endif