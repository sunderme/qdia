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

    DiagramItem* copy();
    void write(QJsonObject &obj);
    QPixmap image() const;
    DiagramType diagramType() const
        { return Element; }
    int type() const
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
        QTransform t;
    };
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    QString mFileName;
    QString mName;
    bool mFilled;
    QList<Path> lstPaths;

    QList<Path> importPathFromFile(const QString &fn);
    QList<Path> createPainterPathFromJSON(QJsonObject json);
};

#endif // DIAGRAMELEMENT_H
