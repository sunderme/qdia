#ifndef DIAGRAMELEMENT_H
#define DIAGRAMELEMENT_H

#include "diagramitem.h"

class DiagramElement : public DiagramItem
{
public:
    enum { Type = UserType + 32 };
    enum DiagramType { Element };
    DiagramElement(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent = nullptr);
    DiagramElement(const QJsonObject &json, QMenu *contextMenu);
    DiagramElement(const DiagramElement& diagram);//copy constructor

    DiagramItem* copy() override;
    void write(QJsonObject &obj) override;
    QPixmap image() const;
    DiagramType diagramType() const
        { return Element; }
    int type() const override
        { return Type;}
    QString getName() {
        return mName;
    }
    QString getFileName() {
        return mFileName;
    }
protected:
    struct Path {
        QPainterPath path;
        bool filled=false;
        bool dontFill=false;
        QTransform t;
    };
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;

    QString mFileName;
    QString mName;
    QList<Path> lstPaths;

    QList<Path> importPathFromFile(const QString &fn);
    QList<Path> createPainterPathFromJSON(QJsonObject json);
};

#endif // DIAGRAMELEMENT_H
