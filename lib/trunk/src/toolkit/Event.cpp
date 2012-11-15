#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"



namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

//#define EVENT_DEBUG

Event::Event()
{
	t=Unknown;
	accepted=false;
	ignored=false;
	eventWidget=NULL;
}

Event::Event(Type t)
{
	this->t=t;
	accepted=false;
	ignored=false;
}

Event::~Event()
{

}

Event::Type Event::type() const
{
	return t;
}

void Event::setType(Type t)
{
	this->t=t;
}

void Event::ignore()
{
	ignored=false;
}

void Event::accept()
{
	accepted=true;
}

bool Event::isIgnored() const
{
	return ignored;
}

bool Event::isAccepted() const
{
	return accepted;
}

String Event::name() const
{
	switch (t) {
		case Unknown: return "Unknown";
		case MouseEnter: return "MouseEnter";
		case MouseMove: return "MouseMove";
		case MouseLeave: return "MouseLeave";
		case MouseDown: return "MouseDown";
		case MouseUp: return "MouseUp";
		case MouseDblClick: return "MouseDblClick";
		case MouseWheel: return "MouseWheel";
		case KeyDown: return "KeyDown";
		case KeyUp: return "KeyUp";
		case Show: return "Show";
		case Close: return "Close";
		case Resize: return "Resize";
		case Move: return "Move";
		case FocusIn: return "FocusIn";
		case FocusOut: return "FocusOut";
		case GeometryChanged: return "GeometryChanged";
	}
	throw UnknownEventException();
}

Widget *Event::widget() const
{
	return eventWidget;
}

void Event::setWidget(Widget *w)
{
	eventWidget=w;
}

MouseEvent::MouseEvent()
{
	p.x=0;
	p.y=0;
	button=(MouseEvent::MouseButton)0;
}

ResizeEvent::ResizeEvent()
{
	width=0;
	height=0;
}



EventHandler::EventHandler()
{
	handler=NULL;
}

EventHandler::~EventHandler()
{

}

void EventHandler::setEventHandler(EventHandler *handler)
{
	this->handler=handler;
}

void EventHandler::mouseMoveEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseMoveEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif
	if (handler) handler->mouseMoveEvent(event);
}

void EventHandler::mouseDownEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseDownEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseDownEvent(event);
}

void EventHandler::mouseUpEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseUpEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseUpEvent(event);
}

void EventHandler::mouseDblClickEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseDblClickEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseDblClickEvent(event);
}

void EventHandler::mouseClickEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseClickEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseClickEvent(event);
}

void EventHandler::mouseEnterEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseEnterEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseEnterEvent(event);
}

void EventHandler::mouseLeaveEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseLeaveEvent(%s, %s)\n",
			event->eventWidget->widgetType().toChar(),
			event->eventWidget->name().toChar());
#endif

	if (handler) handler->mouseLeaveEvent(event);
}

void EventHandler::geometryChangedEvent(Event *event)
{
	if (handler) handler->geometryChangedEvent(event);
}

}	// EOF namespace tk
}	// EOF namespace ppl7
