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
#define COLS 32
#define ROWS 27


#define DEF_METASPRITE_2x2static(name,code,pal)\
const unsigned char name[]={\
        0,      0,      0xA2,   pal, \
        0,      8,      0xA2,   pal, \
        8,      0,      0xA2,   pal, \
        8,      8,      0xA2,   pal, \
        128};
 
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      code,     pal, \
        0,      8,      code+1,   pal, \
        8,      0,      code+2,   pal, \
        8,      8,      code+3,   pal, \
        128};

//creates bullet sprite
DEF_METASPRITE_2x2static(bullet, 0xe4, 0);
bool bullet_exists = false;
//creates hero sprite
DEF_METASPRITE_2x2(metasprite, 0xD8, 2);
Hero heros;
//creates heart sprite
DEF_METASPRITE_2x2(metasprite1, 0xCC, 1);
Heart hearts[7];
//creates enemy[0] sprite
DEF_METASPRITE_2x2(metasprite2, 0xF8, 1);
Enemy enemy[6];
//character set for box
const char BOX_CHARS[8]={0x8D,0x8E,0x87,0x8B,0x8C,0x83,0x85,0x8A};


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
//function moves hero in appropriate direction
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
   
    //stops player from moving before input is read
	dir = D_STAND;
  
  //stops player from moving out of bounds
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
//function moves enemy[0] in appropriate direction
void move_enemy(Enemy* h)
{
  h->x += DIR_X[h->dir];
  h->y += DIR_Y[h->dir];
}
// moves enemy[0] based on hero location
void enemy_movement(Enemy* h)
{
  byte dir;
  if (h->x > heros.x) dir = D_LEFT;	else
  if (h->x < heros.x) dir = D_RIGHT;	else
  if (h->y > heros.y) dir = D_UP;	else
  if (h->y < heros.y) dir = D_DOWN;
  h->dir = dir;
}
//function displays text
void cputcxy(byte x, byte y, char ch) 
{
  vrambuf_put(NTADR_A(x,y), &ch, 1);
}
//function displays text
void cputsxy(byte x, byte y, const char* str) 
{
  vrambuf_put(NTADR_A(x,y), str, strlen(str));
}
//function displays lose screen and waits for input to play again
void game_over()
{
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
//function displays win screen and waits for input to play again
void win_screen()
{
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
//draws left zone border
void draw_left_border()
{
  cputcxy(1,12,0x07);
  cputcxy(1,13,0x07);
  cputcxy(1,14,0x07);
  cputcxy(1,15,0x07);
  cputcxy(1,16,0x07);
}
//draws right zone border
void draw_right_border()
{
  cputcxy(30,12,0x06);
  cputcxy(30,13,0x06);
  cputcxy(30,14,0x06);
  cputcxy(30,15,0x06);
  cputcxy(30,16,0x06);
}
//draws top zone border
void draw_top_border()
{
  cputcxy(13,2,0x05);
  cputcxy(14,2,0x05);
  cputcxy(15,2,0x05);
  cputcxy(16,2,0x05);
  cputcxy(17,2,0x05);
  cputcxy(18,2,0x05);
}
//draws bottom zone border
void draw_bottom_border()
{
  cputcxy(13,27,0x08);
  cputcxy(14,27,0x08);
  cputcxy(15,27,0x08);
  cputcxy(16,27,0x08);
  cputcxy(17,27,0x08); 
  cputcxy(18,27,0x08);
}

//spawns bullet that interacts with enemy[0]
void shoot(Enemy* e){
  struct Actor bullet_player;
  int i;
  pad1_new = pad_trigger(0);
  pad1 = pad_state(0);
  if (pad1 & PAD_A && !bullet_exists)
  {
      //Spawns bullet in front of hero location
      bullet_player.x = heros.x;
      bullet_player.y = heros.y - 12;
      bullet_exists = true;
    }
    //if bullet exists 
    if (bullet_exists){
      
      // Check for enemy[0] collision
      for(i=0; i<2; i++)
      {
        bullet_player.y--;
        oam_meta_spr(bullet_player.x, bullet_player.y, 64, bullet);
        if(e->is_alive && 
          (bullet_player.x == e -> x-11 && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-10 && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-9  && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-8  && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-7  && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-6  && bullet_player.y == e->y)  || 
          (bullet_player.x == e ->x-5  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x-4  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x-3  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x-2  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x-1  && bullet_player.y == e->y)  ||  
          (bullet_player.x == e->x    && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+2  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+1  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+4  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+3  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+5  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+6  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+7  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+8  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+9  && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+10 && bullet_player.y == e->y)  || 
          (bullet_player.x == e->x+11 && bullet_player.y == e->y)    )
        {   
	e->is_alive = false;
        e->hp = e->hp-1;
        //cputsxy(19,1,"BOSS:");
        cputcxy(26,1,e->hp);
        vrambuf_flush();
          break;
        }
      }
      //check if bullet hit enemy[0]
      if(e->is_alive == false)
      {
        bullet_exists = false;
        bullet_player.x = 240;
        bullet_player.y = 240;
        e->is_alive = true;
        oam_meta_spr(bullet_player.x, bullet_player.y, 64, bullet);
      }
      //check if bullet is out of bounds
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
//initialize hero, hearts, and enemy[0]
void init_game()
{
  int i;
  
  clrscrn();
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
    for(i =0; i<8;i++)
  {
    hearts[i].x = 150;
    hearts[i].y = 100;
  }
  for(i = 0; i<6; i++){
    enemy[i].id = i;
    enemy[i].x = 20;
    enemy[i].y = 20;
    enemy[i].is_alive= true;
    enemy[i].hp = 0x39;
  }
}
//creates start area
void create_start_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(240, 240, 20, metasprite1);

  //draw right area border
  draw_right_border();
  //draw left area border
  draw_left_border();
  //draw top area border
  draw_top_border();
  //draw bottom area border
  draw_bottom_border();
  
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
   if(enemy[1].hp == 0x30 && enemy[2].hp == 0x30 && 
     enemy[3].hp == 0x30 && enemy[4].hp == 0x30){
    
    create_boss_area(&enemy[5]);
  }
      if(heros.lives == 0x30 )
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
  draw_right_border();
  //draw bottom area border
  draw_bottom_border();
  
  //draw boss border
  if(enemy[1].hp != 0x30){
  cputsxy(5,2,"DANGER");
  draw_top_border();
  cputsxy(21,2,"DANGER");
  }
  
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
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)&& enemy[1].hp != 0x30)
    {
      heros.y = 194;
      create_boss_area(&enemy[1]);
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
      if(heros.lives == 0x30 )
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
  draw_right_border();
  //draw left area border
  draw_left_border();
  //draw start area border
  draw_bottom_border();
  
  
  
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
      
      if(heros.lives == 0x30 )
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
  draw_left_border();
  //draw bottom area border
  draw_bottom_border();
  if(enemy[1].hp != 0x30){
  cputsxy(5,2,"DANGER");
  draw_top_border();
  cputsxy(21,2,"DANGER");
  }
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
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 20 && heros.y >= 5)&& enemy[2].hp != 0x30)
    {
      heros.y = 194;
      create_boss_area(&enemy[2]);
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
    if(heros.lives == 0x30 )
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
  draw_right_border();
  //draw top area border
  draw_top_border();
  //draw bottom area border
  draw_bottom_border();

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
      if(heros.lives == 0x30 )
        break;
    x++;
  }
}

