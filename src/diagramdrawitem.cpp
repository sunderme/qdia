#include "diagramdrawitem.h"

#include <QtGui>
#include <math.h>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "diagramdrawitem.h"
#include "diagramscene.h"

//! [0]
DiagramDrawItem::DiagramDrawItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent)
    : DiagramItem(contextMenu,parent)
{
    myPos2=pos();
    myDiagramType = diagramType;
    myRadius=5.0;
    mStartPoint=QPointF(20,0);
    mEndPoint=QPointF(0,20);

    mPainterPath=createPath();
    setPath(mPainterPath);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
    myRadius=5.0;
    if(diagramType==Note){
        myRadius=10.;
    }
}

DiagramDrawItem::DiagramDrawItem(const DiagramDrawItem& diagram)
    : DiagramItem(diagram.myContextMenu,diagram.parentItem())
{

    myDiagramType=diagram.myDiagramType;
    myRadius=diagram.myRadius;
    mStartPoint=diagram.mStartPoint;
    mEndPoint=diagram.mEndPoint;
    // copy from general GraphcsItem
    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());
    myPos2=diagram.myPos2;
    mPainterPath=createPath();
    setPath(mPainterPath);
    setFlags(diagram.flags());
    setAcceptHoverEvents(true);
    setPos(diagram.pos());
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;

}

DiagramDrawItem::DiagramDrawItem(const QJsonObject &json, QMenu *contextMenu):DiagramItem(json,contextMenu)
{
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt());
    qreal width=json["width"].toDouble();
    qreal height=json["height"].toDouble();
    myPos2=QPointF(width,height);
    myRadius=5.0;
    qreal x0=json["start_x"].toDouble();
    qreal y0=json["start_y"].toDouble();
    qreal x1=json["end_x"].toDouble();
    qreal y1=json["end_y"].toDouble();
    mStartPoint=QPointF(x0,y0);
    mEndPoint=QPointF(x1,y1);

    mPainterPath=createPath();
    setPath(mPainterPath);
    setAcceptHoverEvents(true);
    myHoverPoint=-1;
    mySelPoint=-1;
    myHandlerWidth=2.0;
}
//! [1]
QPainterPath DiagramDrawItem::createPath()
{
    qreal dx=myPos2.x();
    qreal dy=myPos2.y();

    QPainterPath path;
    switch (myDiagramType) {
    case Rectangle:
        path.moveTo(0, 0);
        path.lineTo(dx,0);
        path.lineTo(dx,dy);
        path.lineTo(0,dy);
        path.lineTo(0,0);
        break;
    case Ellipse:
        path.addEllipse(0,0,dx,dy);
        break;
    case Circle:
        path.addEllipse(0,0,dx,dy);
        break;
    case RoundedRect:
    {
        Qt::SizeMode sizeMode=Qt::AbsoluteSize;
        // circumvent problem when radius larger than width/height
        qreal r=myRadius;
        if ((fabs(dx)<3*myRadius)or(fabs(dy)<3*myRadius)) {
            r=0;
        }
        path.addRoundedRect(0,0,dx,dy,r,r,sizeMode);
        break;
    }
    case Rhombus:
        path.moveTo(dx/2, 0);
        path.lineTo(dx,dy/2);
        path.lineTo(dx/2,dy);
        path.lineTo(0,dy/2);
        path.lineTo(dx/2,0);
        break;
    case Triangle:
        path.moveTo(0, 0);
        path.lineTo(0,dy);
        path.lineTo(dx,dy/2);
        path.lineTo(0,0);
        break;
    case DA:
        path.moveTo(0, 0);
        path.lineTo(dx*0.66,0);
        path.lineTo(dx,dy/2);
        path.lineTo(dx*0.66,dy);
        path.lineTo(0,dy);
        path.lineTo(0,0);
        break;
    case OTA:
        path.moveTo(0, 0);
        path.lineTo(0,dx);
        path.lineTo(dx,dx/2);
        path.lineTo(0,0);
        break;
    case Note:
        path.moveTo(myRadius, 0);
        path.lineTo(dx,0);
        path.lineTo(dx,dy);
        path.lineTo(0,dy);
        path.lineTo(0,myRadius);
        path.lineTo(myRadius,0);
        break;
    case Pie:
    {
        qreal compression=myPos2.y()/myPos2.x(); // calculate compression because it is an ellipse instead of a circle
        QLineF ln(0,0,mStartPoint.x(),mStartPoint.y()/compression);
        qreal startAngle=ln.angle();
        QLineF ln2(0,0,mEndPoint.x(),mEndPoint.y()/compression);
        qreal angle=ln.angleTo(ln2);
        if(abs(angle)<0.1) angle=360;
        path.arcMoveTo(0,0,dx,dy,startAngle);
        path.arcTo(0,0,dx,dy,startAngle,angle);
        break;
    }
    default:
        break;
    }
    return path;
}

