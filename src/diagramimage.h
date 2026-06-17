#ifndef DIAGRAMIMAGE_H
#define DIAGRAMIMAGE_H

/*!
 * @file diagramimage.h
 * @brief Header file for the DiagramImage class.
 *
 * This file declares the DiagramImage class, which represents a raster image
 * item in a diagram scene. It supports resizing with aspect ratio preservation,
 * interaction via resize handles, and serialization to/from JSON.
 */

#include "diagramitem.h"
#include "diagramdrawitem.h"

/*!
 * @class DiagramImage
 * @brief Graphics item that displays and resizes a raster image on a diagram scene.
 *
 * DiagramImage is a QGraphicsItem subclass used to render raster images within
 * a diagram editor. The item maintains an internal pixmap, draws it within a
 * resizable rectangle while preserving the original aspect ratio, and provides
 * interactive resizing via draggable handles.
 *
 * Key features:
 * - Aspect-ratio-preserving scaling.
 * - Resize handles for interactive manipulation.
 * - JSON serialization/deserialization (including base64-encoded PNG data).
 * - Grid snapping for precise positioning.
 * - Support for context menus and partner-item synchronization.
 *
 * The class inherits from DiagramItem, providing integration with the broader
 * diagram element system.
 */
class DiagramImage : public DiagramItem
{
public:
    /*! @brief Runtime Qt graphics item type identifier. */
    enum { Type = UserType + 8 };

    /*! @brief Logical diagram element type. */
    enum DiagramType { Image };

    /*!
     * @brief Construct an image item from a file path.
     *
     * Creates a DiagramImage loaded from the specified file. The image is
     * loaded into an internal pixmap and scaled to fit the initial bounding
     * rectangle while preserving aspect ratio.
     *
     * @param fileName Source image file path. Must point to a valid image file
     *                 supported by Qt (e.g., PNG, JPG).
     * @param contextMenu Context menu used by the base diagram item. Owned by
     *                    the caller; the item does not take ownership.
     * @param parent Optional parent graphics item. If provided, the item will
     *               become a child of this parent in the graphics scene hierarchy.
     */
    DiagramImage(const QString &fileName, QMenu *contextMenu, QGraphicsItem *parent = nullptr);

    /*!
     * @brief Construct an image item from serialized JSON.
     *
     * Reconstructs a DiagramImage from a JSON object containing metadata and
     * image data. The JSON must include a base64-encoded PNG payload and
     * associated properties such as position, size, and filename.
     *
     * @param json JSON object containing image metadata and base64-encoded
     *             PNG payload.
     * @param contextMenu Context menu used by the base diagram item. Owned by
     *                    the caller; the item does not take ownership.
     */
    DiagramImage(const QJsonObject &json, QMenu *contextMenu);

    /*!
     * @brief Copy constructor.
     *
     * Constructs a new DiagramImage as a deep copy of the source item. All
     * internal state, including the pixmap, geometry, and interaction state,
     * is duplicated.
     *
     * @param diagram Source item to copy.
     */
    DiagramImage(const DiagramImage &diagram);

    /*!
     * @brief Create a heap-allocated copy of this item.
     *
     * Returns a dynamically allocated copy of the current DiagramImage. The
     * caller is responsible for deleting the returned pointer.
     *
     * @return New DiagramItem instance with copied state.
     */
    DiagramItem* copy() override;

    /*!
     * @brief Serialize this item to JSON.
     *
     * Writes the item's state into the provided JSON object. This includes
     * metadata (position, size, filename, name) and the image data encoded
     * as a base64 string representing a PNG.
     *
     * @param obj Destination JSON object to populate with the item's data.
     */
    void write(QJsonObject &obj) override;

    /*!
     * @brief Return the image preview pixmap.
     *
     * Returns either the original pixmap or a scaled preview, whichever is
     * smaller, with dimensions capped at 250x250 pixels while preserving
     * aspect ratio. Useful for thumbnails or list views.
     *
     * @return Original pixmap or a scaled preview up to 250x250.
     */
    QPixmap image() const;

    /*!
     * @brief Return the logical diagram item type.
     *
     * Identifies the item as an image element within the diagram system.
     *
     * @return DiagramType::Image.
     */
    DiagramType diagramType() const
    { return Image; }

    /*!
     * @brief Return the Qt graphics item type.
     *
     * Provides the unique type identifier used by Qt's QGraphicsItem type()
     * mechanism for dynamic casting.
     *
     * @return DiagramImage::Type.
     */
    int type() const override
    { return Type; }

