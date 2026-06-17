/*!
 * \file diagramdrawitem.h
 * \brief This file contains the definition of DiagramDrawItem and Rect classes.
 * \details DiagramDrawItem provides various drawable diagram shapes such as rectangles,
 * ellipses, circles, and other geometric figures. Rect is a helper class for
 * manipulating rectangles with proper handling of negative dimensions.
 */

#ifndef DIAGRAMDRAWITEM_H
#define DIAGRAMDRAWITEM_H

#include <QList>
#include "diagramitem.h"

/*!
 * \brief The Rect class provides a simple representation of a rectangle.
 * \details The rectangle can be directly manipulated on all edges and corners.
 * It keeps anchor and second point, so negative width/height is maintained
 * and handled correctly (unlike QRectF). This allows for proper resizing
 * operations regardless of the rectangle orientation.
 */
class Rect
{
public:
    /*!
     * \brief Default constructor for Rect.
    * \details Creates an uninitialized rectangle with all flags set to false.
     */
    Rect();
    
    /*!
     * \brief Constructs a Rect from a point and selection point.
     * \param pt The second point (anchor is at 0,0).
     * \param selPoint The selection point index (0-7).
     * \details Configures which edge or corner will be manipulated based on selPoint.
     */
    Rect(QPointF pt,int selPoint);
    
    /*!
     * \brief Constructor that creates a rectangle from QRectF.
     * \param rect The QRectF rectangle to convert.
     * \details Anchor point is set to (0,0), point is set to (width, height).
     */
    Rect(QRectF rect);
    
    /*!
     * \brief Get the anchor point of the rectangle.
     * \return The anchor point.
     */
    QPointF anchorPoint() const;
    
    /*!
     * \brief Get the second point of the rectangle.
     * \return The second point.
     */
    QPointF point() const;
    
    /*!
     * \brief Gets the rectangle as QRectF.
     * \return QRectF representation using anchor and point.
     */
    QRectF rect() const;

    /*!
     * \brief Sets the selection point and configures manipulation flags.
     * \param selPoint The selection point index (0-7).
     * \details Configures which edges/corners should move during resize:
     * - 0: top-left corner
     * - 1: top edge
     * - 2: top-right corner
     * - 3: right edge
     * - 4: bottom-left corner
     * - 5: bottom edge
     * - 6: bottom-right corner
     * - 7: left edge
     */
    void setSelPoint(int selPoint);
    
    /*!
     * \brief Configures rectangle to manipulate the left edge.
     * \details Determines which point (anchor or point) should move when resizing the
     * left edge. The point with the smaller x coordinate will be moved.
     */
    void setLeft();
    
    /*!
     * \brief Configures rectangle to manipulate the right edge.
     * \details Determines which point (anchor or point) should move when resizing the
     * right edge. The point with the larger x coordinate will be moved.
     */
    void setRight();
    
    /*!
     * \brief Configures rectangle to manipulate the top edge.
     * \details Determines which point (anchor or point) should move when resizing the top edge.
     * The point with the smaller y coordinate will be moved.
     */
    void setTop();
    
    /*!
     * \brief Configures rectangle to manipulate the bottom edge.
     * \details Determines which point (anchor or point) should move when resizing the bottom edge.
     * The point with the larger y coordinate will be moved.
     */
    void setBottom();
    
    /*!
     * \brief Configures rectangle to manipulate the top-left corner.
     * \details Combines setLeft() and setTop().
     */
    void setTopLeft();
    
    /*!
     * \brief Configures rectangle to manipulate the top-right corner.
     * \details Combines setRight() and setTop().
     */
    void setTopRight();
    
    /*!
     * \brief Configures rectangle to manipulate the bottom-left corner.
     * \details Combines setLeft() and setBottom().
     */
    void setBottomLeft();
    
    /*!
     * \brief Configures rectangle to manipulate the bottom-right corner.
     * \details Combines setRight() and setBottom().
     */
    void setBottomRight();
    
