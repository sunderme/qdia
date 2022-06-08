#include "diagramdrawitem.h"

#include <QtGui>
#include <math.h>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "diagramdrawitem.h"
#include "diagramscene.h"

//! [0]
DiagramDrawItem::DiagramDrawItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent)
    : DiagramItem(contextMenu,parent)
{
    myPos2=pos();
    myDiagramType = diagramType;
    myContextMenu = contextMenu;
    myRadius=5.0;

    mPainterPath=createPath();
    setPath(mPainterPath);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
    myRadius=5.0;
}
//! [0]
DiagramDrawItem::DiagramDrawItem(const DiagramDrawItem& diagram)
    : DiagramItem(diagram.myContextMenu,diagram.parentItem())
{

    myDiagramType=diagram.myDiagramType;
    myRadius=diagram.myRadius;
    // copy from general GraphcsItem
    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());
    myPos2=diagram.myPos2;
    mPainterPath=createPath();
    setPath(mPainterPath);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setPos(diagram.pos());
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;

}
DiagramDrawItem::DiagramDrawItem(const QJsonObject &json, QMenu *contextMenu):DiagramItem(json,contextMenu)
{
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt());
    qreal width=json["width"].toDouble();
    qreal height=json["height"].toDouble();
    myPos2=QPointF(width,height);
    myRadius=5.0;

    mPainterPath=createPath();
    setPath(mPainterPath);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;

}
//! [1]
QPainterPath DiagramDrawItem::createPath()
{
    qreal dx=myPos2.x();
    qreal dy=myPos2.y();

    QPainterPath path;
    switch (myDiagramType) {
    case Rectangle:
        path.moveTo(0, 0);
        path.lineTo(dx,0);
        path.lineTo(dx,dy);
        path.lineTo(0,dy);
        path.lineTo(0,0);
        break;
    case Ellipse:
        path.addEllipse(0,0,dx,dy);
        break;
    case Circle:
        path.addEllipse(0,0,dx,dy);
        break;
    case RoundedRect:
    {
        Qt::SizeMode sizeMode=Qt::AbsoluteSize;
        // circumvent problem when radius larger than width/height
        qreal r=myRadius;
        if ((fabs(dx)<3*myRadius)or(fabs(dy)<3*myRadius)) {
            r=0;
        }
        path.addRoundedRect(0,0,dx,dy,r,r,sizeMode);
        break;
    }
    case Rhombus:
        path.moveTo(dx/2, 0);
        path.lineTo(dx,dy/2);
        path.lineTo(dx/2,dy);
        path.lineTo(0,dy/2);
        path.lineTo(dx/2,0);
        break;
    case Triangle:
        path.moveTo(0, 0);
        path.lineTo(0,dy);
        path.lineTo(dx,dy/2);
        path.lineTo(0,0);
        break;
    case DA:
        path.moveTo(0, 0);
        path.lineTo(dx*0.66,0);
        path.lineTo(dx,dy/2);
        path.lineTo(dx*0.66,dy);
        path.lineTo(0,dy);
        path.lineTo(0,0);
        break;
    default:
        break;
    }
    return path;
}

QPixmap DiagramDrawItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(10, 10);
    painter.drawPath(mPainterPath);

    return pixmap;
}

void DiagramDrawItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if(myContextMenu){
        myContextMenu->exec(event->screenPos());
    }
}

QVariant DiagramDrawItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        ;
    }

    return value;
}

DiagramItem* DiagramDrawItem::copy()
{
    DiagramDrawItem* newDiagramDrawItem=new DiagramDrawItem(*this);
    return dynamic_cast<DiagramItem*>(newDiagramDrawItem);
}

void DiagramDrawItem::write(QJsonObject &json)
{
    DiagramItem::write(json);
    json["diagramtype"]=static_cast<int>(myDiagramType);
    json["width"]=myPos2.x();
    json["height"]=myPos2.y();

}

void DiagramDrawItem::setPos2(qreal x,qreal y)
{
    setPos2(QPointF(x,y));
}

