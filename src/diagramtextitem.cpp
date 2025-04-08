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

#include "diagramtextitem.h"
#include "diagramscene.h"
#include <QTextBlockFormat>
#include <QTextDocument>
#include <QTextCursor>

#include <QJsonObject>
#include <qpainter.h>

DiagramTextItem::DiagramTextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    m_alignment=Qt::AlignLeft;

    m_updateGeometry=false;
    m_touched=false;
    updateGeometry();
    connect(document(), SIGNAL(contentsChange(int,int,int)),
             this, SLOT(updateGeometry(int,int,int)));
}
DiagramTextItem::DiagramTextItem(const DiagramTextItem& textItem)
{
    //QGraphicsTextItem();
    m_alignment=textItem.m_alignment;
    setFont(textItem.font());
    setDefaultTextColor(textItem.defaultTextColor());
    setPlainText(textItem.toPlainText());
    setTransform(textItem.transform());
    setFlags(textItem.flags());
    setCorrectedPos(textItem.anchorPoint());
    setZValue(textItem.zValue());

    m_updateGeometry=false;
    m_touched=false;
    updateGeometry();
    connect(document(), SIGNAL(contentsChange(int,int,int)),
             this, SLOT(updateGeometry(int,int,int)));
}

DiagramTextItem::DiagramTextItem(const QJsonObject &json)
    : QGraphicsTextItem(nullptr)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);

    QPointF p;
    p.setX(json["x"].toDouble());
    p.setY(json["y"].toDouble());

    QColor color;
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    color=QColor::fromString(json["color"].toString());
#else
    color.setNamedColor(json["color"].toString());
#endif
    setDefaultTextColor(color);
    const QString fontString=json["font"].toString();
    QFont font;
    font.fromString(fontString);
    setFont(font);

    QString text=json["text"].toString();
    if(text.startsWith("<!")){
        setHtml(text); // to keep compatibility, prefer plaintext from now
    }else{
        setPlainText(text);
    }
    m_alignment=static_cast<Qt::Alignment>(json["alignment"].toInt());
    setCorrectedPos(p);
    setZValue(json["z"].toDouble());


    qreal m11=json["m11"].toDouble();
    qreal m12=json["m12"].toDouble();
    qreal m21=json["m21"].toDouble();
    qreal m22=json["m22"].toDouble();
    qreal dx=json["dx"].toDouble();
    qreal dy=json["dy"].toDouble();
    QTransform tf(m11,m12,m21,m22,dx,dy);
    setTransform(tf);

    m_updateGeometry=false;
    m_touched=false;
    updateGeometry();
    connect(document(), SIGNAL(contentsChange(int,int,int)),
             this, SLOT(updateGeometry(int,int,int)));

}
QVariant DiagramTextItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
        emit selectedChange(this);
    return value;
}

void DiagramTextItem::focusOutEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    emit lostFocus(this);
    QGraphicsTextItem::focusOutEvent(event);
}
void DiagramTextItem::focusInEvent(QFocusEvent *event)
{
    emit receivedFocus(this);
    QGraphicsTextItem::focusInEvent(event);
}

void DiagramTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void DiagramTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    m_normalBoundingRect=true;
    QGraphicsTextItem::paint(painter, option, widget);
    m_normalBoundingRect=false;
    if(isSelected() && textInteractionFlags()==Qt::NoTextInteraction){
        const QBrush selBrush=QBrush(Qt::cyan);
        const QPen selPen=QPen(Qt::cyan);
        painter->setBrush(selBrush);
        painter->setPen(selPen);
        QPointF offset=calcOffset(false);
        const QPointF addOffset(m_handlerWidth,m_handlerWidth);
        const QRectF r(-offset-addOffset,-offset+addOffset);
        painter->drawRect(r);
    }

}
/*!
 * \brief return shape plus selection handles
 * \return
 */
QPainterPath DiagramTextItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    if(isSelected()){
        QPointF offset=calcOffset(false);
        const QPointF addOffset(2*m_handlerWidth,2*m_handlerWidth);
        const QRectF r(-offset-addOffset,-offset+addOffset);
        path.addRect(r);
    }
    return path;
}

QRectF DiagramTextItem::boundingRect() const
{
    // increase bounding rect when selected
    QRectF rect=QGraphicsTextItem::boundingRect();
    if(isSelected() && !m_normalBoundingRect){
        const QPointF addOffset(2*m_handlerWidth,2*m_handlerWidth);
        rect.adjust(-addOffset.x(),-addOffset.y(),
                    +addOffset.x(),+addOffset.y());
    }
    return rect;
}
/*!
 * \brief calculate the offset for item pos to anchorpoint
 * \return offset
 */
