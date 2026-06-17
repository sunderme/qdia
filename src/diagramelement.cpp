#include "diagramelement.h"
#include <QFile>
#include <QCursor>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @class DiagramElement
 * @brief Represents a graphical element in a diagram loaded from a JSON file.
 *
 * The DiagramElement class inherits from DiagramItem and provides functionality to load
 * and render vector paths from a JSON file. It supports selection, movement, hover effects,
 * and serialization/deserialization of element properties.
 */

/**
 * @brief Constructs a DiagramElement from a JSON file.
 * @param fileName Path to JSON file containing path definitions
 * @param contextMenu Context menu for the item
 * @param parent Parent graphics item
 *
 * Loads paths from the specified JSON file and initializes the element with:
 * - Movable and selectable flags
 * - Geometry change notifications
 * - Hover event acceptance
 */
DiagramElement::DiagramElement(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent): DiagramItem(contextMenu,parent)
{
    mFileName=fileName;
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}

/**
 * @brief Copy constructor.
 * @param diagram Element to copy from
 *
 * Creates a deep copy including:
 * - File name and element name
 * - Path data from original file
 * - Transform properties
 * - Visual style (pen/brush)
 * - Position and transformation matrix
 */
DiagramElement::DiagramElement(const DiagramElement& diagram)
    : DiagramItem(diagram.myContextMenu,diagram.parentItem())
{
    mFileName=diagram.mFileName;
    mName=diagram.mName;
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlags(diagram.flags());
        setAcceptHoverEvents(true);
    }
    setTransform(diagram.transform());
    setPen(diagram.pen());
    setBrush(diagram.brush());
    setPos(diagram.pos());
}

/**
 * @brief Creates a copy of this element.
 * @return Pointer to new DiagramItem copy
 *
 * @see DiagramElement(const DiagramElement&)
 */
DiagramItem* DiagramElement::copy()
{
    DiagramElement* newDiagramElement=new DiagramElement(*this);
    return dynamic_cast<DiagramItem*>(newDiagramElement);
}

/**
 * @brief Serializes element properties to JSON.
 * @param obj JSON object to write to
 *
 * Saves:
 * - Base class properties (via DiagramItem::write())
 * - File name (filename)
 * - Element name (name)
 */
void DiagramElement::write(QJsonObject &obj)
{
    DiagramItem::write(obj);
    obj["filename"]=mFileName;
    obj["name"]=mName;
}

/**
 * @brief Generates a preview image of the element.
 * @return 250x250 pixmap with scaled element representation
 *
 * Features:
 * - Transparent background
 * - Automatic scaling (max 240px per dimension)
 * - Centered position
 * - Path filling according to JSON definitions
 */
QPixmap DiagramElement::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);

    QRectF rect=boundingRect();
    qreal w=rect.width();
    if(w<rect.height()){
        w=rect.height();
    }
    qreal scale=qMin(4.,240/w);
    QPointF center=-rect.center()*scale+QPointF(125,125);

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 1));
    painter.translate(center);
    painter.scale(scale,scale);
    foreach(Path lPath,lstPaths){
        painter.save();
        if(lPath.filled){
            painter.setBrush(pen().color());
        }
        painter.setTransform(lPath.t,true);
        painter.drawPath(lPath.path);
        painter.restore();
    }

    return pixmap;
}

/**
 * @brief Custom painting implementation.
 * @param painter QPainter to use for drawing
 *
 * Handles:
 * - Path filling based on JSON specifications
 * - Selection highlight (black dotted border)
 * - Transformations from path definitions
 */
void DiagramElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    foreach(Path lPath,lstPaths){
        painter->save();
        if(lPath.filled){
            painter->setBrush(pen().color());
        }
        if(lPath.dontFill){
            painter->setBrush(Qt::NoBrush);
        }
        painter->setTransform(lPath.t,true);
        painter->drawPath(lPath.path);
        painter->restore();
    }
    // selected
    if(isSelected()){
        // Rect
        QPen selPen=QPen(Qt::DotLine);
        selPen.setWidth(0);
        selPen.setColor(Qt::black);
        selPen.setBrush(Qt::white);
        QBrush selBrush=QBrush(Qt::NoBrush);
        painter->setBrush(selBrush);
        painter->setPen(selPen);
        painter->drawRect(boundingRect());
    }// if
}

/**
 * @brief Calculates the bounding rectangle.
 * @return Union of all transformed path bounding rectangles
 */
