typedef struct {
  byte x; 		// Hero x coordinate
  byte y; 		// Hero y coordinate
  byte dir;		// Hero direction
  int collided:1;	// Hero collided value
  word bit1;		// Hero score    0-9
  word bit2;		// Hero score   10-99
  word bit3;		// Hero score  100-999
  word bit4;		// Hero score 1000-9999

} Hero;

typedef struct {
  byte x;		// Heart x coordinate
  byte y;		// Heart y coordinate

} Heart;

typedef struct Floor {
  byte ypos;		// # of tiles from ground
  int height:4;		// # of tiles to next floor
  int gap:4;		// X position of gap
  int ladder1:4;	// X position of first ladder
  int ladder2:4;	// X position of second ladder
  int objtype:4;	// item type (FloorItem)
  int objpos:4;		// X position of object
} Floor;


//Prototypes;
void play(void);
void init_game(void);
void game_over(void);
void start_game(void);
void main_scroll(void);
void clrscrn(void);
void add_point(Hero*);
void title_screen(void);
void create_start_area(void);
void create_left_area(void);
void create_right_area(void);
void create_top_area(void);
void create_bottom_area(void);
void create_top_left_area(void);
void create_top_right_area(void);
void create_bottom_left_area(void);
void create_bottom_right_area(void);

// various items the player can pick up
typedef enum FloorItem { ITEM_NONE, ITEM_MINE, ITEM_HEART, ITEM_POWER };

