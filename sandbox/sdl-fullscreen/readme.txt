Fullscreen in SDL2.
-------------------

SDL2 does not handle multpile monitors in Fullscreen mode as we want: the Ubuntu GUI always
remains visible (laucher icons on the left, desktop  bar at top).

When using 1 display, SDL2 offers a flag setting for SDL_CreateWindow (SDL_WINDOW_FULLSCREEN_DESKTOP)
that does the right thing, but only in 1 monitor.

For AmbulantPlayer_sdl with multiple monitors, the following workaround is approriatem as on Jan. 20, 2014.

1. Make sure that the following optional package is installed: compizconfig-settings-manager.
2. In Settings->Displays, arrange you monitors. Click 'Apply.' 
3. Adapt the <root-layout/> of your .smil document to cover all monitors.
4. Start Compiz Config Settings Manager (ccsm in Terminal), click Desktop->Ubuntu Unity Plugin and
   unselect Enable Ubuntu Unity Plugin. After some seconds, the Ubuntu GUI disappears, which makes
   the computer almost useless. So leave Compiz Config Settings Manager open so that you can select
   Enable Ubuntu Unity Plugin, if needed,
4. In an remote session (ssh) start AmbulantPlayer as follows from a commandline::
   DISPLAY=:0 SDL_WINDOW_FLAGS=19 AmbulantPlayer_sdl <file>.smil

   
