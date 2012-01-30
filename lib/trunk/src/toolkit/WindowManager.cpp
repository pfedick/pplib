#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"


namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;


static WindowManager *wm=NULL;

WindowManager *GetWindowManager()
{
	if (!wm) throw NoWindowManagerException();
	return wm;
}

const WidgetStyle *GetWidgetStyle()
{
	if (!wm) throw NoWindowManagerException();
	return wm->getWidgetStyle();
}

WindowManager::WindowManager()
{
	if (wm!=NULL) throw DuplicateWindowManagerException();
	wm=this;
	Style.colorWindowBackground.setColor(220,220,220,255);
	Style.frameBackgroundColor.setColor(230,230,230,255);
	Style.frameBorderColor.setColor(96,96,96,255);
	Style.labelFontColor.setColor(0,0,0,255);
	Style.buttonBackgroundColor.setColor(230,230,230,255);
	Style.buttonFontColor.setColor(0,0,0,255);
	LastMouseDown=NULL;
	LastMouseEnter=NULL;
	LastMouseClick=NULL;
}

WindowManager::~WindowManager()
{
	if (wm==this) wm=NULL;
}

const WidgetStyle *WindowManager::getWidgetStyle() const
{
	return &Style;
}


void WindowManager::getMouseStatus(Point &p, int &buttonMask)
{

}

Widget *WindowManager::findMouseWidget(Widget *window, MouseEvent *event)
{
	Widget *w;
	if (!window) return NULL;
	List<Widget*>::Iterator it;
	if (window->childs.num()>0) {
		window->childs.reset(it);
		while (window->childs.getNext(it)) {
			w=it.value();
			if (event->p.x>=w->p.x
					&& event->p.y>=w->p.y
					&& event->p.x < w->p.x+w->s.width
					&& event->p.y < w->p.y+w->s.height) {
				// Passendes Widget gefunden, Koordinaten des Events auf das Widget umrechnen
				event->p.x-=w->p.x;
				event->p.y-=w->p.y;
				return findMouseWidget(w,event);	// Iterieren
			}
		}
	}
	return window;
}

void WindowManager::dispatchEvent(Widget *window, Event &event)
{
	Widget *w;
	switch (event.type()) {
		case Event::MouseEnter:
			event.setWidget(window);
			LastMouseEnter=window;
			window->mouseEnterEvent((MouseEvent*)&event);
			return;
		case Event::MouseLeave:
			if (LastMouseEnter) {
				event.setWidget(LastMouseEnter);
				LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
				LastMouseEnter=NULL;
			}
			event.setWidget(window);
			window->mouseLeaveEvent((MouseEvent*)&event);
			return;
		case Event::MouseMove:
			w=findMouseWidget(window,(MouseEvent*)&event);
			if (w) {
				if (w!=LastMouseEnter) {
					if (LastMouseEnter) {
						event.setWidget(LastMouseEnter);
						LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
					}
					event.setWidget(w);
					w->mouseEnterEvent((MouseEvent*)&event);
					LastMouseEnter=w;
				}
				event.setWidget(w);
				w->mouseMoveEvent((MouseEvent*)&event);
			} else if (LastMouseEnter) {
				event.setWidget(LastMouseEnter);
				LastMouseEnter->mouseLeaveEvent((MouseEvent*)&event);
				LastMouseEnter=NULL;
			}
			return;
		case Event::MouseDown:
			w=findMouseWidget(window,(MouseEvent*)&event);
			if (w) {
				if (w!=LastMouseDown && LastMouseDown!=NULL) {

				}
				LastMouseDown=w;
				event.setWidget(w);
				w->mouseDownEvent((MouseEvent*)&event);
			}
			return;

		case Event::MouseUp:
			w=findMouseWidget(window,(MouseEvent*)&event);
			if (w) {
				event.setWidget(w);
				w->mouseUpEvent((MouseEvent*)&event);
				if (LastMouseDown==w) {
					LastMouseDown->mouseClickEvent((MouseEvent*)&event);
				}
				LastMouseDown=NULL;
			}
			return;

		default:
			printf("WindowManager::dispatchEvent(%tu, %ls)  ==> Unhandled Event\n",(ptrdiff_t)window,event.name().toWchart());
			return;
	}

}

}	// EOF namespace tk
}	// EOF namespace ppl7
