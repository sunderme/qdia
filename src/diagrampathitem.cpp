#include <QtGui>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QJsonObject>

#include "diagrampathitem.h"
#include "diagramscene.h"

DiagramPathItem::DiagramPathItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    myDiagramType = diagramType;
    myRoutingType = free;
    myContextMenu = contextMenu;
    myPoints.clear();

    len = 10.0; // arrow length
    breite = 4.0; // Divisor arrow width

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    setBrush(QBrush(Qt::black));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}

DiagramPathItem::DiagramPathItem(QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    myDiagramType = Path;
    myRoutingType = free;
    myContextMenu = contextMenu;
    myPoints.clear();

    len = 10.0; // Pfeillänge
    breite = 4.0; // Divisor Pfeilbreite

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}
//! [0]
DiagramPathItem::DiagramPathItem(const DiagramPathItem& diagram)
{
    //QGraphicsPathItem(diagram.parentItem(),diagram.scene());

    // copy from general GraphcsItem
    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());

    // copy DiagramPathItem
    myDiagramType = diagram.myDiagramType;
    myRoutingType = diagram.myRoutingType,
    myContextMenu = diagram.myContextMenu;
    myPoints = diagram.myPoints;

    len = diagram.len;
    breite = diagram.breite;

    //setPath(diagram.path());
    createPath();
    setFlags(diagram.flags());
    setAcceptHoverEvents(true);
    setPos(diagram.pos());

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;
}

void DiagramPathItem::createPath()
{
    QPainterPath myPath=getPath();
    if(myPath.elementCount()>0) setPath(myPath);
}
/*!
 * \brief draw the actual arrows (if necessary)
 * Separated from path drawing as it needs to have a filling brush
 * \param painter
 */
void DiagramPathItem::drawArrows(QPainter *painter) const
{
    for(const QPainterPath &path:m_arrows){
        painter->drawPath(path);
    }
}

QPainterPath DiagramPathItem::getPath()
{
    m_arrows.clear();
    QPainterPath myPath;
    QPointF p1,p2;
    if(myPoints.size()>1)
    {
        p2=myPoints.at(0);
        myPath.moveTo(p2);
        for (int i = 1; i < myPoints.size(); ++i) {
            p1=p2;
            p2=myPoints.at(i);
            if( (i==1)&&((myDiagramType==Start) || (myDiagramType==StartEnd)) )
            {
                QPainterPath arrow = createArrow(p2,p1);
                m_arrows.append(arrow);
            }
            myPath.lineTo(p2);
        }
        if((myDiagramType==End) or (myDiagramType==StartEnd)){
            // eliminate effect of several points on the same position
            int k=2;
            while((p1==p2)and(myPoints.size()>k)){
                k++;
                p1=myPoints.at(myPoints.size()-k);
            }
            QPainterPath arrow = createArrow(p1,p2);
            m_arrows.append(arrow);
        }
    }
    return myPath;
}
/*!
 * \brief find line section for a given point
 * \param pt in scene coordinates
 * \return line section as QLineF
 */
QLineF DiagramPathItem::findLineSection(QPointF pt) const
{
    QLineF line;
    QLineF resultLine;
    qreal minDist=-1;
    pt=mapFromScene(pt); // local coordinates
    for(int i=1;i<myPoints.count();++i){
        line.setPoints(myPoints[i-1],myPoints[i]);
        qreal md=minimalDistance(line,pt);
        if(minDist<0 || md<minDist){
            minDist=md;
            if(md<3){
                resultLine=line;
            }
        }

    }
    return resultLine;
}

QPainterPath DiagramPathItem::createArrow(QPointF p1, QPointF p2) const
{
#define pi 3.141592654
    QPainterPath arrow;
    qreal dx=p1.x()-p2.x();
    qreal dy=p1.y()-p2.y();
    qreal m=sqrt(dx*dx+dy*dy);
    if(m>1){
        arrow.moveTo(p2);
        arrow.lineTo(-len/breite*dy/m+len*dx/m+p2.x(),len/breite*dx/m+len*dy/m+p2.y());
        arrow.lineTo(len/breite*dy/m+len*dx/m+p2.x(),-len/breite*dx/m+len*dy/m+p2.y());
        arrow.closeSubpath();
    }
    return arrow;
}

