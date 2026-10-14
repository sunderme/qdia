/*!
 * \file diagramimage.h
 * \brief Header file for DiagramImage class
 *
 * This file contains the declaration of the DiagramImage class, which is used to represent
 * image items in a diagram scene. It inherits from DiagramItem and provides functionality
 * for handling images with selection, resizing, and serialization.
 */

#ifndef DIAGRAMIMAGE_H
#define DIAGRAMIMAGE_H

#include "diagramitem.h"
#include "diagramdrawitem.h"

/*!
 * \class DiagramImage
 * \brief Class representing image items in a diagram scene
 *
 * The DiagramImage class extends DiagramItem to provide functionality for displaying
 * and manipulating image items within a diagram. It supports selection, resizing,
 * movement, and serialization of image data. The class handles image loading from files
 * or JSON data, provides visual representation with selection handles, and supports
 * grid-based positioning.
 */
class DiagramImage : public DiagramItem
{
public:
    enum { Type = UserType + 8 };
    enum DiagramType { Image };
    
    /*!
     * \brief Constructor from file name
     * 
     * Creates a new DiagramImage from an image file. The constructor loads the image
     * and sets up the item with appropriate flags for movement, selection, and geometry changes.
     * If the image fails to load, the item will not be enabled for interaction.
     * 
     * \param fileName Path to the image file to load
     * \param contextMenu Context menu that will be shown when right-clicking on the item
     * \param parent Parent graphics item in the scene hierarchy
     */
    DiagramImage(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent = nullptr);
    
    /*!
     * \brief Constructor from JSON object
     * 
     * Creates a new DiagramImage from serialized data stored in a JSON object.
     * This constructor is used for loading saved diagrams. It reconstructs the image
     * from base64 encoded data and restores all item properties.
     * 
     * \param json JSON object containing serialized image data
     * \param contextMenu Context menu that will be shown when right-clicking on the item
     */
    DiagramImage(const QJsonObject &json, QMenu *contextMenu);
    
    /*!
     * \brief Copy constructor
     * 
     * Creates a copy of an existing DiagramImage instance. This constructor performs
     * a deep copy of all image data and properties, creating an independent duplicate.
     * 
     * \param diagram Source DiagramImage to copy from
     */
    DiagramImage(const DiagramImage& diagram);//copy constructor

    /*!
     * \brief Create a copy of this item
     * 
     * Creates and returns a new instance of DiagramImage that is an exact copy
     * of the current object. This method is used for duplicating items in the diagram.
     * 
     * \return Pointer to the newly created DiagramImage instance
     */
    DiagramItem* copy() override;
    
    /*!
     * \brief Serialize image data to JSON object
     * 
     * Writes all relevant properties of this DiagramImage to a JSON object for
     * serialization purposes. The image data is encoded as base64 to ensure
     * proper storage and transmission.
     * 
     * \param obj JSON object to write the image data to
     */
    void write(QJsonObject &obj) override;
    
    /*!
     * \brief Get scaled pixmap of the image
     * 
     * Returns a scaled version of the image pixmap, constrained to 250x250 pixels
     * while maintaining the aspect ratio. This is used for display purposes where
     * a thumbnail or preview size is needed.
     * 
     * \return QPixmap of the image, scaled to maximum 250x250 pixels
     */
    QPixmap image() const;
    
    /*!
     * \brief Get diagram type
     * 
     * Returns the type identifier for this diagram item, which is always DiagramType::Image.
     * This is used by the diagram system to identify and handle different types of items.
     * 
     * \return DiagramType::Image constant
     */
    DiagramType diagramType() const
    { return Image; }
    
    /*!
     * \brief Get item type
     * 
     * Returns the unique type identifier for this item class. This is used by Qt's
     * object system to distinguish between different types of graphics items.
     * 
     * \return Type constant (UserType + 8)
     */
    int type() const override
    { return Type;}
    
    /*!
     * \brief Get item name
     * 
     * Returns the name associated with this image item. This is typically used
     * for identification purposes in the diagram.
     * 
     * \return QString containing the item's name
     */
    QString getName() {
        return mName;
    }
    
    /*!
     * \brief Get file name
     * 
     * Returns the path to the image file that was used to create this item.
     * This is useful for tracking the original source of the image.
     * 
     * \return QString containing the file path
     */
    QString getFileName() {
        return mFileName;
    }
    