QPixmap DiagramDrawItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(10, 10);
    painter.drawPath(mPainterPath);

    return pixmap;
}

void DiagramDrawItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if(myContextMenu){
        myContextMenu->exec(event->screenPos());
    }
}

QVariant DiagramDrawItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        ;
    }
    if(change == QGraphicsItem::ItemSelectedHasChanged){
        if(value.toBool()){
            // check if text is still in center
            for(auto *i:childItems()){
                DiagramTextItem *textItem=qgraphicsitem_cast<DiagramTextItem*>(i);
                if(textItem){
                    if(textItem->pos()-textItem->calcOffset()!=textItem->anchorPoint()){
                        textItem->setTouched();
                    }
                }
            }
        }
    }

    return value;
}

DiagramItem* DiagramDrawItem::copy()
{
    DiagramDrawItem* newDiagramDrawItem=new DiagramDrawItem(*this);
    return dynamic_cast<DiagramItem*>(newDiagramDrawItem);
}

void DiagramDrawItem::write(QJsonObject &json)
{
    DiagramItem::write(json);
    json["diagramtype"]=static_cast<int>(myDiagramType);
    json["width"]=myPos2.x();
    json["height"]=myPos2.y();
    if(myDiagramType==Pie){
        json["x0"]=mStartPoint.x();
        json["y0"]=mStartPoint.y();
        json["x1"]=mEndPoint.x();
        json["y1"]=mEndPoint.y();
    }

}

void DiagramDrawItem::setPos2(qreal x,qreal y)
{
    setPos2(QPointF(x,y));
}

void DiagramDrawItem::setPos2(QPointF newPos)
{
    prepareGeometryChange();
    myPos2=mapFromScene(newPos);
    if(myDiagramType==Circle){
        // special treatment in case of circle
        if(fabs(myPos2.x())>fabs(myPos2.y())){
            if(myPos2.y()<0) myPos2.setY(-fabs(myPos2.x()));
            else myPos2.setY(fabs(myPos2.x()));
        }
        else {
            if(myPos2.x()<0) myPos2.setX(-fabs(myPos2.y()));
            else myPos2.setX(fabs(myPos2.y()));
        }
    }
    mPainterPath=createPath();
    setPath(mPainterPath);
}

void DiagramDrawItem::setDimension(QPointF newPos)
{
    prepareGeometryChange();
    mySetDimension(newPos);
    mPainterPath=createPath();
    setPath(mPainterPath);
}

void DiagramDrawItem::mySetDimension(QPointF newPos)
{
    if(myDiagramType==Circle){
        // special treatment in case of circle
        if(myPos2.x()!=newPos.x()){
            myPos2=newPos;
            if(myPos2.y()<0) myPos2.setY(-fabs(myPos2.x()));
                        else myPos2.setY(fabs(myPos2.x()));
            }
        else {
            myPos2=newPos;
            if(myPos2.x()<0) myPos2.setX(-fabs(myPos2.y()));
            else myPos2.setX(fabs(myPos2.y()));
        }
    }
    else myPos2=newPos;
}
/*!
 * \brief return position of the stretch handlers
 * \param i number of handler
 * clock-wise, start left top
 * \return
 */