QRectF DiagramElement::boundingRect() const
{
    QRectF rect;
    foreach(const Path &elem,lstPaths){
        QPainterPath path=elem.t.map(elem.path);
        QRectF localRect=path.boundingRect();
        rect=rect.united(localRect);
    }
    return rect;
}

/**
 * @brief Handles hover enter events.
 * @param e Hover event
 *
 * Changes cursor to SizeAllCursor when selected.
 */
void DiagramElement::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::SizeAllCursor);
    }
    DiagramItem::hoverEnterEvent(e);
}

/**
 * @brief Handles hover leave events.
 * @param e Hover event
 *
 * Restores default arrow cursor when selected.
 */
void DiagramElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::ArrowCursor);
    }
    DiagramItem::hoverLeaveEvent(e);
}

/**
 * @brief Loads path definitions from JSON file.
 * @param fn File path to load from
 * @return List of Path objects
 *
 * File format requirements:
 * - Valid JSON structure
 * - Path array with transformation matrices
 * - Fill/dontFill boolean flags
 *
 * @note Emits qWarning() on file read errors
 */
QList<DiagramElement::Path> DiagramElement::importPathFromFile(const QString &fn)
{
    // open and read in text file
    QFile loadFile(fn);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return QList<Path>();
    }
    QByteArray data = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    return createPainterPathFromJSON(loadDoc.object());
}

/**
 * @brief Creates a list of DiagramElement::Path objects from a JSON representation.
 *
 * This function parses a JSON object to construct a list of painter paths for diagram elements.
 * It supports various primitive shapes (rect, circle, line), complex paths (polygon, arc, cubic),
 * text elements, and nested elements from external JSON files.
 *
 * @param json JSON object containing the diagram element specification. Expected structure:
 *             - "name": (string) Element name
 *             - "filled": (bool) Fill flag for main path
 *             - "dontFill": (bool) Fill override flag
 *             - "elements": (array) Array of path components with:
 *               - "type": (string) Element type (rect|circle|line|polygon|arc|text|element etc.)
 *               - Type-specific parameters (coordinates, radii, control points, etc.)
 *
 * @return QList<DiagramElement::Path> List of complete painter paths with transformations applied.
 *         The main path is prepended to the list, followed by additional elements like text.
 *
 * @note JSON structure requirements:
 * - Rectangles require x0, y0, x1, y1
 * - Circles accept either "r" (uniform radius) or "rx"/"ry"
 * - Text elements create separate paths with font handling
 * - "element" type imports external JSON files (from ":/libs/") with scaling/rotation
 * - Coordinate system assumes Y-axis points downward
 *
 * @see QPainterPath, QJsonObject, QTransform
 */
