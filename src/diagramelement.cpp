#include "diagramelement.h"
#include <QFile>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


DiagramElement::DiagramElement(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent): DiagramItem(contextMenu,parent),mFilled(false)
{
    mFileName=fileName;
    if(importPathFromFile(mFileName)){
        setPath(mPainterPath);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        if(mFilled){
            setBrush(pen().color());
        }
    }
}

DiagramElement::DiagramElement(const DiagramElement& diagram)
    : DiagramItem(diagram.myContextMenu,diagram.parentItem())
{
    mFileName=diagram.mFileName;
    mName=diagram.mName;
    mFilled=diagram.mFilled;
    if(importPathFromFile(mFileName)){
        setPath(mPainterPath);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        if(mFilled){
            setBrush(pen().color());
        }
    }
}

DiagramItem* DiagramElement::copy()
{
    DiagramElement* newDiagramElement=new DiagramElement(*this);
    return dynamic_cast<DiagramItem*>(newDiagramElement);
}

void DiagramElement::write(QJsonObject &obj)
{
    DiagramItem::write(obj);
    obj["filename"]=mFileName;
    obj["name"]=mName;
}
QPixmap DiagramElement::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 1));
    if(mFilled){
        painter.setBrush(Qt::black);
    }
    painter.translate(125, 125);
    painter.scale(4.,4.);
    painter.drawPath(mPainterPath);

    return pixmap;
}

bool DiagramElement::importPathFromFile(const QString &fn)
{
    // open and read in text file
    QFile loadFile(fn);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QByteArray data = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    return createPainterPathFromJSON(loadDoc.object());
}

bool DiagramElement::createPainterPathFromJSON(QJsonObject json)
{
    mName=json["name"].toString();
    mFilled=json["filled"].toBool();
    QJsonArray array=json["elements"].toArray();
    for (int index = 0; index < array.size(); ++index) {
        QJsonObject jsonObject = array[index].toObject();
        QString type=jsonObject["type"].toString();
        if(type=="rect") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            mPainterPath.moveTo(QPointF(x0,y0));
            mPainterPath.addRect(x0,y0,x1-x0,y1-y0);
        }
        if(type=="circle") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal rx,ry;
            if(jsonObject.contains("r")){
                rx=jsonObject["r"].toDouble();
                ry=rx;
            }else{
                rx=jsonObject["rx"].toDouble();
                ry=jsonObject["ry"].toDouble();
            }
            QPointF p_center(x0,y0);
            mPainterPath.moveTo(p_center);
            mPainterPath.addEllipse(p_center,rx,ry);
        }
        if(type=="line") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            QVector<QPointF>lst{QPointF(x0,y0),QPointF(x1,y1)};
            QPolygonF polygon{lst};
            mPainterPath.moveTo(lst.first());
            mPainterPath.addPolygon(polygon);
        }
        if(type=="polygon") {
            QVector<QPointF>lst;
            QJsonArray jsonPoints=jsonObject["points"].toArray();
            for(int i=0;i<jsonPoints.size();++i){
                QJsonObject jsonElement=jsonPoints[i].toObject();
                qreal x=jsonElement["x"].toDouble();
                qreal y=jsonElement["y"].toDouble();
                lst<<QPointF(x,y);
            }
            QPolygonF polygon{lst};
            mPainterPath.moveTo(lst.first());
            mPainterPath.addPolygon(polygon);
        }
        if(type=="lines") {
            QList<QPointF>lst;
            QJsonArray jsonPoints=jsonObject["points"].toArray();
            for(int i=0;i<jsonPoints.size();++i){
                QJsonObject jsonElement=jsonPoints[i].toObject();
                qreal x=jsonElement["x"].toDouble();
                qreal y=jsonElement["y"].toDouble();
                lst<<QPointF(x,y);
            }
            for(int i=1;i<lst.length();++i){
                mPainterPath.moveTo(lst.at(i-1));
                mPainterPath.lineTo(lst.at(i));
            }
        }
        if(type=="arc") {
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal rx=jsonObject["rx"].toDouble();
            qreal ry=jsonObject["ry"].toDouble();
            qreal angle=jsonObject["angle"].toDouble();
            qreal length=jsonObject["length"].toDouble();
            mPainterPath.arcMoveTo(x-rx,y-ry,2*rx,2*ry,angle);
            mPainterPath.arcTo(x-rx,y-ry,2*rx,2*ry,angle,length);
        }
        if(type=="quad") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx=jsonObject["cx"].toDouble();
            qreal cy=jsonObject["cy"].toDouble();
            mPainterPath.moveTo(x0,y0);
            mPainterPath.quadTo(cx,cy,x1,y1);
        }
        if(type=="cubic") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx0=jsonObject["cx0"].toDouble();
            qreal cy0=jsonObject["cy0"].toDouble();
            qreal cx1=jsonObject["cx1"].toDouble();
            qreal cy1=jsonObject["cy1"].toDouble();
            mPainterPath.moveTo(x0,y0);
            mPainterPath.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
        if(type=="cubicTo") {
            qreal x1=jsonObject["x1"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx0=jsonObject["cx0"].toDouble();
            qreal cy0=jsonObject["cy0"].toDouble();
            qreal cx1=jsonObject["cx1"].toDouble();
            qreal cy1=jsonObject["cy1"].toDouble();
            mPainterPath.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
    }
    return true;
}

DiagramElement::DiagramElement(const QJsonObject &json, QMenu *contextMenu):DiagramItem(json,contextMenu)
{
    mFileName=json["filename"].toString();
    mName=json["name"].toString();
    if(importPathFromFile(mFileName)){
        setPath(mPainterPath);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        if(mFilled){
            setBrush(pen().color());
        }
    }
}
