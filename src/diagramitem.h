/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/**
 * @file diagramitem.h
 * @brief Header file for the DiagramItem class
 *
 * This file contains the declaration of the DiagramItem class, which represents a graphical item in a diagram that can be moved, selected, and modified. The DiagramItem class serves as a base class for various diagram elements such as steps, conditions, start/end points, and I/O elements in a graphical editor application.
 *
 * @details The DiagramItem class is designed to work within Qt's graphics framework and provides functionality for creating different shapes, handling context menus, and managing item properties like position, size, color, and selection state. It supports serialization to/from JSON format for saving/loading diagrams and implements locking mechanism to prevent movement of items when needed.
 *
 * The class inherits from QGraphicsPathItem and provides a foundation for creating diagram elements in a graphical editor application. It supports different diagram types through the DiagramType enum, each with its own visual representation created by the createPath() method. The class handles user interactions like selection and context menu events, and provides methods for copying items, serialization, and locking functionality.
 *
 * @note This file is part of the Qt Toolkit examples and requires a QGraphicsScene for proper operation. All diagram items should be added to a scene before they can be displayed or interacted with properly. The class is intended to be used in conjunction with Qt's graphics system and may require additional setup for full functionality.
 */

#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

/**
 * @brief The DiagramItem class represents a graphical item in a diagram that can be moved, selected, and modified
 *
 * DiagramItem is a base class for various diagram elements such as steps, conditions, start/end points, and I/O elements.
 * It provides functionality for creating different shapes, handling context menus, and managing item properties like
 * position, size, color, and selection state. The class supports serialization to/from JSON format for saving/loading
 * diagrams and implements locking mechanism to prevent movement of items when needed.
 *
 * @details This class inherits from QGraphicsPathItem and provides a foundation for creating diagram elements in a
 * graphical editor application. It supports different diagram types through the DiagramType enum, each with its own
 * visual representation created by the createPath() method. The class handles user interactions like selection and
 * context menu events, and provides methods for copying items, serialization, and locking functionality.
 *
 * @note This class is designed to be used in Qt's graphics framework and requires a QGraphicsScene for proper operation.
 * @warning All diagram items should be added to a scene before they can be displayed or interacted with properly.
 */
class DiagramItem : public QGraphicsPathItem
{
public:
    /**
     * @brief Enum representing different types of diagram elements
     *
     * @details This enum defines the various shapes that can be created using DiagramItem. Each type has its own
     * visual representation generated by the createPath() method. The types include basic flowchart elements like
     * steps, conditions, start/end points, and I/O operations, as well as a None type for uninitialized items.
     *
     * @sa createPath()
     */
    enum { Type = UserType + 15 };
    enum DiagramType { Step, Conditional, StartEnd, Io , None};

