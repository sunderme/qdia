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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "diagramitem.h"
#include "diagramdrawitem.h"
#include "diagrampathitem.h"
#include "ColorPickerToolButton.h"
#include <QMainWindow>
#include <QShortcut>

class DiagramScene;

QT_BEGIN_NAMESPACE
class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QGraphicsView;
QT_END_NAMESPACE

#define QDIA_VERSION "0.6"

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   MainWindow(int argc, char *argv[],QWidget *parent = nullptr);

private slots:
   void undo();
   void redo();
   void buttonGroupClicked(QAbstractButton *button);
   void deleteItem();
   void pointerGroupClicked(QAbstractButton *button);
   void horzAlignGroupClicked(QAbstractButton *button);
   void vertAlignGroupClicked(QAbstractButton *button);
   void bringToFront();
   void bringUp();
   void sendToBack();
   void sendDown();
   void selectAll();
   void rotateRight();
   void rotateLeft();
   void flipX();
   void flipY();
   void print();
   void exportImage();
   void copyItems();
   void duplicateItems();
   void copyToClipboard();
   void pasteFromClipboard();
   void groupItems();
   void ungroupItems();
   void currentFontChanged(const QFont &font);
   void fontSizeChanged(const QString &size);
   void sceneScaleChanged(const QString &scale);
   void textColorChanged(QColor color);
   void itemColorChanged(QColor color);
   void lineColorChanged(QColor color);
   void lineArrowChanged();
   void lineThicknessChanged();
   void linePatternChanged();
   void textButtonTriggered();
   void fillButtonTriggered();
   void lineButtonTriggered();
   void lineThicknessButtonTriggered();
   void linePatternButtonTriggered();
   void handleFontChange();
   void itemSelected(QGraphicsItem *item);
   void lineArrowButtonTriggered();
   void textAddButtonTriggered();
   void moveCursor(QPointF p);
   void about();
   void activateShortcuts();
   void deactivateShortcuts();
   void zoomIn();
   void zoomOut();
   void zoom(const qreal factor);
   void zoomPointer(const qreal factor,QPointF pointer);
   void zoomRect();
   void doZoomRect(QPointF p1,QPointF p2);
   void zoomFit();
   void changeGridFiner();
   void changeGridCoarser();
   void toggleGrid(bool grid);
   void setGrid();
   void fileSave();
   void fileSaveAs();
   void fileOpen();
   bool openFile(QString fileName);
   void openRecentFile();
   void moveItems();
   void abort();
   void abortFromScene();
   void insertDot();
   void switchToWire();

protected:
   void closeEvent(QCloseEvent *event);

private:
   void createToolBox();
   void createActions();
   void createMenus();
   void createToolbars();
   void populateRecentFiles();
   QWidget *createCellWidget(const QString &text,
                             int type, QButtonGroup *buttonGroup);
   QMenu *createColorMenu(const char *slot, QColor defaultColor);
   QIcon createColorToolButtonIcon(const QString &image, QColor color);
   QIcon createColorIcon(QColor color);

   QMenu *createArrowMenu(const char *slot, const int def);
   QIcon createArrowIcon(const int i);

   QMenu *createLineThicknessMenu(const char *slot, const int def);
   QIcon createLineThicknesIcon(const int i);

   QMenu *createLinePatternMenu(const char *slot, const int def);
   QIcon createLinePatternIcon(const int i);

   QRectF getTotalBoundary(const QList<QGraphicsItem*> items) const;
   QPointF getFirstPoint(const QList<QGraphicsItem*> items) const;
   void transformSelected(const QTransform transform,QList<QGraphicsItem*> items,bool forceOnGrid=false);
   void transformItems(const QTransform transform,QList<QGraphicsItem*> items,QPointF anchorPoint);

   DiagramScene *scene;
   QGraphicsView *view;

   QAction *exitAction;
   QAction *addAction;
   QAction *deleteAction;

   QAction *undoAction;
   QAction *redoAction;

   QAction *toFrontAction;
   QAction *sendBackAction;
   QAction *bringUpAction;
   QAction *sendDownAction;
   QAction *selectAllAction;
   QAction *aboutAction;
   QAction *rotateRightAction;
   QAction *rotateLeftAction;
   QAction *flipXAction;
   QAction *flipYAction;
   QAction *copyAction;
   QAction *duplicateAction;
   QAction *moveAction;
   QAction *groupAction;
   QAction *ungroupAction;

   QAction *zoomInAction;
   QAction *zoomOutAction;
   QAction *zoomAction;
   QAction *zoomFitAction;
   QAction *finerGridAction;
   QAction *coarserGridAction;
   QAction *showGridAction;

   QAction *printAction;
   QAction *exportAction;

   QShortcut *escShortcut;
   QShortcut *dotShortcut;
   QShortcut *wireShortcut;

   QMenu *fileMenu;
   QMenu *m_recentFilesMenu;
   QMenu *viewMenu;
   QMenu *itemMenu;
   QMenu *aboutMenu;

   QToolBar *textToolBar;
   QToolBar *editToolBar;
   QToolBar *colorToolBar;
   QToolBar *pointerToolbar;
   QToolBar *zoomToolbar;

   QComboBox *itemColorCombo;
   QComboBox *textColorCombo;
   QComboBox *fontSizeCombo;
   QFontComboBox *fontCombo;

   QToolBox *toolBox;
   QButtonGroup *pointerTypeGroup;
   QButtonGroup *horzAlignGroup,*vertAlignGroup;
   QAbstractButton *currentToolButton;
   ColorPickerToolButton *fontColorToolButton;
   ColorPickerToolButton *fillColorToolButton;
   ColorPickerToolButton *lineColorToolButton;
   QToolButton *pointerButton;
   QToolButton *linePointerButton;
   QToolButton *lineThicknessButton;
   QToolButton *linePatternButton;
   QAction *boldAction;
   QAction *underlineAction;
   QAction *italicAction;

   QAction *thicknessAction;
   QAction *patternAction;
   QAction *arrowAction;

   QAction *loadAction;
   QAction *saveAction;
   QAction *saveAsAction;
   QAction *copyToClipboardAction;
   QAction *pasteFromClipboardAction;

   QList<QAction*> listOfActions;
   QList<QShortcut*> listOfShortcuts;

   bool myShowGrid; // Grid visible ?

   QColor m_fillColor,m_lineColor,m_textColor;

   QString myFileName; // aktueller Filename
   QStringList m_recentFiles;
};

#endif // MAINWINDOW_H
