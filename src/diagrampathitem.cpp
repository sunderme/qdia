#include <QtGui>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QJsonObject>

#include "diagrampathitem.h"
#include "diagramscene.h"

DiagramPathItem::DiagramPathItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    myDiagramType = diagramType;
    myRoutingType = free;
    myContextMenu = contextMenu;
    myPoints.clear();

    len = 10.0; // arrow length
    breite = 4.0; // Divisor arrow width

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    setBrush(QBrush(Qt::black));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}

DiagramPathItem::DiagramPathItem(QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    myDiagramType = Path;
    myRoutingType = free;
    myContextMenu = contextMenu;
    myPoints.clear();

    len = 10.0; // Pfeillänge
    breite = 4.0; // Divisor Pfeilbreite

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}
//! [0]
DiagramPathItem::DiagramPathItem(const DiagramPathItem& diagram)
{
    //QGraphicsPathItem(diagram.parentItem(),diagram.scene());

    // copy from general GraphcsItem
    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());

    // copy DiagramPathItem
    myDiagramType = diagram.myDiagramType;
    myRoutingType = diagram.myRoutingType,
    myContextMenu = diagram.myContextMenu;
    myPoints = diagram.myPoints;

    len = diagram.len;

    setPath(diagram.path());
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setPos(diagram.pos());

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
}

void DiagramPathItem::createPath()
{
    QPainterPath myPath=getPath();
    if(myPath.elementCount()>0) setPath(myPath);
}

QPainterPath DiagramPathItem::getPath() const
{
    QPainterPath myPath;
    QPointF p1,p2;
    if(myPoints.size()>1)
    {
        for (int i = 1; i < myPoints.size(); ++i) {
            p1=myPoints.at(i-1);
            p2=myPoints.at(i);
            if( (i==1)&&((myDiagramType==Start) || (myDiagramType==StartEnd)) )
            {
                QPainterPath arrow = createArrow(p2,p1);
                myPath.addPath(arrow);
            }

            myPath.moveTo(p2);
            myPath.lineTo(p1);
            myPath.closeSubpath();
        }
        if((myDiagramType==End) or (myDiagramType==StartEnd)){
            // eliminate effect of several points on the same position
            int k=2;
            while((p1==p2)and(myPoints.size()>k)){
                k++;
                p1=myPoints.at(myPoints.size()-k);
            }
            QPainterPath arrow = createArrow(p1,p2);
            myPath.addPath(arrow);
        }
    }
    return myPath;
}

QPainterPath DiagramPathItem::createArrow(QPointF p1, QPointF p2) const
{
#define pi 3.141592654
    QPainterPath arrow;
    qreal dx=p1.x()-p2.x();
    qreal dy=p1.y()-p2.y();
    qreal m=sqrt(dx*dx+dy*dy);
    if(m>1){
        arrow.moveTo(p2);
        arrow.lineTo(-len/breite*dy/m+len*dx/m+p2.x(),len/breite*dx/m+len*dy/m+p2.y());
        arrow.lineTo(len/breite*dy/m+len*dx/m+p2.x(),-len/breite*dx/m+len*dy/m+p2.y());
        arrow.closeSubpath();
    }
    return arrow;
}

//! [4]
QPixmap DiagramPathItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(125, 125);
    QPolygonF myPolygon;
    myPolygon << QPointF(-100,-100) << QPointF(0,-100) << QPointF(0,100) << QPointF(100,100) ;;
    painter.drawPolyline(myPolygon);
    return pixmap;
}
//! [4]
QPixmap DiagramPathItem::icon()
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    myPoints.clear();
    myPoints.append(QPointF(5,40));
    myPoints.append(QPointF(45,40));
    len=10.0;
    breite=1.0;
    painter.drawPath(getPath());
    return pixmap;
}

void DiagramPathItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if(myContextMenu){
        myContextMenu->exec(event->screenPos());
    }
}