QPointF DiagramDrawItem::getHandler(int i) const
{
    QPointF point;
    QRectF rect=innerBoundingRect();

    if(i<3) point=QPointF(rect.left()+rect.width()/2*i,rect.top());
    if(i==3) point=QPointF(rect.right(),rect.bottom()-rect.height()/2);
    if(i>3 && i<7) point=QPointF(rect.left()+rect.width()/2*(i-4),rect.bottom());
    if(i==7) point=QPointF(rect.left(),rect.bottom()-rect.height()/2);
    if(i==8) point=mStartPoint+myPos2/2;
    if(i==9) point=mEndPoint+myPos2/2;
    return point;
}
/*!
 * \brief return number of used handles
 * \return
 */
int DiagramDrawItem::getNumberOfHandles() const
{
    int nHandles=8;
    if(myDiagramType==Pie){
        nHandles=10;
    }
    return nHandles;
}

QPointF DiagramDrawItem::getDimension()
{
    return myPos2;
}
/*!
 * \brief set start point for pie/arc and similar
 * \param pt
 */
void DiagramDrawItem::setStartPoint(const QPointF pt)
{
    mStartPoint=pt;
    if(myDiagramType==Pie){
        mPainterPath=createPath();
    }
}

/*!
 * \brief set end point for pie/arc and similar
 * \param pt
 */
void DiagramDrawItem::setEndPoint(const QPointF pt)
{
    mEndPoint=pt;
    if(myDiagramType==Pie){
        mPainterPath=createPath();
    }
}


void DiagramDrawItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
           QWidget *)
{
     painter->setPen(pen());
     painter->setBrush(brush());
     painter->drawPath(path());
     // selected
     if(isSelected()){
         // Rect
         QPen selPen=QPen(Qt::DashLine);
         selPen.setColor(Qt::black);
         QBrush selBrush=QBrush(Qt::NoBrush);
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         QRectF rect=innerBoundingRect();
         painter->drawRect(rect);
         if(myDiagramType==Pie){
             // extra lines for pie/arc
             painter->drawLine(myPos2/2,mStartPoint+myPos2/2);
             painter->drawLine(myPos2/2,mEndPoint+myPos2/2);
         }
         // Draghandles
         selBrush=QBrush(Qt::cyan,Qt::SolidPattern);
         selPen=QPen(Qt::cyan);
         painter->setBrush(selBrush);
         painter->setPen(selPen);
         QPointF point;
         int nPoints=getNumberOfHandles();

         for(int i=0;i<nPoints;i++)
         {
             point=getHandler(i);
             if(i==myHoverPoint){
                 painter->setBrush(QBrush(Qt::red));
             }
             // Rect around valid point
             painter->drawRect(QRectF(point-QPointF(2,2),point+QPointF(2,2)));
             if(i==myHoverPoint){
                 painter->setBrush(selBrush);
             }
         }// foreach
     }// if
}

void DiagramDrawItem::hoverMoveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        QPointF hover_point = e -> pos();
        QPointF point;
        int numberOfHandles=getNumberOfHandles();
        for(myHoverPoint=0;myHoverPoint<numberOfHandles;myHoverPoint++){
            point=getHandler(myHoverPoint);
            if(hasClickedOn(hover_point,point)) break;
        }//for
        if(myHoverPoint==numberOfHandles) myHoverPoint=-1;
        else update();
    }
    DiagramItem::hoverMoveEvent(e);
}

void DiagramDrawItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    if (isSelected()) {
        if(myHoverPoint>-1){
            myHoverPoint=-1;
            update();
        }
    }
    DiagramItem::hoverLeaveEvent(e);
}

