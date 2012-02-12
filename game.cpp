#include "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

class gamefield;
class tile;

//swap tile coordinates
int i1,j1,i2,j2,k=0;

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;
const int SCREEN_BPP = 32;

//Tile constants
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;
const int TOTAL_TILES = 64;
const int TILE_SPRITES = 5;

//The different tile sprites
const int TILE_EMPTY = 0;
const int TILE_RED = 1;
const int TILE_GREEN = 2;
const int TILE_BLUE = 3;
const int TILE_YELLOW = 4;

//The surfaces
SDL_Surface *screen = NULL;
SDL_Surface *tileSheet = NULL;

//Sprite from the tile sheet
SDL_Rect clips[TILE_SPRITES];

//The event structure
SDL_Event event;

//the gamefield
class gamefield {

private:
	int field[8][8]; //gamefield
	int check[8][8]; //match3 coord
	int match; //match length

public:
	//generates a matrix with random numbers
	void generate_field()
	{
		srand (time(NULL)); //seed random
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				field[i][j] = (rand()%4)+1; //random numbers from 1 to 4
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				check[i][j] = 0; //no matches
		match=0;
	}

	//refills empty spaces
	void fill()
	{
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				if (!field[i][j]) //refill 0 with new values
					field[i][j] = (rand()%4)+1;
	}

	bool check_right(int i, int j)
	{
		int k=j+1; //right element
		if (field[i][j] == field[i][k]) //if match
		{
			match++; //increase match length
			if (j < 6)
				check_right(i,k); //check further
			return true;
		}
		else
			return false;
	}

	bool check_down(int i, int j)
	{
		int k=i+1; //lower element
		if (field[i][j] == field[k][j]) //if match
		{
			match++; //increase match length
			if (i < 6)
				check_down(k,j); //check further
			return true;
		}
		else
			return false;
	}

	//find matching blocks
	void find_match()
	{
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
			{
				if(check_down(i,j)) //check column
				{
					if (match >= 2) //3 or more matches
					while (match != -1)
					{
						check[i+match][j] = 1; //check for delete
						match--;
					}
					match = 0; //no match
				}
				if (check_right(i,j)) //check row
				{
					if (match >= 2) //3 or more matches
					while (match != -1)
					{
						check[i][j+match] = 1; //check for delete
						match--;
					}
					match = 0; //no match
				}
			}
	}
	
	//check for matches
	bool no_match()
	{
		find_match(); //find matches
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				if (check[i][j])
					return false; //there are matches
		return true; //there are no matches
	}

	//delete matching blocks
	void del_match()
	{
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				if (check[i][j]) //if checked for delete
				{
					field[i][j] = 0; //delete
					check[i][j] = 0; //uncheck
				}
	}
		
	//blocks falling
	void zero_up()
	{
		for (int i=0; i<8; i++)
			for (int j=0; j<8; j++)
				if (!field[i][j]) //if empty
				{
					for (int k=i; k>0; k--)
					{
						//column falls
						field[k][j] = field[k-1][j] + field[k][j];
						field[k-1][j] = field[k][j] - field[k-1][j];
						field[k][j] = field[k][j] - field[k-1][j];
					}
				}
	}

	//check if blocks can swap
	bool can_swap(int i1, int j1, int i2, int j2)
	{
		if ((i1-i2==1) && (i2-i1==-1) && (j1==j2) ||
			(i2-i1==1) && (i1-i2==-1) && (j1==j2) ||
			(j1-j2==1) && (j2-j1==-1) && (i1==i2) ||
			(j2-j1==1) && (j1-j2==-1) && (i1==i2))
			return true;
		else
			return false;
	}

	//swap two blocks
	void swap(int i1, int j1, int i2, int j2)
	{
		int tmp = field[i1][j1];
		field[i1][j1] = field[i2][j2];
		field[i2][j2] = tmp;
	}

	//mouse manipulator
	void handle_events()
	{
		//the mouse offsets
		int x=0, y=0;
		//field coordinates
		

		//if the mouse moved
		if(event.type == SDL_MOUSEMOTION)
		{
			//Get the mouse offsets
			x = event.motion.x;
			y = event.motion.y;
		}
						
		//if a mouse button was pressed
		if(event.type == SDL_MOUSEBUTTONDOWN)
		{
			//if the left mouse button was pressed
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				//get the mouse offsets
				x = event.button.x;
				y = event.button.y;

				k++;
				if (k==1)
				{
					i1 = y/80;
					j1 = x/80;
				}
				if (k==2)
				{
					k=0;
					i2 = y/80;
					j2 = x/80;
					if (can_swap(i1,j1,i2,j2))
					{
						swap(i1,j1,i2,j2);
						if (no_match())
							swap(i1,j1,i2,j2);
					}
				}
			}
		}
	}

	friend bool set_tiles(tile *tiles[], gamefield board);
	friend class tile;
};

//the tile
class tile {

private:
	//attributes of the tile
    SDL_Rect box;
    //tile type
    int type;

public:
	tile(int x, int y, int tileType)
	{
		//get the offsets
		box.x = x;
		box.y = y;

		//set the collision box
		box.w = TILE_WIDTH;
		box.h = TILE_HEIGHT;

		//get the tile type
		type = tileType;
	}

