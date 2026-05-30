// 3DS application object stub — mirrors the interface from lnxapp.h
#pragma once

#define APPFLAG_USESERVICE      0x00000100
#define APPFLAG_USESVGA         0x00000200
#define APPFLAG_NOMOUSECAPTURE  0x00000400
#define APPFLAG_NOSHAREDMEMORY  0x00000800
#define APPFLAG_WINDOWEDMODE    0x00001000
#define APPFLAG_DGAMOUSE        0x00002000

struct tLnxAppInfo {
  unsigned flags;
  int wnd_x, wnd_y, wnd_w, wnd_h;
};

// On 3DS "Linux" app == our CTR app — same class name so all the code that
// constructs oeD3LnxApp / oeLnxApplication still compiles.
class oeLnxApplication : public oeApplication {
public:
  unsigned m_Flags;
  int m_X, m_Y, m_W, m_H;

  oeLnxApplication(unsigned flags) : m_Flags(flags), m_X(0), m_Y(0), m_W(400), m_H(240) {}
  oeLnxApplication(tLnxAppInfo *info) : m_Flags(info->flags),
      m_X(info->wnd_x), m_Y(info->wnd_y), m_W(info->wnd_w), m_H(info->wnd_h) {}
  virtual ~oeLnxApplication() = default;

  virtual void init() {}
  virtual void get_info(void *) {}
  virtual unsigned defer() { return 0; }
  virtual const char *get_window_name() { return "Descent3"; }
  virtual void clear_window() {}
  virtual void set_defer_handler(void (*)(bool)) {}
  virtual void delay(float) {}
  virtual int flags() const { return (int)m_Flags; }

  void set_sizepos(int x, int y, int w, int h) { m_X=x; m_Y=y; m_W=w; m_H=h; }
};
