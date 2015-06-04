#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "serial.h"
#include "video.h"
#include "log.h"

typedef struct {
  pixel_t color;
  uint16_t xini,xfin,yini,yfin;
} led_t;

typedef struct {
  int h,w;
  int ml,mr,mt,mb;
  int l,r,t,b,leds;
  int depth;
} dim_t;

typedef struct {
  led_t* prod_buf;
  led_t* cons_buf;

  sem_t full;
  sem_t empty;

  dim_t d;
  uint8_t exit;

  Video* vCtx;
  int serial;
} Context;

// Calculates the area for each led
void calcRanges(led_t* led_v, dim_t* d) {
      int i,curr;
      led_t* tmp = led_v;
      int h = d->h - d->mb - d->mt;
      int w = d->w - d->ml - d->mr;
      // Left
      curr = h + d->mt;
      for(i=0; i < d->l; i++) {
        tmp[i].xini=d->ml;
        tmp[i].xfin=d->ml + d->depth;
        tmp[i].yfin=curr;
        curr -= h/d->l;
        if(i<h%d->l) curr--;
        tmp[i].yini=curr;
      }
      tmp += d->l;
      // Top
      curr = d->ml;
      for(i=0; i < d->t; i++) {
        tmp[i].yini=d->mt;
        tmp[i].yfin=d->mt + d->depth;
        tmp[i].xini=curr;
        curr += w/d->t;
        if(i<w%d->t) curr++;
        tmp[i].xfin=curr;
      }
      tmp += d->t;
      // Right
      curr = d->mt;
      for(i=0; i< d->r; i++) {
        tmp[i].xini=d->ml + w - d->depth;
        tmp[i].xfin=d->ml + w;
        tmp[i].yini=curr;
        curr += h/d->r;
        if(i<h%d->r) curr++;
        tmp[i].yfin=curr;
      }
      tmp += d->r;
      // Bottom
      curr = w + d->ml;
      for(i=0; i<d->t; i++) {
        tmp[i].yini=d->mt + h - d->depth;
        tmp[i].yfin=d->mt + h;
        tmp[i].xfin=curr;
        curr -= w/d->b;
        if(i<w%d->b) curr--;
        tmp[i].xini=curr;
      }
    }

// Prints the current led information (Area and color)
void showLeds(led_t* led_v, int leds) {
      int i;
      for(i=0;i<leds;i++) {
        LOG_INFO("led[%3d] x(%4d,%4d) y(%4d,%4d) c(0x%02X%02X%02X)",
               i,
               led_v[i].xini,led_v[i].xfin,
               led_v[i].yini,led_v[i].yfin,
               led_v[i].color.r,led_v[i].color.g,led_v[i].color.b);
      }
    }

// Scan an image and find the black margins
void adjustImage(pixel_t* image, dim_t* d) {
  int x,y;
  int minl,maxr,mint,maxb;
  int l,r;
  uint8_t brow,bcol;

  l = 0;
  r = 0;
  minl = d->w;
  mint = d->h;
  maxr = 0;
  maxb = 0;
  brow = 1;
  bcol = 1;

  bcol = 1;
  for(y=0;y<d->h;y++) {
    brow = 1;
    for(x=0;x<d->w;x++) {
      if(image[y*d->w + x].r == 0x00 &&
         image[y*d->w + x].g == 0x00 &&
         image[y*d->w + x].b == 0x00) {
        if(brow) l=x;
      } else {
        brow = 0;
        r=x;
      }
    }
    if(!brow) bcol = 0;
    if(l < minl) minl = l;
    if(r > maxr) maxr = r;
    if(brow && bcol) mint = y;
    if(!brow && !bcol) maxb = y;
  }

  d->ml = minl;
  d->mt = mint;
  d->mr = d->w - maxr;
  d->mb = d->h - maxb;
}

int readIntValue(int fd, char* buf, int buflen, int* res) {
  serial_readline(fd, buf, buflen);
  if(strcmp("ERROR",buf)==0) {
    return 1;
  }
  *res = atoi(buf);
  serial_readline(fd, buf, buflen);
  if(strcmp("OK",buf)!=0) {
    return 1;
  }

  return 0;
}

