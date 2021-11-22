#include <stdlib.h>
#include "neslib.h"
#include <string.h>
#include <joystick.h>
#include <nes.h>
#include "metasprites.h"
#include "vrambuf.h"
//#link "vrambuf.c"
//#link "chr_generic.s"
//#link "chr_generic.s"


#define TILE 0xD8	// playable character attributes
#define TILE1 0xCC	// heart attributes
#define TILE2 0xF8
#define ATTR1 01	// heart attributes
#define ATTR 02		// character attributes
#define COLS 32
#define ROWS 27

//creates bullet
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      0xA2,   pal, \
        0,      8,      0xA2,   pal, \
        8,      0,      0xA2,   pal, \
        8,      8,      0xA2,   pal, \
        128};
 
// Hero Metasprite info
const unsigned char metasprite[]={
        0,      0,      TILE+0,   ATTR, 
        0,      8,      TILE+1,   ATTR, 
        8,      0,      TILE+2,   ATTR, 
        8,      8,      TILE+3,   ATTR, 
        128};

// Heart Metasprite info
const unsigned char metasprite1[]={
        0,      0,      TILE1,     ATTR1, 
        0,      8,      TILE1+1,   ATTR1, 
        8,      0,      TILE1+2,   ATTR1, 
        8,      8,      TILE1+3,   ATTR1, 
        128};

// Enemy Metasprite info
const unsigned char metasprite2[]={
        0,      0,      TILE2,     ATTR1, 
        0,      8,      TILE2+1,   ATTR1, 
        8,      0,      TILE2+2,   ATTR1, 
        8,      8,      TILE2+3,   ATTR1, 
        128};

//character set for box
const char BOX_CHARS[8]={0x8D,0x8E,0x87,0x8B,0x8C,0x83,0x85,0x8A};

//creates bullet sprite
DEF_METASPRITE_2x2(bullet, 0xe4, 0);

//initialize sprites
Hero heros;
Heart hearts[8];
Enemy enemy;

bool bullet_exists = false;
unsigned char pad1;	// joystick
unsigned char pad1_new; // joystick


//Direction array affects movement and gravity
typedef enum { D_RIGHT, D_DOWN, D_LEFT, D_UP, D_STAND } dir_t;
const char DIR_X[5] = { 2, 0, -2, 0, 0};
const char DIR_Y[5] = { 0, 2, 0, -2, 0};
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
  
  if (pad1 & JOY_LEFT_MASK) dir = D_LEFT;	else
  if (pad1 & JOY_RIGHT_MASK) dir = D_RIGHT;	else
  if (pad1 & JOY_UP_MASK) dir = D_UP;		else
  if (pad1 & JOY_DOWN_MASK) dir = D_DOWN;	else
  
    dir = D_STAND;
  if (heros.x < 10){
    dir = D_RIGHT;
  }
  if (heros.x > 230){
    dir = D_LEFT;
  }
  if (heros.y < 15){
    dir = D_DOWN;
  }
  if (heros.y > 200){
    dir = D_UP;
  }
  
    
  h->dir = dir;
}

void move_enemy(Enemy* h)
{
  h->x += DIR_X[h->dir];
  h->y += DIR_Y[h->dir];
}

