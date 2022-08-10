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

#include "diagramscene.h"
#include <math.h>

#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QXmlStreamWriter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QtGui>

//! [0]
DiagramScene::DiagramScene(QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemType = DiagramItem::Step;
    textItem = nullptr;
    insertedItem = nullptr;
    insertedDrawItem = nullptr;
    insertedPathItem = nullptr;
    insertedSplineItem = nullptr;
    myDx=0.0;
    myDy=0.0;
    maxZ=0;
    myItemColor = Qt::white;
    myTextColor = Qt::black;
    myLineColor = Qt::black;
    myLineWidth = 1;
    myPenStyle = Qt::SolidLine;
    m_undoPos = -1;

    myRouting=DiagramPathItem::free;
    myGrid=10.0;
    myGridVisible=false;
    myGridScale=1;
    // no Item in Moveitems
    myMoveItems.clear();
    // initialisiere Cursor
    myCursorWidth = 4.0;
    myCursor.setRect(QRectF(-myCursorWidth/2,-myCursorWidth/2,myCursorWidth,myCursorWidth));
    myCursor.setPen(QPen(Qt::gray));
    myCursor.setZValue(10.0);
    addItem(&myCursor);
}

void DiagramScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramItem *item = dynamic_cast<DiagramItem *>(elem);
        if(item){
            QPen pen=item->pen();
            pen.setColor(myLineColor);
            item->setPen(pen);
        }
        DiagramPathItem *pathItem = dynamic_cast<DiagramPathItem *>(elem);
        if(pathItem){
            QPen pen=pathItem->pen();
            pen.setColor(myLineColor);
            pathItem->setPen(pen);
        }
    }
}

void DiagramScene::setTextColor(const QColor &color)
{
    myTextColor = color;
    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramTextItem *item = dynamic_cast<DiagramTextItem *>(elem);
        if(item)
            item->setDefaultTextColor(myTextColor);
    }
}

void DiagramScene::setItemColor(const QColor &color)
{
    myItemColor = color;
    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramItem *item = dynamic_cast<DiagramItem *>(elem);
        if(item){
            item->setBrush(myItemColor);
        }
    }
}

void DiagramScene::setLineWidth(const int w)
{
    myLineWidth = w;
    foreach(QGraphicsItem *elem,selectedItems()){
        QGraphicsPathItem *item = dynamic_cast<QGraphicsPathItem *>(elem);
        if(item){
            QPen pen=item->pen();
            pen.setWidth(w);
            item->setPen(pen);
        }
    }
}

void DiagramScene::setLinePattern(const Qt::PenStyle style)
{
    myPenStyle=style;
    foreach(QGraphicsItem *elem,selectedItems()){
        QGraphicsPathItem *item = dynamic_cast<QGraphicsPathItem *>(elem);
        if(item){
            QPen pen=item->pen();
            pen.setStyle(style);
            item->setPen(pen);
        }
    }
}
/*!
 * \brief set text alignment of selected elements
 * \param alignment
 */
void DiagramScene::setTextAlignment(const Qt::Alignment alignment)
{
    m_textAlignment = alignment;
    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramTextItem *item = dynamic_cast<DiagramTextItem *>(elem);
        if(item){
            item->setAlignment(alignment);
            item->updateGeometry();
        }
    }
}
/*!
 * \brief DiagramScene::textAlignment
 * \return
 */
Qt::Alignment DiagramScene::textAlignment() const
{
    return m_textAlignment;
}
/*!
 * \brief set text font
 * Update selected text elements
 * \param font
 */
void DiagramScene::setFont(const QFont &font)
{
    myFont = font;

    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(elem);
        if (item)
            item->setFont(myFont);
    }
}

void DiagramScene::setMode(DiagramScene::Mode mode, bool m_abort)
{
    if(m_abort) abort(true);

    myMode = mode;
    switch (mode) {
    case MoveItem:
    case MoveItems:
    case CopyItem:
        enableAllItems(true);
        break;
    default:
        enableAllItems(false);
        break;
    }
}

void DiagramScene::enableAllItems(bool enable)
{
    foreach(QGraphicsItem* item,items()){
        item->setEnabled(enable);
    }
}
/*!
 * \brief make a text item as child of item
 * \param item
 * \return text item
 */
DiagramTextItem *DiagramScene::makeTextItem(QGraphicsItem *item)
{
    textItem = new DiagramTextItem();
    textItem->setFont(myFont);
    textItem->setAlignment(m_textAlignment);
    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    textItem->setZValue(1000.0);
    connect(textItem, &DiagramTextItem::lostFocus,
            this, &DiagramScene::editorLostFocus);
    connect(textItem, &DiagramTextItem::receivedFocus,
            this, &DiagramScene::editorReceivedFocus);
    connect(textItem, &DiagramTextItem::selectedChange,
            this, &DiagramScene::itemSelected);
    //addItem(textItem);
    textItem->setParentItem(item);
    textItem->setDefaultTextColor(myTextColor);
    textItem->setSelected(true);
    textItem->setFocus();
    return textItem;
}

void DiagramScene::setItemType(DiagramItem::DiagramType type)
{
    myItemType = type;
}
void DiagramScene::setItemType(DiagramDrawItem::DiagramType type)
{
    myDrawItemType = type;
}

