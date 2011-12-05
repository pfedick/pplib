#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"




namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

Window::Window()
{
	windowFlags=DefaultWindow;
	WindowTitle="PPL7 Window";
	privateData=NULL;
}

Window::~Window()
{
	if (privateData) free(privateData);
}

void *Window::getPrivateData()
{
	return privateData;
}

void Window::setPrivateData(void *data)
{
	if (privateData) free(privateData);
	privateData=NULL;
	privateData=data;
}


const RGBFormat &Window::rgbFormat() const
{
	return myFormat;
}

void Window::setRGBFormat(const RGBFormat &format)
{
	myFormat=format;
}


ppluint32 Window::flags() const
{
	return windowFlags;
}

void Window::setFlags(ppluint32 flags)
{
	windowFlags=flags;
}

const String &Window::windowTitle() const
{
	return WindowTitle;
}

void Window::setWindowTitle(const String &title)
{
	WindowTitle=title;
}

const Drawable &Window::windowIcon() const
{
	return WindowIcon;
}

void Window::setWindowIcon(const Drawable &icon)
{
	WindowIcon=icon;
}

String Window::widgetType() const
{
	return L"Window";
}

void Window::paint(Drawable &draw)
{
	const WidgetStyle *style=GetWidgetStyle();
	draw.cls(style->colorWindowBackground);
}

}	// EOF namespace tk
}	// EOF namespace ppl7
