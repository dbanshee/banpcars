#ifndef __VIDEO_H__
#define __VIDEO_H__

typedef struct {
  uint8_t r,g,b;
} pixel_t;

typedef struct {
  int w;
  int h;
  pixel_t* image;
} Video;

Video* video_create();
int video_init(Video* bctx);
int video_snapshot(Video* bctx);
int video_destroy(Video* bctx);

#endif

