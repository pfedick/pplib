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


AbstractSpinBox::AbstractSpinBox()
{
    up_button=NULL;
    down_button=NULL;
    text_input=NULL;
    createUi();
}

AbstractSpinBox::~AbstractSpinBox()
{
    if (text_input) delete text_input;
    if (up_button) delete up_button;
    if (down_button) delete down_button;
}

AbstractSpinBox::AbstractSpinBox(int x, int y, int width, int height, const String& text)
{
    create(x, y, width, height);
    up_button=NULL;
    down_button=NULL;
    text_input=NULL;
    createUi();
    resizeUi();
    text_input->setText(text);
}

void AbstractSpinBox::createUi()
{
    Grafix* gfx=GetGrafix();
    text_input=new LineInput();
    text_input->setEventHandler(this);
    addChild(text_input);
    up_button=new Button();
    up_button->setEventHandler(this);
    addChild(up_button);
    down_button=new Button();
    down_button->setEventHandler(this);
    addChild(down_button);
    if (gfx) {
        const WidgetStyle& style=GetWidgetStyle();
        img_up=gfx->ButtonSymbolsSmall.getDrawable(3, style.buttonSymbolColor);
        img_down=gfx->ButtonSymbolsSmall.getDrawable(4, style.buttonSymbolColor);
        up_button->setIcon(img_up);
        down_button->setIcon(img_down);
    }
}

void AbstractSpinBox::resizeUi()
{
    Rect client=this->clientRect();
    text_input->setSize(client.width() - 30, client.height());
    text_input->setPos(client.left(), client.top());

    up_button->setSize(30, client.height() / 2);
    up_button->setPos(client.width() - 30, client.top());

    down_button->setSize(30, client.height() / 2);
    down_button->setPos(client.width() - 30, client.top() + client.height() / 2);
}

const Color& AbstractSpinBox::color() const
{
    return text_input->color();
}

void AbstractSpinBox::setColor(const Color& c)
{
    text_input->setColor(c);
}

const Font& AbstractSpinBox::font() const
{
    return text_input->font();
}

void AbstractSpinBox::setFont(const Font& font)
{
    text_input->setFont(font);
}

String AbstractSpinBox::widgetType() const
{
    return "AbstractSpinBox";
}

void AbstractSpinBox::debugEvent(const ppl7::String& name, Event* event)
{
    Widget* w=event->widget();
    ppl7::String wname="unknown";
    if (w == this) wname="AbstractSpinBox";
    else if (w == up_button) wname="UpButton";
    else if (w == down_button) wname="DownButton";
    else if (w == text_input) wname="TextInput";

    printf("Event [%s]: %s\n", (const char*)wname, (const char*)name);

}

void AbstractSpinBox::paint(Drawable& draw)
{

}

void AbstractSpinBox::mouseDownEvent(MouseEvent* event)
{
    debugEvent("AbstractSpinBox::mouseDownEvent", event);
}

void AbstractSpinBox::gotFocusEvent(FocusEvent* event)
{
    debugEvent("AbstractSpinBox::gotFocusEvent", event);
}

void AbstractSpinBox::lostFocusEvent(FocusEvent* event)
{
    debugEvent("AbstractSpinBox::lostFocusEvent", event);
}

void AbstractSpinBox::textInputEvent(TextInputEvent* event)
{
    debugEvent("AbstractSpinBox::textInputEvent", event);
}

void AbstractSpinBox::keyDownEvent(KeyEvent* event)
{
    debugEvent("AbstractSpinBox::keyDownEvent", event);
}

void AbstractSpinBox::keyUpEvent(KeyEvent* event)
{
    debugEvent("AbstractSpinBox::keyUpEvent", event);
}


SpinBox::SpinBox()
{
    my_value=0;
    step_size=1;
    min=0;
    max=0;
}

SpinBox::SpinBox(int x, int y, int width, int height, int64_t value)
    :AbstractSpinBox(x, y, width, height)
{
    my_value=value;
    step_size=1;
    min=0;
    max=0;
    if (value < min) min=value;
    if (value > max) max=value;

}

String SpinBox::widgetType() const
{
    return "SpinBox";
}

void SpinBox::setValue(int64_t value)
{
    if (value<min || value>max) return;
    my_value=value;
}

int64_t SpinBox::value() const
{
    return my_value;
}

void SpinBox::setMinimum(int64_t value)
{
    min=value;
    if (my_value < min) my_value=min;
}

void SpinBox::setMaximum(int64_t value)
{
    max=value;
    if (my_value > max) my_value=max;
}

void SpinBox::setLimits(int64_t min, int64_t max)
{
    setMinimum(min);
    setMaximum(max);
}

int64_t SpinBox::minimum() const
{
    return min;
}

int64_t SpinBox::maximum() const
{
    return max;
}

void SpinBox::setStepSize(int64_t value)
{
    step_size=value;
}

int64_t SpinBox::stepSize() const
{
    return step_size;
}


DoubleSpinBox::DoubleSpinBox()
{
    my_value=0.0f;
    my_decimals=2;
    step_size=1;
    min=0;
    max=0;
}

DoubleSpinBox::DoubleSpinBox(int x, int y, int width, int height, double value, int decimals)
    :AbstractSpinBox(x, y, width, height)
{
    my_decimals=decimals;
    step_size=1;
    min=0;
    max=0;
    if (value < min) min=value;
    if (value > max) max=value;

}

String DoubleSpinBox::widgetType() const
{
    return "DoubleSpinBox";
}

void DoubleSpinBox::setValue(double value)
{
    if (value<min || value>max) return;
    my_value=value;
}

double DoubleSpinBox::value() const
{
    return my_value;
}

void DoubleSpinBox::setMinimum(double value)
{
    min=value;
    if (my_value < min) my_value=min;
}

void DoubleSpinBox::setMaximum(double value)
{
    max=value;
    if (my_value > max) my_value=max;
}

void DoubleSpinBox::setLimits(double min, double max)
{
    setMinimum(min);
    setMaximum(max);
}

double DoubleSpinBox::minimum() const
{
    return min;
}

double DoubleSpinBox::maximum() const
{
    return max;
}

void DoubleSpinBox::setStepSize(double value)
{
    step_size=value;
}

double DoubleSpinBox::stepSize() const
{
    return step_size;
}



}   // EOF namespace
