#include <ppl7.h>
#include <ppl7-grafix.h>
#include "ppl7-tk.h"

namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

HorizontalLayout::HorizontalLayout()
{
	spacing=2;
	lastTotal=0;
}

HorizontalLayout::~HorizontalLayout()
{

}

String HorizontalLayout::widgetType() const
{
	return L"HorizontalLayout";
}

void HorizontalLayout::paint(Drawable &draw)
{

}


void HorizontalLayout::geometryChangedEvent(Event *event)
{
	//printf ("HorizontalLayout::geometryChangedEvent, %i Childs\n",numChilds());
	size_t num=numChilds();
	if (num) {
		typedef struct {
				Widget *widget;
				int		strategy;
				int		preferedWidth;
		} ITEM;
		List<Widget*>::Iterator it;

		ITEM *item=(ITEM*)malloc(sizeof(ITEM)*num);
		if (!item) throw OutOfMemoryException();
		resetIterator(it);
		Widget *w;
		size_t c=0;
		int total=0;
		int ww;
		int hh=height();
		int available=width();
		int maxc=0;
		while ((w=getNextChild(it))) {
			item[c].widget=w;
			item[c].strategy=w->sizeStrategyWidth();
			if (item[c].strategy==Widget::MAXIMUM_EXPANDING) maxc++;
			item[c].preferedWidth=w->preferedSize().width+spacing;
			total+=item[c].preferedWidth;
			//printf ("Widtget %i, strategy=%i, preferedWidth=%i\n",c,item[c].strategy,item[c].preferedWidth);
			c++;
		}
		if (total!=lastTotal) {
			lastTotal=total;
			available-=total;
			int x=0;
			for (size_t i=0;i<num;i++) {
				ww=item[i].preferedWidth;
				if (item[i].strategy==Widget::MAXIMUM_EXPANDING) {
					if (available>0) {
						ww+=(available/maxc);
					}
				}
				item[i].widget->setPos(x,0);
				item[i].widget->setSize(ww-spacing,hh);
				x+=ww;
			}
		}

		free(item);
	}
	event->accept();
}


HorizontalSpacer::HorizontalSpacer()
{
	setSizeStrategyWidth(Widget::MAXIMUM_EXPANDING);
}


String HorizontalSpacer::widgetType() const
{
	return L"HorizontalSpacer";
}

void HorizontalSpacer::paint(Drawable &draw)
{

}

}	// EOF namespace tk
}	// EOF namespace ppl7
