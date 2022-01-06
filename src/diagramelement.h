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
    QString mFileName;
    QString mName;
    bool mFilled;
    QPainterPath mPainterPath;

    bool importPathFromFile(const QString &fn);
    bool createPainterPathFromJSON(QJsonObject json);
};

#endif // DIAGRAMELEMENT_H
