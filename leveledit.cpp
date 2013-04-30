//The headers
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string>
#include <fstream>
#include <iostream>

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

//The frame rate
const int FRAMES_PER_SECOND = 20;

//The dimensions of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 1280;

//Tile constants
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;
const int TOTAL_TILES = 256;
const int TILE_SPRITES = 12;

//Tile floor and wall sprites
const int TILE_FLOOR = 0;
const int TILE_RED = 1;
const int TILE_WALL = 2;
const int TILE_CENTER = 101;
const int TILE_RAMP_TOP = 4;
const int TILE_TOPRIGHT = 5;
const int TILE_RAMP_RIGHT = 6;
const int TILE_BOTTOMRIGHT = 7;
const int TILE_RAMP_BOTTOM = 8;
const int TILE_BOTTOMLEFT = 9;
const int TILE_RAMP_LEFT = 10;
const int TILE_TOPLEFT = 11;
const int TILE_AIR = 3;


//The surfaces
SDL_Surface *screen = NULL;
SDL_Surface *tileSheet = NULL;

//Sprite from the tile sheet
SDL_Rect clips[ TILE_SPRITES ];

//The event structure
SDL_Event event;

//The camera
SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

//The tile
class Tile
{
private:
    //The attributes of the tile
    SDL_Rect box;

    //The tile type
    int type;

public:
    //Initializes the variables
    Tile( int x, int y, int tileType );

    //Shows the tile
    void show();

    //Get the tile type
    int get_type();

    //Get the collision box
    SDL_Rect &get_box();
};

//The timer
class Timer
{
private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    int get_ticks();

    //Checks the status of the timer
    bool is_started();
    bool is_paused();
};

SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool check_collision( SDL_Rect &A, SDL_Rect &B )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = A.x;
    rightA = A.x + A.w;
    topA = A.y;
    bottomA = A.y + A.h;

    //Calculate the sides of rect B
    leftB = B.x;
    rightB = B.x + B.w;
    topB = B.y;
    bottomB = B.y + B.h;

    //If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Level Editor. Current Tile: Floor", NULL );

    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the tile sheet
    tileSheet = load_image( "tiles.png" );

    //If there was a problem in loading the tiles
    if( tileSheet == NULL )
    {
        return false;
    }
    //If everything loaded fine
    return true;
}

void clean_up( Tile *tiles[] )
{
    //Free the surface
    SDL_FreeSurface( tileSheet );

    //Free the tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {
        delete tiles[ t ];
    }
}

void show_type( int tileType )
{
    switch( tileType )
    {
    case TILE_FLOOR:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Floor", NULL );
        break;

    case TILE_RED:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Red", NULL );
        break;

    case TILE_WALL:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Wall", NULL );
        break;

    case TILE_RAMP_TOP:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Top-facing ramp", NULL );
        break;

    case TILE_RAMP_RIGHT:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Right-facing ramp", NULL );
        break;

    case TILE_RAMP_BOTTOM:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Bottom-facing ramp", NULL );
        break;

    case TILE_RAMP_LEFT:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Left-facing ramp", NULL );
        break;

    case TILE_AIR:
        SDL_WM_SetCaption( "Level Editor. Current Tile: Empty", NULL );
        break;

    default:
        break;

    };
}

void set_camera()
{
    //Mouse offsets
    int x = 0, y = 0;

    //Get mouse offsets
    SDL_GetMouseState( &x, &y );

    //Move camera to the left if needed
    if( x < TILE_WIDTH )
    {
        camera.x -= 20;
    }

    //Move camera to the right if needed
    if( x > SCREEN_WIDTH - TILE_WIDTH )
    {
        camera.x += 20;
    }

    //Move camera up if needed
    if( y < TILE_WIDTH )
    {
        camera.y -= 20;
    }

    //Move camera down if needed
    if( y > SCREEN_HEIGHT - TILE_WIDTH )
    {
        camera.y += 20;
    }

    //Keep the camera in bounds.
    if( camera.x < 0 )
    {
        camera.x = 0;
    }
    if( camera.y < 0 )
    {
        camera.y = 0;
    }
    if( camera.x > LEVEL_WIDTH - camera.w )
    {
        camera.x = LEVEL_WIDTH - camera.w;
    }
    if( camera.y > LEVEL_HEIGHT - camera.h )
    {
        camera.y = LEVEL_HEIGHT - camera.h;
    }
}

void put_tile( Tile *tiles[], int tileType )
{
    //Mouse offsets
    int x = 0, y = 0;

    //Get mouse offsets
    SDL_GetMouseState( &x, &y );

    //Adjust to camera
    x += camera.x;
    y += camera.y;

    //Go through tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {
        //Get tile's collision box
        SDL_Rect box = tiles[ t ]->get_box();

        //If the mouse is inside the tile
        if( ( x > box.x ) && ( x < box.x + box.w ) && ( y > box.y ) && ( y < box.y + box.h ) )
        {
            //Get rid of old tile
            delete tiles[ t ];

            //Replace it with new one
            tiles[ t ] = new Tile( box.x, box.y, tileType );
        }
    }
}