QList<DiagramElement::Path> DiagramElement::createPainterPathFromJSON(QJsonObject json)
{
    QString elementName=json["name"].toString();
    bool filled=json["filled"].toBool();
    bool dontFill=json["dontFill"].toBool();
    QList<DiagramElement::Path> result;
    QPainterPath path;
    QJsonArray array=json["elements"].toArray();
    for (int index = 0; index < array.size(); ++index) {
        QJsonObject jsonObject = array[index].toObject();
        QString type=jsonObject["type"].toString();
        if(type=="rect") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            path.moveTo(QPointF(x0,y0));
            path.addRect(x0,y0,x1-x0,y1-y0);
        }
        if(type=="circle") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal rx,ry;
            if(jsonObject.contains("r")){
                rx=jsonObject["r"].toDouble();
                ry=rx;
            }else{
                rx=jsonObject["rx"].toDouble();
                ry=jsonObject["ry"].toDouble();
            }
            QPointF p_center(x0,y0);
            path.moveTo(p_center);
            path.addEllipse(p_center,rx,ry);
        }
        if(type=="line") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            path.moveTo(x0,y0);
            path.lineTo(x1,y1);
        }
        if(type=="lineTo") {
            qreal x0=jsonObject["x"].toDouble();
            qreal y0=jsonObject["y"].toDouble();
            path.lineTo(x0,y0);
        }
        if(type=="polygon") {
            QVector<QPointF>lst;
            QJsonArray jsonPoints=jsonObject["points"].toArray();
            for(int i=0;i<jsonPoints.size();++i){
                QJsonObject jsonElement=jsonPoints[i].toObject();
                qreal x=jsonElement["x"].toDouble();
                qreal y=jsonElement["y"].toDouble();
                lst<<QPointF(x,y);
            }
            QPolygonF polygon{lst};
            path.moveTo(lst.first());
            path.addPolygon(polygon);
        }
        if(type=="lines") {
            QList<QPointF>lst;
            QJsonArray jsonPoints=jsonObject["points"].toArray();
            for(int i=0;i<jsonPoints.size();++i){
                QJsonObject jsonElement=jsonPoints[i].toObject();
                qreal x=jsonElement["x"].toDouble();
                qreal y=jsonElement["y"].toDouble();
                lst<<QPointF(x,y);
            }
            for(int i=1;i<lst.length();++i){
                path.moveTo(lst.at(i-1));
                path.lineTo(lst.at(i));
            }
        }
        if(type=="arc") {
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal rx=jsonObject["rx"].toDouble();
            qreal ry=jsonObject["ry"].toDouble();
            qreal angle=jsonObject["angle"].toDouble();
            qreal length=jsonObject["length"].toDouble();
            path.arcMoveTo(x-rx,y-ry,2*rx,2*ry,angle);
            path.arcTo(x-rx,y-ry,2*rx,2*ry,angle,length);
        }
        if(type=="arcTo") {
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal rx=jsonObject["rx"].toDouble();
            qreal ry=jsonObject["ry"].toDouble();
            qreal angle=jsonObject["angle"].toDouble();
            qreal length=jsonObject["length"].toDouble();
            path.arcTo(x-rx,y-ry,2*rx,2*ry,angle,length);
        }
        if(type=="quad") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx=jsonObject["cx"].toDouble();
            qreal cy=jsonObject["cy"].toDouble();
            path.moveTo(x0,y0);
            path.quadTo(cx,cy,x1,y1);
        }
        if(type=="cubic") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx0=jsonObject["cx0"].toDouble();
            qreal cy0=jsonObject["cy0"].toDouble();
            qreal cx1=jsonObject["cx1"].toDouble();
            qreal cy1=jsonObject["cy1"].toDouble();
            path.moveTo(x0,y0);
            path.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
        if(type=="cubicTo") {
            qreal x1=jsonObject["x1"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx0=jsonObject["cx0"].toDouble();
            qreal cy0=jsonObject["cy0"].toDouble();
            qreal cx1=jsonObject["cx1"].toDouble();
            qreal cy1=jsonObject["cy1"].toDouble();
            path.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
        if(type=="close") {
            path.closeSubpath();
        }
        if(type=="text"){
            Path localPath;
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            QString text=jsonObject["text"].toString();
            qreal sz=jsonObject["size"].toDouble(10.);
            QFont serifFont("Helvetica", sz);
            localPath.path.addText(x,y,serifFont,text);
            localPath.filled=true;
            result<<localPath;
        }
        if(type=="element"){
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal scale=jsonObject["scale"].toDouble(1.);
            qreal rotate=jsonObject["rotate"].toDouble(0.);
            QString fn=jsonObject["name"].toString();
            fn=":/libs/"+fn;
            if(!fn.endsWith(".json")){
                fn+=".json";
            }
            QList<Path> local=importPathFromFile(fn);
            for(Path &elem:local){
                elem.t.translate(x,y);
                elem.t.scale(scale,scale);
                elem.t.rotate(rotate);
            }
            result<<local;
        }
    }
    Path p;
    p.path=path;
    p.filled=filled;
    p.dontFill=dontFill;
    result.prepend(p);
    mName=elementName;
    return result;
}

/**
 * @class DiagramElement
 * @brief A graphical element representing a diagram item constructed from JSON data and file paths.
 *
 * This class inherits from DiagramItem and extends its functionality by loading additional path data
 * from a specified file. The element becomes movable and interactive when valid path data is loaded.
 */

/**
 * @brief Constructs a DiagramElement from JSON data and a context menu.
 *
 * @param json QJsonObject containing the configuration data with:
 *             - "filename": String path to the file containing vector path data
 *             - "name": String identifier for this element
 * @param contextMenu Pointer to the context menu to use for this item
 * @throw May throw exceptions related to file I/O operations during path import
 *
 * @note The item becomes movable and interactive only if valid path data is successfully loaded.
 *
 * The constructor:
 * 1. Extracts filename and name from JSON input
 * 2. Imports vector paths from the specified file using importPathFromFile()
 * 3. If paths are found:
 *    - Combines all paths into a single QPainterPath
 *    - Enables item movement capabilities
 *    - Enables selection highlighting
 *    - Enables geometry change tracking
 *    - Activates hover event handling
 */
DiagramElement::DiagramElement(const QJsonObject &json, QMenu *contextMenu):DiagramItem(json,contextMenu)
{
    mFileName=json["filename"].toString();
    mName=json["name"].toString();
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}
