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

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include "diagramitem.h"
#include "diagramdrawitem.h"
#include "diagramelement.h"
#include "diagramtextitem.h"
#include "diagrampathitem.h"

#include <QGraphicsScene>
#include <QFile>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
QT_END_NAMESPACE

//! [0]
class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertItem, InsertLine, InsertText, MoveItem, CopyItem, CopyingItem, InsertDrawItem, Zoom , MoveItems, InsertElement };

    explicit DiagramScene(QMenu *itemMenu, QObject *parent = nullptr);
    QFont font() const { return myFont; }
    QColor textColor() const { return myTextColor; }
    QColor itemColor() const { return myItemColor; }
    QColor lineColor() const { return myLineColor; }
    void setLineColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setItemColor(const QColor &color);
    void setFont(const QFont &font);
    void setArrow(const int i);
    void setGrid(const qreal grid)
    {
        myGrid=grid;
    }
    qreal grid()
    {
        return myGrid;
    }
    void setGridVisible(const bool vis)
    {
        myGridVisible=vis;
    }
    bool isGridVisible()
    {
        return myGridVisible;
    }
    void setGridScale(const int k)
    {
        myGridScale=k;
    }

    bool save_json(QFile *file);
    bool load_json(QFile *file);

    QPointF onGrid(QPointF pos);
    void setCursorVisible(bool t);

public slots:
    void setMode(DiagramScene::Mode mode,bool m_abort=true);
    void abort(bool keepSelection=false);
    void setItemType(DiagramItem::DiagramType type);
    void setItemType(DiagramDrawItem::DiagramType type);
    void setItemType(QString fn);
    void setRouting(DiagramPathItem::routingType type);
    void editorLostFocus(DiagramTextItem *item);
    void editorReceivedFocus(DiagramTextItem *item);
    void checkOnGrid();
    void clear();

signals:
    void itemInserted(DiagramItem *item);
    void textInserted(QGraphicsTextItem *item);
    void itemSelected(QGraphicsItem *item);
    void editorHasLostFocus();
    void editorHasReceivedFocus();
    void zoomRect(QPointF p1,QPointF p2);
    void zoom(const qreal factor);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void wheelEvent(QGraphicsSceneWheelEvent *mouseEvent) override;
    bool event(QEvent *mEvent) override;
    QGraphicsItem* copy(QGraphicsItem* item);
    void drawBackground(QPainter *p, const QRectF &r) override;
    void enableAllItems(bool enable=true);

private:
    bool isItemChange(int type) const;

    DiagramItem::DiagramType myItemType;
    DiagramDrawItem::DiagramType myDrawItemType;
    QString mItemFileName;
    QMenu *myItemMenu;
    Mode myMode;
    bool leftButtonDown;
    QPointF startPoint;
    QFont myFont;
    DiagramTextItem *textItem;
    QColor myTextColor;
    QColor myItemColor;
    QColor myLineColor;

    DiagramItem *insertedItem;
    DiagramDrawItem *insertedDrawItem;
    DiagramPathItem *insertedPathItem;
    QList<QGraphicsItem *> *copiedItems;
    qreal myDx,myDy;
    DiagramPathItem::DiagramType myArrow;
    DiagramPathItem::routingType myRouting;
    qreal myGrid;
    QGraphicsRectItem myCursor;
    qreal myCursorWidth;
    bool myGridVisible;
    int myGridScale;
    QList<QGraphicsItem*> myMoveItems;
    qreal maxZ;
};
//! [0]

#endif // DIAGRAMSCENE_H
