#ifndef DIAGRAMIMAGE_H
#define DIAGRAMIMAGE_H

#include "diagramitem.h"

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
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;

    QString mFileName;
    QString mName;
    QPixmap mPixmap;
};

#endif // DIAGRAMIMAGE_H
