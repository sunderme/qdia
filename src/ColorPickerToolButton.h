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
 * @file ColorPickerToolButton.h
 * @brief A custom tool button for color selection with a popup menu.
 */

#ifndef COLOR_PICKER_TOOL_BUTTON_H
#define COLOR_PICKER_TOOL_BUTTON_H

#include <QToolButton>
#include <QColor>

QT_FORWARD_DECLARE_CLASS(QMenu)

/**
 * @class ColorPickerToolButton
 * @brief Provides a button with color selection capabilities.
 *
 * This QToolButton subclass combines a default color dialog action with
 * a custom color picker widget in a popup menu. Supports alpha channel
 * selection and emits signals for color selection and cancellation.
 */
class ColorPickerToolButton: public QToolButton
{
    Q_OBJECT
public:

    /**
     * @brief Constructs a ColorPickerToolButton
     * @param parent Parent widget (optional)
     */
    explicit ColorPickerToolButton(QWidget * parent = 0);

Q_SIGNALS:

    /**
     * @brief Emitted when a color is selected
     * @param color The selected QColor
     */
    void colorSelected(QColor color);

    /**
     * @brief Emitted when color selection is canceled
     */
    void rejected();

private Q_SLOTS:

    /**
     * @brief Handles the color dialog activation
     * @internal
     */
    void onColorDialogAction();

private:
    QMenu * m_menu; ///< Popup menu containing color picker widgets
};

#endif // COLOR_PICKER_TOOL_BUTTON_H