// read user input and set hero direction to that value
void enemy_movement(Enemy* h)
{
  byte dir;
  
  if (enemy.x > heros.x) dir = D_LEFT;	else
  if (enemy.x < heros.x) dir = D_RIGHT;	else
  if (enemy.y > heros.y) dir = D_UP;	else
  if (enemy.y < heros.y) dir = D_DOWN;
  
  h->dir = dir;
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

void game_over(){
joy_install (joy_static_stddrv);
  clrscrn();
  vrambuf_flush();
  oam_clear();
  ppu_on_all();
  vrambuf_clear();
  cputsxy(15,5,"Game Over");
  cputsxy(2,20,"Press Any Button");
  cputsxy(2,22," To Play Again");
  vrambuf_flush();
  delay(100);
  while(1)
  {
   byte joy;
   joy = joy_read (JOY_1);
   if(joy)
    break;
  }
}
void you_win(){
  joy_install (joy_static_stddrv);
  clrscrn();
  vrambuf_flush();
  oam_clear();
  ppu_on_all();
  vrambuf_clear();
  cputsxy(15,5,"You win");
  cputsxy(2,20,"Press Any Button");
  cputsxy(2,22," To Play Again");
  vrambuf_flush();
  delay(100);
  while(1)
  {
    byte joy;
    joy = joy_read (JOY_1);
    if(joy)
      break;
  }
}
//function creates border
void draw_box(byte x, byte y, byte x2, byte y2, const char* chars) 
{
  byte x1 = x;
  cputcxy(x, y, chars[2]);
  cputcxy(x2, y, chars[3]);
  cputcxy(x, y2, chars[0]);
  cputcxy(x2, y2, chars[1]);
  while (++x < x2) 
  {
    cputcxy(x, y, chars[5]);
    cputcxy(x, y2, chars[4]);
  }
  while (++y < y2) 
  {
    cputcxy(x1, y, chars[6]);
    cputcxy(x2, y, chars[7]);
  }
}

//spawns bullet that interacts with enemy
void shoot(){
  struct Actor bullet_player;
  int i;

  pad1_new = pad_trigger(0); // read the first controller
  pad1 = pad_state(0);
  
  if (pad1 & PAD_A && !bullet_exists){
      // Bullet must spawn in front of player
      bullet_player.x = heros.x;
      bullet_player.y = heros.y - 12;
      bullet_exists = true;
    }
    // If the player's bullet exists...
    if (bullet_exists){
      
      // Check for enemy collision
      for(i=0; i<2; i++){
        bullet_player.y--;
        oam_meta_spr(bullet_player.x, bullet_player.y, 64, bullet);
        
        if(enemy.is_alive && 
          (bullet_player.x == enemy.x-11 && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-10 && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-9  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-8  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-7  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-6  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-5  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-4  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-3  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-2  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x-1  && bullet_player.y == enemy.y)  ||  
          (bullet_player.x == enemy.x    && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+1  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+2  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+3  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+4  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+5  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+6  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+7  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+8  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+9  && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+10 && bullet_player.y == enemy.y)  || 
          (bullet_player.x == enemy.x+11 && bullet_player.y == enemy.y)    )
        {
          
	enemy.is_alive = false;
        enemy.hp = enemy.hp-1;
        cputsxy(20,1,"BOSS:");
        cputcxy(26,1,enemy.hp);
        vrambuf_flush();
          break;
        }
      }
      
      
      if(enemy.is_alive == false)
      {

        bullet_exists = false;
        bullet_player.x = 240;
        bullet_player.y = 240;
        enemy.is_alive = true;
        oam_meta_spr(bullet_player.x, bullet_player.y, 64, bullet);
      }
      if (bullet_player.y < 1 || bullet_player.y > 190)
      {
        bullet_exists = false;
        bullet_player.x = 240;
        bullet_player.y = 240;
        oam_meta_spr(bullet_player.x, bullet_player.y, 64, bullet);
      }
    }
  
  
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
  int i;
  clrscrn();
  heros.lives == 0x31;
  enemy.hp = 0x39;
  vrambuf_clear();
  heros.lives = 0x31;
  oam_clear();
  heros.x = 120;
  heros.y = 110;
  vrambuf_clear();
  oam_meta_spr(heros.x, heros.y, 4, metasprite);
  vrambuf_clear();
  set_vram_update(updbuf);
  cputsxy(5,1,"LIVES:");
  cputcxy(11,1,'1');

  ppu_on_all();
  vrambuf_clear();
  
    for(i =0; i<9;i++)
  {
    hearts[i].x = 150;
    hearts[i].y = 100;
  }
}

void create_start_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(240, 240, 20, metasprite1);  
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  //draw bottom area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);

  vrambuf_flush();
  
    while (1) 
    {
     
    if(x == 300)
    {
      
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_right_area();
    }
    // move to left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 220;
      create_left_area();
    }    
    // move to top area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_top_area();
    }    
    // move to bottom area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_bottom_area();
    }
      
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;

    x++;
  } 
}

