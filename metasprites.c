#include <stdlib.h>
#include "neslib.h"
#include <string.h>
#include <joystick.h>
#include <nes.h>
#include "metasprites.h"

#include "vrambuf.h"
//#link "vrambuf.c"
// link the pattern table into CHR ROM
//#link "chr_generic.s"

// link the pattern table into CHR ROM
//#link "chr_generic.s"

///// METASPRITES


#define TILE 0xd8	// playable character attributes
#define TILE1 0xCC	// heart attributes
#define ATTR1 01	// heart attributes
#define ATTR 02		// character attributes
#define MAX_FLOORS 20

#define COLS 32
#define ROWS 27
// define a 2x2 metasprite
const unsigned char metasprite[]={
        0,      0,      TILE+0,   ATTR1, 
        0,      8,      TILE+1,   ATTR1, 
        8,      0,      TILE+2,   ATTR1, 
        8,      8,      TILE+3,   ATTR1, 
        128};


// Heart Metasprite info
const unsigned char metasprite1[]={
        0,      0,      TILE1,     ATTR, 
        0,      8,      TILE1+1,   ATTR, 
        8,      0,      TILE1+2,   ATTR, 
        8,      8,      TILE1+3,   ATTR, 
        128};


//character set for box
const char BOX_CHARS[8] = {0x8D,0x8E,0x87,0x8B,0x8C,0x83,0x85,0x8A };

Hero heros;
Heart hearts;
// array of floors
Floor floors[MAX_FLOORS];
unsigned char pad1;	// joystick
unsigned char pad1_new; // joystick


//Direction array affects movement and gravity
typedef enum { D_RIGHT, D_DOWN, D_LEFT, D_UP, D_STAND } dir_t;
const char DIR_X[5] = { 3, 0, -3, 0, 0};
const char DIR_Y[5] = { 0, 3, 0, -3, 0};
/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x11,0x30,0x27,0x0,	// background palette 0
  0x1c,0x20,0x2c,0x0,	// background palette 1
  0x00,0x10,0x20,0x0,	// background palette 2
  0x06,0x16,0x26,0x0,	// background palette 3

  0x16,0x35,0x24,0x0,	// sprite palette 0
  0x00,0x37,0x25,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x2a	// sprite palette 3
};

void move_player(Hero* h)
{
  h->x += DIR_X[h->dir];
  h->y += DIR_Y[h->dir];
}
// read user input and set hero direction to that value
void movement(Hero* h)
{
  byte dir;
  pad1_new = pad_trigger(0); // read the first controller
  pad1 = pad_state(0);
  
  
  if (pad1 & JOY_LEFT_MASK) dir = D_LEFT;else
  if (pad1 & JOY_RIGHT_MASK) dir = D_RIGHT;else
  if (pad1 & JOY_UP_MASK) dir = D_UP;else
  if (pad1 & JOY_DOWN_MASK) dir = D_DOWN;
  
  else dir = D_STAND;
  
  h->dir = dir;
}

void resetmovement(Hero* h){

  h->dir = D_STAND;
  
}
void put_str(unsigned int adr, const char *str) 
{
  vram_adr(adr);        // set PPU read/write address
  vram_write(str, strlen(str)); // write bytes to PPU
}

byte getchar(byte x, byte y) 
{
  // compute VRAM read address
  word addr = NTADR_A(x,y);
  // result goes into rd
  byte rd;
  // wait for VBLANK to start
  ppu_wait_nmi();
  // set vram address and read byte into rd
  vram_adr(addr);
  vram_read(&rd, 1);
  // scroll registers are corrupt
  // fix by setting vram address
  vram_adr(0x0);
  return rd;
}

//function displayes text
void cputcxy(byte x, byte y, char ch) 
{
  vrambuf_put(NTADR_A(x,y), &ch, 1);
}

//function displayes text
void cputsxy(byte x, byte y, const char* str) 
{
  vrambuf_put(NTADR_A(x,y), str, strlen(str));
}


void draw_box(byte x, byte y, byte x2, byte y2, const char* chars) {
  byte x1 = x;
  cputcxy(x, y, chars[2]);
  cputcxy(x2, y, chars[3]);
  cputcxy(x, y2, chars[0]);
  cputcxy(x2, y2, chars[1]);
  while (++x < x2) {
    cputcxy(x, y, chars[5]);
    cputcxy(x, y2, chars[4]);
  }
  while (++y < y2) {
    cputcxy(x1, y, chars[6]);
    cputcxy(x2, y, chars[7]);
  }
}


