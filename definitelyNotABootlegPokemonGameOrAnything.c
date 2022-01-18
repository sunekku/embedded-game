#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "sprites.h"

/*IMPORTANT: if you're reading this, please check out Jibaku Shonen Hanako-kun. Best manga I've read so far. Contrary to what the
incorrectly-translated English title suggests, it has nothing to do with lavatories.*/

/*must be volatile to avoid issues that arise from asynchronous changes to register contents*/
volatile int *pixel_ctrl_ptr;
volatile int *sw_ptr;
int pixel_buffer_start;

/*macros needed for game operation*/
#define NUM_STATS 3
#define ABS(x) (((x) > 0) ? (x) : -(x))


/*DE1-Soc peripheral address values*/
#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000

/*VGA screen resolution*/
#define RESX 320
#define RESY 240

/*VGA colors*/
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

/*sprite and other resolution*/
//character sprite resolution
#define PRESX 92  
#define PRESY 120
//health bar resolution
#define RES_HPX 120
#define RES_HPY 10
//move button resolution
#define RES_BUTTONX 120
#define RES_BUTTONY 15
//animation box resolution
#define ARESX 120
#define ARESY 120

/*image mapping to VGA screen*/ 
//player 1 position
#define P1X 0
#define P1Y 120
//player 2 position
#define P2X 228
#define P2Y 120
//animation box position
#define AX 100
#define AY 120
//player 1 HP box position
#define HP1X 5
#define HP1Y 105
//player 2 HP box position
#define HP2X 195
#define HP2Y 105

/*utility function for plotting pixel*/
void plot_pixel(int x, int y, short int color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
}

/*utility function for polling for video frame end*/
void wait_for_vsync(){
    int status;
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    //apply bitmask
    while((status & 0x01) != 0){
        status = *(pixel_ctrl_ptr + 3);
    }
}

/*utility function for timing animations. Currently the animation timing kinda sucks but I can easily just adjust numbers so it's not a big deal.*/
void timeout(int x){
    for(int i = 0; i < x; i++){}
}

/*utility function for wiping the screen*/
void clear_screen(){
    for (int i = 0; i < RESX ; i++){
        for (int j = 0; j < RESY ; j++){
            *(short int *)(pixel_buffer_start + (j << 10) + (i << 1)) = 0x0;
        }
    }
}

/*player 1 stats*/
#define P1HP_MAX 360  
#define P1_ATK 100

/*player 2 stats*/
#define P2HP_MAX 240
#define P2_ATK 150

/*player 1 and player 2 status arrays*/
int p1[3] = {P1HP_MAX, P1_ATK, 0};
int p2[3] = {P2HP_MAX, P2_ATK, 0};

/*utility function to print image to VGA screen*/
void draw(int resx, int resy, int offsetx, int offsety, int* img){
    for(int x = 0; x < resx; x++){
        for(int y = 0; y < resy; y++){
            if(!img)
                plot_pixel(x + offsetx, y + offsety, 0x0);
            else
                plot_pixel(x + offsetx, y + offsety, img[resx*y + x]);
        }
    }
}

/*utility function to draw health bar*/
void draw_rect(int x, int y, int xsize, int ysize, int color){
    for(int j = 0 ; j < ysize ; j++){
        for(int i = 0 ; i < xsize ; i++){
            plot_pixel(x + i, y + j, color);
        }
    }
}

/*draw player 1's HP bar*/
void draw_hp1(){
    draw_rect(HP1X, HP1Y, RES_HPX, RES_HPY, RED);
    draw_rect(HP1X, HP1Y, p1[0]/3, RES_HPY, GREEN);
}

/*draw player 2's HP bar*/
void draw_hp2(){
    draw_rect(HP2X, HP2Y, RES_HPX, RES_HPY, RED);
    draw_rect(HP2X, HP2Y, p2[0]/2, RES_HPY, GREEN);
}

/*in-game damage formula*/
int damage_calc(int power, int base_dmg){
    return ((rand() % 5) + base_dmg + power/10);
}

/*utility function to perform logical shift right*/
int LRS(int x, int n) {
    return (unsigned)x >> n;
}

/*The following are all moves. If you happen to play Showdown enough to be familiar with these, please don't cringe. I don't actually play 
the game. And make sure to read all the comments. They're definitely useful and document the code. I'm definitely not lying.*/

