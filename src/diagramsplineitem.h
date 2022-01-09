#ifndef DIAGRAMSPLINEITEM_H
#define DIAGRAMSPLINEITEM_H

#include "diagramitem.h"

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

class DiagramSplineItem : public QGraphicsPathItem
{
public:
    enum { Type = UserType + 6 };
    DiagramSplineItem(QMenu *contextMenu, QGraphicsItem *parent=nullptr);
    DiagramSplineItem(const QJsonObject &json, QMenu *contextMenu);
    DiagramSplineItem(const DiagramSplineItem& diagram);//copy constructor

    int type() const
        { return Type;}

    void setP0(const QPointF point);
    void updateLast(const QPointF point);

    DiagramSplineItem* copy();
    void write(QJsonObject &json);

    QPixmap image() const;
    QPixmap icon() const;

protected:
    void createPath();

private:
    QPointF p0,p1,c0,c1;

    int mySelPoint,myHoverPoint;
    qreal myHandlerWidth;
};

#endif // DIAGRAMSPLINEITEM_H
