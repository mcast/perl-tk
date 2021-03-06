/* $XConsortium: ClientWin.c,v 1.4 94/04/17 20:15:50 rws Exp $ */

/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "tkInt.h"

static Window TryChildren();

/* Find a window with WM_STATE, else return win itself, as per ICCCM */

Window XmuClientWindow (dpy, win)
    Display *dpy;
    Window win;
{
#ifdef WIN32
    return None;
#else
    Atom WM_STATE;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf;
    Tk_ErrorHandler handler;

    WM_STATE = XInternAtom(dpy, "WM_STATE", True);
    if (!WM_STATE)
	return win;

    /*
     * Ignore X errors when reading the property.
     * Sometimes (difficult to reproduce) we were called for a
     * ClientMessage{window=0x1}.
     */

    LangDebug("XmuClientWindow(win 0x%x)\n", win);
    handler = Tk_CreateErrorHandler(dpy, -1, -1, -1, (Tk_ErrorProc *) NULL, (ClientData) NULL);
    XGetWindowProperty_LOGGED(__FILE__,__LINE__,  dpy, win, WM_STATE, 0, 0, False, AnyPropertyType,
		       &type, &format, &nitems, &after, &data);
    if (type) {
        LangDebug("  has WM_STATE (t=%d)\n", type);
	inf = win;
    } else {
        inf = TryChildren(dpy, win, WM_STATE);
        if (!inf)
            inf = win;
    }
    LangDebug("  XmuClientWindow returning inf=0x%x\n", inf);

    Tk_DeleteErrorHandler(handler);
    return inf;
#endif
}

static
Window TryChildren (dpy, win, WM_STATE)
    Display *dpy;
    Window win;
    Atom WM_STATE;
{
#ifdef WIN32
    return None;
#else
    Window root, parent;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf = 0;

    LangDebug("    TryChildren of win=0x%x\n", win);
    if (!XQueryTree(dpy, win, &root, &parent, &children, &nchildren))
	return 0;
    for (i = 0; !inf && (i < nchildren); i++) {
        LangDebug("      child[%d] = win 0x%x\n", i, children[i]);
	XGetWindowProperty_LOGGED(__FILE__,__LINE__,  dpy, children[i], WM_STATE, 0, 0, False,
			   AnyPropertyType, &type, &format, &nitems,
			   &after, &data);
	if (type)
	    inf = children[i];
    }
    for (i = 0; !inf && (i < nchildren); i++)
	inf = TryChildren(dpy, children[i], WM_STATE);
    if (children) XFree((char *)children);
    LangDebug("      TryChildren returning 0x%x\n", inf);
    return inf;
#endif
}