	//use image
	void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL)
	{
		//holds offsets
		SDL_Rect offset;

		//get offsets
		offset.x = x;
		offset.y = y;

		//blit
		SDL_BlitSurface(source, clip, destination, &offset);
	}

	void show()
	{
		//show the tile
		apply_surface(box.x, box.y, tileSheet, screen, &clips[type]);
	}

	friend class gamefield;
};

//load image function
SDL_Surface *load_image(string filename)
{
    //the image that's loaded
    SDL_Surface* loadedImage = NULL;

    //the optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //load the image
    loadedImage = IMG_Load(filename.c_str());

    //if the image loaded
    if(loadedImage != NULL)
    {
        //create an optimized surface
        optimizedImage = SDL_DisplayFormat(loadedImage);

        //free the old surface
        SDL_FreeSurface(loadedImage);

        //if the surface was optimized
        if(optimizedImage != NULL)
        {
            //color key surface
            SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB(optimizedImage->format, 0, 0xFF, 0xFF));
        }
    }

    //return the optimized surface
    return optimizedImage;
}

//initialize all SDL subsystems
bool init()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        return false;
    }

    //set up the screen
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

    //if there was an error in setting up the screen
    if(screen == NULL)
    {
        return false;
    }

    //set the window caption
    SDL_WM_SetCaption("Match3", NULL);

    //if everything initialized fine
    return true;
}

//loads image files
bool load_files()
{
    //load the tile sheet
    tileSheet = load_image("tiles.png");

    //if there was a problem in loading the tiles
    if(tileSheet == NULL)
    {
        return false;
    }

    //if everything loaded fine
    return true;
}

//deletes tiles from RAM
void clean_up(tile *tiles[])
{
    //free the surfaces
    SDL_FreeSurface(tileSheet);

    //free the tiles
    for(int t=0; t < TOTAL_TILES; t++)
    {
        delete tiles[t];
    }

    //quit SDL
    SDL_Quit();
}

//cuts the tile sheet 
void clip_tiles()
{
    //clip the sprite sheet
    clips[TILE_RED].x = 0;
    clips[TILE_RED].y = 0;
    clips[TILE_RED].w = TILE_WIDTH;
    clips[TILE_RED].h = TILE_HEIGHT;

    clips[TILE_GREEN].x = 0;
    clips[TILE_GREEN].y = 80;
    clips[TILE_GREEN].w = TILE_WIDTH;
    clips[TILE_GREEN].h = TILE_HEIGHT;

    clips[TILE_BLUE].x = 0;
    clips[TILE_BLUE].y = 160;
    clips[TILE_BLUE].w = TILE_WIDTH;
    clips[TILE_BLUE].h = TILE_HEIGHT;

    clips[TILE_YELLOW].x = 80;
    clips[TILE_YELLOW].y = 0;
    clips[TILE_YELLOW].w = TILE_WIDTH;
    clips[TILE_YELLOW].h = TILE_HEIGHT;

    clips[ TILE_EMPTY ].x = 160;
    clips[ TILE_EMPTY ].y = 80;
    clips[ TILE_EMPTY ].w = TILE_WIDTH;
    clips[ TILE_EMPTY ].h = TILE_HEIGHT;
}

//create tile field
bool set_tiles(tile *tiles[], gamefield board)
{
    //tile offsets
    int x=0, y=0;
	//field coordinates
	int i=0, j=0;
    
	//initialize the tiles
    for(int t=0; t < TOTAL_TILES; t++)
    {
        //determines what kind of tile will be made
        int tileType = -1;

        //read tile type from gamefield matrix
        tileType = board.field[i][j];

        //if the number is a valid tile number
        if((tileType >= 0) && (tileType < TILE_SPRITES))
        {
            tiles[t] = new tile(x, y, tileType);
        }
        //if we don't recognize the tile type
        else        
            return false;

        //move to next tile spot
        x += TILE_WIDTH;
		j++;

        //if we've gone too far
        if( x >= TILE_WIDTH*8 )
        {
            //move back
            x=0;
			j=0;

            //move to the next row
            y += TILE_HEIGHT;
			i++;
        }
    }

    //if the field was created
    return true;
}


int main(int argc, char* args[])
{
	//quit flag
    bool quit = false;

	//the tile map
	gamefield board;

    //the tiles that will be used
    tile *tiles[TOTAL_TILES];

    //initialize
    if(init() == false)
    {
        return 1;
    }
	
    //load the files
    if(load_files() == false)
    {
        return 1;
    }

    //clip the tile sheet
    clip_tiles();

	//generate board without matches
	board.generate_field();
	while(!board.no_match())
	{
		board.find_match();
		board.del_match();
		board.zero_up();
		board.fill();
	}
	
	//set the tiles
    if(set_tiles(tiles, board) == false)
    {
        return 1;
    }

    //while the user hasn't quit
    while(quit == false)
    {
        //while there's events to handle
        while(SDL_PollEvent(&event))
        {
			//user input
			board.handle_events();
            
            //if the user has Xed out the window
            if(event.type == SDL_QUIT)
            {
                //quit the program
                quit = true;
            }
        }
		
		//game loop
		while(!board.no_match())
		{
			board.find_match();
			board.del_match();
			board.zero_up();
			board.fill();
		}

        //show the tiles
		set_tiles(tiles, board);
        for(int t=0; t < TOTAL_TILES; t++)
        {
            tiles[t]->show();
        }

        //update the screen
        if(SDL_Flip(screen) == -1)
        {
            return 1;
        }
    }

    //clean up
    clean_up(tiles);

	return 0;
}