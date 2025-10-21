#include "diagramelement.h"
#include <QFile>
#include <QCursor>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


DiagramElement::DiagramElement(const QString fileName, QMenu *contextMenu, QGraphicsItem *parent): DiagramItem(contextMenu,parent)
{
    mFileName=fileName;
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}

DiagramElement::DiagramElement(const DiagramElement& diagram)
    : DiagramItem(diagram.myContextMenu,diagram.parentItem())
{
    mFileName=diagram.mFileName;
    mName=diagram.mName;
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlags(diagram.flags());
        setAcceptHoverEvents(true);
    }
    setTransform(diagram.transform());
    setPen(diagram.pen());
    setBrush(diagram.brush());
    setPos(diagram.pos());
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

    QRectF rect=boundingRect();
    qreal w=rect.width();
    if(w<rect.height()){
        w=rect.height();
    }
    qreal scale=qMin(4.,240/w);
    QPointF center=-rect.center()*scale+QPointF(125,125);

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 1));
    painter.translate(center);
    painter.scale(scale,scale);
    foreach(Path lPath,lstPaths){
        painter.save();
        if(lPath.filled){
            painter.setBrush(pen().color());
        }
        painter.setTransform(lPath.t,true);
        painter.drawPath(lPath.path);
        painter.restore();
    }

    return pixmap;
}

void DiagramElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    foreach(Path lPath,lstPaths){
        painter->save();
        if(lPath.filled){
            painter->setBrush(pen().color());
        }
        if(lPath.dontFill){
            painter->setBrush(Qt::NoBrush);
        }
        painter->setTransform(lPath.t,true);
        painter->drawPath(lPath.path);
        painter->restore();
    }
    // selected
    if(isSelected()){
        // Rect
        QPen selPen=QPen(Qt::DotLine);
        selPen.setWidth(0);
        selPen.setColor(Qt::black);
        selPen.setBrush(Qt::white);
        QBrush selBrush=QBrush(Qt::NoBrush);
        painter->setBrush(selBrush);
        painter->setPen(selPen);
        painter->drawRect(boundingRect());
    }// if
}

QRectF DiagramElement::boundingRect() const
{
    QRectF rect;
    foreach(const Path &elem,lstPaths){
        QPainterPath path=elem.t.map(elem.path);
        QRectF localRect=path.boundingRect();
        rect=rect.united(localRect);
    }
    return rect;
}

/*!
 * \brief change cursor when move is feasible
 * \param e
 */
void DiagramElement::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::SizeAllCursor);
    }
    DiagramItem::hoverEnterEvent(e);
}
/*!
 * \brief change cursor back to default
 * \param e
 */
void DiagramElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    if (isSelected()) {
        setCursor(Qt::ArrowCursor);
    }
    DiagramItem::hoverLeaveEvent(e);
}

QList<DiagramElement::Path> DiagramElement::importPathFromFile(const QString &fn)
{
    // open and read in text file
    QFile loadFile(fn);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return QList<Path>();
    }
    QByteArray data = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    return createPainterPathFromJSON(loadDoc.object());
}

QList<DiagramElement::Path> DiagramElement::createPainterPathFromJSON(QJsonObject json)
{
    QString elementName=json["name"].toString();
    bool filled=json["filled"].toBool();
    bool dontFill=json["dontFill"].toBool();
    QList<DiagramElement::Path> result;
    QPainterPath path;
    QJsonArray array=json["elements"].toArray();
    for (int index = 0; index < array.size(); ++index) {
        QJsonObject jsonObject = array[index].toObject();
        QString type=jsonObject["type"].toString();
        if(type=="rect") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            path.moveTo(QPointF(x0,y0));
            path.addRect(x0,y0,x1-x0,y1-y0);
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
            path.moveTo(p_center);
            path.addEllipse(p_center,rx,ry);
        }
        if(type=="line") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            path.moveTo(x0,y0);
            path.lineTo(x1,y1);
        }
        if(type=="lineTo") {
            qreal x0=jsonObject["x"].toDouble();
            qreal y0=jsonObject["y"].toDouble();
            path.lineTo(x0,y0);
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
            path.moveTo(lst.first());
            path.addPolygon(polygon);
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
                path.moveTo(lst.at(i-1));
                path.lineTo(lst.at(i));
            }
        }
        if(type=="arc") {
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal rx=jsonObject["rx"].toDouble();
            qreal ry=jsonObject["ry"].toDouble();
            qreal angle=jsonObject["angle"].toDouble();
            qreal length=jsonObject["length"].toDouble();
            path.arcMoveTo(x-rx,y-ry,2*rx,2*ry,angle);
            path.arcTo(x-rx,y-ry,2*rx,2*ry,angle,length);
        }
        if(type=="arcTo") {
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal rx=jsonObject["rx"].toDouble();
            qreal ry=jsonObject["ry"].toDouble();
            qreal angle=jsonObject["angle"].toDouble();
            qreal length=jsonObject["length"].toDouble();
            path.arcTo(x-rx,y-ry,2*rx,2*ry,angle,length);
        }
        if(type=="quad") {
            qreal x0=jsonObject["x0"].toDouble();
            qreal x1=jsonObject["x1"].toDouble();
            qreal y0=jsonObject["y0"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx=jsonObject["cx"].toDouble();
            qreal cy=jsonObject["cy"].toDouble();
            path.moveTo(x0,y0);
            path.quadTo(cx,cy,x1,y1);
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
            path.moveTo(x0,y0);
            path.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
        if(type=="cubicTo") {
            qreal x1=jsonObject["x1"].toDouble();
            qreal y1=jsonObject["y1"].toDouble();
            qreal cx0=jsonObject["cx0"].toDouble();
            qreal cy0=jsonObject["cy0"].toDouble();
            qreal cx1=jsonObject["cx1"].toDouble();
            qreal cy1=jsonObject["cy1"].toDouble();
            path.cubicTo(cx0,cy0,cx1,cy1,x1,y1);
        }
        if(type=="close") {
            path.closeSubpath();
        }
        if(type=="text"){
            Path localPath;
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            QString text=jsonObject["text"].toString();
            QFont serifFont("Helvetica", 10);
            localPath.path.addText(x,y,serifFont,text);
            localPath.filled=true;
            result<<localPath;
        }
        if(type=="element"){
            qreal x=jsonObject["x"].toDouble();
            qreal y=jsonObject["y"].toDouble();
            qreal scale=jsonObject["scale"].toDouble(1.);
            qreal rotate=jsonObject["rotate"].toDouble(0.);
            QString fn=jsonObject["name"].toString();
            fn=":/libs/"+fn;
            if(!fn.endsWith(".json")){
                fn+=".json";
            }
            QList<Path> local=importPathFromFile(fn);
            for(Path &elem:local){
                elem.t.translate(x,y);
                elem.t.scale(scale,scale);
                elem.t.rotate(rotate);
            }
            result<<local;
        }
    }
    Path p;
    p.path=path;
    p.filled=filled;
    p.dontFill=dontFill;
    result.prepend(p);
    mName=elementName;
    return result;
}

DiagramElement::DiagramElement(const QJsonObject &json, QMenu *contextMenu):DiagramItem(json,contextMenu)
{
    mFileName=json["filename"].toString();
    mName=json["name"].toString();
    lstPaths=importPathFromFile(mFileName);
    if(!lstPaths.isEmpty()){
        QPainterPath p;
        for(const auto &lp:lstPaths){
            p|=lp.path;
        }
        setPath(p);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setAcceptHoverEvents(true);
    }
}
