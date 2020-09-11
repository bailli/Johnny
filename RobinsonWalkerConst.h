#ifndef ROBINSONWALKERCONST_H
#define ROBINSONWALKERCONST_H

namespace SCRANTIC {

#define WALKER_MAX_POS 6

#define WALKER_POS_A 0
#define WALKER_POS_B 1
#define WALKER_POS_C 2
#define WALKER_POS_D 3
#define WALKER_POS_E 4
#define WALKER_POS_F 5

#define WALKER_MAX_FACE 8

#define WALKER_FACE_S  0
#define WALKER_FACE_SW 1
#define WALKER_FACE_W  2
#define WALKER_FACE_NW 3
#define WALKER_FACE_N  4
#define WALKER_FACE_NE 5
#define WALKER_FACE_E  6
#define WALKER_FACE_SE 7

    static size_t walkerTurnTags[WALKER_MAX_POS] = { 5,  9, 16, 21, 27, 33 };
    static size_t walkerWaitTags[WALKER_MAX_POS] = { 6, 10, 17, 22, 28, 34 };

    static int walkerXtoYTags[WALKER_MAX_POS][WALKER_MAX_POS] = {
        { -1,  4,  3, -1,  1,  2 },
        {  7, -1,  8, -1, -1, -1 },
        { 11, 12, -1, 13, 14, 15 },
        { -1, -1, 18, -1, 19, 20 },
        { 23, -1, 24, 25, -1, 26 },
        { 29, -1, 30, 31, 32, -1 }
    };

    static int walkerXtoYFace[WALKER_MAX_POS][WALKER_MAX_POS][2] = {
        { {-1, -1},  { 6,  7},  { 6,  5}, {-1, -1},  { 5,  5},  { 5,  5} },
        { { 3,  3},  {-1, -1},  { 5,  5}, {-1, -1},  {-1, -1},  {-1, -1} },
        { { 2,  2},  { 1,  1},  {-1, -1}, { 3,  4},  { 2,  3},  { 2,  3} },
        { {-1, -1},  {-1, -1},  { 7,  7}, {-1, -1},  { 2,  2},  { 1,  1} },
        { { 1,  1},  {-1, -1},  { 7,  6}, { 6,  6},  {-1, -1},  { 7,  7} },
        { { 2,  2},  {-1, -1},  { 6,  6}, { 5,  5},  { 3,  3},  {-1, -1} }
    };

}

#endif // ROBINSONWALKERCONST_H