    /**
     * @brief Constructs a new diagram item with specified type and context menu
     *
     * @details Creates a diagram item of the specified type with the provided context menu. The item is initialized
     * with appropriate properties for movement, selection, and geometry changes. The visual representation is
     * created using the createPath() method based on the diagram type.
     *
     * @param diagramType Type of diagram element to create (Step, Conditional, StartEnd, Io, or None)
     * @param contextMenu Context menu to display when right-clicking on the item
     * @param parent Parent graphics item (default: nullptr)
     *
     * @note The created item will be movable and selectable by default
     * @sa createPath(), setFlag()
     */
    DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent = nullptr);

    /**
     * @brief Constructs a new diagram item with context menu only (for inheritance)
     *
     * @details This constructor is intended for use in inheritance scenarios where a derived class needs to
     * initialize a DiagramItem without specifying a diagram type. It sets up basic properties but leaves the
     * diagram type as None and does not create a visual path until needed.
     *
     * @param contextMenu Context menu to display when right-clicking on the item
     * @param parent Parent graphics item (default: nullptr)
     *
     * @note This constructor should only be used in inheritance scenarios
     * @sa DiagramItem(DiagramType, QMenu*, QGraphicsItem*)
     */
    DiagramItem(QMenu *contextMenu,
                QGraphicsItem *parent);//constructor fuer Vererbung

    /**
     * @brief Constructs a diagram item from JSON data
     *
     * @details Creates a diagram item by loading its properties from a JSON object. This constructor is used for
     * deserializing diagram items from saved files or data streams. All properties including position, size,
     * colors, transformations, and flags are restored from the JSON data.
     *
     * @param json JSON object containing serialized diagram item data
     * @param contextMenu Context menu to display when right-clicking on the item
     *
     * @note The diagram type must be properly set in the JSON data for correct visual representation
     * @sa write(), createPath()
     */
    DiagramItem(const QJsonObject &json, QMenu *contextMenu);

    /**
     * @brief Copy constructor for diagram items
     *
     * @details Creates a copy of an existing diagram item. The copy includes all properties such as position,
     * size, colors, transformations, and flags but maintains the same diagram type and context menu reference.
     *
     * @param diagram DiagramItem to copy from
     *
     * @note This constructor creates a deep copy of the original item's properties
     * @sa copy()
     */
    DiagramItem(const DiagramItem& diagram);//copy constructor

    /**
     * @brief Creates a copy of this diagram item
     *
     * @details Returns a new instance of DiagramItem that is an exact copy of this object. The returned item
     * will have the same properties, visual representation, and behavior as the original but will be independent
     * in memory.
     *
     * @return Pointer to a new DiagramItem instance with identical properties to this one
     *
     * @note The caller is responsible for deleting the returned object to avoid memory leaks
     * @sa DiagramItem(const DiagramItem&)
     */
    virtual DiagramItem* copy();

    /**
     * @brief Serializes diagram item data to JSON format
     *
     * @details Writes all relevant properties of this diagram item to a JSON object. This method is used for
     * saving diagram items to files or data streams. The serialized data includes position, size, colors,
     * transformations, flags, and diagram type information.
     *
     * @param obj JSON object to write the data to
     *
     * @note This method should be called when saving diagram state to persist item properties
     * @sa DiagramItem(const QJsonObject&, QMenu*)
     */
    virtual void write(QJsonObject &obj);

    /**
     * @brief Gets the type of diagram element this item represents
     *
     * @details Returns the diagram type enum value that determines the visual representation and behavior
     * of this item. This value is set during construction and can be used to determine what kind of element
     * this item represents in the diagram.
     *
     * @return DiagramType enum value representing the type of diagram element
     *
     * @note The returned value corresponds to one of the values defined in the DiagramType enum
     * @sa DiagramType, createPath()
     */
    DiagramType diagramType() const { return myDiagramType; }

    /**
     * @brief Gets the painter path used for rendering this item
     *
     * @details Returns the QPainterPath object that defines the visual shape of this diagram item. This path
     * is created by the createPath() method and represents the geometric structure of the item's visual element.
     *
     * @return QPainterPath object representing the visual shape of this item
     *
     * @note The returned path should not be modified directly as it may affect rendering
     * @sa createPath(), setPath()
     */
    QPainterPath painterPath() const { return mPainterPath; }

    /**
     * @brief Creates a painter path based on the diagram type
     *
     * @details Generates a QPainterPath object that defines the visual shape of this diagram item. The path
     * is created according to the diagram type (Step, Conditional, StartEnd, Io, or None) and represents
     * the geometric structure of the corresponding element in a flowchart or diagram.
     *
     * @return QPainterPath object representing the visual shape for this diagram item
     *
     * @note This method is called during construction to set up the initial visual representation
     * @sa DiagramType, createPath()
     */
    QPainterPath createPath();

    /**
     * @brief Gets a pixmap representation of this diagram item
     *
     * @details Creates and returns a QPixmap that shows a visual representation of this diagram item. This is
     * typically used for creating thumbnails or previews of items in the user interface.
     *
     * @return QPixmap containing a visual representation of this item
     *
     * @note The returned pixmap has a fixed size (250x250 pixels) and includes a black outline of the item's path
     * @sa image()
     */
    QPixmap image() const;

    /**
     * @brief Gets the type identifier for this item class
     *
     * @details Returns the unique type identifier for DiagramItem objects. This is used by Qt's graphics system
     * to identify and handle different types of items in a scene.
     *
     * @return Integer type identifier (DiagramItem::Type)
     *
     * @note This method is required for Qt's item identification system and should not be modified
     * @sa QGraphicsItem::type()
     */
    int type() const override { return Type; }

    /**
     * @brief Sets the bounding box for this diagram item
     *
     * @details Replaces the current painter path with a rectangle of specified dimensions. This method is useful
     * for setting a custom bounding box or for resetting the item to a rectangular shape when needed.
     *
     * @param rect Rectangle defining the new bounding box for this item
     *
     * @note This replaces the existing path with a simple rectangle path
     * @sa setPath(), createPath()
     */
    void setBoundingBox(QRectF rect);

    /**
     * @brief Sets whether this item is locked (prevents movement)
     *
     * @details Enables or disables movement of this diagram item. When an item is locked, it cannot be moved by
     * the user but can still be selected and modified in other ways. This is useful for creating fixed elements
     * in a diagram that should not be repositioned.
     *
     * @param locked True to lock the item (prevent movement), false to unlock it (allow movement)
     *
     * @note Locked items remain selectable but cannot be dragged by the user
     * @sa isLocked()
     */
    void setLocked(bool locked);

    /**
     * @brief Checks if this item is currently locked
     *
     * @details Returns true if this diagram item is locked and cannot be moved, false otherwise. This method
     * provides access to the internal lock state of the item for external code that needs to check movement permissions.
     *
     * @return True if the item is locked, false if it's unlocked and movable
     *
     * @note A locked item can still be selected and modified in other ways (color, size, etc.)
     * @sa setLocked()
     */
    bool isLocked();

