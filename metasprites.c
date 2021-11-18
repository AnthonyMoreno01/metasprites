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

Hero heros;
Heart hearts;
// array of floors
Floor floors[MAX_FLOORS];
unsigned char pad1;	// joystick
unsigned char pad1_new; // joystick


//Direction array affects movement and gravity
typedef enum { D_RIGHT, D_DOWN, D_LEFT, D_UP, D_STAND } dir_t;
const char DIR_X[5] = { 1, 0, -1, 0, 0};
const char DIR_Y[5] = { 0, 1, 0, -1, 0};
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

void spawn_item(Heart* h)
{
  
  h->x = 10;
  h->y = 10;
  oam_meta_spr( h->x , h->y, 16, metasprite1);
  
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

 // vram_adr(NTADR_A(0,3));
  //vram_fill(5, 32);
  //vram_adr(0x23c0);
  //vram_fill(0x55, 8);

  oam_clear();
  heros.x = 48;
  heros.y = 120;
  vrambuf_clear();

  oam_meta_spr(heros.x, heros.y, 4, metasprite);
  vrambuf_clear();
  set_vram_update(updbuf);
  
  
  cputsxy(11,1,"Score:");
  cputcxy(17,1,'0');
  cputcxy(18,1,'0');
  cputcxy(19,1,'0');
  cputcxy(20,1,'0');
  
  ppu_on_all();
  
  vrambuf_clear();
  
}


void create_screen1(){
  
  heros.x = 48;
  heros.y = 120;
  
  
  
}

void create_screen2(){
  
}
// main program
void main() {
  //char i;	// actor index
  //char oam_id;	// sprite ID
  int x;
  pal_all(PALETTE);
  //joy_install (joy_static_stddrv);
  
  oam_clear();
  
  
  init_game();
  // loop forever
  clrscrn();
  init_game();
  while (1) {
    
     
    if(x == 500){
      movement(&heros);
      move_player(&heros);
    oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      //resetmovement(&heros);
      x=0;
    }
    
    if((heros.x >= 130 && heros.x <= 150) && (heros.y >= 130 && heros.y <= 150)){
      create_screen1();

    }
    
    
    x++;
  }
}
