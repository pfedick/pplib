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

namespace ppl7 {
namespace tk {


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
    text_input->setSize(client.width() - 25, client.height());
    text_input->setPos(client.left(), client.top());

    up_button->setSize(25, client.height() / 2);
    up_button->setPos(client.width() - 25, client.top());

    down_button->setSize(25, client.height() / 2);
    down_button->setPos(client.width() - 25, client.top() + client.height() / 2);
}

const Color& AbstractSpinBox::color() const
{
    return text_input->color();
}

void AbstractSpinBox::setColor(const Color& c)
{
    text_input->setColor(c);
}

void AbstractSpinBox::setBackgroundColor(const ppl7::grafix::Color& color)
{
    text_input->setBackgroundColor(color);
}

const Color& AbstractSpinBox::backgroundColor() const
{
    return text_input->backgroundColor();
}


const Font& AbstractSpinBox::font() const
{
    return text_input->font();
}

void AbstractSpinBox::setFont(const Font& font)
{
    text_input->setFont(font);
}

void AbstractSpinBox::setText(const String& value)
{
    text_input->setText(value);
    Event new_event(Event::TextChanged);
    new_event.setWidget(this);
    EventHandler::textChangedEvent(&new_event, value);
}

String AbstractSpinBox::text() const
{
    return String(text_input->text());
}

String AbstractSpinBox::widgetType() const
{
    return "AbstractSpinBox";
}



void AbstractSpinBox::debugEvent(const ppl7::String& name, Event* event)
{
    return;
    Widget* w=event->widget();
    ppl7::String wname="unknown";
    if (w == this) wname="AbstractSpinBox";
    else if (w == up_button) wname="UpButton";
    else if (w == down_button) wname="DownButton";
    else if (w == text_input) wname="TextInput";

    printf("Event [%s]: %s\n", (const char*)wname, (const char*)name);

}

void AbstractSpinBox::setInputValidator(InputValidator* validator)
{
    text_input->setInputValidator(validator);
}

void AbstractSpinBox::paint(Drawable& draw)
{

}

void AbstractSpinBox::mouseDownEvent(MouseEvent* event)
{
    debugEvent("AbstractSpinBox::mouseDownEvent", event);
    Widget* w=event->widget();
    if (w == up_button) stepUp();
    else if (w == down_button) stepDown();
    GetWindowManager()->setKeyboardFocus(text_input);

}

void AbstractSpinBox::gotFocusEvent(FocusEvent* event)
{
    debugEvent("AbstractSpinBox::gotFocusEvent", event);
    GetWindowManager()->setKeyboardFocus(text_input);
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
    KeyEvent new_event=*event;
    new_event.setWidget(this);
    EventHandler::keyDownEvent(&new_event);
}

void AbstractSpinBox::keyUpEvent(KeyEvent* event)
{
    debugEvent("AbstractSpinBox::keyUpEvent", event);
}

void AbstractSpinBox::textChangedEvent(Event* event, const String& text)
{
    debugEvent("AbstractSpinBox::textChangedEvent", event);
    Event new_event=*event;
    new_event.setWidget(this);
    EventHandler::textChangedEvent(&new_event, text);
}


SpinBox::SpinBox()
{
    my_value=0;
    step_size=1;
    min=0;
    max=0;
    setInputValidator(this);
}

SpinBox::SpinBox(int x, int y, int width, int height, int64_t value)
    :AbstractSpinBox(x, y, width, height)
{
    my_value=0;
    step_size=1;
    min=0;
    max=0;
    setInputValidator(this);
    if (value < min) min=value;
    if (value > max) max=value;
    setValue(value);
}

String SpinBox::widgetType() const
{
    return "SpinBox";
}

void SpinBox::setValue(int64_t new_value)
{
    if (new_value<min || new_value>max) return;
    if (new_value == value() && this->text().notEmpty()) return;
    my_value=new_value;
    setText(ppl7::ToString("%ld", new_value));
}

int64_t SpinBox::value() const
{
    return text().toInt64();
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


void SpinBox::stepUp()
{
    int64_t v=value() + step_size;
    if (v > max) v=max;
    setValue(v);
}

void SpinBox::stepDown()
{
    int64_t v=value() - step_size;
    if (v < min) v=min;
    setValue(v);
}

bool SpinBox::validateText(const ppl7::WideString& text)
{
    ppl7::String t=text;
    if (t.isEmpty() || t.pregMatch("/^-?[0-9]*$/")) {
        int64_t v=text.toInt64();
        if (v >= min && v <= max) return true;
        return false;
    }
    return false;
}

bool SpinBox::validateInput(const ppl7::WideString& text)
{
    ppl7::String t=text;
    if (t.isEmpty() || t.pregMatch("/^-?[0-9]*$/")) {
        return true;
    }
    return false;
}

void SpinBox::textChangedEvent(Event* event, const String& text)
{
    Event new_event=*event;
    new_event.setWidget(this);
    EventHandler::valueChangedEvent(&new_event, text.toInt64());
}



DoubleSpinBox::DoubleSpinBox()
{
    my_value=0.0f;
    my_decimals=2;
    step_size=1;
    min=0;
    max=0;
    setValue(0);
    setInputValidator(this);
}

DoubleSpinBox::DoubleSpinBox(int x, int y, int width, int height, double value, int decimals)
    :AbstractSpinBox(x, y, width, height)
{
    my_decimals=decimals;
    step_size=1;
    min=0;
    max=0;
    my_value=0.0f;
    if (value < min) min=value;
    if (value > max) max=value;
    setValue(value);
    setInputValidator(this);
}

String DoubleSpinBox::widgetType() const
{
    return "DoubleSpinBox";
}

void DoubleSpinBox::setValue(double new_value)
{
    if (new_value<min || new_value>max) return;
    if (new_value == value() && this->text().notEmpty()) return;
    my_value=new_value;
    ppl7::String format;
    format.setf("%%0.%df", my_decimals);
    setText(ppl7::ToString((const char*)format, new_value));

    Event new_event(Event::ValueChanged);
    new_event.setWidget(this);
    EventHandler::valueChangedEvent(&new_event, new_value);

}

double DoubleSpinBox::value() const
{
    return text().toDouble();
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

void DoubleSpinBox::stepUp()
{
    double v=value() + step_size;
    if (v > max) v=max;
    setValue(v);
}

void DoubleSpinBox::stepDown()
{
    double v=value() - step_size;
    if (v < min) v=min;
    setValue(v);
}

bool DoubleSpinBox::validateText(const ppl7::WideString& text)
{
    ppl7::String t=text;
    if (t.isEmpty() || t.pregMatch("/^-?[0-9\\.,]*$/")) {
        double v=text.toDouble();
        if (v >= min && v <= max) return true;
    }
    return false;
}

bool DoubleSpinBox::validateInput(const ppl7::WideString& text)
{
    ppl7::String t=text;
    if (t.isEmpty() || t.pregMatch("/^-?[0-9\\.,]*$/")) {
        return true;
    }
    return false;
}

void DoubleSpinBox::textChangedEvent(Event* event, const String& text)
{
    Event new_event=*event;
    new_event.setWidget(this);
    EventHandler::valueChangedEvent(&new_event, text.toDouble());
}



}
}   // EOF namespace
