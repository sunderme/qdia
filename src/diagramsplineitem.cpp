#include "diagramsplineitem.h"
#include <QJsonObject>
#include <QtGui>


DiagramSplineItem::DiagramSplineItem(QMenu *, QGraphicsItem *parent):QGraphicsPathItem(parent)
{
    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}

DiagramSplineItem::DiagramSplineItem(const QJsonObject &json, QMenu *)
{
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
    createPath();

    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
}

void DiagramSplineItem::setP0(const QPointF point)
{
    //p0=point;
    //c1=point;
}

void DiagramSplineItem::updateLast(const QPointF point)
{
    //prepareGeometryChange();
    p1=mapFromScene(point);
    //c0=point;
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

void DiagramSplineItem::createPath()
{
    QPainterPath path;
    path.cubicTo(p1,p0,p1);
    setPath(path);
}