void DiagramDrawItem::setPos2(QPointF newPos)
{
    prepareGeometryChange();
    myPos2=mapFromScene(newPos);
    if(myDiagramType==Circle){
        // special treatment in case of circle
        if(fabs(myPos2.x())>fabs(myPos2.y())){
            if(myPos2.y()<0) myPos2.setY(-fabs(myPos2.x()));
            else myPos2.setY(fabs(myPos2.x()));
        }
        else {
            if(myPos2.x()<0) myPos2.setX(-fabs(myPos2.y()));
            else myPos2.setX(fabs(myPos2.y()));
        }
    }
    mPainterPath=createPath();
    setPath(mPainterPath);
}

void DiagramDrawItem::setDimension(QPointF newPos)
{
    prepareGeometryChange();
    mySetDimension(newPos);
    mPainterPath=createPath();
    setPath(mPainterPath);
}

void DiagramDrawItem::mySetDimension(QPointF newPos)
{
    if(myDiagramType==Circle){
        // special treatment in case of circle
        if(myPos2.x()!=newPos.x()){
            myPos2=newPos;
            if(myPos2.y()<0) myPos2.setY(-fabs(myPos2.x()));
                        else myPos2.setY(fabs(myPos2.x()));
            }
        else {
            myPos2=newPos;
            if(myPos2.x()<0) myPos2.setX(-fabs(myPos2.y()));
            else myPos2.setX(fabs(myPos2.y()));
        }
    }
    else myPos2=newPos;
}

QPointF DiagramDrawItem::getDimension()
{
    return myPos2;
}

void DiagramDrawItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
           QWidget *)
{
     painter->setPen(pen());
     painter->setBrush(brush());
     painter->drawPath(path());
     // selected
     if(isSelected()){
         // Rect
         QPen selPen=QPen(Qt::DashLine);
         selPen.setColor(Qt::black);
         QBrush selBrush=QBrush(Qt::NoBrush);
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         painter->drawRect(QRectF(QPointF(0,0),myPos2));
         // Draghandles
         selBrush=QBrush(Qt::cyan,Qt::SolidPattern);
         selPen=QPen(Qt::cyan);
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         QPointF point;
         for(int i=0;i<8;i++)
         {
             if(i<3) point=QPointF(myPos2.x()/2*i,0);
             if(i==3) point=QPointF(myPos2.x(),myPos2.y()/2);
             if(i>3 && i<7) point=QPointF(myPos2.x()/2*(i-4),myPos2.y());
             if(i==7) point=QPointF(0,myPos2.y()/2);
             if(i==myHoverPoint){
                 painter->setBrush(QBrush(Qt::red));
             }
             // Rect around valid point
             painter->drawRect(QRectF(point-QPointF(2,2),point+QPointF(2,2)));
             if(i==myHoverPoint){
                 painter->setBrush(selBrush);
             }
         }// foreach
     }// if
}

void DiagramDrawItem::hoverMoveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        QPointF hover_point = e -> pos();
        QPointF point;
        for(myHoverPoint=0;myHoverPoint<8;myHoverPoint++){
            if(myHoverPoint<3) point=QPointF(myPos2.x()/2*myHoverPoint,0);
            if(myHoverPoint==3) point=QPointF(myPos2.x(),myPos2.y()/2);
            if(myHoverPoint>3 && myHoverPoint<7) point=QPointF(myPos2.x()/2*(myHoverPoint-4),myPos2.y());
            if(myHoverPoint==7) point=QPointF(0,myPos2.y()/2);
            if(hasClickedOn(hover_point,point)) break;
        }//for
        if(myHoverPoint==8) myHoverPoint=-1;
        else update();
    }
    DiagramItem::hoverMoveEvent(e);
}

void DiagramDrawItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    DiagramItem::hoverLeaveEvent(e);
}

bool DiagramDrawItem::hasClickedOn(QPointF press_point, QPointF point) const {
    return (
        press_point.x() >= point.x() - myHandlerWidth &&\
        press_point.x() <  point.x() + myHandlerWidth &&\
        press_point.y() >= point.y() - myHandlerWidth &&\
        press_point.y() <  point.y() + myHandlerWidth
    );
}

