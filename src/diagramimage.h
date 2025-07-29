#ifndef DIAGRAMIMAGE_H
#define DIAGRAMIMAGE_H

#include "diagramitem.h"
#include "diagramdrawitem.h"

class DiagramImage : public DiagramItem
{
public:
    enum { Type = UserType + 8 };
    enum DiagramType { Image };
    DiagramImage(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent = nullptr);
    DiagramImage(const QJsonObject &json, QMenu *contextMenu);
    DiagramImage(const DiagramImage& diagram);//copy constructor

    DiagramItem* copy() override;
    void write(QJsonObject &obj) override;
    QPixmap image() const;
    DiagramType diagramType() const
    { return Image; }
    int type() const override
    { return Type;}
    QString getName() {
        return mName;
    }
    QString getFileName() {
        return mFileName;
    }
    void setImage(QImage img);
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
    bool hasClickedOn(QPointF press_point, QPointF point) const ;
    QPointF onGrid(QPointF pos);

    QPointF getHandler(int i) const;

    QString mFileName;
    QString mName;
    QPixmap mPixmap;
    Rect m_boundingRect;

    int myHoverPoint,mySelPoint;
    qreal myHandlerWidth;

    DiagramImage *m_partnerItem;
};

#endif // DIAGRAMIMAGE_H
