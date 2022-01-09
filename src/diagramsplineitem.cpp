#include "diagramsplineitem.h"
#include <QJsonObject>
#include <QtGui>
#include <QGraphicsSceneMouseEvent>
#include "diagramscene.h"


DiagramSplineItem::DiagramSplineItem(DiagramType diagramType,QMenu *, QGraphicsItem *parent):QGraphicsPathItem(parent)
{
    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
    myActivePoint=1;
    myDiagramType=diagramType;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}

DiagramSplineItem::DiagramSplineItem(const QJsonObject &json, QMenu *)
{
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt());
    QPointF p;
    p.setX(json["x"].toDouble());
    p.setY(json["y"].toDouble());
    setPos(p);
    setZValue(json["z"].toDouble());
    QColor color;
    color.setNamedColor(json["pen"].toString());
    color.setAlpha(json["pen_alpha"].toInt());
    setPen(color);
    color.setNamedColor(json["bruch"].toString());
    color.setAlpha(json["brush_alpha"].toInt());
    setBrush(color);

    p.setX(json["x0"].toDouble());
    p.setY(json["y0"].toDouble());
    p0=p;
    p.setX(json["x1"].toDouble());
    p.setY(json["y1"].toDouble());
    p1=p;
    p.setX(json["cx0"].toDouble());
    p.setY(json["cy0"].toDouble());
    c0=p;
    p.setX(json["cx1"].toDouble());
    p.setY(json["cy1"].toDouble());
    c1=p;
    createPath();

    qreal m11=json["m11"].toDouble();
    qreal m12=json["m12"].toDouble();
    qreal m21=json["m21"].toDouble();
    qreal m22=json["m22"].toDouble();
    qreal dx=json["dx"].toDouble();
    qreal dy=json["dy"].toDouble();
    QTransform tf(m11,m12,m21,m22,dx,dy);
    setTransform(tf);
    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
    myActivePoint=1;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}

DiagramSplineItem::DiagramSplineItem(const DiagramSplineItem &diagram)
{
    p0=diagram.p0;
    p1=diagram.p1;
    c0=diagram.c0;
    c1=diagram.c1;
    myDiagramType = diagram.myDiagramType;
    createPath();

    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
}


void DiagramSplineItem::nextActive()
{
    ++myActivePoint;
    if(myActivePoint>3)
        myActivePoint=0;
}

QPointF DiagramSplineItem::getActivePoint(const int currentActive)
{
    int n=myActivePoint;
    if(currentActive>-1){
        n=currentActive;
    }
    switch (n) {
    case 0:
        return p0;
        break;
    case 1:
        return p1;
        break;
    case 2:
        return c1;
        break;
    case 3:
        return c0;
        break;
    default:
        break;
    }
    return QPointF();
}

void DiagramSplineItem::updateActive(const QPointF point,int currentActive)
{
    //prepareGeometryChange();
    int n=myActivePoint;
    if(currentActive>=0)
        n=currentActive;
    switch(n){
    case 0:
        p0=mapFromScene(point);
        break;
    case 1:
        p1=mapFromScene(point);
        break;
    case 2:
        c1=mapFromScene(point);
        break;
    case 3:
        c0=mapFromScene(point);
        break;
    default:
        break;
    }

    createPath();
}

DiagramSplineItem *DiagramSplineItem::copy()
{
    DiagramSplineItem* newDiagramSplineItem=new DiagramSplineItem(*this);
    return newDiagramSplineItem;
}

void DiagramSplineItem::write(QJsonObject &json)
{
    QPointF p=pos();
    json["x"]=p.x();
    json["y"]=p.y();
    json["x0"]=p0.x();
    json["y0"]=p0.y();
    json["x1"]=p1.x();
    json["y1"]=p1.y();
    json["cx0"]=c0.x();
    json["cy0"]=c0.y();
    json["cx1"]=c1.x();
    json["cy1"]=c1.y();
    json["z"]=zValue();
    json["type"]=type();
    json["pen"]=pen().color().name();
    json["pen_alpha"]=pen().color().alpha();
    json["brush"]=brush().color().name();
    json["brush_alpha"]=brush().color().alpha();

    json["m11"]=transform().m11();
    json["m12"]=transform().m12();
    json["m21"]=transform().m21();
    json["m22"]=transform().m22();
    json["dx"]=transform().dx();
    json["dy"]=transform().dy();
}

QPixmap DiagramSplineItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 2));
    QPainterPath path;
    path.moveTo(50,200);
    path.cubicTo(200,200,100,100,100,100);
    painter.drawPath(path);
    return pixmap;
}

QPixmap DiagramSplineItem::icon() const
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    QPainterPath path;
    path.moveTo(5,70);
    path.cubicTo(45,70,25,0,25,0);
    painter.drawPath(path);
    return pixmap;
}