QPointF DiagramDrawItem::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}

QPainterPath DiagramDrawItem::shape() const {
    QPainterPath myPath;
    myPath=path();
    if(isSelected()){
        QPointF point;
        for(int i=0;i<8;i++)
        {
            if(i<3) point=QPointF(myPos2.x()/2*i,0);
            if(i==3) point=QPointF(myPos2.x(),myPos2.y()/2);
            if(i>3 && i<7) point=QPointF(myPos2.x()/2*(i-4),myPos2.y());
            if(i==7) point=QPointF(0,myPos2.y()/2);
            // Rect around valid point
            myPath.addRect(QRectF(point-QPointF(myHandlerWidth,myHandlerWidth),point+QPointF(myHandlerWidth,myHandlerWidth)));
        }// for
    }// if
    return myPath;
}

QRectF DiagramDrawItem::boundingRect() const
{
    qreal extra = pen().width()+20 / 2.0 + myHandlerWidth;
    qreal minx = myPos2.x() < 0 ? myPos2.x() : 0;
    qreal maxx = myPos2.x() < 0 ? 0 : myPos2.x() ;
    qreal miny = myPos2.y() < 0 ? myPos2.y() : 0;
    qreal maxy = myPos2.y() < 0 ? 0 : myPos2.y() ;

    QRectF newRect = QRectF(minx,miny,maxx-minx,maxy-miny)
    .adjusted(-extra, -extra, extra, extra);
    return newRect;
}

void DiagramDrawItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = e -> pos();
            QPointF point;
            for(mySelPoint=0;mySelPoint<8;mySelPoint++){
                if(mySelPoint<3) point=QPointF(myPos2.x()/2*mySelPoint,0);
                if(mySelPoint==3) point=QPointF(myPos2.x(),myPos2.y()/2);
                if(mySelPoint>3 && mySelPoint<7) point=QPointF(myPos2.x()/2*(mySelPoint-4),myPos2.y());
                if(mySelPoint==7) point=QPointF(0,myPos2.y()/2);
                if(hasClickedOn(mouse_point,point)) break;
            }//for
            if(mySelPoint==8) mySelPoint=-1;
            else e->accept();
        }
    }
    DiagramItem::mousePressEvent(e);
}

void DiagramDrawItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        prepareGeometryChange();
        switch (mySelPoint) {
            case 0:
                mySetDimension(myPos2-mouse_point);
                setPos(mapToScene(mouse_point));
                break;
            case 1:
                setPos(pos().x(),mapToScene(mouse_point).y());
                mySetDimension(QPointF(myPos2.x(),(myPos2.y()-mouse_point.y())));
                break;
            case 2:
                setPos(pos().x(),mapToScene(mouse_point).y());
                mySetDimension(QPointF(mouse_point.x(),myPos2.y()-mouse_point.y()));
                break;
            case 3:
                mySetDimension(QPointF(mouse_point.x(),myPos2.y()));
                break;
            case 6:
                mySetDimension(mouse_point);
                break;
            case 5:
                mySetDimension(QPointF(myPos2.x(),mouse_point.y()));
                break;
            case 4:
                mySetDimension(QPointF(myPos2.x()-mouse_point.x(),mouse_point.y()));
                setPos(mapToScene(mouse_point).x(),pos().y());
                break;
            case 7:
                setPos(mapToScene(mouse_point).x(),pos().y());
                mySetDimension(QPointF(myPos2.x()-mouse_point.x(),myPos2.y()));
                break;
            default:
                break;
        }
        mPainterPath=createPath();
        setPath(mPainterPath);
        // update text position if present
        // currently only center
        for(auto *i:childItems()){
            DiagramTextItem *textItem=qgraphicsitem_cast<DiagramTextItem*>(i);
            if(textItem){
                textItem->setCorrectedPos(boundingRect().center());
            }
        }
        e->setAccepted(true);
    }
    else
        DiagramItem::mouseMoveEvent(e);
}