//! [4]
QPixmap DiagramPathItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(125, 125);
    QPolygonF myPolygon;
    myPolygon << QPointF(-100,-100) << QPointF(0,-100) << QPointF(0,100) << QPointF(100,100) ;;
    painter.drawPolyline(myPolygon);
    return pixmap;
}
//! [4]
QPixmap DiagramPathItem::icon()
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    myPoints.clear();
    myPoints.append(QPointF(5,40));
    myPoints.append(QPointF(45,40));
    len=40.0;
    breite=20.0;
    painter.drawLine(5,40,45,40);
    QVector<QLine>lines;
    if(myDiagramType>Start){
        lines<<QLine(45,40,35,30)<<QLine(35,30,35,50)<<QLine(35,50,45,40);
    }
    if(myDiagramType==Start || myDiagramType==StartEnd){
        lines<<QLine(5,40,15,30)<<QLine(15,30,15,50)<<QLine(15,50,5,40);
    }
    painter.drawLines(lines);
    return pixmap;
}

void DiagramPathItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if(myContextMenu){
        myContextMenu->exec(event->screenPos());
    }
}

void DiagramPathItem::append(const QPointF point)
{
    if(myPoints.size()>1)
    {
        prepareGeometryChange();
        updateLast(point);
        myPoints.append(mapFromScene(point));
        if(myRoutingType!=free){
            myPoints.append(mapFromScene(point));
        }
    }
    else
    {
        myPoints.append(mapFromScene(point));
        myPoints.append(mapFromScene(point));
        if(myRoutingType!=free){
            myPoints.append(mapFromScene(point));
        }
        createPath();
    }
}

void DiagramPathItem::remove()
{
    if(myPoints.size()>1)
    {
        prepareGeometryChange();
        myPoints.removeLast();
        if((myPoints.size()>1)and(myRoutingType!=free)) myPoints.removeLast();
        updateLast(mapToScene(myPoints.last()));
    }
}

void DiagramPathItem::updateLast(const QPointF point)
{
    int i = myPoints.size()-1;
    if (i>0){
        prepareGeometryChange();
        QPointF localPoint=mapFromScene(point);
        myPoints[i]=localPoint;
        if(myRoutingType!=free){
            switch (myRoutingType){
                case xy:
                    myPoints[i-1].setX(localPoint.x());
                    myPoints[i-1].setY(myPoints[i-2].y());
                    break;
                case yx:
                    myPoints[i-1].setY(localPoint.y());
                    myPoints[i-1].setX(myPoints[i-2].x());
                    break;
                case shortest:
                {
                    QPointF diff=point-myPoints[i-2];
                    if(fabs(diff.x())>fabs(diff.y()))
                    {
                        myPoints[i-1].setX(localPoint.x());
                        myPoints[i-1].setY(myPoints[i-2].y());
                    }
                    else
                    {
                        myPoints[i-1].setY(localPoint.y());
                        myPoints[i-1].setX(myPoints[i-2].x());
                    }
                    break;
                }
                default:
                    break;
            }
        }
        createPath();
    }
}

QVariant DiagramPathItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        //foreach (Arrow *arrow, arrows) {
        //    arrow->updatePosition();
        //}
    }

    return value;
}
//! [6]
DiagramPathItem* DiagramPathItem::copy()
{
    DiagramPathItem* newDiagramPathItem=new DiagramPathItem(*this);
    return newDiagramPathItem;
}

void DiagramPathItem::write(QJsonObject &json)
{
    QPointF p=pos();
    json["x"]=p.x();
    json["y"]=p.y();
    json["z"]=zValue();
    json["type"]=type();
    json["diagramtype"]=static_cast<int>(myDiagramType);
    json["pen"]=pen().color().name();
    json["pen_alpha"]=pen().color().alpha();
    json["pen_width"]=pen().width();
    json["pen_style"]=pen().style();
    json["brush"]=brush().color().name();
    json["brush_alpha"]=brush().color().alpha();
    QJsonArray array;
    foreach(const QPointF p,myPoints){
        QJsonObject pointObj;
        pointObj["x"]=p.x();
        pointObj["y"]=p.y();
        array.append(pointObj);
    }
    json["points"]=array;
    json["m11"]=transform().m11();
    json["m12"]=transform().m12();
    json["m21"]=transform().m21();
    json["m22"]=transform().m22();
    json["dx"]=transform().dx();
    json["dy"]=transform().dy();
}

void DiagramPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
           QWidget *)
{
     painter->setPen(pen());
     painter->setBrush(Qt::NoBrush);
     painter->drawPath(getPath());
     painter->setBrush(pen().color());
     drawArrows(painter);
     // selected
     if(isSelected()){
         QBrush selBrush=QBrush(Qt::cyan);
         QPen selPen=QPen(Qt::cyan);
         if(m_textMode){
             selBrush=QBrush(Qt::blue);
             selPen=QPen(Qt::blue);
         }
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         QPointF point;
         for(int i=0;i<myPoints.count();i++)
         {
             point = myPoints.at(i);
             if(i==myHoverPoint){
                 painter->setBrush(QBrush(Qt::red));
             }
             // Rect around valid point
             painter->drawRect(QRectF(point-QPointF(2,2),point+QPointF(2,2)));
             if(i==myHoverPoint){
                 painter->setBrush(selBrush);
             }
             // show center points for text anchors
             if(i>0 && m_textMode){
                 point=(point+myPoints.at(i-1))/2;
                 painter->drawRect(QRectF(point-QPointF(2,2),point+QPointF(2,2)));
             }
         }// foreach
     }else{
         // reset textMode
         m_textMode=false;
     }
}

QRectF DiagramPathItem::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;

    QPolygonF poly(myPoints);
    QRectF r=poly.boundingRect();

    return r.adjusted(-extra, -extra, extra, extra);
}

void DiagramPathItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = e->pos();
            for(mySelPoint=0;mySelPoint<numberOfHandlerPoints();mySelPoint++){
                if(hasClickedOn(mouse_point,getHandlerPoints(mySelPoint))) break;
            }
            if(mySelPoint==numberOfHandlerPoints()) mySelPoint=-1;
            else e->accept();
            // in textMode, use point as text anchor point
            if(mySelPoint>-1 && m_textMode && childItems().count()==1){
                DiagramTextItem* textItem=dynamic_cast<DiagramTextItem*>(childItems().first());
                if(textItem){
                    textItem->setCorrectedPos(getHandlerPoints(mySelPoint));
                }
                mySelPoint=-1;
            }
        }
    }
    QGraphicsPathItem::mousePressEvent(e);
}

void DiagramPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        myPoints.replace(mySelPoint,onGrid(mouse_point));
        createPath();
        e->accept();
    }else{
        QGraphicsPathItem::mouseMoveEvent(e);
    }
}

QPainterPath DiagramPathItem::shape() const {
    QPainterPath myPath = path();
    if(isSelected()){
             foreach (QPointF point, myPoints)
             {
                 // Rect around valid point
                 QPointF pw(2*myHandlerWidth,2*myHandlerWidth);
                 myPath.addRect(QRectF(point-pw,point+pw));
             }// foreach
         }// if
    return myPath;
}

bool DiagramPathItem::hasClickedOn(QPointF press_point, QPointF point) const {
    return (
        press_point.x() >= point.x() - 2*myHandlerWidth &&
        press_point.x() <  point.x() + 2*myHandlerWidth &&
        press_point.y() >= point.y() - 2*myHandlerWidth &&
        press_point.y() <  point.y() + 2*myHandlerWidth
    );
}

void DiagramPathItem::setRoutingType(const routingType newRoutingType)
{
    switch (myRoutingType) {
        case free:
            if(newRoutingType!=free){
                append(mapToScene(myPoints.last()));
                myRoutingType=newRoutingType;
            }
            break;
        default:
            myRoutingType=newRoutingType;
            if(newRoutingType==free){
                remove();
            }
            break;
    }
    prepareGeometryChange();
    createPath();
}

/*!
 * \brief tweak collidesWithPath to only intersect on actual path and no area in between
 * \param path
 * \param mode
 * \return
 */
bool DiagramPathItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
    bool result=false;
    for(int i=0;i<myPoints.length();++i){
        // check handlers if selected
        if(isSelected()){
            QPainterPath testPath;
            testPath.moveTo(myPoints[i]);
            testPath.addRect(QRectF(myPoints[i]-QPointF(myHandlerWidth,myHandlerWidth),myPoints[i]+QPointF(myHandlerWidth,myHandlerWidth)));
            result=testPath.intersects(path);
            if(result) break;
        }
        if(i==0) continue;
        QPainterPath testPath;
        testPath.moveTo(myPoints[i-1]);
        testPath.lineTo(myPoints[i]);
        QPainterPathStroker stroker(pen());
        stroker.setWidth(2*myHandlerWidth);
        QPainterPath strokePath=stroker.createStroke(testPath);
        result=strokePath.intersects(path);
        if(result) break;
        // check handlers if selected
        if(isSelected()){
            QPainterPath testPath;
            testPath.moveTo(myPoints[i]);
            testPath.addRect(QRectF(myPoints[i]-QPointF(2,2),myPoints[i]+QPointF(2,2)));
            result=testPath.intersects(path);
            if(result) break;
        }
    }
    return result;
}
/*!
 * \brief activate text mode
 * When item is selected, in text mode, anchor points for attached texts are offered
 * \param textMode
 */
