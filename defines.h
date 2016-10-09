#ifndef DEFINES_H
#define DEFINES_H

#define ISLAND_RIGHT 0x1
#define ISLAND_LEFT  0x2
#define NO_ISLAND    0x0

/*relative:
   0     0 trunk
 -77   -26 top
-154   131 island
-172   158 wave left
 -78   171 wave middle
  78   155 wave right
-193   155 large island
-292   180 stone
-313   192 wave stone
-209   266 large wave left
 -75   208 large wave middle (might be off)
 116   175 large wave right
  73   118 raft
 -46   131 top shadow*/

#define TOP_X -77
#define TOP_Y -26
#define ISLAND_X -154
#define ISLAND_Y 131
#define WAVE_LEFT_X -172
#define WAVE_LEFT_Y 158
#define WAVE_MID_X -78
#define WAVE_MID_Y 171
#define WAVE_RIGHT_X 78
#define WAVE_RIGHT_Y 155
#define STONE_X -292
#define STONE_Y 180
#define WAVE_STONE_X -313
#define WAVE_STONE_Y 192
#define L_ISLAND_X -193
#define L_ISLAND_Y 155
#define WAVE_L_LEFT_X -209
#define WAVE_L_LEFT_Y 175
#define WAVE_L_MID_X -75
#define WAVE_L_MID_Y 208
#define WAVE_L_RIGHT_X 116
#define WAVE_L_RIGHT_Y 175
#define RAFT_X 73
#define RAFT_Y 118
#define TOP_SHADOW_X -46
#define TOP_SHADOW_Y 131

#define ISLAND_RIGHT_X 443
#define ISLAND_RIGHT_Y 168

#define ISLAND_LEFT_X 171
#define ISLAND_LEFT_Y 178

#define ISLAND_TEMP_X 442
#define ISLAND_TEMP_Y 148

#define ISLAND2_X 170
#define ISLAND2_Y 148

#define SPRITE_ISLAND 0
#define SPRITE_L_ISLAND 1
#define SPRITE_STONE 2
#define SPRITE_WAVE_LEFT 3
#define SPRITE_WAVE_MID 6
#define SPRITE_WAVE_RIGHT 9
#define SPRITE_TOP 12
#define SPRITE_TRUNK 13
#define SPRITE_TOP_SHADOW 14
#define SPRITE_WAVE_L_LEFT 30
#define SPRITE_WAVE_L_MID 33
#define SPRITE_WAVE_L_RIGHT 36
#define SPRITE_WAVE_STONE 39

#define RENDERITEM_SPRITE 0
#define RENDERITEM_LINE 1
#define RENDERITEM_RECT 2
#define RENDERITEM_ELLIPSE 3
#define RENDERITEM_NONE -1

#define RENDERFLAG_MIRROR 0x1

#define MAX_IMAGES 10
#define MAX_AUDIO 24

#endif // DEFINES_H
