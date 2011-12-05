#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"



namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

Frame::Frame()
{
	const WidgetStyle *style=GetWidgetStyle();
	myBorderStyle=Upset;
	myBackground=style->frameBackgroundColor;
	myBorderColor=style->frameBorderColor;
	setTransparent(false);
	setClientOffset(2,2,2,2);
}

Frame::Frame(int x, int y, int width, int height)
{
	const WidgetStyle *style=GetWidgetStyle();
	myBorderStyle=Upset;
	myBackground=style->frameBackgroundColor;
	myBorderColor=style->frameBorderColor;
	create(x,y,width,height);
	setTransparent(false);
	setClientOffset(2,2,2,2);
}

Frame::~Frame()
{

}

Frame::BorderStyle Frame::borderStyle() const
{
	return (BorderStyle)myBorderStyle;
}

void Frame::setBorderStyle(BorderStyle s)
{
	myBorderStyle=s;
	needsRedraw();
}

const Color &Frame::backgroundColor() const
{
	return myBackground;
}

void Frame::setBackgroundColor(const Color &c)
{
	myBackground=c;
	needsRedraw();
}

const Color &Frame::borderColor() const
{
	return myBorderColor;
}

void Frame::setBorderColor(const Color &c)
{
	myBorderColor=c;
	needsRedraw();
}

void Frame::paint(Drawable &draw)
{
	Color white(255,255,255,255);
	Color bg;

	int w=width()-1;
	int h=height()-1;
	bool myTransparent=isTransparent();
	switch(myBorderStyle) {
		case NoBorder:
			if (!myTransparent) draw.cls(myBackground);
			break;
		case Normal:
			if (!myTransparent) draw.cls(myBackground);
			draw.drawRect(0,0,w,h,myBorderColor);
			break;
		case Upset:
			if (!myTransparent) {
				bg=myBackground*1.05f;
				draw.cls(bg);
			}
			draw.line(0,0,w,0,white);
			draw.line(0,0,0,h,white);
			draw.line(0,h,w,h,myBorderColor);
			draw.line(w,0,w,h,myBorderColor);
			break;
		case Inset:
			if (!myTransparent) {
				bg=myBackground*0.95f;
				draw.cls(bg);
			}
			draw.line(0,0,w,0,myBorderColor);
			draw.line(0,0,0,h,myBorderColor);
			draw.line(0,h,w,h,white);
			draw.line(w,0,w,h,white);
			break;
	}
}

String Frame::widgetType() const
{
	return L"Frame";
}



}	// EOF namespace tk
}	// EOF namespace ppl7

