#ifndef FENSTER_MAC_H
#define FENSTER_MAC_H

#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>

#define msg(r, o, s) ((r(*)(id, SEL))objc_msgSend)(o, sel_getUid(s))
#define msg1(r, o, s, A, a) ((r(*)(id, SEL, A))objc_msgSend)(o, sel_getUid(s), a)
#define msg2(r, o, s, A, a, B, b) ((r(*)(id, SEL, A, B))objc_msgSend)(o, sel_getUid(s), a, b)
#define msg3(r, o, s, A, a, B, b, C, c) ((r(*)(id, SEL, A, B, C))objc_msgSend)(o, sel_getUid(s), a, b, c)
#define msg4(r, o, s, A, a, B, b, C, c, D, d) ((r(*)(id, SEL, A, B, C, D))objc_msgSend)(o, sel_getUid(s), a, b, c, d)

#define cls(x) ((id)objc_getClass(x))

extern id const NSDefaultRunLoopMode;
extern id const NSApp;

static void fenster_window_resize(id v, SEL s, id note) {
    (void)s;
    struct fenster *f = (struct fenster *)objc_getAssociatedObject(v, "fenster");
    CGRect frame = msg(CGRect, msg(id, note, "object"), "frame");
    
    uint32_t *new_buf = realloc(f->buf, frame.size.width * frame.size.height * sizeof(uint32_t));
    if (!new_buf) return;
    
    f->buf = new_buf;
    f->width = frame.size.width;
    f->height = frame.size.height;
}

