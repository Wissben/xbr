/*
Small tool to change the brightness of the primary display (or a specified output) because I am lazy.

Usage:
  xbr <device> <brightness>

Where:
  - device: the output name (e.g. "eDP-1", "HDMI-1"); get names via `xrandr --query`
  - brightness: a floating-point value greater than 0 and at most 1 (0 < brightness <= 1)

You can install it with sudo ./install.sh and use it with xbr <device> <brightness>.
*/

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_ERR_BRIGHTNESS 0.5

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <device> <brightness>\n", argv[0]);
    return 1;
  }

  char *output_name = argv[1];
  double brightness = atof(argv[2]);

  if (brightness <= 0 ) {
    fprintf(stdout, "Brightness cannot be zero or negative, defaulting to %f\n", DEFAULT_ERR_BRIGHTNESS);
    brightness = DEFAULT_ERR_BRIGHTNESS;
  }

  if (brightness >= 1 ) {
    fprintf(stdout, "Brightness cannot exceed 1, defaulting to 1\n");
    brightness = 1.0;
  }

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    fprintf(stderr, "Cannot open display\n");
    return 1;
  }

  int screen = DefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);
  XRRScreenResources *res = XRRGetScreenResourcesCurrent(dpy, root);

  RROutput output = 0;
  for (int i = 0; i < res->noutput; i++) {
    XRROutputInfo *output_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
    if (strcmp(output_info->name, output_name) == 0) {
      output = res->outputs[i];
      XRRFreeOutputInfo(output_info);
      break;
    }
    XRRFreeOutputInfo(output_info);
  }

  if (!output) {
    fprintf(stderr, "Output %s not found\n", output_name);
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return 1;
  }

  XRROutputInfo *output_info = XRRGetOutputInfo(dpy, res, output);
  RRCrtc crtc = output_info->crtc;
  XRRFreeOutputInfo(output_info);

  if (!crtc) {
    fprintf(stderr, "Output %s is not active (no CRTC)\n", output_name);
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return 1;
  }

  int gamma_size = XRRGetCrtcGammaSize(dpy, crtc);
  if (gamma_size == 0) {
    fprintf(stderr, "Cannot get gamma size for CRTC\n");
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return 1;
  }

  XRRCrtcGamma *gamma = XRRAllocGamma(gamma_size);
  for (int i = 0; i < gamma_size; i++) {
    unsigned short val =
        (unsigned short)(i * 65535 / (gamma_size - 1) * brightness);
    gamma->red[i] = val;
    gamma->green[i] = val;
    gamma->blue[i] = val;
  }

  XRRSetCrtcGamma(dpy, crtc, gamma);

  XRRFreeGamma(gamma);
  XRRFreeScreenResources(res);
  XCloseDisplay(dpy);

  printf("Brightness for %s set to %.2f\n", output_name, brightness);

  return 0;
}
