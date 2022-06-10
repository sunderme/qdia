#ifndef DIAGRAMDRAWITEM_H
#define DIAGRAMDRAWITEM_H

#include <QList>
#include "diagramitem.h"

//! [0]
class DiagramDrawItem : public DiagramItem
{
public:
    enum { Type = UserType + 16 };
    enum DiagramType { Ellipse, Rectangle, Circle, RoundedRect, Rhombus, Triangle, DA , OTA};

    DiagramDrawItem(DiagramType diagramType, QMenu *contextMenu,
        QGraphicsItem *parent = 0);
    DiagramDrawItem(const QJsonObject &json, QMenu *contextMenu);
    DiagramDrawItem(const DiagramDrawItem& diagram);//copy constructor

    DiagramItem* copy() override;
    void write(QJsonObject &json) override;

    DiagramType diagramType() const
        { return myDiagramType; }
    QPainterPath path() const
        { return mPainterPath; }
    QPixmap image() const;
    int type() const  override
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
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    QPainterPath createPath();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
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
