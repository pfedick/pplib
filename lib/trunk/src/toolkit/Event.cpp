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
		case Unknown: return L"Unknown";
		case MouseEnter: return L"MouseEnter";
		case MouseMove: return L"MouseMove";
		case MouseLeave: return L"MouseLeave";
		case MouseDown: return L"MouseDown";
		case MouseUp: return L"MouseUp";
		case MouseDblClick: return L"MouseDblClick";
		case MouseWheel: return L"MouseWheel";
		case KeyDown: return L"KeyDown";
		case KeyUp: return L"KeyUp";
		case Show: return L"Show";
		case Close: return L"Close";
		case Resize: return L"Resize";
		case Move: return L"Move";
		case FocusIn: return L"FocusIn";
		case FocusOut: return L"FocusOut";
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
	printf ("EventHandler::mouseMoveEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif
	if (handler) handler->mouseMoveEvent(event);
}

void EventHandler::mouseDownEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseDownEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseDownEvent(event);
}

void EventHandler::mouseUpEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseUpEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseUpEvent(event);
}

void EventHandler::mouseDblClickEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseDblClickEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseDblClickEvent(event);
}

void EventHandler::mouseClickEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseClickEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseClickEvent(event);
}

void EventHandler::mouseEnterEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseEnterEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseEnterEvent(event);
}

void EventHandler::mouseLeaveEvent(MouseEvent *event)
{
#ifdef EVENT_DEBUG
	printf ("EventHandler::mouseLeaveEvent(%ls, %ls)\n",
			event->eventWidget->widgetType().toWchart(),
			event->eventWidget->name().toWchart());
#endif

	if (handler) handler->mouseLeaveEvent(event);
}

void EventHandler::geometryChangedEvent(Event *event)
{
	if (handler) handler->geometryChangedEvent(event);
}

}	// EOF namespace tk
}	// EOF namespace ppl7