void DiagramPathItem::append(const QPointF point)
{
    if(myPoints.size()>1)
    {
        prepareGeometryChange();
        updateLast(point);
        myPoints.append(mapFromScene(point));
        if(myRoutingType!=free){
            myPoints.append(mapFromScene(point));
        }
    }
    else
    {
        /*myPoints.append(point-pos());
        myPoints.append(point-pos());*/
        myPoints.append(mapFromScene(point));
        myPoints.append(mapFromScene(point));
        if(myRoutingType!=free){
            myPoints.append(mapFromScene(point));
        }
        createPath();
    }
}

void DiagramPathItem::remove()
{
    if(myPoints.size()>1)
    {
        prepareGeometryChange();
        myPoints.removeLast();
        if((myPoints.size()>1)and(myRoutingType!=free)) myPoints.removeLast();
        updateLast(mapToScene(myPoints.last()));
    }
}

void DiagramPathItem::updateLast(const QPointF point)
{
    int i = myPoints.size()-1;
    if (i>0){
        prepareGeometryChange();
        QPointF localPoint=mapFromScene(point);
        myPoints[i]=localPoint;
        if(myRoutingType!=free){
            switch (myRoutingType){
                case xy:
                    myPoints[i-1].setX(localPoint.x());
                    myPoints[i-1].setY(myPoints[i-2].y());
                    break;
                case yx:
                    myPoints[i-1].setY(localPoint.y());
                    myPoints[i-1].setX(myPoints[i-2].x());
                    break;
                case shortest:
                {
                    QPointF diff=point-myPoints[i-2];
                    if(fabs(diff.x())>fabs(diff.y()))
                    {
                        myPoints[i-1].setX(localPoint.x());
                        myPoints[i-1].setY(myPoints[i-2].y());
                    }
                    else
                    {
                        myPoints[i-1].setY(localPoint.y());
                        myPoints[i-1].setX(myPoints[i-2].x());
                    }
                    break;
                }
                default:
                    break;
            }
        }
        createPath();
    }
}

QVariant DiagramPathItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        //foreach (Arrow *arrow, arrows) {
        //    arrow->updatePosition();
        //}
    }

    return value;
}
//! [6]
DiagramPathItem* DiagramPathItem::copy()
{
    DiagramPathItem* newDiagramPathItem=new DiagramPathItem(*this);
    return newDiagramPathItem;
}

void DiagramPathItem::write(QJsonObject &json)
{
    QPointF p=pos();
    json["x"]=p.x();
    json["y"]=p.y();
    json["z"]=zValue();
    json["type"]=type();
    json["diagramtype"]=static_cast<int>(myDiagramType);
    json["pen"]=pen().color().name();
    json["pen_alpha"]=pen().color().alpha();
    json["brush"]=brush().color().name();
    json["brush_alpha"]=brush().color().alpha();
    QJsonArray array;
    foreach(const QPointF p,myPoints){
        QJsonObject pointObj;
        pointObj["x"]=p.x();
        pointObj["y"]=p.y();
        array.append(pointObj);
    }
    json["points"]=array;
    json["m11"]=transform().m11();
    json["m12"]=transform().m12();
    json["m21"]=transform().m21();
    json["m22"]=transform().m22();
    json["dx"]=transform().dx();
    json["dy"]=transform().dy();
}

void DiagramPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
           QWidget *)
{
     painter->setPen(pen());
     painter->setBrush(brush());
     painter->drawPath(getPath());
     // selected
     if(isSelected()){
         QBrush selBrush=QBrush(Qt::cyan);
         QPen selPen=QPen(Qt::cyan);
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         QPointF point;
         for(int i=0;i<myPoints.count();i++)
         {
             point = myPoints.at(i);
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

QRectF DiagramPathItem::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;

    QPolygonF poly(myPoints);
    QRectF r=poly.boundingRect();

    return r.adjusted(-extra, -extra, extra, extra);
}

void DiagramPathItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = onGrid(e -> pos());
            for(mySelPoint=0;mySelPoint<myPoints.count();mySelPoint++){
                if(hasClickedOn(mouse_point,myPoints.at(mySelPoint))) break;
            }
            if(mySelPoint==myPoints.count()) mySelPoint=-1;
            else e->accept();
        }
    }
    QGraphicsPathItem::mousePressEvent(e);
}

void DiagramPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        myPoints.replace(mySelPoint,onGrid(mouse_point));
        createPath();
        e->accept();
    }else{
        QGraphicsPathItem::mouseMoveEvent(e);
    }
}

QPainterPath DiagramPathItem::shape() const {
    QPainterPath myPath = getPath();
    if(isSelected()){
             foreach (QPointF point, myPoints)
             {
                 // Rect around valid point
                 QPointF pw(2*myHandlerWidth,2*myHandlerWidth);
                 myPath.addRect(QRectF(point-pw,point+pw));
             }// foreach
         }// if
    return myPath;
}

bool DiagramPathItem::hasClickedOn(QPointF press_point, QPointF point) const {
    return (
        press_point.x() >= point.x() - 2*myHandlerWidth &&
        press_point.x() <  point.x() + 2*myHandlerWidth &&
        press_point.y() >= point.y() - 2*myHandlerWidth &&
        press_point.y() <  point.y() + 2*myHandlerWidth
    );
}

void DiagramPathItem::setRoutingType(const routingType newRoutingType)
{
    switch (myRoutingType) {
        case free:
            if(newRoutingType!=free){
                append(mapToScene(myPoints.last()));
                myRoutingType=newRoutingType;
            }
            break;
        default:
            myRoutingType=newRoutingType;
            if(newRoutingType==free){
                remove();
            }
            break;
    }
    prepareGeometryChange();
    createPath();
}

QPointF DiagramPathItem::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}

void DiagramPathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        QPointF hover_point = onGrid(e -> pos());
        for(myHoverPoint=0;myHoverPoint<myPoints.count();myHoverPoint++){
            if(hasClickedOn(hover_point,myPoints.at(myHoverPoint))) break;
        }//for
        if(myHoverPoint==myPoints.count()) myHoverPoint=-1;
        else update();
    }
    QGraphicsPathItem::hoverEnterEvent(e);
}

void DiagramPathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    QGraphicsPathItem::hoverLeaveEvent(e);
}

DiagramPathItem::DiagramPathItem(const QJsonObject &json, QMenu *contextMenu)
{
    QPointF p;
    p.setX(json["x"].toDouble());
    p.setY(json["y"].toDouble());
    setPos(p);
    setZValue(json["z"].toDouble());
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt());
    myRoutingType=free;
    QColor color;
    color.setNamedColor(json["pen"].toString());
    color.setAlpha(json["pen_alpha"].toInt());
    setPen(color);
    //color.setNamedColor(json["brush"].toString());
    //color.setAlpha(json["brush_alpha"].toInt());
    //setBrush(color);

    QVector<QPointF> lst;
    QJsonArray array=json["points"].toArray();
    for(int i=0;i<array.size();++i){
        QJsonObject o=array[i].toObject();
        qreal x=o["x"].toDouble();
        qreal y=o["y"].toDouble();
        lst<<QPointF(x,y);
    }
    myPoints=lst;

    qreal m11=json["m11"].toDouble();
    qreal m12=json["m12"].toDouble();
    qreal m21=json["m21"].toDouble();
    qreal m22=json["m22"].toDouble();
    qreal dx=json["dx"].toDouble();
    qreal dy=json["dy"].toDouble();
    QTransform tf(m11,m12,m21,m22,dx,dy);
    setTransform(tf);

    myContextMenu=contextMenu;

    len = 10.0; // arrow length
    breite = 4.0; // Divisor arrow width

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
}
