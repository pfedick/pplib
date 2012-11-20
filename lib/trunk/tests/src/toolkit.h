/*
 * toolkit.h
 *
 *  Created on: 18.11.2012
 *      Author: patrick
 */

#ifndef TOOLKIT_H_
#define TOOLKIT_H_


class FrameRate
{
	private:
		int desiredFrameRate;
		int fps;
		int framecount;
		ppl7::ppl_time_t lastFpsTime;

	public:
		FrameRate();
		void setFrameRate(int fps);
		int getFrameCount() const;
		int getFPS() const;
		void updateFPS();
};

class MainWindow : public ppl7::tk::Window
{
	private:
		ppl7::grafix::Grafix *gfx;

		ppl7::tk::Frame *menue;
		ppl7::tk::Label *frameCountLabel;
		ppl7::tk::Label *fpsLabel;
		ppl7::tk::Button *exitButton;

		ppl7::tk::Frame *toolbar;

		ppl7::tk::LineInput *input;

		FrameRate fps;

		virtual void closeEvent(ppl7::tk::Event *event);
		virtual void mouseClickEvent(ppl7::tk::MouseEvent *event);

	public:
		MainWindow();
		~MainWindow();

		void create(int width, int height, bool fullscreen);
		void updateFrameRate();


};

#endif /* TOOLKIT_H_ */
