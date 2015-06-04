#include <stdio.h>

#include "bcm_host.h"
#include "video.h"
#include "log.h"

typedef struct {
  Video base;
  DISPMANX_DISPLAY_HANDLE_T   display;
  DISPMANX_MODEINFO_T         info;
  DISPMANX_RESOURCE_HANDLE_T  resource;
  VC_RECT_T                   rect;
  VC_IMAGE_TYPE_T             type;
  VC_IMAGE_TRANSFORM_T        transform;
  uint32_t                    vc_image_ptr;
  int                         screen;
} VideoRPI;

Video* video_create() {
  LOG_TRACE("malloc");
  VideoRPI *ctx = (VideoRPI*)malloc(sizeof(VideoRPI));
  if(ctx == NULL) return NULL;
  memset(ctx,0,sizeof(VideoRPI));

  ctx->type      = VC_IMAGE_RGB888;
  ctx->transform = 0;
  ctx->screen    = 0;

  return (Video*)ctx;
}

int video_init(Video* bctx) {
  int ret;

  VideoRPI *ctx = (VideoRPI*)bctx;

  LOG_TRACE("bcm_host_init");
  bcm_host_init();

  LOG_TRACE("vc_dispmanx_display_open");
  ctx->display = vc_dispmanx_display_open(ctx->screen);
  LOG_DEBUG("display=%d",ctx->display);
  LOG_DEBUG("vc_dispmanx_display_get_info");
  ret = vc_dispmanx_display_get_info(ctx->display, &(ctx->info));
  if(ret != 0)
    return -1;

  bctx->w = ctx->info.width;
  bctx->h = ctx->info.height;

  LOG_TRACE("vc_dispmanx_resource_create");
  ctx->resource = vc_dispmanx_resource_create(ctx->type, bctx->w, bctx->h, &(ctx->vc_image_ptr));
  LOG_DEBUG("resource=%d",ctx->resource);

  LOG_TRACE("realloc");
  bctx->image = (pixel_t*)realloc(bctx->image, bctx->w * bctx->h * sizeof(pixel_t));
  if(bctx->image == NULL)
    return -1;

  return 0;
}

int video_snapshot(Video* bctx) {
  VideoRPI *ctx = (VideoRPI*)bctx;

  LOG_TRACE("vc_dispmanx_snapshot");
  LOG_DEBUG("display=%d",ctx->display);
  LOG_DEBUG("resource=%d",ctx->resource);
  int ret = vc_dispmanx_snapshot(ctx->display, ctx->resource, ctx->transform);
  LOG_DEBUG("vc_dispmanx_snapshot = %d",ret);
  LOG_TRACE("vc_dispmanx_rect_set");
  vc_dispmanx_rect_set(&(ctx->rect), 0, 0, bctx->w, bctx->h);
  LOG_TRACE("vc_dispmanx_resource_read_data");
  vc_dispmanx_resource_read_data(ctx->resource, &(ctx->rect), bctx->image, bctx->w*3);

  return 0;
}

int video_destroy(Video* bctx) {
  VideoRPI *ctx = (VideoRPI*)bctx;
  int ret;

  if(bctx == NULL) return 0;

  if(bctx->image != NULL) {
    free(bctx->image);
    bctx->image = NULL;
  }

  LOG_TRACE("vc_dispmanx_resource_delete");
  ret  = vc_dispmanx_resource_delete(ctx->resource);
  if(ret!=0) return 1;
  LOG_TRACE("vc_dispmanx_display_close");
  ret = vc_dispmanx_display_close(ctx->display);
  if(ret==0) return 1;

  return 0;
}