    /*!
     * @brief Return item display name.
     *
     * Retrieves the human-readable name of the item, typically set during
     * construction or deserialization.
     *
     * @return Stored display name.
     */
    QString getName() const
    { return mName; }

    /*!
     * @brief Return source file name of the image.
     *
     * Returns the path to the original image file, if available. This may
     * be empty if the item was constructed from raw image data or JSON
     * without a file reference.
     *
     * @return Source image file path.
     */
    QString getFileName() const
    { return mFileName; }

    /*!
     * @brief Replace the current image with a new one.
     *
     * Updates the internal pixmap with the provided QImage. The new image
     * is converted to a pixmap for efficient rendering and the item's
     * geometry is adjusted to match the new image dimensions.
     *
     * @param img Source image to set.
     */
    void setImage(const QImage &img);

protected:
    /*!
     * @brief Paint the image, selection frame, and resize handles.
     *
     * Renders the item on the given painter. This includes drawing the
     * pixmap within its bounding rectangle, a selection outline if the
     * item is selected, and resize handles if in edit mode.
     *
     * @param painter The QPainter used for rendering.
     * @param option  Unused style option (provided by Qt for compatibility).
     * @param widget  Unused widget pointer (provided by Qt for compatibility).
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    /*!
     * @brief Return the outer bounding rectangle including resize handles.
     *
     * Defines the total area occupied by the item, including any visible
     * resize handles. Used by Qt for redraw operations and hit testing.
     *
     * @return Bounding rectangle used by Qt for redraw and hit testing.
     */
    QRectF boundingRect() const override;

    /*!
     * @brief Return the image-only bounding rectangle (without handle margin).
     *
     * Returns the rectangle that strictly bounds the image content, excluding
     * the extra space occupied by resize handles. Useful for layout calculations
     * or internal geometry operations.
     *
     * @return Internal image rectangle.
     */
    QRectF innerBoundingRect() const;

    /*!
     * @brief Return precise hit-test shape including resize handles when selected.
     *
     * Defines the exact shape used for mouse interaction, including handles
     * when the item is in edit mode. Allows for accurate click detection even
     * for non-rectangular handle arrangements.
     *
     * @return Item interaction shape.
     */
    QPainterPath shape() const override;

    /*!
     * @brief Update hovered resize handle on mouse move.
     *
     * Tracks mouse movement over the item to determine if the cursor is
     * hovering over a resize handle. Updates internal state accordingly.
     *
     * @param e Event containing mouse position and related data.
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;

    /*!
     * @brief Clear hovered resize handle when cursor leaves the item.
     *
     * Resets the hover state when the mouse exits the item's area, ensuring
     * no handle remains highlighted incorrectly.
     *
     * @param e Event containing mouse position and related data.
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;

    /*!
     * @brief Select resize handle when mouse is pressed on a handle.
     *
     * Detects if the mouse click occurred on a resize handle and sets the
     * appropriate handle as the active (selected) one for dragging.
     *
     * @param e Event containing mouse button and position information.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

    /*!
     * @brief Resize the item while dragging a selected handle.
     *
     * Handles continuous mouse movement during a drag operation. Adjusts
     * the item's size based on the selected handle and enforces aspect ratio
     * constraints if configured.
     *
     * @param e Event containing updated mouse position and button state.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;

    /*!
     * @brief Check whether a point hits a handle hotspot.
     *
     * Determines if a given point (typically the mouse position) falls within
     * the clickable area of a specific resize handle.
     *
     * @param press_point Tested point, usually mouse position.
     * @param point       Handle center point.
     * @return True if the tested point is inside the handle square.
     */
    bool hasClickedOn(const QPointF &press_point, const QPointF &point) const;

    /*!
     * @brief Snap position to scene grid.
     *
     * Aligns a given coordinate to the nearest grid point defined by the
     * scene's grid spacing. Used to ensure items snap to a regular grid for
     * consistent layout.
     *
     * @param pos Input position to snap.
     * @return Grid-aligned position.
     */
    QPointF onGrid(const QPointF &pos);

    /*!
     * @brief Return the position of a resize handle.
     *
     * Calculates the center point of a specific resize handle based on its
     * index. Handles are ordered clockwise starting from the top-left corner.
     *
     * @param i Handle index in clockwise order starting from top-left (0–7 or similar).
     * @return Handle center point in scene coordinates.
     */
    QPointF getHandler(int i) const;

private:
    QString mFileName;
    QString mName;
    QPixmap mPixmap;
    Rect m_boundingRect;
    int myHoverPoint;
    int mySelPoint;
    qreal myHandlerWidth;
    DiagramImage *m_partnerItem;
};

#endif // DIAGRAMIMAGE_H
