// SDL2 sample showing how to resize and toggle fullscreen.
//
// You can resize and drag the window across screens and
// <ENTER> will toggle fullscreen on its current display.
//
// Handy for multi-monitor setups like passive 3d secondaries or oculus.
//
// marius.schilder@gmail.com

#include <stdio.h>

#include "SDL.h"
#include "SDL_main.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

#define MAXDISPLAYS 6

class GFX {
 public:
  GFX() : display_(-1),
          width_(0), height_(0),
          last_x_(SDL_WINDOWPOS_CENTERED),
          last_y_(SDL_WINDOWPOS_CENTERED),
          last_width_(0), last_height_(0),
          fullscreen_(false),
          window_(NULL), renderer_(NULL) {
    SDL_Init(SDL_INIT_VIDEO);
    for (int i = 0; i < SDL_GetNumVideoDisplays() &&
                    i < MAXDISPLAYS; ++i) {
      SDL_GetCurrentDisplayMode(i, &mode_[i]);
      SDL_GetDisplayBounds(i, &rect_[i]);
      printf ("GFX::GFX"": rect[%d] = (%d, %d, %d,%d)\n", i,
	       rect_[i].x, rect_[i].y, rect_[i].w, rect_[i].h);
    }
  }

  ~GFX() {
    reset();
    SDL_Quit();
  }

  void reset() {
    SDL_DestroyRenderer(renderer_);
    renderer_ = NULL;
    SDL_DestroyWindow(window_);
    window_ = NULL;
    display_ = -1;
  }

  void resize(int w, int h) {
    int d = display_;

    // ignore resize events when fullscreen.
    if (fullscreen_) return;

    // ignore resize events for fullscreen width.
    if (d != -1 && w == rect_[d].w) return;

    if (window_) {
      // capture current display.
      d = SDL_GetWindowDisplayIndex(window_);
      // capture current window position.
      SDL_GetWindowPosition(window_, &last_x_, &last_y_);
    }

    reset();

//  printf(__FUNCTION__ ": %dx%d display %d\n", w, h, d);
    printf("resize" ": %dx%d display %d\n", w, h, d);
    window_ = SDL_CreateWindow("test",
       last_x_, last_y_,
       w, h,
       SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, d, 0);
    display_ = d;
    last_width_ = width_ = w;
    last_height_ = height_ = h;
  }

  void toggleFullscreen() {
    if (!window_) return;

    // capture current display.
    int d = SDL_GetWindowDisplayIndex(window_);

    if (!fullscreen_) {
      // capture current window position.
      SDL_GetWindowPosition(window_, &last_x_, &last_y_);
    }
    reset();

    if (!fullscreen_) {
//    printf(__FUNCTION__ ": to fullscreen %dx%d display %d\n",
      printf("toggleFullscreen" ": to fullscreen %dx%d display %d at (%d,%d)\n",
	     rect_[d].w*2, rect_[d].h, d, rect_[d].x, rect_[d].y);
      window_ = SDL_CreateWindow("test",
          rect_[d].x, rect_[d].y,
          rect_[d].w*2, rect_[d].h,
	  SDL_WINDOW_FULLSCREEN);
      width_ = rect_[d].w;
      height_ = rect_[d].h;
      SDL_SetWindowDisplayMode (window_, NULL);
    } else {
//    printf(__FUNCTION__ ": from fullscreen %dx%d display %d\n",
      printf("toggleFullscreen" ": from fullscreen %dx%d display %d at (%d,%d)\n",
	     last_width_, last_height_, d, last_x_, last_y_);
      window_ = SDL_CreateWindow("test",
          last_x_, last_y_,
          last_width_, last_height_,
          SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
      width_ = last_width_;
      height_ = last_height_;
      SDL_SetWindowDisplayMode (window_, &mode_[d]);
    }

    renderer_ = SDL_CreateRenderer(window_, d, 0);
    display_ = d;

    fullscreen_ = !fullscreen_;
  }

  SDL_Renderer* renderer() { return renderer_; }
  int width() const { return width_; }
  int height() const { return height_; }

 private:
  int display_;
  int width_, height_;  // current dimensions, window or fullscreen.
  int last_x_,last_y_;  // last known position of window
  int last_width_, last_height_;  // last known dimension of window.
  bool fullscreen_;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_DisplayMode mode_[MAXDISPLAYS];
  SDL_Rect rect_[MAXDISPLAYS];
};

int main(int argc, char* argv[]) {
  GFX gfx;

  gfx.resize(1280, 720);

  int frame = 0;
  bool done = false;

  while(!done) {
    ++frame;

    // Draw some flickering background.
    SDL_SetRenderDrawColor(gfx.renderer(),
                    200*(frame&1), 100*(frame&2), 60*(frame*4), 255);

    // Clear the entire screen to our selected color.
    SDL_RenderClear(gfx.renderer());

    // Show updated render.
    SDL_RenderPresent(gfx.renderer());

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_RETURN: {
              gfx.toggleFullscreen();
            } break;
            case SDLK_ESCAPE: {
              done = true;
            } break;
          }
        } break;
        case SDL_WINDOWEVENT: {
          if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            gfx.resize(event.window.data1, event.window.data2);
          }
        } break;
      }
    }
  }

  return 0;
}
