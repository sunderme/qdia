/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "diagramitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QJsonObject>

//! [0]
DiagramItem::DiagramItem(DiagramType diagramType, QMenu *contextMenu,
                         QGraphicsItem *parent)
    : QGraphicsPathItem(parent), myContextMenu(contextMenu)
    , myDiagramType(diagramType)
{
    mPainterPath = createPath();
    setPath(mPainterPath);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}
//! [0]
DiagramItem::DiagramItem(const DiagramItem& diagram)
{

    QGraphicsPathItem(diagram.path(),diagram.parentItem());
    // copy from general GraphcsItem
    setBrush(diagram.brush());
    setPen(diagram.pen());
    setTransform(diagram.transform());

    // copy DiagramItem
    myDiagramType = diagram.myDiagramType;
    myContextMenu = diagram.myContextMenu;

    mPainterPath = diagram.mPainterPath;
    setPath(mPainterPath);
    setFlags(diagram.flags());
}

DiagramItem::DiagramItem(QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    myDiagramType = None;
    myContextMenu = contextMenu;

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

DiagramItem::DiagramItem(const QJsonObject &json, QMenu *contextMenu)
{
    QPointF p;
    p.setX(json["x"].toDouble());
    p.setY(json["y"].toDouble());
    setPos(p);
    setZValue(json["z"].toDouble());
    myDiagramType=static_cast<DiagramType>(json["diagramtype"].toInt(DiagramType::None));
    QColor color;
    color=QColor::fromString(json["pen"].toString());
    color.setAlpha(json["pen_alpha"].toInt());
    int width=json["pen_width"].toInt(1);
    Qt::PenStyle pstyle=static_cast<Qt::PenStyle>(json["pen_style"].toInt(1));
    QPen pen(color);
    pen.setWidth(width);
    pen.setStyle(pstyle);
    setPen(pen);
    color=QColor::fromString(json["brush"].toString());
    color.setAlpha(json["brush_alpha"].toInt());
    Qt::BrushStyle style=static_cast<Qt::BrushStyle>(json["brush_style"].toInt());
    QBrush b(color,style);
    setBrush(b);

    qreal m11=json["m11"].toDouble();
    qreal m12=json["m12"].toDouble();
    qreal m21=json["m21"].toDouble();
    qreal m22=json["m22"].toDouble();
    qreal dx=json["dx"].toDouble();
    qreal dy=json["dy"].toDouble();
    QTransform tf(m11,m12,m21,m22,dx,dy);
    setTransform(tf);

    myContextMenu=contextMenu;
    if(myDiagramType!=None){
        mPainterPath = createPath();
        setPath(mPainterPath);
    }

    setFlag(QGraphicsItem::ItemIsMovable, json["moveable"].toBool(true));
    setFlag(QGraphicsItem::ItemIsSelectable, json["selectable"].toBool(true));
}

QPixmap DiagramItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(125, 125);
    painter.drawPath(mPainterPath);

    return pixmap;
}

void DiagramItem::setBoundingBox(QRectF rect)
{
    QPainterPath path;
    path.addRect(rect);
    mPainterPath=path;
    setPath(path);
}

void DiagramItem::setLocked(bool locked)
{
    m_isLocked = locked;
    if (locked) {
        setFlag(QGraphicsItem::ItemIsMovable, false);
    } else {
        setFlag(QGraphicsItem::ItemIsMovable, true);
    }
}
/*!
 * \brief return locked state
 * \return
 */
bool DiagramItem::isLocked()
{
    return m_isLocked;
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if(myContextMenu){
        myContextMenu->exec(event->screenPos());
    }
}

QVariant DiagramItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    return value;
}
DiagramItem* DiagramItem::copy()
{
    DiagramItem* newDiagramItem=new DiagramItem(*this);
    return newDiagramItem;
}

void DiagramItem::write(QJsonObject &json)
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
    json["brush_style"]=brush().style();
    json["m11"]=transform().m11();
    json["m12"]=transform().m12();
    json["m21"]=transform().m21();
    json["m22"]=transform().m22();
    json["dx"]=transform().dx();
    json["dy"]=transform().dy();
    json["moveable"]=flags().testFlag(QGraphicsItem::ItemIsMovable);
    json["selectable"]=flags().testFlag(QGraphicsItem::ItemIsSelectable);
}

QPainterPath DiagramItem::createPath()
{
    QPainterPath path;
    QPolygonF myPolygon;
    switch (myDiagramType) {
        case StartEnd:
            path.moveTo(200, 50);
            path.arcTo(150, 0, 50, 50, 0, 90);
            path.arcTo(50, 0, 50, 50, 90, 90);
            path.arcTo(50, 50, 50, 50, 180, 90);
            path.arcTo(150, 50, 50, 50, 270, 90);
            path.lineTo(200, 25);
            break;
        case Conditional:
            myPolygon << QPointF(-100, 0) << QPointF(0, 100)
                      << QPointF(100, 0) << QPointF(0, -100)
                      << QPointF(-100, 0);
            path.addPolygon(myPolygon);
            break;
        case Step:
            myPolygon << QPointF(-100, -100) << QPointF(100, -100)
                      << QPointF(100, 100) << QPointF(-100, 100)
                      << QPointF(-100, -100);
            path.addPolygon(myPolygon);
            break;
        default:
            myPolygon << QPointF(-120, -80) << QPointF(-70, 80)
                      << QPointF(120, 80) << QPointF(70, -80)
                      << QPointF(-120, -80);
            path.addPolygon(myPolygon);
            break;
    }
    return path;
}
