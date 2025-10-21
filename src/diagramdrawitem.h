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
    Rect();
    Rect(QPointF pt,int selPoint);
    Rect(QRectF rect);
    QPointF anchorPoint() const;
    QPointF point() const;
    QRectF rect() const;

    void setSelPoint(int selPoint);
    void setLeft();
    void setRight();
    void setTop();
    void setBottom();
    void setTopLeft();
    void setTopRight();
    void setBottomLeft();
    void setBottomRight();
    void movePoint(QPointF pt);

    void translate(QPointF pt);
private:
    QPointF m_anchor;
    QPointF m_point;
    int m_selPoint;
    bool m_mvPointX,m_mvPointY;
    bool m_mvAnchorX,m_mvAnchorY;
};

/*!
 * \brief The DiagramDrawItem class provides drawable figures like rectangle, ellipse
 * It always has two definition point (pos() and myPos2) which define the dimension of the drawing
 */
class DiagramDrawItem : public DiagramItem
{
public:
    enum { Type = UserType + 16 };
    enum DiagramType { Ellipse, Rectangle, Circle, RoundedRect,
                       Rhombus, Triangle, DA , OTA, Note, Pie, MUX, DEMUX, Square, CirclePie};

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
        { return mapToParent(myPos2); }

    void setDimension(QPointF newPos);
    QPointF getDimension();

    void setRadius(const qreal radius)
        { myRadius=radius; }
    qreal getRadius()
        { return myRadius; }

    void setStartPoint(const QPointF pt);
    void setEndPoint(const QPointF pt);

    void setPartnerItem(DiagramDrawItem *parterItem);
    DiagramDrawItem *partnerItem();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    QPainterPath createPath();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    QRectF innerBoundingRect() const;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
    bool hasClickedOn(QPointF press_point, QPointF point) const ;
    QPointF onGrid(QPointF pos);
    void mySetDimension(QPointF newPos);


private:
    QPointF getHandler(int i) const;
    int getNumberOfHandles() const;
    DiagramType myDiagramType;
    QPainterPath mPainterPath;
    QPointF myPos2;
    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;
    qreal myRadius;
    QPointF mStartPoint,mEndPoint;
    Rect mRect;
    DiagramDrawItem *m_partnerItem;
};

#endif // DIAGRAMDRAWITEM_H