    /*!
     * \brief Set image from QImage
     * 
     * Updates the image content of this DiagramImage with a new QImage.
     * This method updates the internal pixmap and sets up appropriate flags
     * for interaction after the image is loaded.
     * 
     * \param img QImage to set as the item's image
     */
    void setImage(QImage img);
protected:
    
    /*!
     * \brief Paint the item
     * 
     * Renders the image item on the graphics scene. This method handles the drawing
     * of the actual image content, selection rectangle, and resize handles when the
     * item is selected. The image is scaled to fit within the bounding rectangle
     * while maintaining its aspect ratio.
     * 
     * \param painter QPainter object for drawing operations
     * \param option Style options for rendering
     * \param widget Widget being painted on (optional)
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    
    /*!
     * \brief Get bounding rectangle of the item
     * 
     * Returns the complete bounding rectangle of the item including any helper
     * structures such as selection handles. This is used by Qt's scene system
     * for collision detection and rendering optimization.
     * 
     * \return QRectF representing the complete bounding rectangle
     */
    QRectF boundingRect() const override;
    
    /*!
     * \brief Get inner bounding rectangle (without handles)
     * 
     * Returns the bounding rectangle of the actual image content without any
     * selection handles or helper structures. This represents the core dimensions
     * of the image itself.
     * 
     * \return QRectF representing the inner bounding rectangle
     */
    QRectF innerBoundingRect() const;
    
    /*!
     * \brief Get shape for hit detection
     * 
     * Returns a QPainterPath that defines the item's clickable area. This includes
     * both the main image area and any selection handles for precise interaction.
     * Used by Qt's scene system for determining which items are clicked or hovered.
     * 
     * \return QPainterPath representing the item's shape for hit detection
     */
    QPainterPath shape() const override;
    
    /*!
     * \brief Handle hover move event
     * 
     * Processes mouse movement events when the cursor is hovering over the item.
     * This method detects when the mouse hovers over selection handles and updates
     * the visual feedback accordingly. It also triggers repaint operations when needed.
     * 
     * \param e QGraphicsSceneHoverEvent containing hover information
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;
    
    /*!
     * \brief Handle hover leave event
     * 
     * Processes mouse leave events when the cursor moves away from the item.
     * This method resets any hover state and triggers a repaint to update visual feedback.
     * 
     * \param e QGraphicsSceneHoverEvent containing hover information
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    
    /*!
     * \brief Handle mouse press event
     * 
     * Processes mouse press events for the item. When the item is selected, this method
     * checks if the click occurred on a selection handle and prepares for resizing operations.
     * If no handle is clicked, it allows normal selection behavior.
     * 
     * \param e QGraphicsSceneMouseEvent containing mouse press information
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    
    /*!
     * \brief Handle mouse move event
     * 
     * Processes mouse movement events when the item is being dragged or resized.
     * This method handles the actual movement of selection handles and updates
     * the item's geometry accordingly during resizing operations.
     * 
     * \param e QGraphicsSceneMouseEvent containing mouse move information
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
    
    /*!
     * \brief Check if point was clicked on
     * 
     * Determines whether a given point (typically from a mouse event) falls within
     * the clickable area of a selection handle. This is used for detecting when
     * a user clicks on a resize handle.
     * 
     * \param press_point Point where mouse was pressed
     * \param point Point to check against (handle position)
     * \return True if the press point falls within the handle's clickable area
     */
    bool hasClickedOn(QPointF press_point, QPointF point) const ;
    
    /*!
     * \brief Snap point to grid
     * 
     * Snaps a given point to the nearest grid position as defined by the diagram scene.
     * This ensures that items are aligned to a consistent grid system for better layout.
     * 
     * \param pos Position to snap to grid
     * \return QPointF representing the snapped position
     */
    QPointF onGrid(QPointF pos);

    /*!
     * \brief Get handler position
     * 
     * Calculates and returns the position of a specific selection handle based on
     * its index. Handles are positioned around the edges of the item for resizing.
     * The handles are arranged clockwise starting from the top-left corner.
     * 
     * \param i Handler number (0-7)
     * \return QPointF representing handler position
     */
    QPointF getHandler(int i) const;

    QString mFileName;
    QString mName;
    QPixmap mPixmap;
    Rect m_boundingRect;

    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;

    DiagramImage *m_partnerItem;
};

#endif // DIAGRAMIMAGE_H
