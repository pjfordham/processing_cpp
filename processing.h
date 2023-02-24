#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void setup();
void loop();

SDL_Window *window;
SDL_Renderer *renderer;

void background(int gray) {
   // Clear window to Blue to do blue boarder.
   SDL_SetRenderDrawColor(renderer, gray,gray,gray,0xFF);
   SDL_RenderClear(renderer);
}

void background(int r, int g, int b) {
   // Clear window to Blue to do blue boarder.
   SDL_SetRenderDrawColor(renderer, r,g,b,0xFF);
   SDL_RenderClear(renderer);
}

void size(int width, int height) {
   // Create a window
   window = SDL_CreateWindow("Proce++ing",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             width,
                             height,
                             SDL_WINDOW_SHOWN);

   if (window == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   // Create a renderer
   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   if (renderer == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

}

void fill(int r, int g, int b) {
   SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
   
}
void rect(int x, int y, int width, int height) {
   // draw black background for theatre of life
   SDL_Rect fillRect = { x, y, width, height };
   SDL_RenderFillRect(renderer, &fillRect);
}

float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

int main()
{
   // Initialize SDL
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return 1;
   }
  //  TTF_Font* font = NULL;
  //  if (TTF_Init() != 0) {
  //     printf("TTF_Init failed: %s\n", TTF_GetError());
  //     return 1;
  //  }
  // font = TTF_OpenFont("../Instruction.ttf", 40);
  //  if (font == NULL) {
  //     printf("TTF_OpenFont failed: %s\n", TTF_GetError());
  //     return 1;
  //  }


   setup();

   Uint32 clock = SDL_GetTicks();
   Uint32 frameRateClock = clock;
   bool quit = false;
   int frameCount = 0;

   while (!quit) {

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit = true;
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEWHEEL) {
         } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
               quit = true;
               break;
            case SDLK_i:
               break;
            case SDLK_p:
               break;
            case SDLK_c:
               break;
            case SDLK_SPACE:
               break;
            default:
               break;
            }
         }
      }

      frameCount++;
      // Print the frame rate every 10 seconds
      Uint32 currentTicks = SDL_GetTicks();
      if (currentTicks - frameRateClock >= 10000) {
         float frameRate = 1000 * (float) frameCount / (currentTicks - frameRateClock);
         printf("Frame rate: %f fps\n", frameRate);
         frameCount = 0;
         frameRateClock = currentTicks;
      }

      loop();
      
      // Update the screen
      SDL_RenderPresent(renderer);
   }

   // TTF_CloseFont(font);

   // Destroy the window and renderer
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);

   // Quit SDL
   SDL_Quit();
   return 0;
};