void DiagramScene::setItemType(QString fn)
{
    mItemFileName=fn;
}

//! [5]
void DiagramScene::editorLostFocus(DiagramTextItem *item)
{
    QTextCursor cursor = item->textCursor();
    cursor.clearSelection();
    item->setTextCursor(cursor);

    if (item->toPlainText().isEmpty()) {
        removeItem(item);
        item->deleteLater();
    }else{
        takeSnapshot();
    }
    emit editorHasLostFocus();
}
void DiagramScene::wheelEvent(QGraphicsSceneWheelEvent *mouseEvent)
{
    if(mouseEvent->modifiers()==Qt::ControlModifier){
        int i =  mouseEvent->delta();
        qreal factor;
        if(i>=0){
            factor = i/100.0;
        }
        else {
            factor = -100.0/i; // negative Richtung ...
        }
        emit zoomPointer(factor,mouseEvent->scenePos());
        mouseEvent->setAccepted(true);
        return;
    }
    mouseEvent->ignore();
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::RightButton){
        abort();
        mouseEvent->accept();
        return;
    }

    bool middleButton=false;
    if (mouseEvent->button() == Qt::MiddleButton){
        middleButton=true;
        switch (myMode) {
        case InsertLine:
            if (insertedPathItem != nullptr){
                insertedPathItem->updateLast(onGrid(mouseEvent->scenePos()));
                insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                insertedPathItem->setEnabled(false);
                insertedPathItem = nullptr;
                mouseEvent->accept();
                takeSnapshot();
                return;
            }
            break;
        case InsertSpline:
            if(insertedSplineItem){
                insertedSplineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                insertedSplineItem->setEnabled(false);
                insertedSplineItem=nullptr;
                mouseEvent->accept();
                takeSnapshot();
                return;
            }
            break;
        case InsertItem:
        case InsertElement:
            if (insertedItem){
                insertedItem=nullptr;
                myMode=MoveItem;
                mouseEvent->accept();
                takeSnapshot();
                // switch toolbar !!
                emit abortSignal();
                return;
            }
            break;
        case InsertDrawItem:
            if(insertedDrawItem){
                insertedDrawItem->setPos2(onGrid(mouseEvent->scenePos()));
                insertedDrawItem->setEnabled(false);
                insertedDrawItem = nullptr;
                myMode=MoveItem;
                takeSnapshot();
                mouseEvent->accept();
                // switch toolbar !!
                emit abortSignal();
                return;
            }
        default:
            ;
        }
    }

    switch (myMode) {
    case InsertItem:
        if(insertedItem==nullptr){
            insertedItem = new DiagramItem(myItemType, myItemMenu);
            insertedItem->setBrush(myItemColor);
            QPen pen(myLineColor);
            pen.setWidth(myLineWidth);
            pen.setStyle(myPenStyle);
            insertedItem->setPen(pen);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        emit itemInserted(insertedItem);
        insertedItem->setSelected(false);
        insertedItem->setEnabled(false);
        insertedItem=nullptr;
        takeSnapshot();
        break;
    case InsertLine:
        if (insertedPathItem == nullptr){
            insertedPathItem = new DiagramPathItem(DiagramPathItem::DiagramType(myArrow),myItemMenu);
            QPen pen(myLineColor);
            pen.setWidth(myLineWidth);
            pen.setStyle(myPenStyle);
            insertedPathItem->setPen(pen);
            insertedPathItem->setBrush(myLineColor);
            insertedPathItem->setZValue(maxZ);
            insertedPathItem->setRoutingType(myRouting);
            maxZ+=0.1;
            addItem(insertedPathItem);
            insertedPathItem->setPos(onGrid(mouseEvent->scenePos()));
            insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
        insertedPathItem->append(onGrid(mouseEvent->scenePos()));
        break;
    case InsertSpline:
        if (insertedSplineItem == nullptr){
            insertedSplineItem = new DiagramSplineItem(DiagramSplineItem::DiagramType(myArrow),myItemMenu);
            QPen pen(myLineColor);
            pen.setWidth(myLineWidth);
            pen.setStyle(myPenStyle);
            insertedSplineItem->setPen(pen);
            //insertedSplineItem->setBrush(myLineColor);
            insertedSplineItem->setZValue(maxZ);
            maxZ+=0.1;
            insertedSplineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            insertedSplineItem->setEnabled(true);
            insertedSplineItem->setSelected(true);
            addItem(insertedSplineItem);
            insertedSplineItem->setPos(onGrid(mouseEvent->scenePos()));

        }else{
            insertedSplineItem->nextActive();
            insertedSplineItem->setSelected(true);
            QPointF p=insertedSplineItem->getActivePoint();
            p=insertedSplineItem->mapToScene(p);
            emit forceCursor(p);
        }
        insertedSplineItem->updateActive(onGrid(mouseEvent->scenePos()));
        break;
    case InsertText:
        textItem = new DiagramTextItem();
        textItem->setFont(myFont);
        textItem->setAlignment(m_textAlignment);
        textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
        textItem->setZValue(1000.0);
        connect(textItem, &DiagramTextItem::lostFocus,
                this, &DiagramScene::editorLostFocus);
        connect(textItem, &DiagramTextItem::receivedFocus,
                this, &DiagramScene::editorReceivedFocus);
        connect(textItem, &DiagramTextItem::selectedChange,
                this, &DiagramScene::itemSelected);
        addItem(textItem);
        textItem->setDefaultTextColor(myTextColor);
        textItem->setCorrectedPos(onGrid(mouseEvent->scenePos()));
        textItem->setSelected(true);
        textItem->setFocus();
        emit textInserted(textItem);
        takeSnapshot();
        mouseEvent->accept();
        return;
        break;
    case InsertDrawItem:
        if (insertedDrawItem == nullptr){
            insertedDrawItem = new DiagramDrawItem(myDrawItemType, myItemMenu);
            insertedDrawItem->setBrush(myItemColor);
            QPen pen(myLineColor);
            pen.setWidth(myLineWidth);
            pen.setStyle(myPenStyle);
            insertedDrawItem->setPen(pen);
            insertedDrawItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedDrawItem);
            insertedDrawItem->setPos(onGrid(mouseEvent->scenePos()));
        }
        else
        {
            insertedDrawItem->setPos2(onGrid(mouseEvent->scenePos()));
            insertedDrawItem->setEnabled(false);
            insertedDrawItem = nullptr;
            takeSnapshot();
        }
        break;
    case InsertElement:
        if(insertedItem==nullptr){
            insertedItem = new DiagramElement(mItemFileName, myItemMenu);
            insertedItem->setBrush(myItemColor);
            QPen p(myLineColor);
            p.setCapStyle(Qt::RoundCap);
            insertedItem->setPen(p);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        emit itemInserted(insertedItem);
        insertedItem->setSelected(false);
        insertedItem->setEnabled(false);
        // add next item, same orientation if rotated/flipped
        {
            DiagramItem *item=new DiagramElement(mItemFileName, myItemMenu);
            item->setBrush(myItemColor);
            QPen p{myLineColor};
            p.setCapStyle(Qt::RoundCap);
            item->setPen(p);
            item->setZValue(maxZ);
            item->setTransform(insertedItem->transform());
            item->setSelected(true);
            maxZ+=0.1;
            addItem(item);
            insertedItem=item;
        }
        takeSnapshot();
        if(middleButton){
            // switch toolbar !!
            emit abortSignal();
        }
        break;
    case MoveItems:
    {
        QPointF point=onGrid(mouseEvent->scenePos());
        if(!myMoveItems.isEmpty()){
            qreal dx=point.rx()-myDx;
            qreal dy=point.ry()-myDy;
            foreach(QGraphicsItem* item,myMoveItems){
                if(item->parentItem()!=0){
                    if(!item->parentItem()->isSelected()) item->moveBy(-dx,-dy);
                }
                else {
                    item->moveBy(dx,dy);
                }
            }
            myMoveItems.clear();
            myMode=MoveItem;
            takeSnapshot();
        }
        else
        {
            if(!selectedItems().isEmpty()){
                // lösche doppelte Verweise (Child&selected)
                myMoveItems=selectedItems();
                foreach(QGraphicsItem* item,myMoveItems){
                    if(item->parentItem())
                        if(item->parentItem()->isSelected()) {
                            item->setSelected(false);
                            myMoveItems.removeOne(item);
                        }
                }
                // speichere Referenzpunkt
                myDx=point.rx();
                myDy=point.ry();
            }
        }
        if(middleButton){
            // switch toolbar !!
            emit abortSignal();
        }
        break;
    }
    case CopyItem:
        if (!selectedItems().empty()){
            copiedItems.clear();
            // remove duplicated references (child&selected)
            QList<QGraphicsItem*> myList=selectedItems();
            foreach(QGraphicsItem* item,myList){
                if(item->parentItem())
                    if(item->parentItem()->isSelected()) {
                        item->setSelected(false);
                        myList.removeOne(item);
                    }
            }
            // prepare copy
            QGraphicsItem *insItem;
            insItem=myList.first();
            QPointF point=onGrid(mouseEvent->scenePos());
            myDx=insItem->pos().rx()-point.rx();
            myDy=insItem->pos().ry()-point.ry();
            // copy
            foreach(QGraphicsItem* item,myList){
                insItem=copy(item);

                if(!insItem) continue;
                addItem(insItem);
                insItem->setZValue(item->zValue());
                insItem->setPos(item->pos());
                copiedItems.append(item);
                // set Z of new item
                item->setZValue(maxZ);
                maxZ+=0.1;
                //check for children but not group
                if(item->type()!=QGraphicsItemGroup::Type && item->childItems().count()>0){
                    foreach(QGraphicsItem* item_l1,item->childItems()){
                        QGraphicsItem* addedItem=copy(item_l1);
                        addItem(addedItem);
                        addedItem->setParentItem(insItem);
                        addedItem->setPos(item_l1->pos());
                    }
                }

                //move original to knew position
                item->setSelected(true);
            }
            myMode=CopyingItem;
        }
        if(middleButton){
            // switch toolbar !!
            emit abortSignal();
        }
        break;
    case CopyingItem:
        if (copiedItems.count() > 0){
            insertedItem=static_cast<DiagramItem*>(copiedItems.first());
            QPointF point=onGrid(mouseEvent->scenePos());
            qreal dx=insertedItem->pos().rx()-point.rx()-myDx;
            qreal dy=insertedItem->pos().ry()-point.ry()-myDy;
            foreach(QGraphicsItem* item,copiedItems){
                if(item->parentItem()){
                    if(!item->parentItem()->isSelected()) item->moveBy(-dx,-dy);
                }else{
                    item->moveBy(-dx,-dy);
                }
            }
            // place copy of the items and keep the currents items in copyList
            foreach(QGraphicsItem* item,copiedItems){
                QGraphicsItem *insItem=copy(item);
                addItem(insItem);
                insItem->setPos(item->pos());
                item->setZValue(maxZ);
                maxZ+=0.1;
                //check for children
                if(item->type()!=QGraphicsItemGroup::Type && item->childItems().count()>0){
                    foreach(QGraphicsItem* item_l1,item->childItems()){
                        QGraphicsItem* addedItem=copy(item_l1);
                        addItem(addedItem);
                        addedItem->setParentItem(insItem);
                        addedItem->setPos(item_l1->pos());
                    }
                }
            }
            takeSnapshot();
        }
        if(middleButton){
            // switch toolbar !!
            emit abortSignal();
        }
        break;
    case Zoom:
        startPoint=mouseEvent->scenePos();
        break;
    default:
        ;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    // move cursor
    myCursor.setPos(onGrid(mouseEvent->scenePos()));

    switch (myMode){
    case InsertLine:
        if (insertedPathItem != nullptr) {
            currentPoint=onGrid(mouseEvent->scenePos());
            insertedPathItem->updateLast(currentPoint);
        }
        break;
    case InsertSpline:
        if (insertedSplineItem != nullptr) {
            insertedSplineItem->updateActive(onGrid(mouseEvent->scenePos()));
        }
        break;
    case MoveItem:
        QGraphicsScene::mouseMoveEvent(mouseEvent);
        if(mouseEvent->buttons()==Qt::LeftButton){
            checkOnGrid();
        }
        break;
    case MoveItems:
    {
        QPointF point=onGrid(mouseEvent->scenePos());
        qreal dx=point.rx()-myDx;
        qreal dy=point.ry()-myDy;
        foreach(QGraphicsItem* item,myMoveItems){
            if(item->parentItem()!=0){
                if(!item->parentItem()->isSelected()) item->moveBy(dx,dy);
            }
            else {
                item->moveBy(dx,dy);
            }
        }
        myDx=point.rx();
        myDy=point.ry();
        break;
    }
    case InsertItem:
        if (insertedItem == nullptr){
            insertedItem = new DiagramItem(myItemType, myItemMenu);
            insertedItem->setBrush(myItemColor);
            QPen pen(myLineColor);
            pen.setWidth(myLineWidth);
            pen.setStyle(myPenStyle);
            insertedItem->setPen(pen);
            insertedItem->setSelected(true);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        break;
    case InsertElement:
        if(insertedItem==nullptr){
            insertedItem = new DiagramElement(mItemFileName, myItemMenu);
            insertedItem->setBrush(myItemColor);
            QPen p(myLineColor);
            p.setCapStyle(Qt::RoundCap);
            insertedItem->setPen(p);
            insertedItem->setSelected(true);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        break;
    case InsertDrawItem:
        if (insertedDrawItem){
            insertedDrawItem->setPos2(onGrid(mouseEvent->scenePos()));
        }
        break;
    case CopyingItem:
        if (copiedItems.count() > 0){
            //copiedItems->setPos(onGrid(mouseEvent->scenePos()));
            insertedItem=static_cast<DiagramItem*>(copiedItems.first());
            QPointF point=onGrid(mouseEvent->scenePos());
            qreal dx=insertedItem->pos().rx()-point.rx()-myDx;
            qreal dy=insertedItem->pos().ry()-point.ry()-myDy;
            foreach(QGraphicsItem* item,copiedItems){
                if(item->parentItem()!=0){
                    if(!item->parentItem()->isSelected()) item->moveBy(-dx,-dy);
                }
                else {
                    item->moveBy(-dx,-dy);
                }
            }
        }
        break;

    default:
        ;
    }
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == Zoom) {
        emit zoomRect(mouseEvent->scenePos(),startPoint);
        return;
    }
    if (myMode == InsertText) {
        mouseEvent->accept();
        return;
    }
    if(myMode== MoveItem && !selectedItems().isEmpty()){
        // update anchor points of textitems
        for(QGraphicsItem* item:selectedItems()){
            if(item->type()==DiagramTextItem::Type){
                auto *textItem=qgraphicsitem_cast<DiagramTextItem*>(item);
                QPointF offset=textItem->getLastOffset();
                if(textItem->anchorPoint()+offset != textItem->pos()){
                    textItem->setCorrectedPos(textItem->pos()-offset);
                }
            }
        }
        takeSnapshot();
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch (myMode){
    case InsertLine:
        //insertedPathItem->updateLast(onGrid(mouseEvent->scenePos()));
        if(insertedPathItem){
            insertedPathItem->remove();
            insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            insertedPathItem->setEnabled(false);
            insertedPathItem=nullptr;
            takeSnapshot();
        }
        mouseEvent->accept();
        break;
    case MoveItem:
        if(selectedItems().count()==1){
            QGraphicsItem *item=selectedItems().first();
            if(item->type()==DiagramDrawItem::Type){
                if(item->childItems().count()==1){
                    // already has text item
                    textItem=qgraphicsitem_cast<DiagramTextItem *>(item->childItems().first());
                    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                    textItem->setSelected(true);
                    textItem->setFocus();
                }else{
                    // draw item, add text in center
                    textItem = makeTextItem(item);
                    textItem->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
                    textItem->setCorrectedPos(item->boundingRect().center());

                    emit textInserted(textItem);
                }
                mouseEvent->accept();
                break;
            }
            if(item->type()==DiagramPathItem::Type){
                if(item->childItems().count()==1){
                    // already has text item
                    textItem=qgraphicsitem_cast<DiagramTextItem *>(item->childItems().first());
                    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                    textItem->setSelected(true);
                    textItem->setFocus();
                }else{
                    // path item, add text in center of line segment
                    textItem = makeTextItem(item);
                    textItem->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
                    // find correct position for tex
                    DiagramPathItem *path=qgraphicsitem_cast<DiagramPathItem *>(item);
                    QLineF line=path->findLineSection(mouseEvent->scenePos());
                    textItem->setCorrectedPos(line.center());
                    // alignment dpending on line angle
                    qreal angle=line.angle();
                    if(angle>80 && angle<100){
                        textItem->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                    }
                    if(angle<10 || (angle<190 && angle>170)){
                        textItem->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
                    }
                    if(angle>260 && angle<280){
                        textItem->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
                    }
                    if(angle>=10 && angle<=80){
                        textItem->setAlignment(Qt::AlignBottom|Qt::AlignRight);
                    }
                    if(angle>=100 && angle<=170){
                        textItem->setAlignment(Qt::AlignBottom|Qt::AlignLeft);
                    }
                    if(angle>=190 && angle<=260){
                        textItem->setAlignment(Qt::AlignBottom|Qt::AlignRight);
                    }
                    if(angle>=280 && angle<=350){
                        textItem->setAlignment(Qt::AlignBottom|Qt::AlignLeft);
                    }

                    emit textInserted(textItem);
                }
                mouseEvent->accept();
                break;
            }
            if(item->type()==DiagramTextItem::Type){
                textItem=qgraphicsitem_cast<DiagramTextItem *>(item);
                textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                textItem->setSelected(true);
                textItem->setFocus();
                mouseEvent->accept();
                break;
            }
        }
        [[fallthrough]];
    default:
        QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    }
}

void DiagramScene::checkOnGrid()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if(item->parentItem()) continue; // don't change elements which are bound to other items, e.g. text for rectangle
        if(item->type()==DiagramTextItem::Type){
            DiagramTextItem *textItem=qgraphicsitem_cast<DiagramTextItem *>(item);
            QPointF pt=item->pos()-textItem->calcOffset();
            qreal x = qRound(pt.x()/myGrid)*myGrid;
            qreal y = qRound(pt.y()/myGrid)*myGrid;
            textItem->setCorrectedPos(QPointF(x,y));
        }
        else
        {
            qreal x = qRound(item->x()/myGrid)*myGrid;
            qreal y = qRound(item->y()/myGrid)*myGrid;
            item->setPos(x,y);
        }
    }
}

QPointF DiagramScene::onGrid(QPointF pos)
{
    qreal x = qRound(pos.x()/myGrid)*myGrid;
    qreal y = qRound(pos.y()/myGrid)*myGrid;
    QPointF result = QPointF(x,y);
    return result;
}

void DiagramScene::setRouting(DiagramPathItem::routingType type)
{
    myRouting=type;
    if(insertedPathItem!=0){
        insertedPathItem->setRoutingType(type);
    }
}
QGraphicsItem* DiagramScene::copy(QGraphicsItem* item)
{
    switch(item->type()){
    case DiagramTextItem::Type:
        return qgraphicsitem_cast<QGraphicsItem*>(qgraphicsitem_cast<DiagramTextItem*>(item)->copy());
        break;
    case DiagramPathItem::Type:
        return qgraphicsitem_cast<QGraphicsItem*>(qgraphicsitem_cast<DiagramPathItem*>(item)->copy());
        break;
    case DiagramSplineItem::Type:
        return qgraphicsitem_cast<QGraphicsItem*>(qgraphicsitem_cast<DiagramSplineItem*>(item)->copy());
        break;
    case QGraphicsItemGroup::Type:
    {
        QPointF p=item->pos();
        QList<QGraphicsItem*>copied;
        for(auto *i:item->childItems()){
            auto *newItem=copy(i);
            newItem->setPos(i->pos());
            newItem->setParentItem(nullptr);
            newItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            newItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            addItem(newItem);
            newItem->moveBy(p.x(),p.y());
            copied<<newItem;
        }
        QGraphicsItemGroup *ig=createItemGroup(copied);
        ig->setFlag(QGraphicsItem::ItemIsMovable, true);
        ig->setFlag(QGraphicsItem::ItemIsSelectable, true);
        return ig;
    }
        break;
    default:
        DiagramItem* newItem=dynamic_cast<DiagramItem*>(item)->copy();
        return dynamic_cast<QGraphicsItem*>(newItem);
        break;
    }
}

void DiagramScene::clear()
{
    foreach(QGraphicsItem *item,items()){
        if(item!=&myCursor)
        {
            removeItem(item);
            delete item;
        }
    }
}

void DiagramScene::copyToBuffer()
{
    // copy
    qDeleteAll(bufferedItems);
    bufferedItems.clear();
    foreach(QGraphicsItem* item,selectedItems()){
        QGraphicsItem *insItem=copy(item);
        bufferedItems.append(insItem);
        //check for children but not group
        if(item->type()!=QGraphicsItemGroup::Type && item->childItems().count()>0){
            foreach(QGraphicsItem* item_l1,item->childItems()){
                QGraphicsItem* addedItem=copy(item_l1);
                bufferedItems.append(addedItem);
                addedItem->setParentItem(insItem);
                addedItem->setPos(item_l1->pos());
            }
        }
    }
}

void DiagramScene::pasteFromBuffer()
{
    copiedItems.clear();
    foreach(QGraphicsItem* item,bufferedItems){
        QGraphicsItem *newItem=copy(item);
        addItem(newItem);
        copiedItems.append(newItem);
    }
    myMode=CopyingItem;
}

void DiagramScene::setCursorVisible(bool vis)
{
    if(vis){
        if(myCursor.scene()==0){
            addItem(&myCursor);
        }
    }
    else
    {
        if(myCursor.scene()) removeItem(&myCursor);
    }
}

void DiagramScene::deleteItem(QGraphicsItem *item)
{
    if(item==insertedItem){
        insertedItem=nullptr;
    }
    if(item==insertedDrawItem){
        insertedDrawItem=nullptr;
    }
    this->removeItem(item);
}

void DiagramScene::insertElementDirectly(const QString element)
{
    DiagramElement *item = new DiagramElement(element,nullptr);
    QPen p(myLineColor);
    p.setCapStyle(Qt::RoundCap);
    item->setPen(p);
    item->setZValue(maxZ);
    maxZ+=0.1;
    addItem(item);
    QPointF pos=myCursor.pos();
    item->setPos(onGrid(pos));
}
/*!
 * \brief return active items
 * active items are selected items or insertItem
 * \return
 */
QList<QGraphicsItem *> DiagramScene::activeItems() const
{
    if(!selectedItems().isEmpty()){
        return selectedItems();
    }
    return (QList<QGraphicsItem*>()<<insertedItem);
}
/*!
 * \brief duplicate selected items
 */
void DiagramScene::duplicateItems()
{
    if(!selectedItems().isEmpty()){
        for(auto *item:selectedItems()){
            //TODO !
            // copy item
            item->setSelected(false);
            QGraphicsItem *newItem=copy(item);
            if(!newItem) continue;
            addItem(newItem);
            newItem->setZValue(maxZ);
            maxZ+=0.1;
            // move to side/down
            newItem->moveBy(myGrid,myGrid);
            newItem->setSelected(true);
            //check for children but not group
            if(item->type()!=QGraphicsItemGroup::Type && item->childItems().count()>0){
                foreach(QGraphicsItem* item_l1,item->childItems()){
                    QGraphicsItem* addedItem=copy(item_l1);
                    addItem(addedItem);
                    addedItem->setParentItem(newItem);
                    addedItem->setPos(item_l1->pos());
                }
            }
        }
    }
}

void DiagramScene::setMaxZ(qreal z)
{
    if(z>=maxZ){
        maxZ=z+0.1;
    }
}
/*!
 * \brief save current scene content as json into m_snapshots
 */
void DiagramScene::takeSnapshot()
{
    auto doc=create_json_save();
    if(!m_snapshots.isEmpty()){
        // check if duplicate
        // happens with area select but also with moving around to the same original position
        if(m_snapshots.at(m_undoPos)==doc)
            return;
    }
    if(m_snapshots.size()>m_undoPos+1){
        m_snapshots.insert(m_undoPos+1,doc);
        ++m_undoPos;
    }else{
        // append snapshot

        m_snapshots<<doc;
        ++m_undoPos;
    }
}
/*!
 * \brief restore snapshot
 * \param pos
 */
void DiagramScene::restoreSnapshot(int pos)
{
    if(pos<0){
        // at m_undoPos
        if(m_undoPos>0){
            --m_undoPos;
            clear();
            read_in_json(m_snapshots.at(m_undoPos));
        }
    }else{
        clear();
        read_in_json(m_snapshots.at(pos));
        m_undoPos=pos;
    }
}
/*!
 * \brief get current snaphot position
 * Is not last if undo was performed
 * Snapshots are kept to allow redo
 * \return
 */
int DiagramScene::getSnaphotPosition()
{
    return m_undoPos;
}
/*!
 * \brief get number of available snapshots;
 * \return
 */
int DiagramScene::getSnapshotSize()
{
    return m_snapshots.size();
}
/*!
 * \brief in inserting several points, remove last one
 * Usually employed when setting lines
 */
void DiagramScene::backoutOne()
{
    switch(myMode){
    case InsertLine:
        if(insertedPathItem){
            if(insertedPathItem->getPoints().size()>2){
                insertedPathItem->remove();
                insertedPathItem->updateLast(currentPoint);
            }else{
                // remove first, i.e. start anew
                removeItem(insertedPathItem);
                insertedPathItem=nullptr;
            }
        }
        break;
    default:
        ;
    }
}

void DiagramScene::editorReceivedFocus(DiagramTextItem *item)
{
    emit editorHasReceivedFocus();
}
/*!
 * \brief abort current operation
 * esc or right mouse click
 * \param keepSelection
 */
void DiagramScene::abort(bool keepSelection)
{
    switch(myMode){
    case CopyingItem:
        foreach(QGraphicsItem* item,copiedItems){
            removeItem(item);
        }
        copiedItems.clear();
        break;
    case InsertItem:
    case InsertElement:
        if(insertedItem)
            removeItem(insertedItem);
        break;
    case InsertDrawItem:
        if(insertedDrawItem)
            removeItem(insertedDrawItem);
        break;
    case InsertLine:
        if(insertedPathItem){
            removeItem(insertedPathItem);
        }else{
            myMode=MoveItem;
        }
        break;
    case InsertSpline:
        if(insertedSplineItem){
            removeItem(insertedSplineItem);
        }else{
            myMode=MoveItem;
        }
        break;
    default:
        ;
    }
    if(myMode!=InsertLine && myMode!=InsertSpline){
        myMode=MoveItem;
    }
    insertedItem=nullptr;
    insertedDrawItem=nullptr;
    insertedPathItem=nullptr;
    insertedSplineItem=nullptr;
    copiedItems.clear();

    if(!keepSelection) clearSelection();
    if(!keepSelection && myMode==MoveItem){
        emit abortSignal();
    }
}

bool DiagramScene::save_json(QFile *file, bool selectedItemsOnly)
{
    QJsonDocument doc=create_json_save(selectedItemsOnly);
    file->write(doc.toJson());
    return true;
}
/*!
 * \brief create json save data
 * \return
 */
QJsonDocument DiagramScene::create_json_save(bool selectedItemsOnly)
{
    QJsonArray array;
    QList<QGraphicsItem*> lst=selectedItemsOnly ? selectedItems() : items();
    foreach(QGraphicsItem* item, lst){
        if(item->parentItem()) continue;
        addElementToJSON(item,array);
    }
    QJsonDocument doc(array);
    return doc;
}

bool DiagramScene::load_json(QFile *file)
{
    QByteArray data = file->readAll();

    read_in_json(QJsonDocument::fromJson(data));

    return true;
}
/*!
 * \brief read in json
 * \param doc
 */
void DiagramScene::read_in_json(QJsonDocument doc)
{
    QJsonArray array=doc.array();
    for(int i=0;i<array.size();++i){
        QJsonObject json=array[i].toObject();
        QGraphicsItem *item=getElementFromJSON(json);
        addItem(item);
    }
    // Aufräumen
    insertedItem = nullptr;
    insertedDrawItem = nullptr;
    insertedPathItem = nullptr;
    insertedSplineItem = nullptr;
    textItem = nullptr;
    myMode = MoveItem;
}
/*!
 * \brief add item as json to JSON array
 * \param array
 */
void DiagramScene::addElementToJSON(QGraphicsItem *item, QJsonArray &array)
{
    QJsonObject json;
    if(!item->childItems().isEmpty()){
        QJsonArray ar;
        for(auto *i:item->childItems()){
            addElementToJSON(i,ar);
        }
        json["children"]=ar;
    }
    if(item->type()>QGraphicsItem::UserType){
        switch (item->type()) {
        case DiagramTextItem::Type:
        {
            DiagramTextItem *mItem = dynamic_cast<DiagramTextItem *>(item);
            mItem->write(json);
        }
            break;
        case DiagramPathItem::Type:
        {
            DiagramPathItem *mItem = dynamic_cast<DiagramPathItem *>(item);
            mItem->write(json);
        }
            break;
        case DiagramSplineItem::Type:
        {
            DiagramSplineItem *mItem = dynamic_cast<DiagramSplineItem *>(item);
            mItem->write(json);
        }
            break;
        default:
        {
            DiagramItem *mItem = dynamic_cast<DiagramItem *>(item);
            mItem->write(json);
        }
            break;
        }
        array.append(json);
    }
    if(item->type()==QGraphicsItemGroup::Type){
        json["type"]=item->type();
        QPointF p=item->pos();
        json["x"]=p.x();
        json["y"]=p.y();
        json["z"]=item->zValue();
        json["m11"]=item->transform().m11();
        json["m12"]=item->transform().m12();
        json["m21"]=item->transform().m21();
        json["m22"]=item->transform().m22();
        json["dx"]=item->transform().dx();
        json["dy"]=item->transform().dy();
        array.append(json);
    }

}
/*!
 * \brief interpret json and return appropriate DiagramItem
 * \param json
 * \return
 */
QGraphicsItem *DiagramScene::getElementFromJSON(QJsonObject json)
{
    QGraphicsItem *item=nullptr;
    int mDiaType=json["type"].toInt();
    switch (mDiaType) {
    case DiagramItem::Type:
        insertedItem = new DiagramItem(json,myItemMenu);
        item=insertedItem;
        break;
    case DiagramElement::Type:
        insertedItem = new DiagramElement(json,myItemMenu);
        item=insertedItem;
        break;
    case DiagramDrawItem::Type:
        insertedDrawItem = new DiagramDrawItem(json,myItemMenu);
        item=insertedDrawItem;
        break;
    case DiagramPathItem::Type:
        insertedPathItem = new DiagramPathItem(json,myItemMenu);
        item=insertedPathItem;
        break;
    case DiagramSplineItem::Type:
        insertedSplineItem = new DiagramSplineItem(json,myItemMenu);
        item=insertedSplineItem;
        break;
    case DiagramTextItem::Type:
        textItem = new DiagramTextItem(json);
        textItem->setTextInteractionFlags(Qt::NoTextInteraction);
        connect(textItem, &DiagramTextItem::lostFocus,
                this, &DiagramScene::editorLostFocus);
        connect(textItem, &DiagramTextItem::receivedFocus,
                this, &DiagramScene::editorReceivedFocus);
        connect(textItem, &DiagramTextItem::selectedChange,
                this, &DiagramScene::itemSelected);
        item=textItem;
        break;
    case QGraphicsItemGroup::Type:
    {
        QPointF p;
        p.setX(json["x"].toDouble());
        p.setY(json["y"].toDouble());
        if(json["children"].isArray()){
            QList<QGraphicsItem*>children;
            QJsonArray array=json["children"].toArray();
            for(int i=0;i<array.size();++i){
                QJsonObject json=array[i].toObject();
                QGraphicsItem *it=getElementFromJSON(json);
                it->moveBy(p.x(),p.y());
                children<<it;
            }
            QGraphicsItemGroup *ig=createItemGroup(children);
            ig->setFlag(QGraphicsItem::ItemIsMovable, true);
            ig->setFlag(QGraphicsItem::ItemIsSelectable, true);
            return ig;
        }
    }
        break;
    default:
        break;
    }
    // handle children
    if(json["children"].isArray()){
        QJsonArray array=json["children"].toArray();
        for(int i=0;i<array.size();++i){
            QJsonObject json=array[i].toObject();
            QGraphicsItem *it=getElementFromJSON(json);
            it->setParentItem(item);
        }
    }
    return item;
}

bool DiagramScene::event(QEvent *mEvent)
{
    if (mEvent->type()==QEvent::Enter) {
        myCursor.setVisible(true);
        return true;
    }
    if (mEvent->type()==QEvent::Leave) {
        myCursor.setVisible(false);
        return true;
    }
    return QGraphicsScene::event(mEvent);
}

void DiagramScene::drawBackground(QPainter *p, const QRectF &r) {
    p -> save();

    // desactive tout antialiasing, sauf pour le texte
    p -> setRenderHint(QPainter::Antialiasing, false);
    p -> setRenderHint(QPainter::TextAntialiasing, true);
    p -> setRenderHint(QPainter::SmoothPixmapTransform, false);

    // dessine un fond blanc
    p -> setPen(Qt::NoPen);
    p -> setBrush(Qt::white);
    p -> drawRect(r);

    if (myGridVisible) {
        // to ease transition from qelec
        qreal xGrid=myGridScale * myGrid;
        qreal yGrid=myGridScale * myGrid;
        QPen pen(Qt::black);
        pen.setCosmetic(true);
        pen.setWidth(2.0);
        p -> setPen(pen);
        p -> setBrush(Qt::NoBrush);
        qreal limite_x = r.x() + r.width();
        qreal limite_y = r.y() + r.height();

        qreal g_x = floor(r.x()/xGrid)*xGrid;
        qreal g_y = floor(r.y()/yGrid)*yGrid;

        for (qreal gx = g_x ; gx < limite_x ; gx += xGrid) {
            for (qreal gy = g_y ; gy < limite_y ; gy += yGrid) {
                p -> drawPoint(gx, gy);
            }
        }
    }

    p -> restore();
}

void DiagramScene::setArrow(const int i)
{
    myArrow=i;
    if(insertedPathItem!=0){
        insertedPathItem->setDiagramType(DiagramPathItem::DiagramType(myArrow));
    }
    if(insertedSplineItem!=0){
        insertedSplineItem->setDiagramType(DiagramSplineItem::DiagramType(myArrow));
    }
    if (!selectedItems().empty()){
        foreach(QGraphicsItem* item,selectedItems()){
            switch(item->type()){
            case DiagramPathItem::Type:
                qgraphicsitem_cast<DiagramPathItem*>(item)->setDiagramType(DiagramPathItem::DiagramType(myArrow));
                break;
            case DiagramSplineItem::Type:
                qgraphicsitem_cast<DiagramSplineItem*>(item)->setDiagramType(DiagramSplineItem::DiagramType(myArrow));
                break;
            default:
                // nothing to do
                break;
            }
        }
    }


}