void create_top_left_area()
{
  int x;
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[0].x, hearts[0].y, 20, metasprite1);
  
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw bottom area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);

  vrambuf_flush();
    while (1) 
    {
    
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
    // move to top area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_top_area();
    }
    // move to left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_left_area();
    }
    // check for heart collision    
    if(hearts[0].x == heros.x && hearts[0].y == heros.y)
    {
      hearts[0].x = 240;
      hearts[0].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[0].x, hearts[0].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  }
}

void create_top_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[1].x, hearts[1].y, 20, metasprite1); 
  
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw start area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);
  
  vrambuf_flush();
    while (1) 
    {
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to top right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_top_right_area();
    }
    // move to top left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 220;
      create_top_left_area();
    }      
    // move to start area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_start_area();
    }
    // check for heart collision    
    if(hearts[1].x == heros.x && hearts[1].y == heros.y)
    {
      hearts[1].x = 240;
      hearts[1].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[1].x, hearts[1].y, 20, metasprite1);    
    }
      
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  }
}

void create_top_right_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[2].x, hearts[2].y, 20, metasprite1); 
  
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw bottom area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);

  vrambuf_flush();
    while (1) 
    {
    
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
  
    // move to top area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 220;
      create_top_area();
    }    
    // start right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_right_area();
    }
    // check for heart collision    
    if(hearts[2].x == heros.x && hearts[2].y == heros.y)
    {
      hearts[2].x = 240;
      hearts[2].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[2].x, hearts[2].y, 20, metasprite1);    
    }
    if(heros.lives == 0x30 || enemy.hp == 0x30)
      break;
    x++;
  }
}

void create_left_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[3].x, hearts[3].y, 20, metasprite1); 
  
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  //draw bottom area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);

  vrambuf_flush();
  
    while (1) 
    {
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
    // move to start area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_start_area();
    }  
    // move to top left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_top_left_area();
    }    
    // move to bottom left area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_bottom_left_area();
    }
    // check for heart collision    
    if(hearts[3].x == heros.x && hearts[3].y == heros.y)
    {
      hearts[3].x = 240;
      hearts[3].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[3].x, hearts[3].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  }
}

void create_right_area()
{
  int x; 
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[5].x, hearts[5].y, 20, metasprite1); 
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  //draw bottom area border
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);
  
  vrambuf_flush();
    while (1) 
    {
    
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }

    // move to start area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 220;
      create_start_area();
    }    
    // move to top right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_top_right_area();
    }    
    // move to bottom right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_bottom_right_area();
    }
    // check for heart collision  
    if(hearts[5].x == heros.x && hearts[5].y == heros.y)
    {
      hearts[5].x = 240;
      hearts[5].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[5].x, hearts[5].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  }
}

void create_bottom_left_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[6].x, hearts[6].y, 20, metasprite1); 
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
  //draw boss area border
  cputsxy(5,27,"DANGER");
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);
  cputsxy(21,27,"DANGER");
  
  
  vrambuf_flush();
    while (1) 
    {
  
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // start right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_bottom_area();
    }
    // start top area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_left_area();
    }    
    // start bottom area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200))
    {
      heros.y = 24;
      create_boss_area();
    }  
    // check for heart collision  
    if(hearts[6].x == heros.x && hearts[6].y == heros.y)
    {
      hearts[6].x = 240;
      hearts[6].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[6].x, hearts[6].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  } 
}


void create_bottom_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[7].x, hearts[7].y, 20, metasprite1); 
  //draw right area border
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);

  
  vrambuf_flush();
    while (1) 
    {
    
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
  
    // move to bottom right area
    if((heros.x <= 240 && heros.x >= 230) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 14;
      create_bottom_right_area();
    }
    // move to bottom left area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 224;
      create_bottom_left_area();
    }    
    // move to start area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_start_area();
    }
    // check for heart collision 
    if(hearts[7].x == heros.x && hearts[7].y == heros.y)
    {
      hearts[7].x = 240;
      hearts[7].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[7].x, hearts[7].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30 ||enemy.hp == 0x30)
        break;
    x++;
  }
}