// Get the dimensions from the leduino
int readDimensions(int fd, dim_t* d) {
  char buf[256];
  int res;

  write(fd,"GETL\r",5);
  res = readIntValue(fd, buf, 256, &(d->l));
  if(res!=0) return 1;

  write(fd,"GETT\r",5);
  res = readIntValue(fd, buf, 256, &(d->t));
  if(res!=0) return 1;

  write(fd,"GETR\r",5);
  res = readIntValue(fd, buf, 256, &(d->r));
  if(res!=0) return 1;

  write(fd,"GETB\r",5);
  res = readIntValue(fd, buf, 256, &(d->b));
  if(res!=0) return 1;

  return 0;
}

// Sends all the pixels colors to the leduino
int sendData(int fd, led_t* leds, int n) {
  char buf[256];

  write(fd,"DATA\r",5);
  int i;
  for(i=0;i<n;i++) {
    write(fd,&(leds[i].color),3);
  }

  serial_readline(fd, buf, 256);
  if(strcmp("OK",buf)!=0) {
    return 1;
  }

  return 0;
}

void *capture(void * arg) {
    logger_set_thread_name("Capture");
    LOG_TRACE("Capture starts");
    Context * ctx = (Context *) arg;
    struct timeval t;

    uint32_t mr,mg,mb;
    int i,x,y,cnt,ret;
    led_t *tmp;
    while (1) {
        // Wait to dequeue
        LOG_TRACE("Waiting buffer empty");
        gettimeofday(&t, NULL);
        sem_wait(&ctx->empty);
        LOG_DEBUG_TIME(&t, "Wait buffer empty");

        // Exit condition
        if (ctx->exit) break;

        // Take a snapshot
        LOG_TRACE("Grab frame");
        gettimeofday(&t, NULL);

        // Take Snapshot
        ret = video_snapshot(ctx->vCtx);
        if(ret!=0) {
          LOG_ERROR("Grabbing frame");
          sem_post(&ctx->empty);
          continue;
        }

        // Processing
        for(i=0; i<ctx->d.leds; i++) {
          mr = mg = mb = cnt = 0;
          for(y = ctx->prod_buf[i].yini; y < ctx->prod_buf[i].yfin; y++) {
            for(x = ctx->prod_buf[i].xini; x < ctx->prod_buf[i].xfin; x++) {
              mr += ctx->vCtx->image[y*ctx->d.w + x].r;
              mg += ctx->vCtx->image[y*ctx->d.w + x].g;
              mb += ctx->vCtx->image[y*ctx->d.w + x].b;
              cnt ++;
            }
          }
          ctx->prod_buf[i].color.r = mr / cnt;
          ctx->prod_buf[i].color.g = mg / cnt;
          ctx->prod_buf[i].color.b = mb / cnt;
        }

        // Swap buffers
        tmp = ctx->prod_buf;
        ctx->prod_buf = ctx->cons_buf;
        ctx->cons_buf = tmp;

        // Notify buffer full
        LOG_TRACE("Notify buffer available");
        sem_post(&ctx->full);
    }

    LOG_TRACE("Capture exit");
    pthread_exit(0);
}

void *viewer(void * arg) {
    logger_set_thread_name("Viewer");
    LOG_TRACE("Viewer starts");
    Context * ctx = (Context *) arg;
    struct timeval t;
    struct timeval tf;
    gettimeofday(&tf, NULL);

    int ret;
    while (1) {
        // Wait for buffer
        LOG_TRACE("Waiting data");
        gettimeofday(&t, NULL);
        sem_wait(&ctx->full);
        LOG_DEBUG_TIME(&t, "Wait data");

        // Exit condition
        if (ctx->exit) break;

        // Send data
        gettimeofday(&t, NULL);
        LOG_TRACE("Send data");
        ret = sendData(ctx->serial,ctx->cons_buf,ctx->d.leds);
        if(ret != 0) {
          LOG_ERROR("Sending data");
          sem_post(&ctx->full);
          continue;
        }
        LOG_DEBUG_TIME(&t, "Send data");

        // Notify buffer empty
        LOG_TRACE("Notify buffer empty");
        sem_post(&ctx->empty);

        LOG_INFO_TIME(&tf, "Total Time");
        gettimeofday(&tf, NULL);
    }

    LOG_TRACE("Viewer exit");
    pthread_exit(0);
}