QRectF DiagramSplineItem::boundingRect() const
{
    QVector<QPointF> pts{p0,p1,c0,c1};
    QPolygonF p(pts);
    QRectF r=p.boundingRect();
    r.adjust(-myHandlerWidth,-myHandlerWidth,+myHandlerWidth,+myHandlerWidth);
    return r;
}

void DiagramSplineItem::createPath()
{
    QPainterPath path;
    path.moveTo(p0);
    switch (myDiagramType) {
    case quad:
        path.quadTo(c0,p1);
        break;
    default:
        path.cubicTo(c0,c1,p1);
    }
    setPath(path);
}

void DiagramSplineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(path());
    // selected
    if(isSelected()){
        QPen connectPen=QPen(Qt::green);
        connectPen.setStyle(Qt::DashLine);
        painter->setPen(connectPen);
        painter->drawLine(p0,c0);
        painter->drawLine(p1,c1);
        QBrush selBrush=QBrush(Qt::cyan);
        QPen selPen=QPen(Qt::cyan);
        painter->setBrush(selBrush);
        painter->setPen(selPen);

        if(myHoverPoint==0){
            painter->setBrush(QBrush(Qt::red));
        }
        // Rect around valid point
        painter->drawRect(QRectF(p0-QPointF(2,2),p0+QPointF(2,2)));
        painter->setBrush(selBrush);

        if(myHoverPoint==1){
            painter->setBrush(QBrush(Qt::red));
        }
        // Rect around valid point
        painter->drawRect(QRectF(p1-QPointF(2,2),p1+QPointF(2,2)));
        painter->setBrush(selBrush);

        if(myHoverPoint==2){
            painter->setBrush(QBrush(Qt::red));
        }
        // Rect around valid point
        painter->drawRect(QRectF(c1-QPointF(2,2),c1+QPointF(2,2)));
        painter->setBrush(selBrush);

        if(myHoverPoint==3){
            painter->setBrush(QBrush(Qt::red));
        }
        // Rect around valid point
        painter->drawRect(QRectF(c0-QPointF(2,2),c0+QPointF(2,2)));
        painter->setBrush(selBrush);
    }// if
}

QPainterPath DiagramSplineItem::shape() const
{
    QPainterPath myPath = path();
    if(isSelected()){
        QPointF pw(2*myHandlerWidth,2*myHandlerWidth);
        myPath.addRect(QRectF(p0-pw,p0+pw));
        myPath.addRect(QRectF(p1-pw,p1+pw));
        myPath.addRect(QRectF(c0-pw,c0+pw));
        myPath.addRect(QRectF(c1-pw,c1+pw));
    }// if
    return myPath;
}

void DiagramSplineItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        myHoverPoint=-1;
        QPointF hover_point(onGrid(e->pos()));
        if(hasClickedOn(hover_point,p0)) myHoverPoint=0;
        if(hasClickedOn(hover_point,p1)) myHoverPoint=1;
        if(hasClickedOn(hover_point,c1)) myHoverPoint=2;
        if(hasClickedOn(hover_point,c0)) myHoverPoint=3;
        qDebug()<<myHoverPoint;
        update();
    }
    QGraphicsPathItem::hoverEnterEvent(e);
}

void DiagramSplineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    QGraphicsPathItem::hoverLeaveEvent(e);
}

void DiagramSplineItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = onGrid(e -> pos());
            mySelPoint=-1;
            if(hasClickedOn(mouse_point,p0)) mySelPoint=0;
            if(hasClickedOn(mouse_point,p1)) mySelPoint=1;
            if(hasClickedOn(mouse_point,c1)) mySelPoint=2;
            if(hasClickedOn(mouse_point,c0)) mySelPoint=3;
            if(mySelPoint>-1)
                e->accept();
        }
    }
    QGraphicsPathItem::mousePressEvent(e);
}

void DiagramSplineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        switch (mySelPoint) {
        case 0:
            p0=mouse_point;
            break;
        case 1:
            p1=mouse_point;
            break;
        case 2:
            c1=mouse_point;
            break;
        case 3:
            c0=mouse_point;
            break;
        default:
            break;
        }

        createPath();
        e->accept();
    }else{
        QGraphicsPathItem::mouseMoveEvent(e);
    }
}

QPointF DiagramSplineItem::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}

bool DiagramSplineItem::hasClickedOn(QPointF press_point, QPointF point) const
{
    return (
        press_point.x() >= point.x() - 2*myHandlerWidth &&
        press_point.x() <  point.x() + 2*myHandlerWidth &&
        press_point.y() >= point.y() - 2*myHandlerWidth &&
        press_point.y() <  point.y() + 2*myHandlerWidth
    );
}