void create_right_area()
{
  int x; 
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[4].x, hearts[4].y, 20, metasprite1); 
  //draw left area border
  draw_left_border();
  //draw top area border
  draw_top_border();
  //draw bottom area border
  draw_bottom_border();
  
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
    if(hearts[4].x == heros.x && hearts[4].y == heros.y)
    {
      hearts[4].x = 240;
      hearts[4].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[4].x, hearts[4].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 )
        break;
    x++;
  }
}

void create_bottom_left_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[5].x, hearts[5].y, 20, metasprite1); 
  //draw right area border
  draw_right_border();
  //draw top area border
  draw_top_border();
  //draw boss area border
  
  if(enemy[3].hp != 0x30){
  cputsxy(5,27,"DANGER");
  draw_bottom_border();
  cputsxy(21,27,"DANGER");
  }
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
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)&& enemy[3].hp != 0x30)
    {
      heros.y = 24;
      
      create_boss_area(&enemy[3]);
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
    
      if(heros.lives == 0x30)
        break;
    x++;
  }
  
}


void create_bottom_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[6].x, hearts[6].y, 20, metasprite1); 
  //draw right area border
  draw_right_border();
  //draw left area border
  draw_left_border();
  //draw top area border
  draw_top_border();
  

  
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
    if(hearts[6].x == heros.x && hearts[6].y == heros.y)
    {
      hearts[6].x = 240;
      hearts[6].y = 240;
      heros.lives++;
      cputcxy(11,1, heros.lives);
      vrambuf_flush();
      oam_meta_spr(hearts[6].x, hearts[6].y, 20, metasprite1);    
    }
      if(heros.lives == 0x30 )
        break;
    x++;
  }
}

void create_bottom_right_area()
{
  int x;
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  oam_meta_spr(hearts[7].x, hearts[7].y, 20, metasprite1); 
  //draw left area border
  draw_left_border();
  //draw top area border
  draw_top_border();
  if(enemy[3].hp != 0x30){
  cputsxy(5,27,"DANGER");
  draw_bottom_border();
  cputsxy(21,27,"DANGER");
  }
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
    if((heros.x <= 150 && heros.x >= 90) && (heros.y <= 220 && heros.y >= 200)&& enemy[4].hp != 0x30)
    {
      heros.y = 24;
      
      create_boss_area(&enemy[3]);
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
      if(heros.lives == 0x30 )
        break;
    x++;
  }
}