void clip_tiles()
{
    //Clip the sprite sheet
    clips[ TILE_FLOOR ].x = 0;
    clips[ TILE_FLOOR ].y = 0;
    clips[ TILE_FLOOR ].w = TILE_WIDTH;
    clips[ TILE_FLOOR ].h = TILE_HEIGHT;

    clips[ TILE_RED ].x = 0;
    clips[ TILE_RED ].y = 80;
    clips[ TILE_RED ].w = TILE_WIDTH;
    clips[ TILE_RED ].h = TILE_HEIGHT;

    clips[ TILE_WALL ].x = 0;
    clips[ TILE_WALL ].y = 160;
    clips[ TILE_WALL ].w = TILE_WIDTH;
    clips[ TILE_WALL ].h = TILE_HEIGHT;



    clips[ TILE_RAMP_LEFT ].x = 80;
    clips[ TILE_RAMP_LEFT ].y = 80;
    clips[ TILE_RAMP_LEFT ].w = TILE_WIDTH;
    clips[ TILE_RAMP_LEFT ].h = TILE_HEIGHT;

    clips[ TILE_AIR ].x = 80;
    clips[ TILE_AIR ].y = 0;
    clips[ TILE_AIR ].w = TILE_WIDTH;
    clips[ TILE_AIR ].h = TILE_HEIGHT;


    clips[ TILE_RAMP_TOP ].x = 160;
    clips[ TILE_RAMP_TOP ].y = 0;
    clips[ TILE_RAMP_TOP ].w = TILE_WIDTH;
    clips[ TILE_RAMP_TOP ].h = TILE_HEIGHT;



    clips[ TILE_RAMP_BOTTOM ].x = 160;
    clips[ TILE_RAMP_BOTTOM ].y = 160;
    clips[ TILE_RAMP_BOTTOM ].w = TILE_WIDTH;
    clips[ TILE_RAMP_BOTTOM ].h = TILE_HEIGHT;


    clips[ TILE_RAMP_RIGHT ].x = 240;
    clips[ TILE_RAMP_RIGHT ].y = 80;
    clips[ TILE_RAMP_RIGHT ].w = TILE_WIDTH;
    clips[ TILE_RAMP_RIGHT ].h = TILE_HEIGHT;


}

bool set_tiles( Tile *tiles[])
{
    //The tile offsets
    int x = 0, y = 0;


    //Initialize the tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {
        //Put a floor tile
        tiles[ t ] = new Tile( x, y, TILE_AIR);

        //Move to next tile spot
        x += TILE_WIDTH;

        //If we've gone too far
        if( x >= LEVEL_WIDTH )
        {
            //Move back
            x = 0;

            //Move to the next row
            y += TILE_HEIGHT;
        }
    }



    return true;
}

void save_tiles( Tile *tiles[], std::string filename )
{
    //Open the map
    std::ofstream lvl(filename.c_str());

    //Go through the tiles
    for( int t = 0; t < TOTAL_TILES; t++ )
    {
        //Write tile type to file
        lvl << tiles[ t ]->get_type() << " ";
    }

    std::cout << "Level saved at "<<filename;
    //Close the file
    lvl.close();
}

Tile::Tile( int x, int y, int tileType )
{
    //Get the offsets
    box.x = x;
    box.y = y;

    //Set the collision box
    box.w = TILE_WIDTH;
    box.h = TILE_HEIGHT;

    //Get the tile type
    type = tileType;
}

void Tile::show()
{
    //If the tile is on screen
    if( check_collision( camera, box ) == true )
    {
        //Show the tile
        apply_surface( box.x - camera.x, box.y - camera.y, tileSheet, screen, &clips[ type ] );
    }
}

int Tile::get_type()
{
    return type;
}

SDL_Rect &Tile::get_box()
{
    return box;
}

Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}

int main( int argc, char* args[] )
{
    //Quit flag
    bool quit = false;

    //Current tile type
    int currentType = TILE_FLOOR;

    //The tiles that will be used
    Tile *tiles[ TOTAL_TILES ];

    //The frame rate regulator
    Timer fps;

    //Initialize
    if( init() == false )
    {
        return 1;
    }

    //Load the files
    if( load_files() == false )
    {
        return 1;
    }

    //Clip the tile sheet
    clip_tiles();

    //std::string openfilename;

    // std::cout << "Enter filename to open:";
    // std::cin >> openfilename;

    //Set the tiles
    if( set_tiles( tiles) == false )
    {
        return 1;
    }

    //While the user hasn't quit
    while( quit == false )
    {
        //Start the frame timer
        fps.start();

        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //When the user presses a key
            if( event.type == SDL_KEYDOWN )
            {
                //Changes tile type based on number key pressed
                switch(event.key.keysym.sym)
                {
                case SDLK_1:
                    currentType = TILE_FLOOR;
                    break;

                case SDLK_2:
                    currentType = TILE_RED;
                    break;

                case SDLK_3:
                    currentType = TILE_WALL;
                    break;

                case SDLK_4:
                    currentType = TILE_RAMP_BOTTOM;
                    break;

                case SDLK_5:
                    currentType = TILE_RAMP_RIGHT;
                    break;

                case SDLK_6:
                    currentType = TILE_RAMP_TOP;
                    break;

                case SDLK_7:
                    currentType = TILE_RAMP_LEFT;
                    break;

                case SDLK_0:
                    currentType = TILE_AIR;
                    break;

                default:
                    break;
                }
                show_type( currentType );
            }
            //When the user clicks
            if( event.type == SDL_MOUSEBUTTONDOWN )
            {
                //On left mouse click
                if( event.button.button == SDL_BUTTON_LEFT )
                {
                    //Put the tile
                    put_tile( tiles, currentType );
                }
            }

            //If the user has closed the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }

        //Set the camera
        set_camera();

        //Show the tiles
        for( int t = 0; t < TOTAL_TILES; t++ )
        {
            tiles[ t ]->show();
        }

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }

        //Cap the frame rate
        if( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
        {
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
        }
    }

    SDL_Quit();

    std::string savefilename;

    std::cout << "Enter filename to save as:";
    std::cin >> savefilename;

    //Save the tile map
    save_tiles( tiles, savefilename);

    //Clean up
    clean_up( tiles );

    return 0;
}
