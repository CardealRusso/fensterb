# FensterB

[Fenster](https://github.com/zserge/fenster) with some bloated code

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.

### Main changes
- Source split by system
- Platform-agnostic structure for audio
- Mouse: Left, Right, Middle, Scrollup/down
- Added: fenster_sync
- Changed: array on modkeys, mouse pos

```C
struct fenster {
  const char *title;
  const int width;
  const int height;
  uint32_t *buf;
  int keys[256];      // keys are mostly ASCII, but arrows are 17..20
  int modkeys[4];     // ctrl, shift, alt, meta
  int mpos[2];        // mouse x, y
  int mclick[5];      // left, right, middle, scroll up, scroll down (refreshed after loop)
  int mhold[3];       // left, right, middle (persistent until button release)
  int64_t lastsync;   // last sync time
};

FENSTER_API void fenster_sync(struct fenster *f, int fps);
```
