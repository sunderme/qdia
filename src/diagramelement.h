/**
 * @file diagramelement.h
 * @brief Declaration of the DiagramElement class and related structures.
 */

#ifndef DIAGRAMELEMENT_H
#define DIAGRAMELEMENT_H

#include "diagramitem.h"

/**
 * @class DiagramElement
 * @brief Represents a graphical element in a diagram with path-based rendering.
 *
 * Inherits from DiagramItem and adds capabilities for loading vector paths from files,
 * JSON serialization, and hover effects. Supports copy operations and custom painting.
 */
class DiagramElement : public DiagramItem
{
public:
    /// @name Type Identifiers
    /// @{
    enum { Type = UserType + 32 };  /**< Unique QGraphicsItem type identifier */
    enum DiagramType { Element };   /**< Diagram element type classification */
    /// @}

    /**
     * @brief Constructs a DiagramElement from a file
     * @param fileName Path to SVG file containing vector data
     * @param contextMenu Context menu pointer for item interactions
     * @param parent Parent graphics item
     */
    DiagramElement(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent = nullptr);

    /**
     * @brief Constructs a DiagramElement from JSON data
     * @param json JSON object containing serialized element data
     * @param contextMenu Context menu pointer for item interactions
     */
    DiagramElement(const QJsonObject &json, QMenu *contextMenu);

    /**
     * @brief Copy constructor
     * @param diagram Element to copy from
     */
    DiagramElement(const DiagramElement& diagram);//copy constructor

    /// @name Core Functionality
    /// @{
    /**
     * @brief Creates a deep copy of the element
     * @return New DiagramElement instance with copied data
     * @override
     */
    DiagramItem* copy() override;

    /**
     * @brief Serializes element to JSON format
     * @param obj JSON object to write data to
     * @override
     */
    void write(QJsonObject &obj) override;
    // @}

    /// @name Visual Properties
    /// @{
    /**
     * @brief Generates pixmap representation of the element
     * @return Rendered pixmap at default size
     */
    QPixmap image() const;

    /**
     * @brief Returns element type classification
     * @return Always returns DiagramType::Element
     */
    DiagramType diagramType() const { return Element; }

        /**
     * @brief Returns QGraphicsItem type identifier
     * @return int Type identifier from DiagramElement::Type
     * @override
     */
    int type() const override { return Type;}
    /// @}

    /// @name Data Access
    /// @{
    /**
     * @brief Gets user-defined element name
     * @return QString Current element name
     */
    QString getName() {return mName;}

    /**
     * @brief Gets source file path
     * @return QString Original file path used for creation
     */
    QString getFileName() {return mFileName;}
    /// @}

protected:
    /**
     * @struct Path
     * @brief Container for path data and rendering parameters
     */
    struct Path {
        QPainterPath path;      /**< Geometric path data */
        bool filled=false;      /**< Fill path flag */
        bool dontFill=false;    /**< Explicit no-fill flag */
        QTransform t;           /**< Transformation matrix */
    };
    /// @name Rendering Implementation
    /// @{
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;
    /// @}

    /// @name Interaction Handling
    /// @{
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    /// @}

    QString mFileName;      /**< Source file path */
    QString mName;          /**< User-defined element name */
    QList<Path> lstPaths;   /**< Collection of paths for rendering */

    /// @name Path Loading Utilities
    /// @{
    QList<Path> importPathFromFile(const QString &fn);
    QList<Path> createPainterPathFromJSON(QJsonObject json);
    /// @}
};

#endif // DIAGRAMELEMENT_H