protected:
    /**
     * @brief Handles context menu events for this diagram item
     *
     * @details This method is called when a context menu event occurs on this item (typically right-click). It
     * clears the current selection, selects this item, and displays the associated context menu at the event position.
     *
     * @param event Context menu event information
     *
     * @note This method is automatically called by Qt's graphics system when appropriate events occur
     * @sa QGraphicsItem::contextMenuEvent()
     */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    /**
     * @brief Handles item change notifications from the scene
     *
     * @details This method is called when various properties of this item change, such as position or size. It
     * currently returns the value unchanged but can be overridden in derived classes to implement custom behavior
     * for handling item changes.
     *
     * @param change Type of change that occurred
     * @param value New value for the changed property
     *
     * @return The value parameter passed in (default implementation)
     *
     * @note This method is part of Qt's graphics system and is called automatically when item properties change
     * @sa QGraphicsItem::itemChange()
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**
     * @brief Context menu associated with this diagram item
     *
     * @details Pointer to the context menu that should be displayed when right-clicking on this item. This menu
     * is initialized during construction and can contain actions specific to this type of diagram element.
     *
     * @note This is a protected member accessible only within the class hierarchy
     */
    QMenu *myContextMenu;

private:
    /**
     * @brief Type of diagram element this item represents
     *
     * @details Stores the diagram type enum value that determines the visual representation and behavior of this item.
     * This value is set during construction and affects how the item appears in the diagram and what operations are available.
     *
     * @note This is a private member used internally by the class implementation
     */
    DiagramType myDiagramType;

    /**
     * @brief Painter path defining the visual shape of this item
     *
     * @details Stores the QPainterPath object that defines the geometric structure of this diagram item's visual representation.
     * This path is created based on the diagram type and used for rendering the item in the graphics scene.
     *
     * @note This is a private member used internally by the class implementation
     */
    QPainterPath mPainterPath;

    /**
     * @brief Flag indicating if this item is locked (prevents movement)
     *
     * @details When true, prevents this item from being moved by the user. The item remains selectable but cannot be dragged.
     * This flag is managed through setLocked() and isLocked() methods.
     *
     * @note This is a private member used internally by the class implementation
     */
    bool m_isLocked=false; /// locked items can't be moved
};
//! [0]

#endif // DIAGRAMITEM_H
