
/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


typedef struct {
	int screen;
	Window root, win;
	Pixmap pmap;
	unsigned long colors[2];
} Lock;

static Lock **locks;
static int nscreens;
static Bool running = True;

static void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}


static void
unlockscreen(Display *dpy, Lock *lock) {
	if(dpy == NULL || lock == NULL)
		return;

	XUngrabPointer(dpy, CurrentTime);
	XFreeColors(dpy, DefaultColormap(dpy, lock->screen), lock->colors, 2, 0);
	XFreePixmap(dpy, lock->pmap);
	XDestroyWindow(dpy, lock->win);

	free(lock);
}

static Lock *
lockscreen(Display *dpy, int screen) {
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int len;
	Lock *lock;
	XColor color, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;

	if(dpy == NULL || screen < 0)
		return NULL;

	lock = malloc(sizeof(Lock));
	if(lock == NULL)
		return NULL;

	lock->screen = screen;

	lock->root = RootWindow(dpy, lock->screen);

	/* init */
	wa.override_redirect = 1;
	wa.background_pixel = BlackPixel(dpy, lock->screen);
	lock->win = XCreateWindow(dpy, lock->root, 0, 0, DisplayWidth(dpy, lock->screen), DisplayHeight(dpy, lock->screen),
			0, DefaultDepth(dpy, lock->screen), CopyFromParent,
			DefaultVisual(dpy, lock->screen), CWOverrideRedirect | CWBackPixel, &wa);
	XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen), COLOR2, &color, &dummy);
	lock->colors[1] = color.pixel;
	XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen), COLOR1, &color, &dummy);
	lock->colors[0] = color.pixel;
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap, &color, &color, 0, 0);
	XDefineCursor(dpy, lock->win, invisible);
	XMapRaised(dpy, lock->win);
	for(len = 1000; len; len--) {
		if(XGrabPointer(dpy, lock->root, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync, GrabModeAsync, None, invisible, CurrentTime) == GrabSuccess)
			break;
		usleep(1000);
	}
	if(running && (len > 0)) {
		for(len = 1000; len; len--) {
			if(XGrabKeyboard(dpy, lock->root, True, GrabModeAsync, GrabModeAsync, CurrentTime)
				== GrabSuccess)
				break;
			usleep(1000);
		}
	}

	running &= (len > 0);
	if(!running) {
		unlockscreen(dpy, lock);
		lock = NULL;
	}
	else 
		XSelectInput(dpy, lock->root, SubstructureNotifyMask);

	return lock;
}

static void
usage(void) {
	fprintf(stderr, "usage: hlock [-v]\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv) {

	Display *dpy;
	int screen;

	if((argc == 2) && !strcmp("-v", argv[1]))
		die("hlock-%s, © 2014 Nuertey D Noi\n", VERSION);
	else if(argc != 1)
		usage();

	if(!(dpy = XOpenDisplay(0)))
		die("hlock: cannot open display");
	/* Get the number of screens in display "dpy" and blank them all. */
	nscreens = ScreenCount(dpy);
	locks = malloc(sizeof(Lock *) * nscreens);
	if(locks == NULL)
		die("hlock: malloc: %s", strerror(errno));
	int nlocks = 0;
	for(screen = 0; screen < nscreens; screen++) {
		if ( (locks[screen] = lockscreen(dpy, screen)) != NULL)
			nlocks++;
	}
	XSync(dpy, False);

	/* Did we actually manage to lock something? */
	if (nlocks == 0) { // nothing to protect
		free(locks);
		XCloseDisplay(dpy);
		return 1;
	}

	/*alright take a break, blank screen for Xmins*/
	usleep(BREAK_DELAY*60000000);

	/* break over, unlock everything and quit. */
	for(screen = 0; screen < nscreens; screen++)
		unlockscreen(dpy, locks[screen]);

	free(locks);
	XCloseDisplay(dpy);

	return 0;
}