void DiagramPathItem::setTextMode(bool textMode)
{
    m_textMode=textMode;
}

QPointF DiagramPathItem::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}
/*!
 * \brief find the minimal distance between a point and a line
 * \param line
 * \param pt
 * \return minimal distance
 */
qreal DiagramPathItem::minimalDistance(QLineF &line, QPointF &pt) const
{
    QLineF testLine=QLineF(QPointF(0,0),pt-line.p1());
    qreal distX=QPointF::dotProduct(line.p2()-line.p1(),testLine.p2())/line.length();
    qreal distY=0;
    if(distX<0){
        distY=QLineF(pt,line.p1()).length();
    }else{
        if(distX>line.length()){
            distY=QLineF(pt,line.p2()).length();
        }else{
            distY=sqrt(testLine.length()*testLine.length()-distX*distX);
        }
    }
    return distY;
}
/*!
 * \brief get position of handler at index
 * Handler change during normal selection (end points of lines)
 * and during text mode (center points of lines plus end points of lines)
 * \param index
 * \return
 */
QPointF DiagramPathItem::getHandlerPoints(int index)
{
    if(m_textMode){
        if(index%2==0){
            return myPoints.at(index/2);
        }else{
            return (myPoints.at(index/2)+myPoints.at(index/2+1))/2;
        }
    }
    return myPoints.at(index);
}
/*!
 * \brief return number of handlers
 * Handler change during normal selection (end points of lines)
 * and during text mode (center points of lines plus end points of lines)
 * \return
 */
int DiagramPathItem::numberOfHandlerPoints()
{
    if(m_textMode){
        return myPoints.count()*2-1;
    }
    return myPoints.count();
}

void DiagramPathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        setCursor(Qt::SizeAllCursor);
        QPointF hover_point = onGrid(e -> pos());
        for(myHoverPoint=0;myHoverPoint<myPoints.count();myHoverPoint++){
            if(hasClickedOn(hover_point,myPoints.at(myHoverPoint))) break;
        }//for
        if(myHoverPoint==myPoints.count()) myHoverPoint=-1;
        else update();
    }
    QGraphicsPathItem::hoverEnterEvent(e);
}

void DiagramPathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        setCursor(Qt::ArrowCursor);
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    QGraphicsPathItem::hoverLeaveEvent(e);
}

DiagramPathItem::DiagramPathItem(const QJsonObject &json, QMenu *contextMenu)
{
    QPointF p;
    p.setX(json["x"].toDouble());
    p.setY(json["y"].toDouble());
    setPos(p);
    setZValue(json["z"].toDouble());
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt());
    myRoutingType=free;
    QColor color;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    QString colorString=json["pen"].toString();
    color=QColor::fromString(colorString);
#else
    color.setNamedColor(json["pen"].toString());
#endif
    color.setAlpha(json["pen_alpha"].toInt());
    QPen pen(color);
    int width=json["pen_width"].toInt();
    Qt::PenStyle pstyle=static_cast<Qt::PenStyle>(json["pen_style"].toInt(1));
    pen.setWidth(width);
    pen.setStyle(pstyle);
    setPen(pen);

    QVector<QPointF> lst;
    QJsonArray array=json["points"].toArray();
    for(int i=0;i<array.size();++i){
        QJsonObject o=array[i].toObject();
        qreal x=o["x"].toDouble();
        qreal y=o["y"].toDouble();
        lst<<QPointF(x,y);
    }
    myPoints=lst;

    qreal m11=json["m11"].toDouble();
    qreal m12=json["m12"].toDouble();
    qreal m21=json["m21"].toDouble();
    qreal m22=json["m22"].toDouble();
    qreal dx=json["dx"].toDouble();
    qreal dy=json["dy"].toDouble();
    QTransform tf(m11,m12,m21,m22,dx,dy);
    setTransform(tf);

    myContextMenu=contextMenu;

    len = 10.0; // arrow length
    breite = 4.0; // Divisor arrow width

    // standard initialize
    mySelPoint=-1;
    myHandlerWidth = 2.0;
    myHoverPoint=-1;

    createPath();

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
}