void create_bottom_right_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[8].x, hearts[8].y, 20, metasprite1); 
  
  //draw left area border
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
  //draw top area border
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);

  vrambuf_flush();
    while (1) 
    {
    
    if(x == 300)
    {
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      x=0;
    }
   
    // move to bottom area
    if((heros.x <= 10 && heros.x >= 1) && (heros.y <= 120 && heros.y >= 90))
    {
      heros.x = 220;
      create_bottom_area();
    } 
    // move to right area
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5))
    {
      heros.y = 194;
      create_right_area();
    } 
    // check for heart collision 
    if(hearts[8].x == heros.x && hearts[8].y == heros.y)
    {
      hearts[8].x = 240;
      hearts[8].y = 240;
      heros.lives++;
        cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[8].x, hearts[8].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 || enemy.hp == 0x30)
        break;
    x++;
  }
}


void create_boss_area()
{
  int x,i,y, p;
  p = 1000;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
 //draw tip
 cputsxy(6,27,"PRESS SPACE TO SHOOT");
  for(i =0; i<9;i++)
  {
    hearts[i].x = 240;
    hearts[i].y = 240;
    oam_meta_spr(hearts[i].x, hearts[i].y, 20, metasprite1);  
  }
  enemy.x = 20;
  enemy.y = 20;
  oam_meta_spr(enemy.x, enemy.y, 48, metasprite2); 
  enemy.hp = 0x39;
  enemy.is_alive = true;
 
  cputsxy(20,1,"BOSS:");
  cputcxy(26,1,enemy.hp);
  vrambuf_flush();
  vrambuf_flush();
    while (1) 
    {
     if(x == 300)
     {
      shoot();
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      oam_meta_spr(enemy.x, enemy.y, 48, metasprite2); 
      x=0;
    }
    x++;
      
      if(y == p){
        enemy_movement(&enemy);
        move_enemy(&enemy);
        if(
          (heros.x == enemy.x-11 && heros.y == enemy.y)  || 
          (heros.x == enemy.x-10 && heros.y == enemy.y)  || 
          (heros.x == enemy.x-9  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-8  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-7  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-6  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-5  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-4  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-3  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-2  && heros.y == enemy.y)  || 
          (heros.x == enemy.x-1  && heros.y == enemy.y)  ||  
          (heros.x == enemy.x    && heros.y == enemy.y)  || 
          (heros.x == enemy.x+1  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+2  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+3  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+4  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+5  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+6  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+7  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+8  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+9  && heros.y == enemy.y)  || 
          (heros.x == enemy.x+10 && heros.y == enemy.y)  || 
          (heros.x == enemy.x+11 && heros.y == enemy.y))  {
        
        
          heros.lives--;
          heros.x = 120;
          heros.y = 120;
        if(heros.lives == 0x30 || enemy.hp == 0x30)
        {
          game_over();
          break;
        }
        else
        {
        cputcxy(11,1, heros.lives);
          vrambuf_flush();
        }
        }
        y = 0;
      }
      if(enemy.hp == 0x33)
      {
       p = 750;
      }
      if(enemy.hp == 0x31)
      {
       p = 600;
      }
      
     if(enemy.hp == 0x30)
     {
       you_win();
       break;
     }
      y++;
  }
}

void title_screen()
{
  pal_all(PALETTE);
  init_game();
  clrscrn();
  vrambuf_flush();
  oam_clear();
  ppu_on_all();
  vrambuf_clear();
  cputsxy(10,8,"Dungeon Crawl");
  cputsxy(5,10,"Collect the Hearts");
  cputsxy(7,12,"to with the Game");
  cputsxy(2,20,"Press Any Button To Play");
  vrambuf_flush();
}

void play()
{
  oam_clear();
  init_game();
  clrscrn();
  init_game();
  create_start_area();
}
// main program
void main() 
{
joy_install (joy_static_stddrv);
  title_screen();
  
  while(1)
  {
    byte joy;
    joy = joy_read (JOY_1);
    if(joy)
      break;
  }
  
  while(1)
play();

}
