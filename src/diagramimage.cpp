#include "diagramimage.h"
#include <QPainter>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qbuffer.h>

DiagramImage::DiagramImage(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent): DiagramItem(contextMenu,parent)
{
    mFileName=fileName;
    mPixmap.load(fileName);
    if(!mPixmap.isNull()){
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}

DiagramImage::DiagramImage(const QJsonObject &json, QMenu *contextMenu): DiagramItem(json, contextMenu)
{
    mFileName=json["fileName"].toString();
    // load the pixmap from the base64 string
    const QByteArray byteArray = QByteArray::fromBase64(json["imageData"].toString().toLatin1());
    mPixmap.loadFromData(byteArray, "PNG");
    // Check if the pixmap was loaded successfully
    if(!mPixmap.isNull()){
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}

DiagramImage::DiagramImage(const DiagramImage& diagram): DiagramItem(diagram)
{
    mFileName=diagram.mFileName;
    mPixmap=diagram.mPixmap;
}
DiagramItem* DiagramImage::copy()
{
    return new DiagramImage(*this);
}
/*!
 * \brief write image data to json object
 * \param obj
 */
void DiagramImage::write(QJsonObject &obj)
{
    DiagramItem::write(obj);
    obj["fileName"] = mFileName;
    // Save the pixmap as a base64 string
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    mPixmap.save(&buffer, "PNG");
    obj["imageData"] = QString::fromLatin1(byteArray.toBase64());
}
QPixmap DiagramImage::image() const
{
    // Returns the pixmap of the image, 250x250 pixels
    const int w=mPixmap.width();
    const int h=mPixmap.height();
    if(w>250 || h>250){
        return mPixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return mPixmap;
}

void DiagramImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(!mPixmap.isNull()){
        painter->drawPixmap(boundingRect(), mPixmap, mPixmap.rect());
    }
}
QRectF DiagramImage::boundingRect() const
{
    if(!mPixmap.isNull()){
        return QRectF(0, 0, mPixmap.width(), mPixmap.height());
    }
    return QRectF();
}

/*!
 * \brief change cursor when move is feasible
 * \param e
 */
void DiagramImage::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::SizeAllCursor);
    }
    DiagramItem::hoverEnterEvent(e);
}
/*!
 * \brief change cursor back to default
 * \param e
 */
void DiagramImage::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::ArrowCursor);
    }
    DiagramItem::hoverLeaveEvent(e);
}