bool DiagramDrawItem::hasClickedOn(QPointF press_point, QPointF point) const {
    return (
        press_point.x() >= point.x() - myHandlerWidth &&\
        press_point.x() <  point.x() + myHandlerWidth &&\
        press_point.y() >= point.y() - myHandlerWidth &&\
        press_point.y() <  point.y() + myHandlerWidth
    );
}

QPointF DiagramDrawItem::onGrid(QPointF pos)
{
    DiagramScene* myScene = dynamic_cast<DiagramScene*>(scene());
    QPointF result = myScene->onGrid(pos);
    return result;
}

QPainterPath DiagramDrawItem::shape() const {
    QPainterPath myPath;
    myPath=path();
    if(isSelected()){
        QPointF point;
        int numberOfHandles=getNumberOfHandles();
        for(int i=0;i<numberOfHandles;i++)
        {
            point=getHandler(i);
            // Rect around valid point
            myPath.addRect(QRectF(point-QPointF(myHandlerWidth,myHandlerWidth),point+QPointF(myHandlerWidth,myHandlerWidth)));
        }// for
    }// if
    return myPath;
}
/*!
 * \brief return boundrect of actual structure plus helper structures
 * Helperstructure are usually the handles in selected state
 * \return
 */
QRectF DiagramDrawItem::boundingRect() const
{
    qreal extra = isSelected() ? pen().width()+20 / 2.0 + myHandlerWidth : 0.0;

    QRectF newRect = innerBoundingRect().adjusted(-extra, -extra, extra, extra);

    if(myDiagramType==Pie){
        QRectF helper(mStartPoint+myPos2/2,mEndPoint+myPos2/2);
        newRect=newRect.united(helper);
    }

    return newRect;
}
/*!
 * \brief return raw bounding rect without extra space fopr handlers
 * \return
 */
QRectF DiagramDrawItem::innerBoundingRect() const
{
    QRectF newRect = path().boundingRect();
    if(myDiagramType==Pie){
        newRect=QRectF(QPointF(0,0),myPos2);
    }

    return newRect;
}

void DiagramDrawItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if(isSelected()){
        if (e -> buttons() & Qt::LeftButton) {
            QPointF mouse_point = e -> pos();
            QPointF point;
            int numberOfHandles=getNumberOfHandles();
            for(mySelPoint=0;mySelPoint<numberOfHandles;mySelPoint++){
                point=getHandler(mySelPoint);
                if(hasClickedOn(mouse_point,point)) break;
            }//for
            if(mySelPoint==getNumberOfHandles()){
                mySelPoint=-1;
            }else{
                if(mySelPoint<8){
                    mRect=Rect(myPos2,mySelPoint);
                }
                e->accept();
            }
        }
    }
    DiagramItem::mousePressEvent(e);
}

void DiagramDrawItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
    // left click
    if ((e -> buttons() & Qt::LeftButton)&&(mySelPoint>-1)) {
        QPointF mouse_point = onGrid(e -> pos());
        prepareGeometryChange();
        if(mySelPoint<8){
            mRect.movePoint(mouse_point);
            QPointF anchorPoint=mRect.anchorPoint();
            mRect.translate(-anchorPoint); // renormalize: anchor is at 0/0, the item is moved instead
            mySetDimension(mRect.point());
            setPos(mapToScene(anchorPoint));
        }else{
            if(mySelPoint==8){
                mStartPoint=mouse_point-myPos2/2;
            }
            if(mySelPoint==9){
                mEndPoint=mouse_point-myPos2/2;
            }
        }
        mPainterPath=createPath();
        setPath(mPainterPath);
        // update text position if present
        // currently only center
        for(auto *i:childItems()){
            DiagramTextItem *textItem=qgraphicsitem_cast<DiagramTextItem*>(i);
            if(textItem && !textItem->touched() ){
                if(textItem->getFixedGeometry()){
                    // update text width for fixed text scenarios (e.g. Note)
                    textItem->setTextWidth(myPos2.x());
                }else{
                    textItem->setCorrectedPos(boundingRect().center());
                }
            }
        }
        e->setAccepted(true);
    }
    else
        DiagramItem::mouseMoveEvent(e);
}

