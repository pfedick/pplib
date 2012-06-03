#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"


namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;


Label::Label()
	:Frame()
{
	const WidgetStyle *style=GetWidgetStyle();
	setBorderStyle(NoBorder);
	myColor=style->labelFontColor;
	myFont=style->labelFont;
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(true);

}

Label::Label(int x, int y, int width, int height, const String &text)
	:Frame(x,y,width,height)
{
	const WidgetStyle *style=GetWidgetStyle();
	setBorderStyle(NoBorder);
	myColor=style->labelFontColor;
	myFont=style->labelFont;
	setSizeStrategyWidth(Widget::MINIMUM_EXPANDING);
	setTransparent(true);
}

Label::~Label()
{

}

const String &Label::text() const
{
	return myText;
}

void Label::setText(const String &text)
{
	myText=text;
	needsRedraw();
	geometryChanged();
}

const Color &Label::color() const
{
	return myColor;
}

void Label::setColor(const Color &c)
{
	myColor=c;
	needsRedraw();
}

const Drawable &Label::icon() const
{
	return myIcon;
}

void Label::setIcon(const Drawable &icon)
{
	myIcon=icon;
	needsRedraw();
	geometryChanged();
}

const Font &Label::font() const
{
	return myFont;
}

void Label::setFont(const Font &font)
{
	myFont=font;
	needsRedraw();
	geometryChanged();
}


Size Label::contentSize() const
{
	Size s;
	s=myFont.measure(myText);
	if (myIcon.isEmpty()==false) {
		s.width+=4+myIcon.width();
		int h=2+myIcon.height();
		if (s.height<h) s.height=h;
	}
	return s;
}

String Label::widgetType() const
{
	return L"Label";
}


void Label::paint(Drawable &draw)
{
	Frame::paint(draw);
	Drawable d=clientDrawable(draw);
	//printf ("Text: %s, width: %i, height: %i\n",(const char*)myText, d.width(), d.height());
	int x=0;
	if (myIcon.isEmpty()==false) {
		d.bltAlpha(myIcon,x,(d.height())/2-myIcon.height()/2);
		x+=4+myIcon.width();
	}
	myFont.setColor(myColor);
	myFont.setOrientation(Font::TOP);
	d.print(myFont,x,0,myText);
}


}	// EOF namespace tk
}	// EOF namespace ppl7