void spawn_item(Heart* h)
{
  
  h->x = 100;
  h->y = 100;
  oam_meta_spr( h->x , h->y, 20, metasprite1);
  
}
// setup PPU and tables
void setup_graphics() {
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}

//reset game
void clrscrn()
{
  vrambuf_clear();
  ppu_off();
  vram_adr(0x2000);
  vram_fill(0, 32*28);
  vram_adr(0x0);
}


void init_game()
{
  vrambuf_clear();
  
  heros.bit1 = 0;
  heros.bit2 = 0;
  heros.bit3 = 0;
  heros.bit4 = 0;

  oam_clear();
  heros.x = 150;
  heros.y = 120;
  vrambuf_clear();

  oam_meta_spr(heros.x, heros.y, 4, metasprite);
  vrambuf_clear();
  set_vram_update(updbuf);
  
  cputsxy(11,1,"Score:");
  cputcxy(17,1,'0');
  cputcxy(18,1,'0');
  cputcxy(19,1,'0');
  cputcxy(20,1,'1');
  
  ppu_on_all();
  
  vrambuf_clear();
  
}

void create_start_area(){
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  
  //draw bottom area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);



  vrambuf_flush();
    while (1) {
    
     
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // start right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_right_area();
    }
    // start left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_left_area();
    }    
    // start top area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_top_area();
    }    
    // start bottom area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_bottom_area();
    }     
    x++;
  } 
}


void create_left_area(){
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  
  //draw bottom area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);

  
  vrambuf_flush();
  
    while (1) {
       
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to start area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_start_area();
    }  
    // move to top left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_top_left_area();
    }    
    // move to bottom left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_bottom_left_area();
    }
      
    
    x++;
  }}

void create_right_area(){
  int x;
  
  oam_meta_spr(150, 150, 20, metasprite1);    
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  
  //draw bottom area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);
  
  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }

    // move to start area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_start_area();
    }    
    // move to top right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_top_right_area();
    }    
    // move to bottom right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_bottom_right_area();
    }
    
    x++;
  }
}


void create_top_area(){
    int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw start area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);

  
  vrambuf_flush();
    while (1) {
     
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to top right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_top_right_area();
    }
    // move to top left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_top_left_area();
    }      
    // move to start area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_start_area();
    }
      
    x++;
  }
}

void create_bottom_area(){
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);

  
  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to bottom right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_bottom_right_area();
    }
    // move to bottom left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_bottom_left_area();
    }    
    // move to start area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_start_area();
    }    

    x++;
  }
}

void create_top_left_area(){
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  //draw bottom area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);

  
  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to top area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_top_area();
    }
  
    // move to left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_left_area();
    }
      
    x++;
  }
}

void create_bottom_left_area(){
int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw right area border
  cputcxy(30,12,0x05);
  cputcxy(30,13,0x05);
  cputcxy(30,14,0x05);
  cputcxy(30,15,0x05);
  cputcxy(30,16,0x05);
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  
  cputcxy(13,27,0x44);
  cputcxy(14,27,0x41);
  cputcxy(15,27,0x4E);
  cputcxy(16,27,0x47);
  cputcxy(17,27,0x45);
  cputcxy(18,27,0x52);
  
  vrambuf_flush();
    while (1) {
    
     
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // start right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 15;
      create_right_area();
    }
    // start top area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_top_area();
    }    
    // start bottom area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_boss_area();
    }     
    x++;
  } 
}

void create_top_right_area(){
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw bottom area border
  cputcxy(13,27,0x05);
  cputcxy(14,27,0x05);
  cputcxy(15,27,0x05);
  cputcxy(16,27,0x05);
  cputcxy(17,27,0x05); 
  cputcxy(18,27,0x05);

  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
  
    // move to top area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_top_area();
    }    
  
    // start right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)){
      heros.y = 25;
      create_right_area();
    }
      
    x++;
  }
}

void create_bottom_right_area(){
int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  
  //draw left area border
  cputcxy(1,12,0x05);
  cputcxy(1,13,0x05);
  cputcxy(1,14,0x05);
  cputcxy(1,15,0x05);
  cputcxy(1,16,0x05);
  
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  
  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to bottom area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90)){
      heros.x = 220;
      create_bottom_area();
    }    
    // move to right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)){
      heros.y = 195;
      create_right_area();
    }    
      
    x++;
  }
}

void create_boss_area(){
int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
  
  vrambuf_flush();
    while (1) {
    
    if(x == 500){
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    x++;
  }

}


// main program
void main() {
  
  pal_all(PALETTE);
  
  oam_clear();
  
  init_game();
  // loop forever
  clrscrn();
  init_game();
  spawn_item(&hearts);
  
  while(1){
  create_start_area();
  }

}