    /*!
     * \brief Moves the point or anchor depending on the current selection configuration.
     * \param pt The new point position.
     * \details Updates either anchor or point coordinates based on which edges
     * are configured to move (m_mvAnchorX, m_mvPointX, m_mvAnchorY, m_mvPointY).
     */
    void movePoint(QPointF pt);

    /*!
     * \brief Translates the rectangle by a given offset.
     * \param pt The translation offset.
     * \details Moves both anchor and point by the same offset, preserving
     * the rectangle's size and orientation.
     */
    void translate(QPointF pt);
private:
    QPointF m_anchor;
    QPointF m_point;
    int m_selPoint;
    bool m_mvPointX,m_mvPointY;
    bool m_mvAnchorX,m_mvAnchorY;
};

/*!
 * \brief The DiagramDrawItem class provides drawable figures like rectangle, ellipse, etc.
 * \details It always has two definition points (pos() and myPos2) which define the
 * dimension of the drawing. The class supports various diagram types and provides
 * interactive resizing through handles. It can maintain partner items for synchronized
 * manipulation in complex scenarios.
 */
class DiagramDrawItem : public DiagramItem
{
public:
    /*! \brief Type identifier for this graphics item type. */
    enum { Type = UserType + 16 };
    
    /*!
     * \brief Enumeration of supported diagram types.
     * \details Defines all available geometric shapes that can be drawn.
     */
    enum DiagramType { 
        Ellipse,      /*!< Ellipse shape */
        Rectangle,    /*!< Rectangle shape */
        Circle,       /*!< Circle shape */
        RoundedRect,  /*!< Rectangle with rounded corners */
        Rhombus,      /*!< Rhombus (diamond) shape */
        Triangle,     /*!< Triangle shape */
        DA,           /*!< DA (special shape) */
        OTA,          /*!< OTA (special shape) */
        Note,         /*!< Note shape with folded corner */
        Pie,          /*!< Pie/arc segment */
        MUX,          /*!< Multiplexer shape */
        DEMUX,        /*!< Demultiplexer shape */
        Square,       /*!< Square shape */
        CirclePie     /*!< Circle pie shape */
    };

    /*!
     * \brief Constructs a DiagramDrawItem with the specified type.
     * \param diagramType The type of diagram to create.
     * \param contextMenu The context menu for right-click operations.
     * \param parent The parent graphics item.
     * \details Initializes the item with default values. The second position
     * is initially set to the item's position, creating a zero-size item.
     * For Note type, the radius is set to 10.0 instead of the default 5.0.
     */
    DiagramDrawItem(DiagramType diagramType, QMenu *contextMenu,
        QGraphicsItem *parent = 0);
    
    /*!
     * \brief Constructs a DiagramDrawItem from JSON data.
     * \param json The JSON object containing diagram item data.
     * \param contextMenu The context menu for right-click operations.
     * \details Restores the diagram item from saved JSON data, including
     * diagram type, dimensions, and start/end points for Pie type diagrams.
     */
    DiagramDrawItem(const QJsonObject &json, QMenu *contextMenu);
    
    /*!
     * \brief Copy constructor for DiagramDrawItem.
     * \param diagram The diagram item to copy from.
     * \details Creates a deep copy of the diagram item, including all properties
     * such as diagram type, radius, start/end points, brush, pen, transform,
     * and position. The partner item is not copied and set to nullptr.
     */
    DiagramDrawItem(const DiagramDrawItem& diagram);

    /*!
     * \brief Create a copy of this diagram item.
     * \return A pointer to the newly created copy.
     */
    DiagramItem* copy() override;
    
    /*!
     * \brief Writes the diagram item data to a JSON object.
     * \param json The JSON object to write to.
     * \details Saves diagram type, dimensions, and start/end points
     * for Pie type diagrams. Calls base class write() first.
     */
    void write(QJsonObject &json) override;

    /*!
     * \brief Get the diagram type.
     * \return The type of diagram.
     */
    DiagramType diagramType() const
        { return myDiagramType; }
    
    /*!
     * \brief Get the painter path for this item.
     * \return The QPainterPath representing the shape.
     */
    QPainterPath path() const
        { return mPainterPath; }