//creates boss zone and allows player to shoot
void create_boss_area(Enemy* e)
{
  int x,i,y, p;

  switch(e->id){


     case 1: 
      p = 1000;    
      e->x = 20;
      e->y = 20;
      break;
      case 2: 
      p = 900;
      e->x = 20;
      e->y = 20;
      break;
      case 3: 
      p = 800;    
      e->x = 20;
      e->y = 20;
      break;
      case 4: 
      p = 700;    
      e->x = 20;
      e->y = 20;
      break;
    case 5:
      p = 600;
      e->x = 20;
      e->y = 20;

  }
  
  
  draw_box(1,2,COLS-2,ROWS,BOX_CHARS);
  
 //draw tip
 cputsxy(6,27,"PRESS SPACE TO SHOOT");
  for(i =0; i<9;i++)
  {
    hearts[i].x = 0;
    hearts[i].y = 0;
    oam_meta_spr(hearts[i].x, hearts[i].y, 20, metasprite1);  
  }

  oam_meta_spr(e->x, e->y, 48, metasprite2); 
  e->hp = 0x39;
  e->is_alive = true;
 
 switch(e->id){
     case 1: cputsxy(19,1,"BOSS1:"); break;
     case 2: cputsxy(19,1,"BOSS2:"); break;
     case 3: cputsxy(19,1,"BOSS3:"); break;
     case 4: cputsxy(19,1,"BOSS4:"); break;
     case 5: cputsxy(19,1,"WYLIE:"); break;
 }
  
  cputcxy(26,1,e->hp);
  vrambuf_flush();
  vrambuf_flush();
    while (1) 
    {
     if(x == 300)
     {
      shoot(e);
      movement(&heros);
      move_player(&heros);
      oam_meta_spr(heros.x, heros.y, 4, metasprite); 
      oam_meta_spr(e->x, e->y, 48, metasprite2); 
      x=0;
    }
    x++;
      if(y == p)
      {
        enemy_movement(e);
        move_enemy(e);
        //check for collision between enemy[0] and hero
        if(
          (heros.x == e->x-11 && heros.y == e->y)  || 
          (heros.x == e->x-10 && heros.y == e->y)  || 
          (heros.x == e->x-9  && heros.y == e->y)  || 
          (heros.x == e->x-8  && heros.y == e->y)  || 
          (heros.x == e->x-7  && heros.y == e->y)  || 
          (heros.x == e->x-6  && heros.y == e->y)  || 
          (heros.x == e->x-5  && heros.y == e->y)  || 
          (heros.x == e->x-4  && heros.y == e->y)  || 
          (heros.x == e->x-3  && heros.y == e->y)  || 
          (heros.x == e->x-2  && heros.y == e->y)  || 
          (heros.x == e->x-1  && heros.y == e->y)  ||  
          (heros.x == e->x    && heros.y == e->y)  || 
          (heros.x == e->x+1  && heros.y == e->y)  || 
          (heros.x == e->x+2  && heros.y == e->y)  || 
          (heros.x == e->x+3  && heros.y == e->y)  || 
          (heros.x == e->x+4  && heros.y == e->y)  || 
          (heros.x == e->x+5  && heros.y == e->y)  || 
          (heros.x == e->x+6  && heros.y == e->y)  || 
          (heros.x == e->x+7  && heros.y == e->y)  || 
          (heros.x == e->x+8  && heros.y == e->y)  || 
          (heros.x == e->x+9  && heros.y == e->y)  || 
          (heros.x == e->x+10 && heros.y == e->y)  || 
          (heros.x == e->x+11 && heros.y == e->y))  
        {
          //if collision hero hp drops 1 and reset player to middle of screen
          heros.lives--;
          heros.x = 120;
          heros.y = 120;
          //when heros hp is 0 game over
        if(heros.lives == 0x30)
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
      //when enemy[0] hp is 3 increase movement
      if(e->hp == 0x33)
      {
       p = 500;
      }
      //when enemy[0] hp is 1 increase movement
      if(e->hp == 0x31)
      {
       p = 400;
      } 
      //when enemy[0] hp is 0 you win
     if(e->hp == 0x30)
     {
       e->x = 240;
       e->y = 240;
       
       oam_meta_spr(e->x, e->y, 48, metasprite2); 
       
       switch(e->id){
         case 1: 
           heros.y = 14;
           create_top_left_area();
           break;
         case 2: 
           heros.y = 14;
           create_top_right_area();
           break;
         case 3: 
           heros.y = 194;
           create_bottom_left_area();
           break;
         case 4: 
           heros.y = 194;
           create_bottom_right_area();
           break;
         case 5: 
           win_screen();
           break;
       }     
       
       //win_screen();
       break;
     }
      y++;
  }
}//creates title screen
void title_screen()
{
  pal_all(PALETTE);
  init_game();
  clrscrn();
  vrambuf_flush();
  oam_clear();
  ppu_on_all();
  vrambuf_clear();
  cputsxy(10,6,"Dungeon Crawl");
  cputsxy(4,10,"Collect Hearts");
  cputsxy(4,12,"And Defeat the Boss");
  cputsxy(4,14,"to Beat the Game");
  cputsxy(8,18,"Press Any Button");
  cputsxy(13,20,"To Play");
  vrambuf_flush();
}
//endless function
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
