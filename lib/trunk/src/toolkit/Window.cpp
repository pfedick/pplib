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
	setSize(640,480);
	privateData=NULL;
	wm=NULL;
}

Window::~Window()
{
	if (wm!=NULL && privateData!=NULL) wm->destroyWindow(*this);
}

/*!\brief Private Daten des Window-Managers
 *
 * \desc
 * Diese Funktion wird intern vom Window-Manager aufgerufen und sollte vom der Anwendung
 * nicht verwendet werden.
 *
 * \return Liefert einen Pointer auf die private Datenstruktur des Window-Managers für dieses
 * Fenster zurück
 *
 */
void *Window::getPrivateData()
{
	return privateData;
}

/*!\brief Private Daten des Window-Managers setzen
 *
 * \desc
 * Diese Funktion wird intern vom Window-Manager aufgerufen und sollte vom der Anwendung
 * nicht verwendet werden. Der Window-Manager verwendet sie, um einen Pointer auf die
 * interne Datenstruktur für das Fenster zu setzen. Der Window-Manager ist für die Freigabe des
 * Speichers verantwortlich.
 *
 * \param data Pointer auf die interne Datenstruktur
 * \param wm Pointer auf die Klasse des Window-Manager
 */
void Window::setPrivateData(void *data, WindowManager *wm)
{
	privateData=data;
	this->wm=wm;
}


const RGBFormat &Window::rgbFormat() const
{
	return WindowRGBFormat;
}

void Window::setRGBFormat(const RGBFormat &format)
{
	WindowRGBFormat=format;
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
	if (wm!=NULL && privateData!=NULL) wm->setWindowTitle(*this);
}

const Drawable &Window::windowIcon() const
{
	return WindowIcon;
}

void Window::setWindowIcon(const Drawable &icon)
{
	WindowIcon=icon;
	if (wm!=NULL && privateData!=NULL) wm->setWindowIcon(*this);
}

String Window::widgetType() const
{
	return "Window";
}

void Window::paint(Drawable &draw)
{
	const WidgetStyle *style=GetWidgetStyle();
	draw.cls(style->colorWindowBackground);
}

}	// EOF namespace tk
}	// EOF namespace ppl7
