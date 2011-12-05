#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"




namespace ppl7 {
namespace tk {

using namespace ppl7;
using namespace ppl7::grafix;

static void defaultSurfaceDestroy (void *data)
{

}

static void defaultSurfaceLock (void *data, Drawable &draw)
{

}

static void defaultSurfaceUnlock (void *data)
{

}

static void defaultSurfaceDraw (void *target, void *data, int x, int y)
{

}


Surface::Surface()
{
	myFlags=DefaultSurface;
	myData=NULL;
	privDestroy=defaultSurfaceDestroy;
	privLock=defaultSurfaceLock;
	privUnlock=defaultSurfaceUnlock;
	privDraw=defaultSurfaceDraw;
	w=0;
	h=0;
}

Surface::~Surface()
{
	if (myData) {
		privDestroy(myData);
		free(myData);
	}
}

const RGBFormat &Surface::rgbFormat() const
{
	return myFormat;
}

Surface::SurfaceFlags Surface::flags() const
{
	return (SurfaceFlags)myFlags;
}

int Surface::width() const
{
	return w;
}

int Surface::height() const
{
	return h;
}

void Surface::lock(Drawable &draw)
{
	privLock(myData,draw);
}

void Surface::unlock()
{
	privUnlock(myData);
}

void Surface::draw(Surface *target, int x, int y)
{
	if (!target) throw NullPointerException();
	privDraw(target->myData, myData,x,y);
}

void Surface::setLockFunction( void (*fnLock) (void *, Drawable &))
{
	privLock=fnLock;
}

void Surface::setUnlockFunction( void (*fnUnlock) (void *))
{
	privUnlock=fnUnlock;
}

void Surface::setDestroyFunction( void (*fnDestroy) (void *))
{
	privDestroy=fnDestroy;
}

void Surface::setDrawFunction( void (*fnDraw) (void *, void *, int, int))
{
	privDraw=fnDraw;
}

void Surface::setFlags(SurfaceFlags flags)
{
	myFlags=flags;
}

void Surface::setRGBFormat(const RGBFormat &format)
{
	myFormat=format;
}

void Surface::setSize(int width, int height)
{
	w=width;
	h=height;
}

void Surface::setPrivateData(void *data)
{
	myData=data;
}


}	// EOF namespace tk
}	// EOF namespace ppl7