QPointF DiagramTextItem::calcOffset(bool transformed) const
{
    QPointF offset;
    const QRectF r=QGraphicsTextItem::boundingRect();
    if(m_alignment & Qt::AlignRight){
        qreal w=r.width();
        offset+=transformed ? QPointF(-w,0)*transform() : QPointF(-w,0);
    }
    if(m_alignment & Qt::AlignHCenter){
        qreal w=r.width()/2;
        offset+=transformed ? QPointF(-w,0)*transform() : QPointF(-w,0);
    }
    if(m_alignment & Qt::AlignBottom){
        qreal h=r.height();
        offset+=transformed ?  QPointF(0,-h)*transform() : QPointF(0,-h);
    }
    if(m_alignment & Qt::AlignVCenter){
        qreal h=r.height()/2;
        offset+=transformed ? QPointF(0,-h)*transform() : QPointF(0,-h);
    }
    return offset;
}
/*!
 * \brief get last calculated offset between anchor and pos
 * Needed to keep track on moved element
 * \return
 */
QPointF DiagramTextItem::getLastOffset() const
{
    return m_offset;
}
/*!
 * \brief set state for item as touched/tainted
 * Used in drawitem to check if it was manually offset from the center
 * \param state
 */
void DiagramTextItem::setTouched(bool state)
{
        m_touched=state;
}
/*!
 * \brief return if textitem was touched/tainted
 * \return
 */
bool DiagramTextItem::touched() const
{
    return m_touched;
}
/*!
 * \brief set fixed geometry
 * Keeps item from changing position/geometry when changing alignment (for Note)
 * \param state
 */
void DiagramTextItem::setFixedGeometry(bool state)
{
    m_fixedGeometry=state;
}
/*!
 * \brief get fixed geometry state
 * \return
 */
bool DiagramTextItem::getFixedGeometry() const
{
    return m_fixedGeometry;
}
/*!
 * \brief copy from this item
 * \return
 */
DiagramTextItem* DiagramTextItem::copy()
{
    DiagramTextItem* newTextItem=new DiagramTextItem(*this);
    return newTextItem;
}
/*!
 * \brief write data to json object for saving
 * \param json
 */
void DiagramTextItem::write(QJsonObject &json)
{
    const QPointF &p=m_anchorPoint;
    json["x"]=p.x();
    json["y"]=p.y();
    json["z"]=zValue();
    json["type"]=type();
    json["text"]=toPlainText();
    json["font"]=font().toString();
    json["alignment"]=static_cast<int>(m_alignment);
    json["color"]=defaultTextColor().name(QColor::HexArgb);
    json["m11"]=transform().m11();
    json["m12"]=transform().m12();
    json["m21"]=transform().m21();
    json["m22"]=transform().m22();
    json["dx"]=transform().dx();
    json["dy"]=transform().dy();
}
/*!
 * \brief set text alignment
 * \param alignment
 */
void DiagramTextItem::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();      // save cursor position
    int position = textCursor().position();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    cursor.setPosition(position);           // restore cursor position
    setTextCursor(cursor);
}

Qt::Alignment DiagramTextItem::alignment() const
{
    return m_alignment;
}

void DiagramTextItem::setCorrectedPos(QPointF pt)
{
    m_anchorPoint=pt;
    m_offset=calcOffset();
    setPos(pt+m_offset);
}
/*!
 * \brief set anchor point
 * \param pt
 */
void DiagramTextItem::setAnchorPoint(QPointF pt)
{
    m_anchorPoint=pt;
}
/*!
 * \brief return anchor point
 * \return
 */
QPointF DiagramTextItem::anchorPoint() const
{
    return m_anchorPoint;
}

void DiagramTextItem::updateGeometry(int, int, int)
{
    updateGeometry();
}
/*!
 * \brief update Geometry when text is changed
 * Necessary to keep alignment
 * item position needs to be updated depending on text width/height
 */
void DiagramTextItem::updateGeometry()
{
    if(m_updateGeometry || m_fixedGeometry) return;
    m_updateGeometry=true;
    setTextWidth(-1);
    qreal w=document()->idealWidth();
    setTextWidth(w);
    setAlignment(m_alignment);
    m_offset=calcOffset();
    setPos(m_anchorPoint+m_offset);
    m_updateGeometry=false;
}

