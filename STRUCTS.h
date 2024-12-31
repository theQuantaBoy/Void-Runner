#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#define MAX_ROOM_NUM 8
#define MAX_ROOM_LENGTH 60
#define MIN_DISTANCE 10

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