    /*!
    * \brief Generates an image representation of the diagram item.
    * \return QPixmap containing the rendered diagram on transparent background.
    * \details Creates a 250x250 pixel image with the diagram drawn in black
    * with an 8-pixel wide pen. The diagram is offset by 10 pixels from the top-left.
    */
    QPixmap image() const;
    
    /*!
     * \brief Get the type identifier for this graphics item.
     * \return The type identifier (Type).
     */
    int type() const  override
        { return Type;}

    /*!
     * \brief Sets the second position point using individual coordinates.
     * \param x The x coordinate.
     * \param y The y coordinate.
     */
    void setPos2(qreal x,qreal y);

    /*!
     * \brief Sets the second position point (dimension point).
     * \param newPos The new position in scene coordinates.
     * \details Updates the dimension of the diagram. For Circle and Square types,
     * maintains equal width and height by using the larger dimension.
     * Also updates the partner item if one is set.
     */
    void setPos2(QPointF pos);
    
    /*!
     * \brief Get the second position point in parent coordinates.
     * \return The position point in parent coordinates.
     */
    QPointF getPos2() const
        { return mapToParent(myPos2); }

    /*!
     * \brief Sets the dimension of the diagram item.
     * \param newPos The new dimension point in local coordinates.
     * \details Updates the size while preserving aspect ratio for Circle and Square types.
     * Also updates the partner item if one is set.
     */
    void setDimension(QPointF newPos);

    /*!
     * \brief Gets the current dimension of the diagram item.
     * \return The dimension point in local coordinates.
     */
    QPointF getDimension();

    /*!
     * \brief Set the corner radius for rounded rectangles.
     * \param radius The corner radius value.
     */
    void setRadius(const qreal radius)
        { myRadius=radius; }
    
    /*!
     * \brief Get the corner radius for rounded rectangles.
     * \return The corner radius value.
     */
    qreal getRadius()
        { return myRadius; }

    /*!
     * \brief Sets the start point for pie/arc segments.
     * \param pt The start point in local coordinates.
     * \details Updates the start point and regenerates the path for Pie type diagrams.
     * Also updates the partner item if one is set.
     */
    void setStartPoint(const QPointF pt);

    /*!
     * \brief Sets the end point for pie/arc segments.
     * \param pt The end point in local coordinates.
     * \details Updates the end point and regenerates the path for Pie type diagrams.
     * Also updates the partner item if one is set.
     */
    void setEndPoint(const QPointF pt);

    /*!
     * \brief Sets a partner item for synchronized manipulation.
     * \param parterItem The partner item to synchronize with.
     * \details The partner item is the same item, but properly placed in the scene.
     * This item is drawn on top of the scene for visibility to show handlers and covered lines.
     * Surfaces are transparent. Changes on this item are propagated to the partner item.
     */
    void setPartnerItem(DiagramDrawItem *parterItem);
    
    /*!
     * \brief Gets the partner item for synchronized manipulation.
     * \return Pointer to the partner item, or nullptr if none is set.
     */
    DiagramDrawItem *partnerItem();

protected:
    /*!
     * \brief Handles context menu events (right-click).
     * \param event The context menu event.
     * \details Clears scene selection, selects this item, and displays
     * the context menu at the cursor position if a menu is available.
     */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    
    /*!
     * \brief Handles item property changes.
     * \param change The type of change.
     * \param value The new value.
     * \return The value (potentially modified).
     * \details Monitors selection changes and checks if child text items
     * need to be marked as "touched" if they are no longer in their
     * expected center position.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    
    /*!
     * \brief Creates the painter path for the current diagram type.
     * \return QPainterPath representing the shape of the diagram.
     * \details Generates the appropriate path based on the diagram type:
     * - Rectangle/Square: rectangular path
     * - Ellipse/Circle: elliptical path
     * - RoundedRect: rounded rectangle with specified radius
     * - Rhombus: diamond shape
     * - Triangle: triangular path
     * - Pie: arc segment defined by start and end points
     * - MUX/DEMUX: trapezoidal shapes
     * - Note: rectangle with folded corner
     * - DA/OTA: custom polygon shapes
     */
    QPainterPath createPath();
    
