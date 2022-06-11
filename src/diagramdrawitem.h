#ifndef DIAGRAMDRAWITEM_H
#define DIAGRAMDRAWITEM_H

#include <QList>
#include "diagramitem.h"

/*!
 * \brief The Rect class provides a simple repesentation of an rectangle
 * The rectangle can be directly manipulated on all edges/corner
 * It keeps anchor and second point, so negative width/height is maintained and handled corrctly (unlike QRectF)
 */
class Rect
{
public:
    Rect(QPointF pt);
    QPointF anchorPoint() const;
    QPointF point() const;

    void setLeft(qreal x);
    void setRight(qreal x);
    void setTop(qreal y);
    void setBottom(qreal y);
    void setTopLeft(QPointF pt);
    void setTopRight(QPointF pt);
    void setBottomLeft(QPointF pt);
    void setBottomRight(QPointF pt);

    void translate(QPointF pt);
private:
    QPointF m_anchor;
    QPointF m_point;
};

/*!
 * \brief The DiagramDrawItem class provides drawable figures like rectangle, ellipse
 * It always has two definition point (pos() and myPos2) which define the dimension of the drawing
 */
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
    QPointF getHandler(int i) const;
    DiagramType myDiagramType;
    QPainterPath mPainterPath;
    QMenu *myContextMenu;
    QPointF myPos2;
    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;
    qreal myRadius;
};

#endif // DIAGRAMDRAWITEM_H
