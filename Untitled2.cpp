#include <vector>
#include <string>
#include <ctime>                
#include <cstdlib>              
#include <SDL.h>           
#include <SDL_image.h>
#include <iostream>     

using namespace std;
const int deadiamond_VALUE     = 9;
const int bluediamond_VALUE     = 8;
const int heath                 = 3;
const int score                 = 0;
const int man                   = 1;
const int bluediamond           = 9;
const int deadiamond            = 9;    
const int sprite_total          = 5;
const int SPRITE_CELL_WIDTH     = 16;   
const int SPRITE_CELL_HEIGHT    = 16;   

const int WINDOW_CELL_WIDTH     = 32;  
const int WINDOW_CELL_HEIGHT    = 32;   

const int DEFAULT_NUM_ROWS      = 9;    
const int DEFAULT_NUM_COLS      = 9;    
const int DEFAULT_NUM_deadiamond= 10;
const int DEFAULT_NUM_bluediamond= 9;   

const string SCREEN_TITLE       = "maddigger";   
const string SPRITE_PATH        = "sprite.png"; 

int a;
int b;

enum CellState {        
    CELL_HIDDEN,        
    CELL_SHOWN,  
	CELL_ACT,       
	CELL_DIA1,
	CELL_DIA2        
};

enum GameState {        
    GAME_PLAYING,       
    GAME_WON,           
    GAME_LOST           
};
enum SpriteType { 
   SPRITE_SHOWN,     
   SPRITE_HIDDEN,
   SPRITE_bluediamond,
   SPRITE_deadiamond,
   SPRITE_man,
   SPRITE_TOTAL,
};
struct Cell {           
    int value;          
    CellState state;    
};
struct CellPos {        
    int row;            
    int col;            
};
typedef vector<vector<Cell> > CellTable;
struct Game {  
    int man;    
    int nRows;          
    int nCols;      
	int nbluediamond;    
    int ndeadiamond;       
    int nShownCells;
    CellTable cells;
    CellPos lastPos;    
    GameState state;   
};
struct Graphic {
    SDL_Window *window;                 
    SDL_Texture *spriteTexture;         
    vector<SDL_Rect> spriteRects;       
    SDL_Renderer *renderer;   
	SDL_Event event;         

};
SDL_Texture* createTexture(SDL_Renderer *renderer, const string &path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == NULL) {
        cout <<"Unable to load image " << path << "!";
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
void initSpriteRects(std::vector<SDL_Rect> &rects) {
        for (int i = 0; i < SPRITE_TOTAL ; i ++) {
        SpriteType type = (SpriteType) i;
        SDL_Rect rect = {0, 0, SPRITE_CELL_WIDTH, SPRITE_CELL_HEIGHT};
            int row, col;
            switch (type) {
            	case SPRITE_SHOWN:
            		col = 0;
            		row = 2;
					break;            		
                case SPRITE_man:
                    col = 0;
                    row = 0;
                    break;
                case SPRITE_bluediamond:
				    col = 0;
					row = 1;    
				case SPRITE_deadiamond:
				     col = 1;
					 row = 1;	
                case SPRITE_HIDDEN:
                    col = 2;
                    row = 0;
                    break;
                default:
                    col = 2;
                    row = 0;
                    break;
            }
            rect.x = col * SPRITE_CELL_WIDTH;
            rect.y = row * SPRITE_CELL_HEIGHT;
        }
}


bool initGraphic(Graphic &g, int nRows, int nCols) {
    g.window = NULL;
    g.renderer = NULL;
    g.spriteTexture = NULL;

    int sdlFlags = SDL_INIT_VIDEO;  
    if (SDL_Init(sdlFlags) != 0) {
        cout << "SDL could not initialize!";
        return false;
    }

    int imgFlags = IMG_INIT_PNG;    
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cout << "SDL_Image could not initialize!";
        return false;
    }

    g.window = SDL_CreateWindow(SCREEN_TITLE.c_str(),       
                                SDL_WINDOWPOS_UNDEFINED,    
                                SDL_WINDOWPOS_UNDEFINED,    
                                nCols * WINDOW_CELL_WIDTH,  
                                nRows * WINDOW_CELL_HEIGHT, 
                                SDL_WINDOW_SHOWN);          

    if (g.window == NULL) {
        cout <<"Window could not be created!";
        return false;
    }

    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
    if (g.renderer == NULL) {
        cout << "Renderer could not be created!";
        return false;
    }

    g.spriteTexture = createTexture(g.renderer, SPRITE_PATH);
    if (g.spriteTexture == NULL) {
        cout <<"Unable to create texture from " <<" " <<  SPRITE_PATH << "!";
        return false;
    }

    initSpriteRects(g.spriteRects);
    return true;
}

