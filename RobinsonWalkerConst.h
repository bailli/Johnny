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

    struct MovieData {
        i8 startPos;
        i8 startFacing;
        i8 endPos;
        i8 endFacing;
    };

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

    std::map<std::string, std::map<u16, MovieData> > movieData = {
        { "FISHING.ADS",
            {
                { 1, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E } },
                { 2, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E } },
                { 3, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_F, WALKER_FACE_S } },
                { 4, { WALKER_POS_E, WALKER_FACE_E,            -1,            -1 } },
                { 5, { WALKER_POS_E, WALKER_FACE_E,            -1,            -1 } },
                { 6, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_A, WALKER_FACE_W } },
                { 7, { WALKER_POS_E, WALKER_FACE_E,  WALKER_POS_E, WALKER_FACE_W } },
                { 8, { WALKER_POS_E, WALKER_FACE_E,  WALKER_POS_E, WALKER_FACE_W } }
            }

        },

        { "STAND.ADS",
            {
                { 1, { WALKER_POS_A, WALKER_FACE_SW, WALKER_POS_A, WALKER_FACE_SW } },
                { 2, { WALKER_POS_A, WALKER_FACE_W,  WALKER_POS_A, WALKER_FACE_W  } },
                { 3, { WALKER_POS_A, WALKER_FACE_NW, WALKER_POS_A, WALKER_FACE_NW } },
                { 4, { WALKER_POS_B, WALKER_FACE_SW, WALKER_POS_B, WALKER_FACE_SW } },
                { 5, { WALKER_POS_B, WALKER_FACE_S,  WALKER_POS_B, WALKER_FACE_S  } },
                { 6, { WALKER_POS_B, WALKER_FACE_SE, WALKER_POS_B, WALKER_FACE_SE } },
                { 7, { WALKER_POS_C, WALKER_FACE_NE, WALKER_POS_C, WALKER_FACE_NE } },
                { 8, { WALKER_POS_C, WALKER_FACE_E,  WALKER_POS_C, WALKER_FACE_E  } },
                { 9, { WALKER_POS_D, WALKER_FACE_NW, WALKER_POS_D, WALKER_FACE_NW } },
                {10, { WALKER_POS_D, WALKER_FACE_NE, WALKER_POS_D, WALKER_FACE_NE } },
                {11, { WALKER_POS_E, WALKER_FACE_NW, WALKER_POS_E, WALKER_FACE_NW } },
                {12, { WALKER_POS_F, WALKER_FACE_NE, WALKER_POS_F, WALKER_FACE_NE } },
                {14, {           -1,            -1,            -1,             -1 } },
                {15, { WALKER_POS_A, WALKER_FACE_S,  WALKER_POS_A, WALKER_FACE_S  } },
                {16, { WALKER_POS_C, WALKER_FACE_S,  WALKER_POS_C, WALKER_FACE_S  } }
            }

        },

        { "MISCGAG.ADS",
            {
                { 1, { WALKER_POS_D, WALKER_FACE_W,            -1,             -1 } },
                { 2, { WALKER_POS_D, WALKER_FACE_W,            -1,             -1 } }
            }

        },

        { "ACTIVITY.ADS",
            {
                { 1, { WALKER_POS_E, WALKER_FACE_SW,           -1,             -1 } },
                { 4, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_E, WALKER_FACE_SE } },
                { 5, { WALKER_POS_E, WALKER_FACE_S,            -1,             -1 } },
                { 6, { WALKER_POS_D, WALKER_FACE_SE,           -1,             -1 } },
                { 7, { WALKER_POS_D, WALKER_FACE_NE, WALKER_POS_F, WALKER_FACE_SW } },
                { 8, {           -1,            -1,  WALKER_POS_D, WALKER_FACE_S  } },
                { 9, { WALKER_POS_E, WALKER_FACE_NW,           -1,             -1 } },
                {10, { WALKER_POS_D, WALKER_FACE_NE,           -1,             -1 } },
                {11, {           -1,            -1,            -1,             -1 } },
                {12, { WALKER_POS_D, WALKER_FACE_NE,           -1,             -1 } }
            }

        },

        { "VISITOR.ADS",
            {
                { 1, { WALKER_POS_A, WALKER_FACE_S,  WALKER_POS_A, WALKER_FACE_S  } },
                { 3, { WALKER_POS_B, WALKER_FACE_NW,           -1,             -1 } },
                { 4, { WALKER_POS_D, WALKER_FACE_SW, WALKER_POS_D, WALKER_FACE_W  } },
                { 5, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_D, WALKER_FACE_NE } },
                { 6, { WALKER_POS_D, WALKER_FACE_SW, WALKER_POS_D, WALKER_FACE_SW } },
                { 7, { WALKER_POS_D, WALKER_FACE_SW, WALKER_POS_D, WALKER_FACE_SW } }
            }

        },

        { "BUILDING.ADS",
            {
                { 1, { WALKER_POS_F, WALKER_FACE_W,  WALKER_POS_A, WALKER_FACE_W  } },
                { 2, { WALKER_POS_F, WALKER_FACE_W,            -1,             -1 } },
                { 3, { WALKER_POS_A, WALKER_FACE_E,  WALKER_POS_C, WALKER_FACE_SE } },
                { 4, { WALKER_POS_A, WALKER_FACE_E,            -1,             -1 } },
                { 5, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E  } },
                { 6, { WALKER_POS_A, WALKER_FACE_E,            -1,             -1 } },
                { 7, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E  } },
                { 8, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E  } },
                { 9, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_D, WALKER_FACE_E  } }
            }

        },

        { "MARY.ADS",
            {
                { 1, { WALKER_POS_E, WALKER_FACE_SW,           -1,             -1 } },
                { 2, { WALKER_POS_E, WALKER_FACE_E,            -1,             -1 } },
                { 3, { WALKER_POS_F, WALKER_FACE_SW,           -1,             -1 } },
                { 4, { WALKER_POS_E, WALKER_FACE_E,            -1,             -1 } },
                { 5, { WALKER_POS_E, WALKER_FACE_NW,           -1,             -1 } }
            }

        },

        { "JOHNNY.ADS",
            {
                { 1, {           -1,             -1,           -1,             -1 } },
                { 2, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_F, WALKER_FACE_NE } },
                { 3, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_F, WALKER_FACE_NE } },
                { 4, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_F, WALKER_FACE_NE } },
                { 5, { WALKER_POS_E, WALKER_FACE_SW, WALKER_POS_F, WALKER_FACE_NE } },
                { 6, {           -1,             -1,           -1,             -1 } }
            }

        },

        { "SUZY.ADS",
            {
                { 1, {           -1,             -1,           -1,             -1 } },
                { 2, {           -1,             -1,           -1,             -1 } }

            }
        },

        { "WALKSTUF.ADS",
            {
                { 1, { WALKER_POS_A, WALKER_FACE_NE,           -1,             -1 } },
                { 2, { WALKER_POS_E, WALKER_FACE_E,  WALKER_POS_D, WALKER_FACE_E  } },
                { 3, { WALKER_POS_D, WALKER_FACE_W,  WALKER_POS_E, WALKER_FACE_W  } }
            }
        }

    };

}

#endif // ROBINSONWALKERCONST_H
