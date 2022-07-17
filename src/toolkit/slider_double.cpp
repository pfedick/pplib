/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2022, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "prolog_ppl7.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"
#include "ppl7-grafix.h"
#include "ppl7-tk.h"

namespace ppl7::tk {


DoubleAbstractSlider::DoubleAbstractSlider(int x, int y, int width, int height)
    : ppl7::tk::Widget()
{
    create(x, y, width, height);
    setClientOffset(0, 0, 0, 0);
    min=0.0f;
    max=1.0f;
    current_value=0.0f;
    my_steps=0.1f;
}


void DoubleAbstractSlider::setMinimum(double value)
{
    min=value;
}

void DoubleAbstractSlider::setMaximum(double value)
{
    max=value;
}

void DoubleAbstractSlider::setSteps(double value)
{
    my_steps=value;
}

void DoubleAbstractSlider::setLimits(double min, double max)
{
    this->min=min;
    this->max=max;
}

void DoubleAbstractSlider::setValue(double value)
{
    if (value >= min && value <= max) current_value=value;
    if (value < min) current_value=min;
    if (value > max) current_value=max;
    parentMustRedraw();
    needsRedraw();
}

double DoubleAbstractSlider::value() const
{
    return current_value;
}

double DoubleAbstractSlider::minimum() const
{
    return min;
}

double DoubleAbstractSlider::maximum() const
{
    return max;
}

double DoubleAbstractSlider::steps() const
{
    return my_steps;
}

double DoubleAbstractSlider::stepSize() const
{
    int step=(max - min) / my_steps;
    if (step < 1) step=1;
    return step;
}



DoubleHorizontalSlider::DoubleHorizontalSlider(int x, int y, int width, int height)
    : DoubleAbstractSlider(x, y, width, height)
{
    drag_started=false;
}

DoubleHorizontalSlider::~DoubleHorizontalSlider()
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}


void DoubleHorizontalSlider::paint(ppl7::grafix::Drawable& draw)
{
    const ppl7::tk::WidgetStyle& style=ppl7::tk::GetWidgetStyle();


    int y1=draw.height() * 1 / 5;
    int y2=draw.height() * 4 / 5;
    int h=y2 - y1;
    int draw_range=draw.width() - h;
    double slider_range=maximum() - minimum();
    int x1=(value() - minimum()) * draw_range / slider_range;
    int x2=x1 + h;
    slider_pos.setRect(x1, y1, h, h);


    y1=draw.height() * 2 / 5;
    y2=draw.height() * 3 / 5;

    draw.fillRect(0, y1, draw.width(), y2, style.buttonBackgroundColor);
    draw.fillRect(0, y1, x1, y2, style.sliderHighlightColor);
    draw.drawRect(0, y1, draw.width(), y2, style.frameBorderColorLight);

    y1=draw.height() * 1 / 5;
    y2=draw.height() * 4 / 5;


    draw.fillRect(x1, y1, x2, y2, style.windowBackgroundColor);
    draw.drawRect(x1, y1, x2, y2, style.frameBorderColorLight);


}

void DoubleHorizontalSlider::mouseDownEvent(ppl7::tk::MouseEvent* event)
{
    if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
        //printf("HorizontalSlider::mouseDownEvent: %d, %d\n", event->p.x, event->p.y);
        if (event->p.inside(slider_pos)) {
            drag_started=true;
            drag_offset=event->p.x - slider_pos.x1;
            drag_start_pos=event->p;
            ppl7::tk::GetWindowManager()->grabMouse(this);
        } else if (event->p.x < slider_pos.x1) {
            setValue(value() - stepSize());
            ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
            ev.setWidget(this);
            valueChangedEvent(&ev, value());
        } else if (event->p.x > slider_pos.x2) {
            setValue(value() + stepSize());
            ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
            ev.setWidget(this);
            valueChangedEvent(&ev, value());
        }
    }
}

void DoubleHorizontalSlider::lostFocusEvent(ppl7::tk::FocusEvent* event)
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}

void DoubleHorizontalSlider::mouseUpEvent(ppl7::tk::MouseEvent* event)
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}

void DoubleHorizontalSlider::mouseMoveEvent(ppl7::tk::MouseEvent* event)
{
    if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
        if (drag_started) {
            int y1=height() * 1 / 5;
            int y2=height() * 4 / 5;
            int h=y2 - y1;
            int draw_range=width() - h;
            double v=minimum() + (event->p.x - drag_offset) * (maximum() - minimum()) / draw_range;
            setValue(v);
            ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
            ev.setWidget(this);
            valueChangedEvent(&ev, value());

        }
    } else if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }

}

void DoubleHorizontalSlider::mouseWheelEvent(ppl7::tk::MouseEvent* event)
{
    if (event->wheel.y < 0 && value()>minimum()) {
        setValue(value() - stepSize());
        ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
        ev.setWidget(this);
        valueChangedEvent(&ev, value());
    } else if (event->wheel.y > 0 && value() < maximum()) {
        setValue(value() + stepSize());
        ppl7::tk::Event ev(ppl7::tk::Event::ValueChanged);
        ev.setWidget(this);
        valueChangedEvent(&ev, value());
    }
}

}   // EOF namespace