static void fenster_draw_rect(id v, SEL s, CGRect r) {
  (void)r, (void)s;
  struct fenster *f = (struct fenster *)objc_getAssociatedObject(v, "fenster");
  CGContextRef context =
      msg(CGContextRef, msg(id, cls("NSGraphicsContext"), "currentContext"),
          "graphicsPort");
  CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider = CGDataProviderCreateWithData(
      NULL, f->buf, f->width * f->height * 4, NULL);
  CGImageRef img =
      CGImageCreate(f->width, f->height, 8, 32, f->width * 4, space,
                    kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,
                    provider, NULL, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(space);
  CGDataProviderRelease(provider);
  CGContextDrawImage(context, CGRectMake(0, 0, f->width, f->height), img);
  CGImageRelease(img);
}

static BOOL fenster_should_close(id v, SEL s, id w) {
  (void)v, (void)s, (void)w;
  msg1(void, NSApp, "terminate:", id, NSApp);
  return YES;
}

FENSTER_API int fenster_open(struct fenster *f) {
  f->buf = malloc(f->width * f->height * sizeof(uint32_t));
  if (!f->buf) return -1;
  msg(id, cls("NSApplication"), "sharedApplication");
  msg1(void, NSApp, "setActivationPolicy:", NSInteger, 0);
  f->wnd = msg4(id, msg(id, cls("NSWindow"), "alloc"),
                "initWithContentRect:styleMask:backing:defer:", CGRect,
                CGRectMake(0, 0, f->width, f->height), NSUInteger, 3,
                NSUInteger, 2, BOOL, NO);
  Class windelegate =
      objc_allocateClassPair((Class)cls("NSObject"), "FensterDelegate", 0);
  class_addMethod(windelegate, sel_getUid("windowShouldClose:"),
                  (IMP)fenster_should_close, "c@:@");
  objc_registerClassPair(windelegate);
  msg1(void, f->wnd, "setDelegate:", id,
       msg(id, msg(id, (id)windelegate, "alloc"), "init"));
  Class c = objc_allocateClassPair((Class)cls("NSView"), "FensterView", 0);
  class_addMethod(c, sel_getUid("drawRect:"), (IMP)fenster_draw_rect, "i@:@@");
  objc_registerClassPair(c);

  id v = msg(id, msg(id, (id)c, "alloc"), "init");
  msg1(void, f->wnd, "setContentView:", id, v);
  objc_setAssociatedObject(v, "fenster", (id)f, OBJC_ASSOCIATION_ASSIGN);

  id title = msg1(id, cls("NSString"), "stringWithUTF8String:", const char *,
                  f->title);
  msg1(void, f->wnd, "setTitle:", id, title);
  msg1(void, f->wnd, "makeKeyAndOrderFront:", id, nil);
  msg(void, f->wnd, "center");
  msg1(void, NSApp, "activateIgnoringOtherApps:", BOOL, YES);
  class_addMethod(c, sel_getUid("windowDidResize:"), (IMP)fenster_window_resize, "v@:@");
  msg4(void, msg(id, cls("NSNotificationCenter"), "defaultCenter"),
       "addObserver:selector:name:object:", id, v,
       SEL, sel_getUid("windowDidResize:"),
       id, msg1(id, cls("NSString"), "stringWithUTF8String:", const char*, "NSWindowDidResizeNotification"),
       id, f->wnd);
  return 0;
}

FENSTER_API void fenster_close(struct fenster *f) {
    free(f->buf);
    msg(void, f->wnd, "close");
}

// clang-format off
static const uint8_t FENSTER_KEYCODES[128] = {65,83,68,70,72,71,90,88,67,86,0,66,81,87,69,82,89,84,49,50,51,52,54,53,61,57,55,45,56,48,93,79,85,91,73,80,10,76,74,39,75,59,92,44,47,78,77,46,9,32,96,8,0,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,2,3,127,0,5,0,4,0,20,19,18,17,0};
// clang-format on
FENSTER_API int fenster_loop(struct fenster *f) {
  msg1(void, msg(id, f->wnd, "contentView"), "setNeedsDisplay:", BOOL, YES);
  id ev = msg4(id, NSApp,
               "nextEventMatchingMask:untilDate:inMode:dequeue:", NSUInteger,
               NSUIntegerMax, id, NULL, id, NSDefaultRunLoopMode, BOOL, YES);
  memset(f->mclick, 0, sizeof(f->mclick));
  if (!ev)
    return 0;
  NSUInteger evtype = msg(NSUInteger, ev, "type");
  switch (evtype) {
  case 1: /* NSEventTypeLeftMouseDown */
    f->mclick[0] = f->mhold[0] = 1;
    break;
  case 2: /* NSEventTypeLeftMouseUp */
    f->mhold[0] = 0;
    break;
  case 3: /* NSEventTypeRightMouseDown */
    f->mclick[1] = f->mhold[1] = 1;
    break;
  case 4: /* NSEventTypeRightMouseUp */
    f->mhold[1] = 0;
    break;
  case 25: /* NSEventTypeOtherMouseDown */
    f->mclick[2] = f->mhold[2] = 1;
    break;
  case 26: /* NSEventTypeOtherMouseUp */
    f->mhold[2] = 0;
    break;
  case 22: /* NSEventTypeScrollWheel */
    if (msg(CGFloat, ev, "scrollingDeltaY") > 0) {
      f->mclick[3] = 1;
    } else if (msg(CGFloat, ev, "scrollingDeltaY") < 0) {
      f->mclick[4] = 1;
    }
    break;
  case 5:
  case 6: { /* NSEventTypeMouseMoved */
    CGPoint xy = msg(CGPoint, ev, "locationInWindow");
    f->mpos[0] = (int)xy.x;
    f->mpos[1] = (int)(f->height - xy.y);
    return 0;
  }
  case 10: /*NSEventTypeKeyDown*/
  case 11: /*NSEventTypeKeyUp:*/ {
    NSUInteger k = msg(NSUInteger, ev, "keyCode");
    f->keys[k < 127 ? FENSTER_KEYCODES[k] : 0] = evtype == 10;
    NSUInteger mod = msg(NSUInteger, ev, "modifierFlags") >> 17;
    f->modkeys[0] = (mod & 1) ? 1 : 0;      // Shift
    f->modkeys[1] = (mod & 2) ? 1 : 0;      // Control
    f->modkeys[2] = (mod & 4) ? 1 : 0;      // Alt/Option
    f->modkeys[3] = (mod & 8) ? 1 : 0;      // Command
    return 0;
  }
  }
  msg1(void, NSApp, "sendEvent:", id, ev);
  return 0;
}

FENSTER_API void fenster_sleep(int64_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

FENSTER_API int64_t fenster_time(void) {
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  return time.tv_sec * 1000 + (time.tv_nsec / 1000000);
}

FENSTER_API void fenster_sync(struct fenster *f, int fps) {
  int64_t frame_time = 1000 / fps;
  int64_t elapsed = fenster_time() - f->lastsync;
  if (elapsed < frame_time) {
    fenster_sleep(frame_time - elapsed);
  }

  f->lastsync = fenster_time();
}

FENSTER_API void fenster_resize(struct fenster *f, int width, int height) {
    id contentView = msg(id, f->wnd, "contentView");
    struct objc_super super = { contentView, cls("NSView") };
    CGRect newFrame = CGRectMake(0, 0, width, height);
    
    msg2(void, f->wnd, "setFrame:display:", CGRect, newFrame, BOOL, YES);
    msg(void, f->wnd, "center");
}
#endif /* FENSTER_MAC_H */