void finalizeGraphic(Graphic &g) {
    SDL_DestroyTexture(g.spriteTexture);
    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);

    IMG_Quit();
    SDL_Quit();
}
vector<CellPos> randomdeadiamond(int nRows, int nCols, int ndeadiamond) {
    int maxVal = nRows * nCols;

    vector<CellPos> deadiamondPos;
    vector<bool> chosenVals(maxVal, false);

    while (deadiamondPos.size() < ndeadiamond) {
        int val = rand() % maxVal;
        if (!chosenVals[val]) {
            chosenVals[val] = true;
            int row = val / nCols;
            int col = val % nCols;
            deadiamondPos.push_back((CellPos) {row, col});
        }
    }
    return deadiamondPos;
}
vector<CellPos> randombluediamond(int nRows, int nCols, int nbluediamond) {
    int maxVal = nRows * nCols;

    vector<CellPos> bluediamondPos;
    vector<bool> chosenVals(maxVal, false);

    while (bluediamondPos.size() < nbluediamond) {
        int val = rand() % maxVal;
        if (!chosenVals[val]) {
            chosenVals[val] = true;
            int row = val / nCols;
            int col = val % nCols;
            bluediamondPos.push_back((CellPos) {row, col});
        }
    }
    return bluediamondPos;
}
void initGame(Game &game, int nRows, int nCols, int ndeadiamond,int nbluediamond) {
    game.cells = CellTable(nRows, vector<Cell>(nCols));
    for (int i = 0; i < nRows; i ++) {
        for (int j = 0; j < nCols; j ++) {
            game.cells[i][j] = (Cell) {0, CELL_HIDDEN};
        }
    }

    vector<CellPos> deadiamondPos = randomdeadiamond(nRows, nCols, ndeadiamond);
    vector<CellPos> bluediamondPos= randombluediamond(nRows,nCols,nbluediamond);
    for (int i = 0; i < deadiamondPos.size(); i ++) {
        int row = deadiamondPos[i].row;
        int col = deadiamondPos[i].col;
        game.cells[row][col].value = deadiamond_VALUE;
    }

    for (int i = 0; i < bluediamondPos.size(); i ++) {
            if (game.cells[i] [i].value != deadiamond_VALUE) {
                int row = bluediamondPos[i].row;
                int col = bluediamondPos[i].col;
                game.cells[row][col].value = bluediamond_VALUE;            
			}        
    }   

    game.nRows = nRows;
    game.nCols = nCols;
    game.ndeadiamond = ndeadiamond;
    game.nbluediamond= nbluediamond;
    game.man   = man;   
    game.state = GAME_PLAYING;
    game.nShownCells = 0;
}
int showCells(CellTable &cells, int x,int y) {
    Cell &cell = cells[x][y];

    cell.state = CELL_SHOWN;
}
int showmanCells(CellTable &cells, int x,int y) {
	Cell &cell = cells[x][y];

    cell.state = CELL_ACT;
}
void check(Game&game,int a,int b,int &score,int &heath) {
	if(game.cells[a][b].value =bluediamond_VALUE)
	       score += 200;
	if(game.cells[a][b].value =deadiamond_VALUE)
	       heath -=1;       
}
vector<CellPos> randomman(Game&game,CellTable &cells, int nCols,int man, int &a,int &b) {
    int maxVal = 1 * nCols;

    vector<CellPos> manPos;
    vector<bool> chosenVals(maxVal, false);
        int val = rand() % maxVal;
        if (!chosenVals[val]) {
            chosenVals[val] = true;
                b = val % nCols;
                a=1;         
    manPos.push_back((CellPos) {a, b});
    showmanCells(cells,a,b);
}  
    return manPos;
}

