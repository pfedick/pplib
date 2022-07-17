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


AbstractSlider::AbstractSlider(int x, int y, int width, int height)
    : ppl7::tk::Widget()
{
    create(x, y, width, height);
    setClientOffset(0, 0, 0, 0);
    min=0;
    max=100;
    current_value=0;
    my_steps=20;
}

void AbstractSlider::setMinimum(int64_t value)
{
    min=value;
}

void AbstractSlider::setMaximum(int64_t value)
{
    max=value;
}

void AbstractSlider::setSteps(int64_t value)
{
    my_steps=value;
}

void AbstractSlider::setLimits(int64_t min, int64_t max)
{
    this->min=min;
    this->max=max;
}

void AbstractSlider::setValue(int64_t value)
{
    if (value == current_value) return;
    if (value >= min && value <= max) current_value=value;
    if (value < min) current_value=min;
    if (value > max) current_value=max;
    parentMustRedraw();
    sliderValueChanged(value);
    needsRedraw();
}

int64_t AbstractSlider::value() const
{
    return current_value;
}

int64_t AbstractSlider::minimum() const
{
    return min;
}

int64_t AbstractSlider::maximum() const
{
    return max;
}

int64_t AbstractSlider::steps() const
{
    return my_steps;
}

int64_t AbstractSlider::stepSize() const
{
    int step=(max - min) / my_steps;
    if (step < 1) step=1;
    return step;
}


HorizontalSlider::HorizontalSlider(int x, int y, int width, int height)
    : AbstractSlider(x, y, width, height)
{
    drag_started=false;
    start_x=0;
    spinbox=NULL;
}

HorizontalSlider::~HorizontalSlider()
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}

void HorizontalSlider::enableSpinBox(bool enabled, int64_t stepsize, int width)
{
    if (spinbox) delete spinbox;
    spinbox=NULL;
    start_x=0;
    if (enabled) {
        start_x=width + 10;
        spinbox=new SpinBox(0, 0, width, height());
        spinbox->setLimits(minimum(), maximum());
        spinbox->setStepSize(stepsize);
        spinbox->setValue(value());
        spinbox->setEventHandler(this);
        addChild(spinbox);
    }
    needsRedraw();
}


void HorizontalSlider::sliderValueChanged(int64_t value)
{
    if (spinbox) spinbox->setValue(value);
}

void HorizontalSlider::valueChangedEvent(ppl7::tk::Event* event, int64_t value)
{
    if (event->widget() == spinbox && spinbox != NULL) {
        setValue(value);
    }
    AbstractSlider::valueChangedEvent(event, value);
}

void HorizontalSlider::paint(ppl7::grafix::Drawable& draw)
{
    const ppl7::tk::WidgetStyle& style=ppl7::tk::GetWidgetStyle();


    int y1=draw.height() * 1 / 5;
    int y2=draw.height() * 4 / 5;
    int h=y2 - y1;
    int draw_range=draw.width() - h - start_x;
    int slider_range=maximum() - minimum();
    int x1=start_x + (value() - minimum()) * draw_range / slider_range;
    int x2=x1 + h;
    slider_pos.setRect(x1, y1, h, h);


    y1=draw.height() * 2 / 5;
    y2=draw.height() * 3 / 5;

    draw.fillRect(start_x, y1, draw.width(), y2, style.buttonBackgroundColor);
    draw.fillRect(start_x, y1, x1, y2, style.sliderHighlightColor);
    draw.drawRect(start_x, y1, draw.width(), y2, style.frameBorderColorLight);

    y1=draw.height() * 1 / 5;
    y2=draw.height() * 4 / 5;


    draw.fillRect(x1, y1, x2, y2, style.windowBackgroundColor);
    draw.drawRect(x1, y1, x2, y2, style.frameBorderColorLight);


}

void HorizontalSlider::mouseDownEvent(ppl7::tk::MouseEvent* event)
{
    if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
        //printf("HorizontalSlider::mouseDownEvent: %d, %d\n", event->p.x, event->p.y);
        if (event->p.inside(slider_pos)) {
            drag_started=true;
            drag_offset=event->p.x - slider_pos.x1 + start_x;
            drag_start_pos=event->p;
            ppl7::tk::GetWindowManager()->grabMouse(this);
        } else if (event->p.x < slider_pos.x1 && event->p.x>start_x) {
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

void HorizontalSlider::lostFocusEvent(ppl7::tk::FocusEvent* event)
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}

void HorizontalSlider::mouseUpEvent(ppl7::tk::MouseEvent* event)
{
    if (drag_started) {
        drag_started=false;
        ppl7::tk::GetWindowManager()->releaseMouse(this);
    }
}

void HorizontalSlider::mouseMoveEvent(ppl7::tk::MouseEvent* event)
{
    if (event->buttonMask & ppl7::tk::MouseEvent::MouseButton::Left) {
        if (drag_started) {
            int y1=height() * 1 / 5;
            int y2=height() * 4 / 5;
            int h=y2 - y1;
            int draw_range=width() - h - start_x;
            int64_t v=minimum() + (event->p.x - drag_offset) * (maximum() - minimum()) / draw_range;
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

void HorizontalSlider::mouseWheelEvent(ppl7::tk::MouseEvent* event)
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
