#include "diagramimage.h"
#include "diagramscene.h"
#include <QPainter>
#include <QCursor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qbuffer.h>
#include <QGraphicsSceneContextMenuEvent>

DiagramImage::DiagramImage(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent): DiagramItem(contextMenu,parent)
{
    mFileName=fileName;
    mPixmap.load(fileName);
    if(!mPixmap.isNull()){
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
        m_boundingRect= QRectF(0, 0, mPixmap.width(), mPixmap.height());
    }
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
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
        m_boundingRect = QRectF(0, 0, mPixmap.width(), mPixmap.height());
    }
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
}

DiagramImage::DiagramImage(const DiagramImage& diagram): DiagramItem(diagram)
{
    mFileName=diagram.mFileName;
    mPixmap=diagram.mPixmap;
    m_boundingRect = diagram.m_boundingRect;
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
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
/*!
 * \brief set image from QIMage
 * \param img
 */
void DiagramImage::setImage(QImage img)
{
    mPixmap=QPixmap::fromImage(img);
    if(!mPixmap.isNull()){
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}

void DiagramImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(!mPixmap.isNull()){
        QRectF targetRect = boundingRect();
        QRectF sourceRect = mPixmap.rect();
        // Draw the pixmap scaled to fit the bounding rectangle
        // maintain aspect ratio
        // first try to fit the width
        qreal scaleFactor = targetRect.width() / sourceRect.width();
        if(sourceRect.height()*scaleFactor<= targetRect.height()){
            // fits
            // adapt height to maintain aspect ratio
            const qreal h = targetRect.height();
            const qreal hn = sourceRect.height()*scaleFactor;
            targetRect.setHeight(hn);
            targetRect.translate(QPointF(0,(h-hn)/2));
        }else{
            // fits height
            scaleFactor = targetRect.height() / sourceRect.height();
            const qreal w= targetRect.width();
            const qreal wn= sourceRect.width()*scaleFactor;
            targetRect.setWidth(wn);
            targetRect.translate(QPointF((w-wn)/2,0));
        }

        painter->drawPixmap(targetRect, mPixmap, sourceRect);
    }
    // selected
    if(isSelected()){
        // Rect
        QPen selPen=QPen(Qt::DotLine);
        selPen.setWidth(0);
        selPen.setColor(Qt::black);
        QBrush selBrush=QBrush(Qt::NoBrush);
        painter->setBrush(selBrush);
        painter->setPen(selPen);
        painter->drawRect(boundingRect());
        // selected
        if(isSelected()){
            QRectF rect=boundingRect();
            painter->drawRect(rect);
            // Draghandles
            selBrush=QBrush(Qt::cyan,Qt::SolidPattern);
            selPen=QPen(Qt::cyan);
            painter->setBrush(selBrush);
            painter->setPen(selPen);
            QPointF point;
            const int nPoints=8;

            for(int i=0;i<nPoints;i++)
            {
                point=getHandler(i);
                if(i==myHoverPoint){
                    painter->setBrush(QBrush(Qt::red));
                }
                // Rect around valid point
                painter->drawRect(QRectF(point-QPointF(myHandlerWidth,myHandlerWidth),point+QPointF(myHandlerWidth,myHandlerWidth)));
                if(i==myHoverPoint){
                    painter->setBrush(selBrush);
                }
            }// foreach
        }// if
    }// if
}
QRectF DiagramImage::boundingRect() const
{
    if(!mPixmap.isNull()){
        return m_boundingRect.rect();
    }
    return QRectF();
}

QPainterPath DiagramImage::shape() const
{
    QPainterPath path;
    if(!mPixmap.isNull()){
        path.addRect(boundingRect());
    }
    return path;
}

void DiagramImage::hoverMoveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        QPointF hover_point = e -> pos();
        QPointF point;
        for(myHoverPoint=0;myHoverPoint<8;myHoverPoint++){
            point=getHandler(myHoverPoint);
            if(hasClickedOn(hover_point,point)) break;
        }//for
        if(myHoverPoint==8) myHoverPoint=-1;
        else update();
    }
    DiagramItem::hoverMoveEvent(e);
}

void DiagramImage::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    DiagramItem::hoverLeaveEvent(e);
}

bool DiagramImage::hasClickedOn(QPointF press_point, QPointF point) const {
    return (
        press_point.x() >= point.x() - myHandlerWidth &&\
                                                            press_point.x() <  point.x() + myHandlerWidth &&\
              press_point.y() >= point.y() - myHandlerWidth &&\
              press_point.y() <  point.y() + myHandlerWidth
        );
}


QPointF DiagramImage::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}
/*!
 * \brief return position of the stretch handlers
 * \param i number of handler
 * clock-wise, start left top
 * \return
 */
QPointF DiagramImage::getHandler(int i) const
{
    QPointF point;
    QRectF rect=m_boundingRect.rect();

    if(i<3) point=QPointF(rect.left()+rect.width()/2*i,rect.top());
    if(i==3) point=QPointF(rect.right(),rect.bottom()-rect.height()/2);
    if(i>3 && i<7) point=QPointF(rect.left()+rect.width()/2*(i-4),rect.bottom());
    if(i==7) point=QPointF(rect.left(),rect.bottom()-rect.height()/2);
    return point;
}

void DiagramImage::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        prepareGeometryChange();
        if(mySelPoint<8){
            m_boundingRect.movePoint(mouse_point);
            QPointF anchorPoint=m_boundingRect.anchorPoint();
            m_boundingRect.translate(-anchorPoint); // renormalize: anchor is at 0/0, the item is moved instead
            setPos(mapToScene(anchorPoint));
            /*if(m_partnerItem){
                m_partnerItem->set;
            }*/
        }
        e->setAccepted(true);
    }
    else
        DiagramItem::mouseMoveEvent(e);
}

void DiagramImage::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = e -> pos();
            QPointF point;
            for(mySelPoint=0;mySelPoint<8;mySelPoint++){
                point=getHandler(mySelPoint);
                if(hasClickedOn(mouse_point,point)) break;
            }//for
            if(mySelPoint==8){
                mySelPoint=-1;
            }else{
                if(mySelPoint<8){
                    m_boundingRect=Rect(m_boundingRect.point(),mySelPoint);
                }
                e->accept();
            }
        }
    }
    DiagramItem::mousePressEvent(e);
}
