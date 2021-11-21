typedef struct {
  byte x; 		// Hero x coordinate
  byte y; 		// Hero y coordinate
  byte dir;		// Hero direction
  int collided:1;	// Hero collided value
  byte lives;		// Hero score 1000-9999

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

void init_game(void);
void game_over(void);
void start_game(void);
void clrscrn(void);
void title_screen(void);

void create_top_left_area(void); //heart 0
void create_top_area(void); //heart 1
void create_top_right_area(void); //heart 2
void create_left_area(void); //heart 3
void create_start_area(void); //heart 4
void create_right_area(void); //heart 5
void create_bottom_left_area(void); //heart 6
void create_bottom_area(void); //heart 7
void create_bottom_right_area(void); //heart 8
void create_boss_area(void);


