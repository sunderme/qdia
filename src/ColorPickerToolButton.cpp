/*
* The MIT License (MIT)
*
* Copyright (c) 2015 Dmitry Ivanov
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

/**
 * @file ColorPickerToolButton.cpp
 * @brief Implementation of the ColorPickerToolButton class
 */

#include "ColorPickerToolButton.h"
#include "ColorPickerActionWidget.h"
#include <QMenu>
#include <QColorDialog>
#include <QScopedPointer>

/**
 * @brief Initializes the button with color picker components
 *
 * Creates:
 * - A popup menu with ColorPickerActionWidget
 * - Default action triggering color dialog
 * - Signal connections for color selection propagation
 * - Menu show/hide handlers for the color widget
 */
ColorPickerToolButton::ColorPickerToolButton(QWidget * parent) :
    QToolButton(parent),
    m_menu(new QMenu(this))
{
    ColorPickerActionWidget * colorPickerActionWidget = new ColorPickerActionWidget(this);
    m_menu->addAction(colorPickerActionWidget);

    setMenu(m_menu);

    QAction * colorDialogAction = new QAction(this);
    setDefaultAction(colorDialogAction);

    //QObject::connect(colorDialogAction, SIGNAL(triggered(bool)), this, SLOT(onColorDialogAction()));

    QObject::connect(colorPickerActionWidget, SIGNAL(colorSelected(QColor)),
                     this, SIGNAL(colorSelected(QColor)));
    QObject::connect(colorPickerActionWidget, SIGNAL(rejected()),
                     this, SIGNAL(rejected()));

    QObject::connect(colorPickerActionWidget, SIGNAL(colorSelected(QColor)),
                     m_menu, SLOT(hide()));
    QObject::connect(colorPickerActionWidget, SIGNAL(rejected()),
                     m_menu, SLOT(hide()));

    QObject::connect(m_menu, SIGNAL(aboutToShow()), colorPickerActionWidget, SLOT(aboutToShow()));
    QObject::connect(m_menu, SIGNAL(aboutToHide()), colorPickerActionWidget, SLOT(aboutToHide()));
}

/**
 * @brief Shows a QColorDialog and handles selection results
 *
 * Features:
 * - Uses non-native dialog for consistent cross-platform behavior
 * - Shows alpha channel but forces full opacity (alpha=255)
 * - Emits colorSelected() with chosen color or rejected() on cancel
 */
void ColorPickerToolButton::onColorDialogAction()
{
    QScopedPointer<QColorDialog> colorDialogPtr(new QColorDialog(this));
    QColorDialog * colorDialog = colorDialogPtr.data();
    colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);

    QColor currentColor = colorDialog->currentColor();
    currentColor.setAlpha(255);
    colorDialog->setCurrentColor(currentColor);

    if (colorDialog->exec() == QColorDialog::Accepted)
    {
        QColor color = colorDialog->currentColor();
        emit colorSelected(color);
    }
    else
    {
        emit rejected();
    }
}