//very elaborate animation. Inspired by pokemon. definitely not because pokemon animations are primitive and easy to replicate. of course not.
void dragon_rage(){
    int dmg = damage_calc(p1[1], 40);
    p2[0] -= dmg;
	
    draw(PRESX, PRESY, P1X, P1Y, NULL);
    draw(PRESX, PRESY, AX, AY, p1_spr);
	timeout(200000);
	wait_for_vsync();

	timeout(200000);
    wait_for_vsync();

    draw(PRESX, PRESY, P1X, P1Y, p1_spr);
    draw(PRESX, PRESY, AX, AY, NULL);
	draw(PRESX, PRESY, P2X, P2Y, NULL);
	timeout(200000);
	wait_for_vsync();

	timeout(200000);
    wait_for_vsync();

	pixel_buffer_start = *(pixel_ctrl_ptr + 1);
	draw(PRESX, PRESY, P2X, P2Y, p2_spr);
	timeout(200000);
	wait_for_vsync();

	pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

//this thing moves first in the actual game, but i guess gamefreak programmers are just better than i am. time to resign.
void quick_attack(){
    int dmg = damage_calc(p2[1], 30);
    p1[0] -= dmg;
    draw(PRESX, PRESY, P2X, P2Y, NULL);
    draw(PRESX, PRESX, AX, AY, p2_spr);
    timeout(200000);
    wait_for_vsync();
    
    timeout(200000);
    wait_for_vsync();

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    draw(PRESX, PRESY, P2X, P2Y, p2_spr);
    draw(PRESX, PRESX, AX, AY, NULL);
    draw(PRESX, PRESY, P1X, P1Y, NULL);
    timeout(200000);
	wait_for_vsync();
	
    timeout(200000);
    wait_for_vsync();
	
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    draw(PRESX, PRESY, P1X, P1Y, p1_spr);
    timeout(200000);
	wait_for_vsync();
	
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*it throws flames*/
void flamethrower(){
    int dmg = damage_calc(p1[1], 80);
    p2[0] -= dmg;
    draw(ARESX, ARESY, AX, AY, flame);
    wait_for_vsync();
	
	timeout(200000);
	wait_for_vsync(); 
	
	timeout(200000);
    wait_for_vsync(); 
	
	timeout(200000);
    wait_for_vsync(); 

    pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
    draw(ARESX, ARESY, AX, AY, NULL);
    wait_for_vsync();
    
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
}

/*it bolts thunder*/
void thunderbolt(){
    int dmg = damage_calc(p2[1], 80);
    p1[0] -= dmg;
    draw(ARESX, ARESY, AX, AY, bolt);
    wait_for_vsync(); 
	
    timeout(200000);
    wait_for_vsync();
	
    timeout(200000);
    wait_for_vsync();
	
    timeout(200000);
    wait_for_vsync();
    
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    draw(ARESX, ARESY, AX, AY, NULL);
    wait_for_vsync();

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*nasty plot? they must be talking about 86. unlike jibaku shonen hanako kun, i do not recommend you read/watch this one.*/
void nasty_plot(){
    p2[1] += 10;
    draw(ARESX, ARESY, AX, AY, question);
    wait_for_vsync(); 
	
	timeout(200000);
    wait_for_vsync();
	
	timeout(200000);
    wait_for_vsync(); 
	
    timeout(200000);
    wait_for_vsync();
	
    timeout(200000);
	draw(ARESX, ARESY, AX, AY, NULL);
	wait_for_vsync();

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*it's that one 'a song of ice and fire' book. the third one.*/
void swords_dance(){
    p1[1] += 10;
    draw(ARESX, ARESY, AX, AY, sword);
    wait_for_vsync();
	
	timeout(200000);
    wait_for_vsync();
	
	timeout(200000);
    wait_for_vsync();
	
    timeout(200000);
	draw(ARESX, ARESY, AX, AY, NULL);
	wait_for_vsync();

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*player 1's metronome*/
void metronome1(){
    int move = rand() % 3;
    switch(move){
        case 0:
            swords_dance();
            break;
        case 1:
            flamethrower();
            break;
        case 2: 
            dragon_rage();
            break;
    }
}

/*player 2's metronome*/
void metronome2(){
    int move = rand() % 3;
    switch(move){
        case 0:
            nasty_plot();
            break;
        case 1:
            thunderbolt();
            break;
        case 2: 
            quick_attack();
            break;
    }
}

/*the program starts here. too lazy to make a title screen. epic cringe. will do later if i feel like it.*/
int main(void){
START:
    //initialize base stats
    p1[0] = P1HP_MAX;
    p1[1] = P1_ATK;
    p1[2] = rand() % 100;
    p2[0] = P2HP_MAX;
    p2[1] = P2_ATK;
    p2[2] = rand() % 100;
    
    //initialize switch data register pointer
    sw_ptr = (int*)0xFF200040;
    //initialize pixel buffer controller pointer
    pixel_ctrl_ptr = (int *)0xFF203020;
    //set contents of back buffer to some location in memory
    *(pixel_ctrl_ptr + 1) = 0xC8000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                                  
    wait_for_vsync();
    //swap has occurred, so it is necessary to update the pixel buffer start address after initializing back buffer contents to SDRAM memory
    //we draw on the back buffer
    //this will done every time a swap occurs

    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    draw(320, 110, 0, 0, buttons);
    draw_hp1();
    draw_hp2();
    draw(PRESX, PRESY, P2X, P2Y, p2_spr);
    draw(PRESX, PRESY, P1X, P1Y, p1_spr);
    wait_for_vsync();

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);

    draw(320, 110, 0, 0, buttons);
    draw_hp1();
    draw_hp2();
    draw(PRESX, PRESY, P2X, P2Y, p2_spr);
    draw(PRESX, PRESY, P1X, P1Y, p1_spr);

    while(1){
        //check for endgame
        if(p1[0] <= 0){
            draw(320, 240, 0, 0, pikawin);
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
            timeout(30000000);
            goto START;
        }   
        if(p2[0] <= 0){
            draw(320, 240, 0, 0, charwin);
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
            timeout(30000000);
            goto START;
        }

        //default move bools
        bool mv1b = false;
        bool mv2b = false;

        //temp storage of moves, index = 0 is first to move, index = 1 second
        int temp_move_storage[2];
        
        while(!mv1b || !mv2b){
            //sw data reg into sw_ptr for polling
            volatile int *sw_ptr = (int*)0xFF200040;
            
            //loads sw 7-4 into charmander move
            int mv1 = *sw_ptr >> 4;
            // loads sw 3-0 into pikachu move
            int mv2 = *sw_ptr << 28;
            mv2 = LRS(mv2, 28);

            //reason we need to do this is because of polling shenanigans.
            if(mv1 == 1){
                temp_move_storage[0] = 4;
                mv1b = true;
            }
            if(mv1 == 2){
                temp_move_storage[0] = 5;
                mv1b = true;
            }
            if(mv1 == 4){
                temp_move_storage[0] = 6;
                mv1b = true;
            }
            if(mv1 == 8){
                temp_move_storage[0] = 7;
                mv1b = true;
            }
            if(mv2 == 1){
                temp_move_storage[1] = 0;
                mv2b = true;
            }
            if(mv2 == 2){
                temp_move_storage[1] = 1;
                mv2b = true;
            }
            if(mv2 == 4){
                temp_move_storage[1] = 2;
                mv2b = true;
            }
            if(mv2 == 8){
                temp_move_storage[1] = 3;
                mv2b = true;
            }
        }

        // poll to make sure switches no longer pressed
        while(*sw_ptr){
            // empty
        }

        //check speeds of dudes, reorganize moves if true
        if(p1[2] < p2[2]){
            int temp = temp_move_storage[0];
            temp_move_storage[0] = temp_move_storage[1];
            temp_move_storage[1] = temp;
        }

        //case statements to draw the moves
        for(int i = 0 ; i < 2 ; i++){

            //this is necessary
            //i swear
            //im a perfect logician so everything i do is mathematically optimal and necessary
            draw_hp1();
            draw_hp2();
            
            if(p1[0] <= 0){
                draw(320, 240, 0, 0, pikawin);
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(30000000);
                goto START;
            }
            if(p2[0] <= 0){
                draw(320, 240, 0, 0, charwin);
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(30000000);
                goto START;
            }
            if(temp_move_storage[i] == 0){
                metronome2();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 1){
                nasty_plot();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 2){
                quick_attack();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);       
            }
            if(temp_move_storage[i] == 3){
                thunderbolt();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 4){
                metronome1();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 5){
                swords_dance();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 6){
                dragon_rage();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
            if(temp_move_storage[i] == 7){
                flamethrower();
                draw_hp1();
                draw_hp2();
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                timeout(5000000);
            }
        }
        draw_hp1();
        draw_hp2();
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
}