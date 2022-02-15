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
    myCursor.setAcceptHoverEvents(true);
    addItem(&myCursor);
}

void DiagramScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    foreach(QGraphicsItem *elem,selectedItems()){
        DiagramItem *item = dynamic_cast<DiagramItem *>(elem);
        if(item)
            item->setPen(myLineColor);
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

void DiagramScene::setFont(const QFont &font)
{
    myFont = font;

    foreach(QGraphicsItem *elem,selectedItems()){
        QGraphicsTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(elem);
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
    }
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
        emit zoom(factor);
        mouseEvent->setAccepted(true);
        return;
    }
    QGraphicsScene::wheelEvent(mouseEvent);
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::RightButton){
        abort();
        mouseEvent->accept();
        return;
    }

    if (mouseEvent->button() == Qt::MiddleButton){
        switch (myMode) {
        case InsertLine:
            if (insertedPathItem != nullptr){
                insertedPathItem->updateLast(onGrid(mouseEvent->scenePos()));
                insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                insertedPathItem->setEnabled(false);
                insertedPathItem = nullptr;
            }
            myMode=MoveItem;
            break;
        case InsertSpline:
            if(insertedSplineItem){
                insertedSplineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                insertedSplineItem->setEnabled(false);
                insertedSplineItem=nullptr;
            }
            myMode=MoveItem;
            break;
        case InsertItem:
        case InsertElement:
            if (insertedItem){
                insertedItem=nullptr;
                mouseEvent->setAccepted(true);
                myMode=MoveItem;
            }
            break;
        default:
            ;
        }
        return;
    }

    if (mouseEvent->button() != Qt::LeftButton)
        return;
    switch (myMode) {
    case InsertItem:
        if(insertedItem==nullptr){
            insertedItem = new DiagramItem(myItemType, myItemMenu);
            insertedItem->setBrush(myItemColor);
            insertedItem->setPen(myLineColor);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        emit itemInserted(insertedItem);
        break;
    case InsertLine:
        if (insertedPathItem == 0){
            insertedPathItem = new DiagramPathItem(myArrow,myItemMenu);
            insertedPathItem->setPen(myLineColor);
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
            insertedSplineItem = new DiagramSplineItem(DiagramSplineItem::cubic,myItemMenu);
            insertedSplineItem->setPen(myLineColor);
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
        textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
        textItem->setZValue(1000.0);
        connect(textItem, &DiagramTextItem::lostFocus,
                this, &DiagramScene::editorLostFocus);
        connect(textItem, &DiagramTextItem::selectedChange,
                this, &DiagramScene::itemSelected);
        addItem(textItem);
        textItem->setDefaultTextColor(myTextColor);
        textItem->setPos(mouseEvent->scenePos());
        emit textInserted(textItem);
        break;
    case InsertDrawItem:
        if (insertedDrawItem == nullptr){
            insertedDrawItem = new DiagramDrawItem(myDrawItemType, myItemMenu);
            insertedDrawItem->setBrush(myItemColor);
            insertedDrawItem->setPen(myLineColor);
            insertedDrawItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedDrawItem);
            insertedDrawItem->setPos(onGrid(mouseEvent->scenePos()));
        }
        else
        {
            insertedDrawItem->setPos2(onGrid(mouseEvent->scenePos()));
            insertedDrawItem->setEnabled(false);
            insertedDrawItem = 0;
        }

        break;
    case InsertElement:
        if(insertedItem==nullptr){
            insertedItem = new DiagramElement(mItemFileName, myItemMenu);
            //insertedItem->setBrush(myItemColor);
            QPen p(myLineColor);
            p.setCapStyle(Qt::RoundCap);
            insertedItem->setPen(p);
            insertedItem->setZValue(maxZ);
            maxZ+=0.1;
            addItem(insertedItem);
        }
        insertedItem->setPos(onGrid(mouseEvent->scenePos()));
        emit itemInserted(insertedItem);
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
                addItem(insItem);
                insItem->setPos(item->pos());
                copiedItems.append(item);
                item->setZValue(maxZ);
                maxZ+=0.1;
                //check for children
                if(item->childItems().count()>0){
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
        break;
    case CopyingItem:
        if (copiedItems.count() > 0){
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
            // place copy of the items and keep the currents items in copyList
            foreach(QGraphicsItem* item,copiedItems){
                QGraphicsItem *insItem=copy(item);
                addItem(insItem);
                insItem->setPos(item->pos());
                item->setZValue(maxZ);
                maxZ+=0.1;
                //check for children
                if(item->childItems().count()>0){
                    foreach(QGraphicsItem* item_l1,item->childItems()){
                        QGraphicsItem* addedItem=copy(item_l1);
                        addItem(addedItem);
                        addedItem->setParentItem(insItem);
                        addedItem->setPos(item_l1->pos());
                    }
                }
            }
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
            insertedPathItem->updateLast(onGrid(mouseEvent->scenePos()));
        }
        break;
    case InsertSpline:
        if (insertedSplineItem != nullptr) {
            insertedSplineItem->updateActive(onGrid(mouseEvent->scenePos()));
        }
        break;
    case MoveItem:
        QGraphicsScene::mouseMoveEvent(mouseEvent);
        checkOnGrid();
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
            insertedItem->setPen(myLineColor);
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
            //insertedItem->setBrush(myItemColor);
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
    if(insertedItem && (myMode == InsertItem || myMode == InsertElement)){
        insertedItem->setSelected(false);
        insertedItem->setEnabled(false);
        insertedItem=nullptr;
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch (myMode){
    case InsertLine:
        //insertedPathItem->updateLast(onGrid(mouseEvent->scenePos()));
        insertedPathItem->remove();
        insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        insertedPathItem->setEnabled(false);
        insertedPathItem=nullptr;
        break;
    default:
        QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    }
}

void DiagramScene::checkOnGrid()
{
    foreach (QGraphicsItem *item, selectedItems()) {

        if(item->type()==DiagramTextItem::Type){
            // not item position but center position needs to be on grid
            /*QRectF rect=item->sceneBoundingRect();
            QPointF centerPoint=rect.center();
            qreal dx = qRound(centerPoint.x()/10)*10.0 - centerPoint.x();
            qreal dy = qRound(centerPoint.y()/10)*10.0 - centerPoint.y();
            item->moveBy(-dx,-dy);*/
            qreal x = qRound(item->x()/10)*10.0;
            qreal y = qRound(item->y()/10)*10.0;
            item->setPos(x,y);
        }
        else
        {
            qreal x = qRound(item->x()/10)*10.0;
            qreal y = qRound(item->y()/10)*10.0;
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
    case QGraphicsItem::UserType+3:
        return qgraphicsitem_cast<QGraphicsItem*>(qgraphicsitem_cast<DiagramTextItem*>(item)->copy());
        break;
    case QGraphicsItem::UserType+6:
        return qgraphicsitem_cast<QGraphicsItem*>(qgraphicsitem_cast<DiagramPathItem*>(item)->copy());
        break;
    case QGraphicsItemGroup::Type:
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

void DiagramScene::editorReceivedFocus(DiagramTextItem *item)
{
    emit editorHasReceivedFocus();
}

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
        if(insertedPathItem)
            removeItem(insertedPathItem);
        break;
    case InsertSpline:
        if(insertedSplineItem)
            removeItem(insertedSplineItem);
        break;
    default:
        ;
    }

    insertedItem=nullptr;
    insertedDrawItem=nullptr;
    insertedPathItem=nullptr;
    insertedSplineItem=nullptr;
    copiedItems.clear();
    myMode=MoveItem;
    if(!keepSelection) clearSelection();
}

bool DiagramScene::save_json(QFile *file)
{
    QJsonArray array;
    foreach(QGraphicsItem* item, items()){
        QJsonObject json;
        if(item->type()>QGraphicsItem::UserType){
            switch (item->type()) {
            case QGraphicsItem::UserType+3:
            {
                DiagramTextItem *mItem = dynamic_cast<DiagramTextItem *>(item);
                mItem->write(json);
            }
                break;
            case QGraphicsItem::UserType+6:
            {
                DiagramPathItem *mItem = dynamic_cast<DiagramPathItem *>(item);
                mItem->write(json);
            }
                break;
            case QGraphicsItem::UserType+7:
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
    }
    QJsonDocument doc(array);
    file->write(doc.toJson());
    return true;
}

bool DiagramScene::load_json(QFile *file)
{
    QByteArray data = file->readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonArray array=loadDoc.array();
    for(int i=0;i<array.size();++i){
        QJsonObject json=array[i].toObject();
        int mDiaType=json["type"].toInt();
        switch (mDiaType) {
        case QGraphicsItem::UserType+15:
            insertedItem = new DiagramItem(json,myItemMenu);
            addItem(insertedItem);
            break;
        case QGraphicsItem::UserType+32:
            insertedItem = new DiagramElement(json,myItemMenu);
            addItem(insertedItem);
            break;
        case QGraphicsItem::UserType+16:
            insertedDrawItem = new DiagramDrawItem(json,myItemMenu);
            addItem(insertedDrawItem);
            break;
        case QGraphicsItem::UserType+6:
            insertedPathItem = new DiagramPathItem(json,myItemMenu);
            addItem(insertedPathItem);
            insertedPathItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            insertedPathItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            break;
        case QGraphicsItem::UserType+7:
            insertedSplineItem = new DiagramSplineItem(json,myItemMenu);
            addItem(insertedSplineItem);
            insertedSplineItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            insertedSplineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            break;
        case QGraphicsItem::UserType+3:
            textItem = new DiagramTextItem(json);
            textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
            connect(textItem, SIGNAL(lostFocus(DiagramTextItem *)),
                    this, SLOT(editorLostFocus(DiagramTextItem *)));
            connect(textItem, SIGNAL(receivedFocus(DiagramTextItem *)),
                    this, SLOT(editorReceivedFocus(DiagramTextItem *)));
            connect(textItem, SIGNAL(selectedChange(QGraphicsItem *)),
                    this, SIGNAL(itemSelected(QGraphicsItem *)));
            addItem(textItem);
            break;
        default:
            break;
        }
    }
    // Aufräumen
    insertedItem = nullptr;
    insertedDrawItem = nullptr;
    insertedPathItem = nullptr;
    insertedSplineItem = nullptr;
    textItem = nullptr;
    myMode = MoveItem;

    return true;
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
        int xGrid=myGridScale * (int) myGrid;
        int yGrid=myGridScale * (int) myGrid;
        // dessine les points de la grille
        p -> setPen(Qt::black);
        p -> setBrush(Qt::NoBrush);
        qreal limite_x = r.x() + r.width();
        qreal limite_y = r.y() + r.height();

        int g_x = (int)ceil(r.x());
        while (g_x % xGrid) ++ g_x;
        int g_y = (int)ceil(r.y());
        while (g_y % yGrid) ++ g_y;

        for (int gx = g_x ; gx < limite_x ; gx += xGrid) {
            for (int gy = g_y ; gy < limite_y ; gy += yGrid) {
                p -> drawPoint(gx, gy);
            }
        }
    }

    p -> restore();
}

void DiagramScene::setArrow(const int i)
{
    myArrow=DiagramPathItem::DiagramType(i);
    if(insertedPathItem!=0){
        insertedPathItem->setDiagramType(myArrow);
    }
    if (!selectedItems().empty()){
        foreach(QGraphicsItem* item,selectedItems()){
            switch(item->type()){
            case QGraphicsItem::UserType+3:
                // Textitem does not possess Linecolor !
                break;
            case QGraphicsItem::UserType+6:
                qgraphicsitem_cast<DiagramPathItem*>(item)->setDiagramType(myArrow);
                break;
            default:
                // nothing to do
                break;
            }
        }
    }


}