Rect::Rect()
{
    m_selPoint=-1;
    m_mvAnchorX=false;
    m_mvPointX=false;
    m_mvAnchorY=false;
    m_mvPointY=false;
}

Rect::Rect(QPointF pt,int selPoint)
{
    m_point=pt;
    m_mvAnchorX=false;
    m_mvPointX=false;
    m_mvAnchorY=false;
    m_mvPointY=false;
    // anchor is 0/0
    setSelPoint(selPoint);
}

QPointF Rect::anchorPoint() const
{
    return m_anchor;
}

QPointF Rect::point() const
{
    return m_point;
}

void Rect::setSelPoint(int selPoint)
{
    m_selPoint=selPoint;
    switch (m_selPoint) {
        case 0:
            setTopLeft();
            break;
        case 1:
            setTop();
            break;
        case 2:
            setTopRight();
            break;
        case 3:
            setRight();
            break;
        case 6:
            setBottomRight();
            break;
        case 5:
            setBottom();
            break;
        case 4:
            setBottomLeft();
            break;
        case 7:
            setLeft();
            break;
        default:
            break;
    }
}
/*!
 * \brief manipulate left side of rect
 * change anchor or point, depending which one is more left
 * \param x
 */
void Rect::setLeft()
{
    if(m_anchor.x()<m_point.x()){
        m_mvAnchorX=true;
        m_mvPointX=false;
    }else{
        m_mvAnchorX=false;
        m_mvPointX=true;
    }
}
/*!
 * \brief manipulate right side of rect
 * change anchor or point, depending which one is more right
 * \param x
 */
void Rect::setRight()
{
    if(m_anchor.x()>m_point.x()){
        m_mvAnchorX=true;
        m_mvPointX=false;
    }else{
        m_mvAnchorX=false;
        m_mvPointX=true;
    }
}
/*!
 * \brief manipulate top side of rect
 * change anchor or point, depending which one is more up
 * \param x
 */
void Rect::setTop()
{
    if(m_anchor.y()<m_point.y()){
        m_mvAnchorY=true;
        m_mvPointY=false;
    }else{
        m_mvAnchorY=false;
        m_mvPointY=true;
    }
}
/*!
 * \brief manipulate bottom side of rect
 * change anchor or point, depending which one is more down
 * \param x
 */
void Rect::setBottom()
{
    if(m_anchor.y()>m_point.y()){
        m_mvAnchorY=true;
        m_mvPointY=false;
    }else{
        m_mvAnchorY=false;
        m_mvPointY=true;
    }
}
/*!
 * \brief setTopLeft of rectangle
 * \param pt
 */
void Rect::setTopLeft()
{
    setLeft();
    setTop();
}
/*!
 * \brief setTopLeft of rectangle
 * \param pt
 */
void Rect::setTopRight()
{
    setRight();
    setTop();
}
/*!
 * \brief setTopLeft of rectangle
 * \param pt
 */
void Rect::setBottomLeft()
{
    setLeft();
    setBottom();
}
/*!
 * \brief setTopLeft of rectangle
 * \param pt
 */
void Rect::setBottomRight()
{
    setRight();
    setBottom();
}
/*!
 * \brief move pos/anchor depending on sel point
 * \param pt
 */
void Rect::movePoint(QPointF pt)
{
    if(m_mvPointX){
        m_point.setX(pt.x());
    }
    if(m_mvAnchorX){
        m_anchor.setX(pt.x());
    }
    if(m_mvPointY){
        m_point.setY(pt.y());
    }
    if(m_mvAnchorY){
        m_anchor.setY(pt.y());
    }
}

/*!
 * \brief translate rect by pt
 * \param pt
 */
void Rect::translate(QPointF pt)
{
    m_anchor+=pt;
    m_point+=pt;
}
