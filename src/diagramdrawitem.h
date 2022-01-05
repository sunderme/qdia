#ifndef DIAGRAMDRAWITEM_H
#define DIAGRAMDRAWITEM_H

#include <QList>
#include "diagramitem.h"

//! [0]
class DiagramDrawItem : public DiagramItem
{
public:
    enum { Type = UserType + 16 };
    enum DiagramType { Ellipse, Rectangle, Circle, RoundedRect, Rhombus, Triangle, DA };

    DiagramDrawItem(DiagramType diagramType, QMenu *contextMenu,
        QGraphicsItem *parent = 0);
    DiagramDrawItem(const QJsonObject &json, QMenu *contextMenu);
    DiagramDrawItem(const DiagramDrawItem& diagram);//copy constructor

    DiagramItem* copy();
    void write(QJsonObject &json);

    DiagramType diagramType() const
        { return myDiagramType; }
    QPainterPath path() const
        { return mPainterPath; }
    QPixmap image() const;
    int type() const
        { return Type;}

    void setPos2(qreal x,qreal y);
    void setPos2(QPointF pos);
    QPointF getPos2() const
        { return mapToScene(myPos2); }

    void setDimension(QPointF newPos);
    QPointF getDimension();

    void setRadius(const qreal radius)
        { myRadius=radius; }
    qreal getRadius()
        { return myRadius; }


protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    QPainterPath createPath();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QPainterPath shape() const;
    QRectF boundingRect() const;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
    void mousePressEvent(QGraphicsSceneMouseEvent *e);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
    bool hasClickedOn(QPointF press_point, QPointF point) const ;
    QPointF onGrid(QPointF pos);
    void mySetDimension(QPointF newPos);


private:
    DiagramType myDiagramType;
    QPainterPath mPainterPath;
    QMenu *myContextMenu;
    QPointF myPos2;
    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;
    qreal myRadius;
};

#endif // DIAGRAMDRAWITEM_H