// Main app
int main(int ac, char** av)
{
    if(ac <= 1) {
      printf("USE: %s <dev>\n",av[0]);
      exit(1);
    }

    logger_init(LEVEL_INFO, stdout);
    logger_set_thread_name("Main");

    LOG_INFO("leduino");

    int ret;
    Context ctx;
    memset(&ctx,0,sizeof(Context));

    // Video Init
    LOG_TRACE("video_create");
    Video* vCtx = video_create();
    assert(vCtx!=NULL);
    ctx.vCtx = vCtx;

    LOG_TRACE("video_init");
    ret = video_init(vCtx);
    assert(ret==0);

    // Serial Init
    LOG_TRACE("serial_init");
    int fd = serial_init(av[1],19200);
    ctx.serial = fd;

    // Dimensions:
    //   Video (width, height)
    //   Leds  (left, top, right, bottom)
    ctx.d.w = vCtx->w;
    ctx.d.h = vCtx->h;
    LOG_DEBUG("w=%d,h=%d",ctx.d.w,ctx.d.h);

    ctx.d.depth = 32;
    LOG_TRACE("readDimensions");
    ret = readDimensions(fd,&ctx.d);
    assert(ret==0);
    LOG_DEBUG("l=%d,t=%d,r=%d,b=%d",ctx.d.l,ctx.d.t,ctx.d.r,ctx.d.b);

    ctx.d.leds = ctx.d.l + ctx.d.t + ctx.d.r + ctx.d.b;

    // Led buffers
    ctx.prod_buf = (led_t*)calloc(ctx.d.leds, sizeof(led_t));
    assert(ctx.prod_buf!=NULL);
    ctx.cons_buf = (led_t*)calloc(ctx.d.leds, sizeof(led_t));
    assert(ctx.cons_buf!=NULL);

    memset(ctx.prod_buf,0,sizeof(led_t)*ctx.d.leds);

    // Calculate margins
    LOG_TRACE("video_snapshot");
    ret = video_snapshot(vCtx);
    assert(ret==0);

    LOG_TRACE("adjustImage");
    adjustImage(vCtx->image,&ctx.d);
    LOG_DEBUG("ml=%4d,mt=%4d,mr=%4d,mb=%4d",ctx.d.ml,ctx.d.mt,ctx.d.mr,ctx.d.mb);
    LOG_DEBUG("x(%4d,%4d) y(%4d,%4d)",ctx.d.ml,ctx.d.w-ctx.d.mr,ctx.d.mt,ctx.d.h-ctx.d.mr);
    LOG_TRACE("calcRanges");
    calcRanges(ctx.prod_buf,&ctx.d);
    LOG_TRACE("showLeds");
    showLeds(ctx.prod_buf,ctx.d.leds);

    // Copy the margin information from ctx.prod_buf to ctx.cons_buf
    memcpy(ctx.cons_buf,ctx.prod_buf,sizeof(led_t)*ctx.d.leds);

    // Semaphores to sync threads
    LOG_TRACE("Initialize semaphores");
    sem_init(&ctx.full,  0, 0);
    sem_init(&ctx.empty, 0, 1);

    // Launch Threads
    LOG_TRACE("Launch threads");
    pthread_t capture_t, viewer_t;
    pthread_create(&capture_t, NULL, &capture, &ctx);
    pthread_create(&viewer_t,  NULL, &viewer,  &ctx);

    // Wait Threads
    LOG_TRACE("Wait threads");
    pthread_join(capture_t, NULL);
    pthread_join(viewer_t,  NULL);

    // Free resources
    LOG_TRACE("Cleanup");

    LOG_TRACE("Close serial");
    close(fd);

    LOG_TRACE("Free semaphores");
    sem_destroy(&ctx.full);
    sem_destroy(&ctx.empty);

    LOG_TRACE("Free buffers");
    free(ctx.prod_buf);
    free(ctx.cons_buf);

    LOG_TRACE("Free video");
    video_destroy(vCtx);

    LOG_TRACE("Close logger");
    logger_destroy();

    return 0;
}