    /*!
     * \brief Paints the diagram item.
     * \param painter The painter to use for drawing.
     * \details If a partner item is set, draws only a dashed outline.
     * Otherwise, draws the shape with the configured pen and brush.
     * When selected, draws a dashed bounding rectangle and resize handles.
     * For Pie type, also draws lines from center to start/end points.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    
    /*!
     * \brief Gets the shape of the item for hit testing.
     * \return QPainterPath representing the clickable area.
     * \details Includes the diagram path plus resize handles when selected,
     * allowing handles to be clickable even when small.
     */
    QPainterPath shape() const override;
    
    /*!
     * \brief Gets the bounding rectangle including handles and pen width.
     * \return The bounding rectangle.
     * \details Returns the bounding rect of the actual structure plus helper structures.
     * Helper structures are usually the handles in selected state.
     */
    QRectF boundingRect() const override;
    
    /*!
     * \brief Gets the inner bounding rectangle without extra space for handles.
     * \return The inner bounding rectangle.
     * \details Returns the raw bounding rect of the path without extra space for handlers.
     * For Pie type, returns the bounding rect of the ellipse bounds.
     */
    QRectF innerBoundingRect() const;

    /*!
     * \brief Handles mouse hover enter events.
     * \param e The hover event.
     * \details When the item is selected, checks if the cursor is over
     * any resize handle and updates the hover indicator accordingly.
     */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    
    /*!
     * \brief Handles mouse hover move events.
     * \param e The hover event.
     * \details When the item is selected, checks if the cursor is over
     * any resize handle and updates the hover indicator accordingly.
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;
    
    /*!
     * \brief Handles mouse hover leave events.
     * \param e The hover event.
     * \details Clears the hovered handle indicator when the cursor leaves the item.
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    
    /*!
     * \brief Handles mouse press events.
     * \param e The mouse event.
     * \details When the item is selected and a handle is clicked, initiates
     * resizing by creating a Rect object configured for the selected handle.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    
    /*!
     * \brief Handles mouse move events.
     * \param e The mouse event.
     * \details When a handle is being dragged, updates the dimension or
     * start/end points depending on which handle is selected. Also updates
     * child text items to maintain their relative positions. Updates the
     * partner item if one is set.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
    
    /*!
     * \brief Checks if a point is within click tolerance of another point.
     * \param press_point The point to check (typically mouse position).
     * \param point The reference point (typically handle position).
     * \return True if press_point is within myHandlerWidth of point.
     */
    bool hasClickedOn(QPointF press_point, QPointF point) const ;
    
    /*!
     * \brief Snaps a point to the scene grid.
     * \param pos The position to snap.
     * \return The snapped position.
     * \details Uses the DiagramScene's onGrid() method to align the point.
     */
    QPointF onGrid(QPointF pos);
    
    /*!
     * \brief Internal method to set dimension without updating partner item.
     * \param newPos The new dimension point in local coordinates.
     * \details This is a helper method used internally during resizing operations.
     * For Circle and Square types, maintains equal width and height.
     */
    void mySetDimension(QPointF newPos);


private:
    /*!
     * \brief Gets the position of a resize handle.
     * \param i The handle index (0-9).
     * \return The handle position in local coordinates.
     * \details Handles are numbered clockwise starting from top-left (0):
     * - 0-2: top edge (left, center, right)
     * - 3: right edge (center)
     * - 4-6: bottom edge (left, center, right)
     * - 7: left edge (center)
     * - 8-9: start and end point handles (for Pie type only)
     */
    QPointF getHandler(int i) const;
    
    /*!
     * \brief Gets the number of handles for the current diagram type.
     * \return The number of handles (8 for most types, 10 for Pie).
     */
    int getNumberOfHandles() const;
    DiagramType myDiagramType;
    QPainterPath mPainterPath;
    QPointF myPos2;
    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;
    qreal myRadius;
    QPointF mStartPoint,mEndPoint;
    Rect mRect;
    DiagramDrawItem *m_partnerItem;
};

#endif // DIAGRAMDRAWITEM_H
