// From: http://dranger.com/ffmpeg/. Adapted fro SDL2. Kees Blom.
//
// tutorial02-SDL2.c
// A pedagogical video player that will stream through every video frame as fast as it can.
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard, 
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
// Use
//
// gcc -o tutorial02-SDL tutorial02-SDL2.c -lavutil -lavformat -lavcodec -lz -lm `sdl2-config --cflags --libs`
// to build (assuming libavformat and libavcodec are correctly installed, 
// and assuming you have sdl-config. Please refer to SDL docs for your installation.)
//
// Run using
// tutorial02-SDL myvideofile.mpg
//
// to play the video stream on your screen.


#include <avcodec.h>
#include <avformat.h>
#include <swscale.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>

int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame; 
  AVPacket        packet;
  int             frameFinished;
  float           aspect_ratio;

  SDL_Texture*    texture = NULL; // A structure that contains an efficient, driver-specific representation of pixel data
  SDL_Renderer*   renderer = NULL; // A 2D rendering context for a window
  struct SwsContext* context = NULL;
  SDL_Window*     window = NULL; // A window on the screen
  SDL_Rect        rect;
  SDL_Event       event;

  int w, h;

  if(argc < 2) {
    fprintf(stderr, "Usage: test <file>\n");
    exit(1);
  }
  // Register all formats and codecs
  av_register_all();
  
  if(SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }
  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
    return -1; // Couldn't open file
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information
  
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  
  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=avcodec_alloc_frame();

  w = pCodecCtx->width;
  h = pCodecCtx->height;

  // Create a window with the specified position, dimensions, and flags. 
  window = SDL_CreateWindow("SDL2 Video Test",0,0,w,h,0/*SDL_WINDOW_BORDERLESS*/);
  if (window == NULL) {
    fprintf(stderr, "No SDLwindow !\n");
    exit(1);
  }
  
  // Create a 2D rendering context for that window.
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "No accelerated SDLrenderer !\n");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
      fprintf(stderr, "No SDLrenderer !\n");
      return -2; // No SDLrenderer
    }
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, w , h);
  if (texture == NULL) {
    fprintf(stderr, "No SDLtexture !\n");
    return -3; // No SDLtexture
  }
  SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_BLEND);

  // Read frames, decode them and render the images on the screen
  i=0;
  struct SwsContext* sws_ctx = NULL;
  sws_ctx = sws_getCachedContext(sws_ctx, w, h,  pCodecCtx->pix_fmt, w, h, PIX_FMT_RGB24,/*PIX_FMT_YUV420P,*/ SWS_BICUBIC, NULL, NULL, NULL );
  int red = 255;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	uint8_t* pixels[AV_NUM_DATA_POINTERS];
	int pitch[AV_NUM_DATA_POINTERS];

	int err = SDL_LockTexture (texture, NULL, (void**) &pixels, &pitch[0]);
	if (err != 0) printf("Error %d\n", err);
	err = sws_scale(sws_ctx, (const uint8_t* const*) pFrame->data, pFrame->linesize, 0, h, pixels, pitch);
	SDL_UnlockTexture (texture);

	// Update the screen with rendering performed.
	SDL_RenderPresent(renderer);

	// Copy a portion of the texture to the current rendering target.
	rect.x = 0;
	rect.y = 0;
	rect.w = w;
	rect.h = h;
	err = SDL_RenderCopy(renderer, texture, NULL, &rect);
	if (err != 0) printf("SDL_RenderCopy returns %d\n", err);
      }
    }
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
    SDL_PollEvent(&event);
    switch(event.type) {
    case SDL_QUIT:
      SDL_Quit();
      exit(0);
      break;
    default:
      break;
    }
  }
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}
