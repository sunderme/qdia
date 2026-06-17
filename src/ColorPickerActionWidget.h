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

/*!
 * @file ColorPickerActionWidget.h
 * @brief Implementation of a class for color selection in a Qt-based application.
 *
 * This file defines the ColorPickerActionWidget class, which inherits from QWidgetAction and provides functionality for color selection using QColorDialog.
 */

#ifndef COLOR_PICKER_ACTION_WIDGET_H
#define COLOR_PICKER_ACTION_WIDGET_H

#include <QWidgetAction>

QT_FORWARD_DECLARE_CLASS(QColorDialog)

/*!
 * @class ColorPickerActionWidget
 * @brief Class for color selection inheriting from QWidgetAction.
 *
 * The class provides the ability to select a color using the QColorDialog dialog and emits signals when a color is selected or the action is canceled.
 */
class ColorPickerActionWidget: public QWidgetAction
{
    Q_OBJECT
public:
    explicit ColorPickerActionWidget(QWidget * parent = 0);

Q_SIGNALS:

    /*!
     * @brief Signal emitted when a color is selected.
     *
     * @param color Selected color.
     */
    void colorSelected(QColor color);

    /*!
     * @brief Signal emitted when the action is canceled.
     */
    void rejected();

public Q_SLOTS:
    void aboutToShow();
    void aboutToHide();

private:
    QColorDialog * m_colorDialog;
};

#endif // COLOR_PICKER_ACTION_WIDGET_H