SDL_Rect getSpriteRect(const Game &game, const CellPos &pos,
                       const vector<SDL_Rect> &spriteRects) {

    Cell cell = game.cells[pos.row][pos.col];
    if (game.state == GAME_PLAYING) {
        switch (cell.state) {
            case CELL_HIDDEN:
                return spriteRects[SPRITE_HIDDEN];
            case CELL_SHOWN:
                return spriteRects[SPRITE_SHOWN];
            case CELL_ACT:
            	return spriteRects[SPRITE_man];
        }
    } else if (game.state == GAME_LOST) {
            if (cell.state == CELL_HIDDEN) {
                return spriteRects[SPRITE_HIDDEN];
            }
          else if (cell.value == deadiamond_VALUE) {
            int lastRow = game.lastPos.row;
            int lastCol = game.lastPos.col;
            if (pos.row == lastRow && pos.col == lastCol) {
                return spriteRects[SPRITE_deadiamond];
            } else {
                return spriteRects[SPRITE_SHOWN];
            }
        }
    }
}



void displayGame(const Game &game, const Graphic &graphic) {
    SDL_RenderClear(graphic.renderer);      

    for (int i = 0; i < game.nRows; i ++) {
        for (int j = 0; j < game.nCols; j ++){
            SDL_Rect destRect = {          
                j * WINDOW_CELL_WIDTH,
                i * WINDOW_CELL_HEIGHT,
                WINDOW_CELL_WIDTH,
                WINDOW_CELL_HEIGHT
            };

            CellPos pos = {i, j};

            SDL_Rect srcRect = getSpriteRect(game, pos, graphic.spriteRects);

            SDL_RenderCopy(graphic.renderer, graphic.spriteTexture, &srcRect,
                           &destRect);
        }
    }
    SDL_RenderPresent(graphic.renderer);    
}
void moveD(Game& game, Graphic& g, int &a,int &b,int &score,int &heath) {
     b=b+1;
     check(game,a, b,score,heath);
     showmanCells(cells,a,b);
     return manPos;		
}
void moveL(Game& game, Graphic& g, int &a,int &b,int &score,int &heath) {
	a=a-1;
    check(game,a,b,score,heath);
    manPos.push_back((CellPos) {a,b});
    return manPos;
}
void moveR(Game& game, Graphic& g, int &a,int &b,int &score,int &heath) {
	a=a+1;
    check(game,int a,int b,int &score,int &heath);
    manPos.push_back((CellPos) {a,b});
    return manPos;
}

void updateGame(Game &game,Graphic &g, const SDL_Event &event,int &a,int &b, int &heath,int &score) {
    if (game.state != GAME_PLAYING) {
        return;
    }
   for(int i=0;i<=9;i++) {
   	 for (int j=0;j<=9;j++) {
   	  Cell &cell = game.cells[i][j];
    if (cell.state == CELL_SHOWN) {
        return;
    }
    if (cell.state== CELL_ACT ) {
    	return;
	}

	
     switch (g.event.key.keysym.sym)
    {
        case SDLK_DOWN:          moveD(game, g);   
        case SDLK_LEFT:          moveL(game, g);   
        case SDLK_RIGHT:         moveR(game, g);   
        default: return;
    }
    if (heath=0) {
    	game.state = GAME_LOST;
    	return;
	}
	if (score=1200) {
		game.state = GAME_WON;
	}
}

int main() {
    srand(time(0)); 

    int nRows = DEFAULT_NUM_ROWS,
        nCols = DEFAULT_NUM_COLS,
        ndeadiamond = DEFAULT_NUM_deadiamond,
        nbluediamond = DEFAULT_NUM_bluediamond;

    Graphic graphic;
    if (!initGraphic(graphic, nRows, nCols)) {
        finalizeGraphic(graphic);
        return EXIT_FAILURE;
    }

    Game game;
    initGame(game, nRows, nCols, nbluediamond , ndeadiamond , man);

    bool quit = false;
    while (!quit) {
        displayGame(game, graphic);

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }

            updateGame(game, event);
        }
    }

    finalizeGraphic(graphic);
    return EXIT_SUCCESS;
}	


 





    

