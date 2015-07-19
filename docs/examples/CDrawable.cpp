#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ppl6.h>
#include <ppl6-grafix.h>

using namespace ppl6;
using namespace ppl6::grafix;

int main(int argc, char **argv)
{
	CGrafix Grafix;
	Grafix.LoadFont("../../fonts/dejavusans4.fnt5");
	CFilter_PNG png;
	CImage Image(390,300,RGBFormat::A8R8G8B8);
	CFont Font, FontCaption;
	Font.setName("DejaVu Sans");
	Font.setAntialias(true);
	Font.setSize(8);
	Font.setColor(Color(0,0,0));

	FontCaption=Font;
	FontCaption.setSize(12);
	FontCaption.setBold(true);
	Image.cls(Color(255,255,255));
	Image.print(FontCaption,140,150,"CDrawable");

	Image.draw(Grafix.ButtonSymbolsSmall,8,23,42,Color(0,0,255));
	Image.print(Font,32,50,"P(0/0)");
	Image.draw(Grafix.ButtonSymbolsSmall,8,330,268,Color(0,0,255));
	Image.print(Font,235,275,"P(Breite-1/Höhe-1)");


	Image.drawRect(Rect(20,40,320,240),Color(0,0,0));
	Image.drawRect(Rect(21,41,318,238),Color(0,0,0));
	Image.line(20,25,20,35,Color(128,128,128));
	Image.line(20,30,339,30,Color(128,128,128));
	Image.line(339,25,339,35,Color(128,128,128));
	Image.print(Font,170,25,"Breite");

	Image.line(345,40,355,40,Color(128,128,128));
	Image.line(345,279,355,279,Color(128,128,128));
	Image.line(350,40,350,279,Color(128,128,128));
	Image.print(Font,353,140,"Höhe");

	Font.setSize(10);
	Font.setBold(true);
	Image.lineAA(35,60,320,60,Color(0,0,192),2);
	Image.lineAA(320,60,315,55,Color(0,0,192),2);
	Image.lineAA(320,60,315,65,Color(0,0,192),2);
	Font.setColor(Color(0,0,192));
	Image.print(Font,325,63,"X");

	Image.lineAA(35,70,35,260,Color(192,0,0),2);
	Image.lineAA(35,260,30,255,Color(192,0,0),2);
	Image.lineAA(35,260,40,255,Color(192,0,0),2);
	Font.setColor(Color(192,0,0));
	Image.print(Font,32,272,"Y");

	png.SaveFile(Image,"graph.png",Rect(15,15,375,275));

	return 0;
}
