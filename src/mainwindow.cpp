/****************************************************************************
**
** Copyright (C) 2022 Jan Sundermeyer
**
** License: GLP v3
**
****************************************************************************/

#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "diagramdrawitem.h"
#include "diagramelement.h"
#include "diagrampathitem.h"
#include "mainwindow.h"
#include "config.h"

#include <QtWidgets>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtSvg/QSvgGenerator>

const int InsertTextButton = 10;
const int InsertDrawItemButton = 64;

Config configuration;

/*!
 * \brief construct GUI
 * Read in settings, build menu&GUI
 * Interpret CLI options
 * \param argc
 * \param argv
 * \param parent
 */
MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    // read config
    QSettings settings("QDia","QDia");
    m_recentFiles=settings.value("recentFiles").toStringList();
    m_lastPath=settings.value("lastPath").toString();
    m_lastPathImage=settings.value("lastPathImage").toString();
    QString fontName=settings.value("font").toString();
    int fontSize=settings.value("fontsize").toInt();
    // restore style
    if(configuration.style!="<default>"){
        QApplication::setStyle(QStyleFactory::create(configuration.style));
    }
    // setup GUI
    createActions();
    createToolBox();
    createMenus();

    currentToolButton=nullptr; // none selected at start

    m_scene = new DiagramScene(itemMenu, this);
    m_scene->setSceneRect(QRectF(0, 0, 5000, 5000));
    m_scene->setGridVisible(configuration.showGrid);
    connect(m_scene, &DiagramScene::textItemSelected,
            this, &MainWindow::itemSelected);
    connect(m_scene, &DiagramScene::forceCursor,
            this, &MainWindow::moveCursor);
    // activate/deactivate shortcuts when text is edited in scene
    connect(m_scene, &DiagramScene::editorHasReceivedFocus,
            this, &MainWindow::deactivateShortcuts);
    connect(m_scene, &DiagramScene::editorHasLostFocus,
            this, &MainWindow::activateShortcuts);
    connect(m_scene, &DiagramScene::zoomRect,
            this, &MainWindow::doZoomRect);
    connect(m_scene, &DiagramScene::zoom,
            this, &MainWindow::zoom);
    connect(m_scene, &DiagramScene::zoomPointer,
            this, &MainWindow::zoomPointer);
    connect(m_scene, &DiagramScene::abortSignal,
            this, &MainWindow::abortFromScene);
    createToolbars();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(toolBox);
    m_view = new QGraphicsView(m_scene);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    m_view->setCacheMode(QGraphicsView::CacheBackground);
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    m_view->setMouseTracking(true);
    layout->addWidget(m_view);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);
    setUnifiedTitleAndToolBarOnMac(true);

    m_view->setFocus();

    // update font combo
    fontCombo->setCurrentFont(QFont(fontName));
    fontSizeCombo->setCurrentText(QString("%1").arg(fontSize));

    // restore geometry
    if(settings.contains("geometry")){
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

    if(argc>1){
        // assume last argument as filename
        // maybe more elaborate later
        QString fn=QString(argv[argc-1]);
        openFile(fn);
    }
}
/*!
 * \brief undo operation
 */
void MainWindow::undo()
{
    m_scene->restoreSnapshot();
}
/*!
 * \brief redo operation
 * (undo undo)
 */
void MainWindow::redo()
{
    int max=m_scene->getSnapshotSize();
    int current=m_scene->getSnaphotPosition();
    if(current+1<max){
        m_scene->restoreSnapshot(current+1);
    }
}
/*!
 * \brief write settings on closeEvent
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_scene->clearSelection(); // avoid crash
    QSettings settings("QDia", "QDia");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("recentFiles",m_recentFiles);
    settings.setValue("font",fontCombo->currentFont().toString());
    settings.setValue("fontsize",fontSizeCombo->currentText().toInt());
    settings.setValue("lastPath",m_lastPath);
    settings.setValue("lastPathImage",m_lastPathImage);
    event->accept();
}

void MainWindow::buttonGroupClicked(QAbstractButton *button)
{
    QButtonGroup *buttonGroup=button->group();//qobject_cast<QButtonGroup *>(sender());
    for (QAbstractButton *button : pointerTypeGroup->buttons()) {
        button->setChecked(false);
    }
    const QList<QAbstractButton *> buttons = buttonGroup->buttons();
    for (QAbstractButton *myButton : buttons) {
        if (myButton != button)
            myButton->setChecked(false);
    }
    currentToolButton=button;
    const int id = buttonGroup->id(button);
    if (id == InsertTextButton) {
        m_scene->setMode(DiagramScene::InsertText);
    } else {
        if ((id&192) == InsertDrawItemButton){
            m_scene->setItemType(DiagramDrawItem::DiagramType(id&63));
            m_scene->setMode(DiagramScene::InsertDrawItem);
        }
        else {
            if(id==256){
                // user element
                QString fn=button->property("fn").toString();
                m_scene->setItemType(fn);
                m_scene->setMode(DiagramScene::InsertUserElement);
            }else{
                if(id==128){
                    QString fn=button->property("fn").toString();
                    m_scene->setItemType(fn);
                    m_scene->setMode(DiagramScene::InsertElement);
                }else{
                    m_scene->setItemType(DiagramItem::DiagramType(id));
                    m_scene->setMode(DiagramScene::InsertItem);
                }
            }
        }
    }
}

void MainWindow::deleteItem()
{
    QList<QGraphicsItem *> selectedItems = m_scene->selectedItems(true);

    for (int i=0;i<selectedItems.length();++i) {
        QGraphicsItem *it=selectedItems[i];
        m_scene->deleteItem(it);
        for(QGraphicsItem *item:it->childItems()){
            selectedItems.removeAll(item);
        }
        delete it;
    }
    m_scene->takeSnapshot();
}
/*!
 * \brief set selected items as locked
 * Locked items can't be moved or deleted
 */
void MainWindow::lockItem()
{
    QList<QGraphicsItem *> selectedItems = m_scene->selectedItems();
    for (int i=0;i<selectedItems.length();++i) {
        QGraphicsItem *it=selectedItems[i];
        DiagramItem *item=dynamic_cast<DiagramItem *>(it);
        if(item){
            item->setLocked(true);
            item->setSelected(false);
        }
        DiagramPathItem *pathItem=dynamic_cast<DiagramPathItem *>(it);
        if(pathItem){
            pathItem->setLocked(true);
            pathItem->setSelected(false);
        }
        DiagramSplineItem *splineItem=dynamic_cast<DiagramSplineItem *>(it);
        if(pathItem){
            splineItem->setLocked(true);
            splineItem->setSelected(false);
        }
    }
}
/*!
 * \brief unlock selected items
 */
void MainWindow::unlockItem()
{
    QList<QGraphicsItem *> selectedItems = m_scene->selectedItems();
    for (int i=0;i<selectedItems.length();++i) {
        QGraphicsItem *it=selectedItems[i];
        DiagramItem *item=qgraphicsitem_cast<DiagramItem *>(it);
        if(item){
            item->setLocked(false);
        }
    }
}

void MainWindow::pointerGroupClicked(QAbstractButton *button)
{
    // uncheck toolbox
    if(currentToolButton){
        currentToolButton->setChecked(false);
        currentToolButton=nullptr;
    }
    QList<QAbstractButton *> buttons = pointerTypeGroup->buttons();
    foreach (QAbstractButton *mButton, buttons) {
        if(mButton!=button){
            mButton->setChecked(false);
        }
    }
    if(pointerTypeGroup->checkedId()!=DiagramScene::MoveItem) m_view->setDragMode(QGraphicsView::NoDrag);
    else m_view->setDragMode(QGraphicsView::RubberBandDrag);
    int mode=pointerTypeGroup->checkedId();
    if(mode==DiagramScene::InsertLine){
        lineArrowButtonTriggered();
    }else{
        m_scene->setMode(DiagramScene::Mode(mode));
    }
}
/*!
 * \brief set horizontal text Align
 * \param button
 */
void MainWindow::horzAlignGroupClicked(QAbstractButton *)
{
    int id=horzAlignGroup->checkedId();
    int align=static_cast<int>(m_scene->textAlignment());
    align=align & 0xf0;
    m_scene->setTextAlignment(static_cast<Qt::Alignment>(id|align));
}
/*!
 * \brief set vertical text Align
 * \param button
 */
void MainWindow::vertAlignGroupClicked(QAbstractButton *button)
{
    int id=vertAlignGroup->checkedId();
    int align=static_cast<int>(m_scene->textAlignment());
    align=align & 0xf;
    m_scene->setTextAlignment(static_cast<Qt::Alignment>(id | align));
}

void MainWindow::bringToFront()
{
    if (m_scene->selectedItems().isEmpty())
        return;
    m_scene->setCursorVisible(false);

    QGraphicsItem *selectedItem = m_scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    for (const QGraphicsItem *item : overlapItems) {
        if (item->zValue() >= zValue)
            zValue = item->zValue() + 0.1;
    }
    selectedItem->setZValue(zValue);
    m_scene->takeSnapshot();
    m_scene->setCursorVisible(true);
}

void MainWindow::bringUp()
{
    if (m_scene->selectedItems().isEmpty())
        return;
    m_scene->setCursorVisible(false);

    QGraphicsItem *selectedItem = m_scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue=selectedItem->zValue();
    QList<qreal> zValues;
    for (const QGraphicsItem *item : overlapItems) {
            zValues << item->zValue();
    }
    std::sort(zValues.begin(),zValues.end());
    int i=0;
    bool found=false;
    for(;i<zValues.size();++i) {
        qreal z=zValues.at(i);
        if(z<=zValue) continue;
        zValue=z;
        found=true;
        break;
    }
    if( (i+1)<zValues.size()){
        // make sure that it is above one but not the next higher one
        zValue=(zValue+zValues[i+1])/2;
    }else{
        if(found) zValue+=0.1;
    }
    m_scene->setMaxZ(zValue);
    selectedItem->setZValue(zValue);
    m_scene->takeSnapshot();
    m_scene->setCursorVisible(true);
}

void MainWindow::sendToBack()
{
    if (m_scene->selectedItems().isEmpty())
        return;
    m_scene->setCursorVisible(false);
    QGraphicsItem *selectedItem = m_scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    for (const QGraphicsItem *item : overlapItems) {
        if (item->zValue() <= zValue)
            zValue = item->zValue() - 0.1;
    }
    selectedItem->setZValue(zValue);
    m_scene->takeSnapshot();
    m_scene->setCursorVisible(true);
}

void MainWindow::sendDown()
{
    if (m_scene->selectedItems().isEmpty())
        return;
    m_scene->setCursorVisible(false);

    QGraphicsItem *selectedItem = m_scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue=selectedItem->zValue();
    QList<qreal> zValues;
    for (const QGraphicsItem *item : overlapItems) {
            zValues << item->zValue();
    }
    std::sort(zValues.begin(),zValues.end());
    bool found=false;
    int i=zValues.size()-1;
    for(;i>=0;--i) {
        qreal z=zValues[i];
        if(z>zValue) continue;
        found=true;
        zValue=z;
        break;
    }
    if( (i-1)>=0){
        // make sure that it is above one but not the next higher one
        zValue=(zValue+zValues[i-1])/2;
    }else{
        if(found) zValue-=0.1;
    }
    selectedItem->setZValue(zValue);
    m_scene->takeSnapshot();
    m_scene->setCursorVisible(true);
}
/*!
 * \brief select all item
 * ctrl+a
 */
void MainWindow::selectAll()
{
    for(auto *item:m_scene->items()){
        item->setSelected(true);
    }
}
/*!
 * \brief start selection rectangle
 * Only elements completely inside are selected
 */
void MainWindow::selectRectInner()
{
    m_scene->setMode(DiagramScene::SelectInner);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
}
/*!
 * \brief start selection rectangle
 * Elements crossing rectangle are selected
 */
void MainWindow::selectRectOuter()
{
    m_scene->setMode(DiagramScene::SelectOuter);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
}

void MainWindow::rotateRight()
{
    if (m_scene->activeItems().isEmpty())
        return;

    transformSelected(QTransform().rotate(90),m_scene->activeItems(true),true);
}

void MainWindow::rotateLeft()
{
    if (m_scene->activeItems().isEmpty())
        return;

    transformSelected(QTransform().rotate(-90),m_scene->activeItems(true),true);
}

void MainWindow::flipX()
{
    if (m_scene->activeItems().isEmpty())
        return;

    transformSelected(QTransform(-1,0,0,1,0,0),m_scene->activeItems(true));
}

void MainWindow::flipY()
{
    if (m_scene->activeItems().isEmpty())
        return;

    transformSelected(QTransform(1,0,0,-1,0,0),m_scene->activeItems(true));
}

void MainWindow::scale()
{
    if (m_scene->activeItems().isEmpty())
        return;

    // simple input message box to ask for scale factor
    bool ok;
    double scaleFactor = QInputDialog::getDouble(this, tr("Scale"),
                                                 tr("Scale factor:"), 1.0, 0.01, 100.0, 2, &ok);
    if (!ok)
        return;

    transformSelected(QTransform().scale(scaleFactor,scaleFactor),m_scene->activeItems(true),true);
}

void MainWindow::currentFontChanged(const QFont &)
{
    handleFontChange();
}

void MainWindow::fontSizeChanged(const QString &)
{
    handleFontChange();
}

void MainWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QTransform oldMatrix = m_view->transform();
    m_view->resetTransform();
    m_view->translate(oldMatrix.dx(), oldMatrix.dy());
    m_view->scale(newScale, newScale);
}

void MainWindow::textColorChanged(QColor color)
{
    m_textColor=color;
    fontColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/textpointer.png",
                                     color));
    textButtonTriggered();
}

void MainWindow::itemColorChanged(QColor color)
{
    m_fillColor=color;
    fillColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-fill-color.svg",
                                     color));
    fillButtonTriggered();
}

void MainWindow::lineColorChanged(QColor color)
{
    m_lineColor=color;
    lineColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-stroke-color.svg",
                                     color));
    lineButtonTriggered();
}

void MainWindow::textButtonTriggered()
{
    m_scene->setTextColor(m_textColor);
}

void MainWindow::fillButtonTriggered()
{
    m_scene->setItemColor(m_fillColor);
}

void MainWindow::lineButtonTriggered()
{
    m_scene->setLineColor(m_lineColor);
}

void MainWindow::lineThicknessButtonTriggered()
{
    int w=thicknessAction->data().toInt();
    m_scene->setLineWidth(w);
}

void MainWindow::linePatternButtonTriggered()
{
    int w=patternAction->data().toInt();
    Qt::PenStyle style=static_cast<Qt::PenStyle>(w);
    m_scene->setLinePattern(style);
}

void MainWindow::handleFontChange()
{
    QFont font = fontCombo->currentFont();
    font.setPointSize(fontSizeCombo->currentText().toInt());
    font.setWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    font.setItalic(italicAction->isChecked());
    font.setUnderline(underlineAction->isChecked());

    m_scene->setFont(font);
}

void MainWindow::itemSelected(QGraphicsItem *item)
{
    DiagramTextItem *textItem =
    qgraphicsitem_cast<DiagramTextItem *>(item);

    QList<QGraphicsItem*> lst=m_scene->selectedItems();
    if(lst.size()>1){
        // only change text control for single element
        return;
    }

    QFont font = textItem->font();
    fontCombo->setCurrentFont(font);
    fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
    boldAction->setChecked(font.weight() == QFont::Bold);
    italicAction->setChecked(font.italic());
    underlineAction->setChecked(font.underline());
    int align=static_cast<int>(textItem->alignment());
    if( (align & 0xf) == 0){
        // fall back to align left
        align|=1;
    }
    for(auto *bt:horzAlignGroup->buttons()){
        auto *button=qobject_cast<QToolButton*>(bt);
        if(align & horzAlignGroup->id(button)){
            button->setChecked(true);
        }
    }
    if( (align & 0xf0) == 0){
        // fall back to align top
        align|=0x20;
    }
    for(auto *bt:vertAlignGroup->buttons()){
        auto *button=qobject_cast<QToolButton*>(bt);
        if(align & vertAlignGroup->id(button)){
            button->setChecked(true);
        }
    }
}

void MainWindow::about()
{
    /*
    // provoke crash
    char *c = nullptr;
    *c = 'A';
    */
    QMessageBox::about(this, tr("About QDia"),
                       tr("Version %1\n"
                           "Written by Jan Sundermeyer (C) 2022\n"
                          "Simple schematic/diagram entry editor.").arg(QDIA_VERSION));
}

void MainWindow::createToolBox()
{
    struct Element {QString name; int type; };
    toolBox = new QToolBox;
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored));

    QButtonGroup *bG = new QButtonGroup(this);
    bG->setExclusive(false);
    connect(bG, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::buttonGroupClicked);
    QGridLayout *layout = new QGridLayout;
    // added DrawItem
    QList<Element> elements{
        {tr("Rectangle"),DiagramDrawItem::Rectangle},
        {tr("Square"),DiagramDrawItem::Square},
        {tr("Ellipse"),DiagramDrawItem::Ellipse},
        {tr("Circle"),DiagramDrawItem::Circle},
        {tr("RoundedRect"),DiagramDrawItem::RoundedRect},
        {tr("Arc"),DiagramDrawItem::Pie},
        {tr("Rhombus"),DiagramDrawItem::Rhombus},
        {tr("Triangle"),DiagramDrawItem::Triangle},
        {tr("DA"),DiagramDrawItem::DA},
        {tr("OTA"),DiagramDrawItem::OTA},
        {tr("MUX"),DiagramDrawItem::MUX},
        {tr("DEMUX"),DiagramDrawItem::DEMUX},
        {tr("Note"),DiagramDrawItem::Note}
    };
    int row=0;
    int col=0;
    for (const Element &element: elements) {
        QWidget *bt=createCellWidget(element.name,element.type+InsertDrawItemButton,bG);
        layout->addWidget(bt, row, col);
        ++col;
        if(col>2){
            col=0;
            ++row;
        }
    }
    if(col>0) ++row;
    layout->setRowStretch(row, 10);
    layout->setColumnStretch(2, 10);

    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(layout);

    toolBox->addItem(itemWidget, tr("Basic Shapes"));

    QStringList paths,names;
    QList<QStringList> lstOfElements;
    paths<<":/libs/analog/";
    names<<tr("Basic Electronic Elements");
    lstOfElements<<QStringList{
                   "circle.json",
                   "arrow.json",
                   "dot.json",
                   "res.json",
                   "cap.json",
                   "ind.json",
                   "nmos.json",
                   "pmos.json",
                   "diode.json",
                   "led.json",
                   "pd.json",
                   "vsource.json",
                   "isource.json",
                   "acsource.json",
                   "gnd.json",
                   "vdd.json",
                   "switch_open.json",
                   "switch_closed.json",
                   "switch_2t.json"
    };
    paths<<":/libs/agates/";
    names<<tr("Analog Blocks");
    lstOfElements<<QStringList{
                   "ota.json"
    };
    paths<<":/libs/gates/";
    names<<tr("Basic Digital Gates");
    lstOfElements<<QStringList{
                   "and.json",
                   "nand.json",
                   "or.json",
                   "nor.json",
                   "xor.json",
                   "xnor.json",
                   "buffer.json",
                   "inverter.json",
                   "dff.json",
                   "not.json"
    };
    paths<<":/libs/rf/";
    names<<tr("RF");
    lstOfElements<<QStringList{
                   "mixer.json",
                   "lpf.json",
                   "bpf.json",
                   "hpf.json"
    };
    paths<<":/libs/signal/";
    names<<tr("Signal Processing");
    lstOfElements<<QStringList{
                   "add.json",
                   "subtract.json",
                   "mult.json",
                   "div.json",
                   "limit.json",
                   "sum.json",
                   "int.json",
                   "ad.json",
                   "da.json",
    };

    for(int i=0;i<paths.size();++i){
        bG = new QButtonGroup(this);
        bG->setExclusive(false);
        connect(bG, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, &MainWindow::buttonGroupClicked);
        layout = new QGridLayout;
        // added DrawItem
        QString path=paths.value(i);

        row=0;
        col=0;
        for (const QString &fn: lstOfElements.value(i)) {
            QWidget *bt=createCellWidget(path+fn,128,bG);
            layout->addWidget(bt, row, col);
            ++col;
            if(col>2){
                col=0;
                ++row;
            }
        }
        if(col>0) ++row;

        layout->setRowStretch(row, 10);
        layout->setColumnStretch(2, 10);

        itemWidget = new QWidget;
        itemWidget->setLayout(layout);

        toolBox->addItem(itemWidget, names.value(i));
    }
    // add user pane !
#ifdef Q_OS_WIN
    const QString elementPath="%appdata%/Roaming/QDia/userElements";
   #else
    const QString elementPath=QDir::homePath()+"/.config/QDia/userElements/";
#endif
    QDir dir(elementPath);
    QStringList userElements=dir.entryList({"*.qdia"},QDir::Files);
    if(!userElements.isEmpty()){
        // add pane and fill
        bG = new QButtonGroup(this);
        bG->setExclusive(false);
        connect(bG, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, &MainWindow::buttonGroupClicked);
        layout = new QGridLayout;
        // added DrawItem
        row=0;
        col=0;
        for (const QString &fn: userElements) {
            QWidget *bt=createCellWidget(elementPath+fn,256,bG);
            if(!bt) continue; // skip invalid entries
            layout->addWidget(bt, row, col);
            ++col;
            if(col>2){
                col=0;
                ++row;
            }
        }
        if(col>0) ++row;

        layout->setRowStretch(row, 10);
        layout->setColumnStretch(2, 10);

        itemWidget = new QWidget;
        itemWidget->setLayout(layout);

        toolBox->addItem(itemWidget, "user");
    }
}

void MainWindow::createActions()
{
    undoAction = new QAction(QIcon(":/images/edit-undo.svg"),
                                tr("&Undo"), this);
    undoAction->setShortcut(tr("Ctrl+Z"));
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);

    redoAction = new QAction(QIcon(":/images/edit-redo.svg"),
                                tr("&Redo"), this);
    redoAction->setShortcut(tr("Ctrl+Shift+Z"));
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    toFrontAction = new QAction(QIcon(":/images/bringtofront.svg"),
                                tr("Bring to &Front"), this);
    toFrontAction->setShortcut(tr("Ctrl+F"));
    toFrontAction->setStatusTip(tr("Bring item to front"));
    connect(toFrontAction, &QAction::triggered, this, &MainWindow::bringToFront);

    sendBackAction = new QAction(QIcon(":/images/sendtoback.svg"), tr("Send to &Back"), this);
    sendBackAction->setShortcut(tr("Ctrl+T"));
    sendBackAction->setStatusTip(tr("Send item to back"));
    connect(sendBackAction, &QAction::triggered, this, &MainWindow::sendToBack);

    bringUpAction = new QAction(QIcon(":/images/bringtofront.svg"),
                                tr("Bring &up"), this);
    bringUpAction->setShortcut(tr("Ctrl+U"));
    bringUpAction->setStatusTip(tr("Bring item one up"));
    connect(bringUpAction, &QAction::triggered, this, &MainWindow::bringUp);

    sendDownAction = new QAction(QIcon(":/images/sendtoback.svg"),
                                tr("Send &down"), this);
    sendDownAction->setShortcut(tr("Ctrl+Shift+U"));
    sendDownAction->setStatusTip(tr("Send item one down"));
    connect(sendDownAction, &QAction::triggered, this, &MainWindow::sendDown);

    rotateRightAction = new QAction(QIcon(":/images/object-rotate-right.svg"),
                                    tr("rotate &Right"), this);
    rotateRightAction->setShortcut(tr("R"));
    rotateRightAction->setStatusTip(tr("rotate item 90 degrees right"));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);
    listOfActions.append(rotateRightAction);

    rotateLeftAction = new QAction(QIcon(":/images/object-rotate-left.svg"),
                                   tr("rotate &Left"), this);
    rotateLeftAction->setShortcut(tr("Shift+R"));
    rotateLeftAction->setStatusTip(tr("rotate item 90 degrees left"));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);
    listOfActions.append(rotateLeftAction);

    groupAction = new QAction(QIcon(":/images/object-group.svg"),
                              tr("&group Items"), this);
    groupAction->setShortcut(tr("Ctrl+G"));
    groupAction->setStatusTip(tr("group Items"));
    connect(groupAction, &QAction::triggered, this, &MainWindow::groupItems);
    listOfActions.append(groupAction);

    ungroupAction = new QAction(QIcon(":/images/object-ungroup.svg"),
                                tr("&ungroup Item"), this);
    ungroupAction->setShortcut(tr("Shift+Ctrl+G"));
    ungroupAction->setStatusTip(tr("ungroup Items"));
    connect(ungroupAction, &QAction::triggered, this, &MainWindow::ungroupItems);
    listOfActions.append(ungroupAction);

    makeElementAction  = new QAction(tr("&make user element"), this);
    connect(makeElementAction, &QAction::triggered, this, &MainWindow::makeElement);
    listOfActions.append(makeElementAction);

    deleteAction = new QAction(tr("&Delete Item"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete item from diagram"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);
    listOfActions.append(deleteAction);

    lockAction = new QAction(tr("&Lock Item"), this);
    lockAction->setShortcut(tr("L"));
    connect(lockAction, &QAction::triggered, this, &MainWindow::lockItem);

    unlockAction = new QAction(tr("&Lock Item"), this);
    unlockAction->setShortcut(tr("Shift+L"));
    connect(unlockAction, &QAction::triggered, this, &MainWindow::unlockItem);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit QDia"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::fileExit);
    listOfActions.append(exitAction);

    selectAllAction = new QAction(tr("Select &All"), this);
    selectAllAction->setShortcuts(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
    listOfActions.append(selectAllAction);

    selectRectOuterAction = new QAction(tr("Select Rect &Outer"), this);
    selectRectOuterAction->setShortcut(tr("Shift+s"));
    connect(selectRectOuterAction, &QAction::triggered, this, &MainWindow::selectRectOuter);
    listOfActions.append(selectRectOuterAction);

    selectRectInnerAction = new QAction(tr("Select &Rect Inner"), this);
    selectRectInnerAction->setShortcut(tr("s"));
    connect(selectRectInnerAction, &QAction::triggered, this, &MainWindow::selectRectInner);
    listOfActions.append(selectRectInnerAction);

    searchAndReplaceAction = new QAction(tr("&Search and Replace Texts"), this);
    connect(searchAndReplaceAction, &QAction::triggered, this, &MainWindow::searchAndReplaceTexts);

    preferenceAction = new QAction(tr("&Preferences ..."), this);
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::showPreferences);

    boldAction = new QAction(tr("Bold"), this);
    boldAction->setCheckable(true);
    QPixmap pixmap(":/images/bold.svg");
    boldAction->setIcon(QIcon(pixmap));
    boldAction->setShortcut(tr("Ctrl+B"));
    connect(boldAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    italicAction = new QAction(QIcon(":/images/italic.svg"), tr("Italic"), this);
    italicAction->setCheckable(true);
    italicAction->setShortcut(tr("Ctrl+I"));
    connect(italicAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    underlineAction = new QAction(QIcon(":/images/underline.svg"), tr("Underline"), this);
    underlineAction->setCheckable(true);
    //underlineAction->setShortcut(tr("Ctrl+U"));
    connect(underlineAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(tr("F1"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    printAction = new QAction(QIcon(":/images/document-print.svg"),tr("&Print"), this);
    printAction->setStatusTip(tr("Print Diagram"));
    connect(printAction, &QAction::triggered, this, &MainWindow::print);

    exportAction = new QAction(tr("&Export Diagram"), this);
    exportAction->setStatusTip(tr("Export Diagram to image"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportImage()));

    importAction = new QAction(tr("&Import Diagram"), this);
    importAction->setStatusTip(tr("Import Diagram from file"));
    connect(importAction, SIGNAL(triggered()), this, SLOT(importDiagram()));

    copyAction = new QAction(QIcon(":/images/edit-copy.svg"),tr("&Copy"), this);
    copyAction->setShortcut(tr("c"));
    connect(copyAction, &QAction::triggered,this, &MainWindow::copyItems);
    listOfActions.append(copyAction);

    duplicateAction = new QAction(tr("&Duplicate"), this);
    duplicateAction->setShortcut(tr("ctrl+d"));
    connect(duplicateAction,&QAction::triggered,this,&MainWindow::duplicateItems);
    listOfActions.append(duplicateAction);

    tapAction = new QAction(tr("&Tap"), this);
    tapAction->setShortcut(tr("shift+t"));
    connect(tapAction,&QAction::triggered,this,&MainWindow::tapItem);
    listOfActions.append(tapAction);

    moveAction = new QAction(QIcon(":/images/transform-move.svg"),tr("&Move"), this);
    moveAction->setShortcut(tr("m"));
    connect(moveAction, &QAction::triggered, this, &MainWindow::moveItems);
    listOfActions.append(moveAction);

    flipXAction = new QAction(QIcon(":/images/object-flip-horizontal.svg"),
                              tr("Flip &X"), this);
    flipXAction->setShortcut(tr("f"));
    connect(flipXAction, &QAction::triggered,this, &MainWindow::flipX);
    listOfActions.append(flipXAction);

    flipYAction = new QAction(QIcon(":/images/object-flip-vertical.svg"),tr("Flip &Y"), this);
    flipYAction->setShortcut(tr("Shift+F"));
    connect(flipYAction, &QAction::triggered,this, &MainWindow::flipY);
    listOfActions.append(flipYAction);

    scaleAction = new QAction(QIcon(":/images/transform-scale.svg"),
                              tr("&Scale"), this);
    //scaleAction->setShortcut(tr("s"));
    connect(scaleAction, &QAction::triggered,this, &MainWindow::scale);
    listOfActions.append(scaleAction);

    escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape),
                                this);
    connect(escShortcut,&QShortcut::activated,this,&MainWindow::abort);
    listOfShortcuts.append(escShortcut);

    backoutOneShortcut = new QShortcut(QKeySequence(Qt::Key_Backspace),
                                this);
    connect(backoutOneShortcut,&QShortcut::activated,this,&MainWindow::backoutOne);
    listOfShortcuts.append(backoutOneShortcut);

    dotAction = new QAction(tr("Create &Dot"), this);
    dotAction->setShortcut(QKeySequence("."));
    connect(dotAction,&QAction::triggered,this,&MainWindow::insertDot);
    listOfActions.append(dotAction);

    lineAction = new QAction(tr("Create &Line"), this);
    lineAction->setShortcut(QKeySequence("w"));
    connect(lineAction,&QAction::triggered,this,&MainWindow::switchToWire);
    listOfActions.append(lineAction);

    rectAction = new QAction(tr("Create &Rect"), this);
    rectAction->setShortcut(QKeySequence("b"));
    connect(rectAction,&QAction::triggered,this,&MainWindow::switchToRect);
    listOfActions.append(rectAction);

    textAction = new QAction(tr("Create &Text"), this);
    textAction->setShortcut(QKeySequence("t"));
    connect(textAction,&QAction::triggered,this,&MainWindow::switchToText);
    listOfActions.append(textAction);

    // Zoom in/out
    zoomInAction = new QAction(QIcon(":/images/zoomin.svg"),tr("Zoom &in"), this);
    //zoomInAction->setShortcut(tr("Shift+z"));
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    listOfActions.append(zoomInAction);

    zoomOutAction = new QAction(QIcon(":/images/zoomout.svg"),tr("Zoom &out"), this);
    zoomOutAction->setShortcut(tr("Shift+z"));
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    listOfActions.append(zoomOutAction);

    zoomAction = new QAction(QIcon(":/images/zoom.svg"),tr("&Zoom area"), this);
    zoomAction->setShortcut(tr("z"));
    connect(zoomAction, &QAction::triggered,            this, &MainWindow::zoomRect);
    listOfActions.append(zoomAction);

    zoomFitAction = new QAction(QIcon(":/images/zoompage.svg"),tr("Zoom &Fit"), this);
    zoomFitAction->setShortcut(tr("v"));
    connect(zoomFitAction, &QAction::triggered,
            this, &MainWindow::zoomFit);
    listOfActions.append(zoomFitAction);

    finerGridAction = new QAction(tr("&Finer Grid"), this);
    finerGridAction->setShortcut(tr("+"));
    connect(finerGridAction, &QAction::triggered,
            this, &MainWindow::changeGridFiner);
    listOfActions.append(finerGridAction);

    coarserGridAction = new QAction(tr("&Coarser Grid"), this);
    coarserGridAction->setShortcut(tr("-"));
    connect(coarserGridAction, &QAction::triggered,
            this, &MainWindow::changeGridCoarser);
    listOfActions.append(coarserGridAction);

    showGridAction = new QAction(QIcon(":/images/view-grid.svg"),tr("Show &Grid"), this);
    showGridAction->setCheckable(true);
    showGridAction->setChecked(configuration.showGrid);
    connect(showGridAction, &QAction::toggled,
            this, &MainWindow::toggleGrid);
    listOfActions.append(showGridAction);

    loadAction = new QAction(QIcon(":/images/document-open.svg"),tr("&Open ..."), this);
    loadAction->setShortcut(tr("Ctrl+o"));
    connect(loadAction, &QAction::triggered,
            this, &MainWindow::fileOpen);
    listOfActions.append(loadAction);

    saveAction = new QAction(QIcon(":/images/document-save.svg"),tr("&Save ..."), this);
    saveAction->setShortcut(tr("Ctrl+s"));
    connect(saveAction, &QAction::triggered,
            this, &MainWindow::fileSave);
    listOfActions.append(saveAction);

    saveAsAction = new QAction(QIcon(":/images/document-save-as.svg"),tr("Save &As ..."), this);
    saveAsAction->setShortcut(tr("Ctrl+Shift+s"));
    connect(saveAsAction, &QAction::triggered,[this]{fileSaveAs();});
    listOfActions.append(saveAsAction);

    copyToClipboardAction=new QAction(QIcon(":/images/edit-copy.svg"),tr("&Copy to clipboard"), this);
    copyToClipboardAction->setShortcut(tr("Ctrl+c"));
    connect(copyToClipboardAction,&QAction::triggered,
            this, &MainWindow::copyToClipboard);
    listOfActions.append(copyToClipboardAction);

    pasteFromClipboardAction=new QAction(QIcon(":/images/edit-paste.svg"),tr("&Paste from clipboard"), this);
    pasteFromClipboardAction->setShortcut(tr("Ctrl+v"));
    connect(pasteFromClipboardAction,&QAction::triggered,
            this, &MainWindow::pasteFromClipboard);
    listOfActions.append(pasteFromClipboardAction);
}

void MainWindow::createMenus()
{
    m_recentFilesMenu=new QMenu(tr("Open recent files..."));
    populateRecentFiles();

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(loadAction);
    fileMenu->addMenu(m_recentFilesMenu);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addAction(printAction);
    fileMenu->addAction(exportAction);
    fileMenu->addAction(importAction);
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAction);
    editMenu->addAction(selectRectInnerAction);
    editMenu->addAction(selectRectOuterAction);
    editMenu->addAction(copyToClipboardAction);
    editMenu->addAction(pasteFromClipboardAction);
    editMenu->addSeparator();
    editMenu->addAction(searchAndReplaceAction);
    editMenu->addSeparator();
    editMenu->addAction(preferenceAction);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(zoomAction);
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addAction(zoomFitAction);
    viewMenu->addSeparator();
    viewMenu->addAction(finerGridAction);
    viewMenu->addAction(coarserGridAction);
    viewMenu->addSeparator();
    viewMenu->addAction(showGridAction);

    createMenu = menuBar()->addMenu(tr("&Create"));
    createMenu->addAction(dotAction);
    createMenu->addAction(lineAction);
    createMenu->addAction(rectAction);
    createMenu->addAction(textAction);

    itemMenu = menuBar()->addMenu(tr("&Item"));
    itemMenu->addAction(deleteAction);
    itemMenu->addAction(copyAction);
    itemMenu->addAction(duplicateAction);
    itemMenu->addAction(moveAction);
    itemMenu->addSeparator();
    itemMenu->addAction(toFrontAction);
    itemMenu->addAction(sendBackAction);
    itemMenu->addAction(bringUpAction);
    itemMenu->addAction(sendDownAction);
    itemMenu->addSeparator();
    itemMenu->addAction(rotateRightAction);
    itemMenu->addAction(rotateLeftAction);
    itemMenu->addAction(flipXAction);
    itemMenu->addAction(flipYAction);
    itemMenu->addAction(scaleAction);
    itemMenu->addAction(groupAction);
    itemMenu->addAction(ungroupAction);
    itemMenu->addAction(tapAction);
    itemMenu->addAction(makeElementAction);
    itemMenu->addSeparator();
    itemMenu->addAction(lockAction);
    itemMenu->addAction(unlockAction);

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);
}

void MainWindow::createToolbars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("file");
    fileToolBar->addAction(loadAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->addAction(copyToClipboardAction);
    fileToolBar->addAction(pasteFromClipboardAction);
    fileToolBar->addAction(undoAction);
    fileToolBar->addAction(redoAction);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("edit");
    editToolBar->addAction(toFrontAction);
    editToolBar->addAction(sendBackAction);
    editToolBar->addAction(moveAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(rotateRightAction);
    editToolBar->addAction(rotateLeftAction);
    editToolBar->addAction(flipXAction);
    editToolBar->addAction(flipYAction);

    fontCombo = new QFontComboBox();
    connect(fontCombo, &QFontComboBox::currentFontChanged,
            this, &MainWindow::currentFontChanged);

    fontSizeCombo = new QComboBox;
    fontSizeCombo->setEditable(true);
    for (int i = 8; i < 30; i = i + 2)
        fontSizeCombo->addItem(QString().setNum(i));
    QIntValidator *validator = new QIntValidator(2, 64, this);
    fontSizeCombo->setValidator(validator);
    connect(fontSizeCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::fontSizeChanged);

    fontColorToolButton = new ColorPickerToolButton;
    fontColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    connect(fontColorToolButton,&ColorPickerToolButton::colorSelected,this,&MainWindow::textColorChanged);
    m_textColor=Qt::black;
    fontColorToolButton->setIcon(createColorToolButtonIcon(":/images/textpointer.png", Qt::black));
    fontColorToolButton->setAutoFillBackground(true);
    connect(fontColorToolButton, &QAbstractButton::clicked,
            this, &MainWindow::textButtonTriggered);

    fillColorToolButton = new ColorPickerToolButton;
    fillColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    connect(fillColorToolButton,&ColorPickerToolButton::colorSelected,this,&MainWindow::itemColorChanged);
    m_fillColor=Qt::white;
    fillColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-fill-color.svg", Qt::white));
    connect(fillColorToolButton, &QAbstractButton::clicked,
            this, &MainWindow::fillButtonTriggered);

    lineColorToolButton = new ColorPickerToolButton;
    lineColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    connect(lineColorToolButton,&ColorPickerToolButton::colorSelected,this,&MainWindow::lineColorChanged);
    m_lineColor=Qt::black;
    lineColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-stroke-color.svg", Qt::black));
    connect(lineColorToolButton, &QAbstractButton::clicked,
            this, &MainWindow::lineButtonTriggered);

    textToolBar = addToolBar(tr("Font"));
    textToolBar->setObjectName("text");
    textToolBar->addWidget(fontCombo);
    textToolBar->addWidget(fontSizeCombo);
    textToolBar->addAction(boldAction);
    textToolBar->addAction(italicAction);
    textToolBar->addAction(underlineAction);

    horzAlignGroup = new QButtonGroup(this);
    horzAlignGroup->setExclusive(true);
    QToolButton *bt= new QToolButton;
    bt->setCheckable(true);
    bt->setChecked(true);
    bt->setIcon(QIcon(":/images/format-justify-left.svg"));
    horzAlignGroup->addButton(bt, int(Qt::AlignLeft));
    textToolBar->addWidget(bt);
    bt= new QToolButton;
    bt->setCheckable(true);
    bt->setIcon(QIcon(":/images/format-justify-center.svg"));
    horzAlignGroup->addButton(bt, int(Qt::AlignHCenter));
    textToolBar->addWidget(bt);
    bt= new QToolButton;
    bt->setCheckable(true);
    bt->setIcon(QIcon(":/images/format-justify-right.svg"));
    horzAlignGroup->addButton(bt, int(Qt::AlignRight));
    textToolBar->addWidget(bt);
    connect(horzAlignGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::horzAlignGroupClicked);

    vertAlignGroup = new QButtonGroup(this);
    vertAlignGroup->setExclusive(true);
    bt= new QToolButton;
    bt->setCheckable(true);
    bt->setChecked(true);
    bt->setIcon(QIcon(":/images/format-align-vertical-top.svg"));
    vertAlignGroup->addButton(bt, int(Qt::AlignTop));
    textToolBar->addWidget(bt);
    bt= new QToolButton;
    bt->setCheckable(true);
    bt->setIcon(QIcon(":/images/format-align-vertical-center.svg"));
    vertAlignGroup->addButton(bt, int(Qt::AlignVCenter));
    textToolBar->addWidget(bt);
    bt= new QToolButton;
    bt->setCheckable(true);
    bt->setIcon(QIcon(":/images/format-align-vertical-bottom.svg"));
    vertAlignGroup->addButton(bt, int(Qt::AlignBottom));
    textToolBar->addWidget(bt);
    connect(vertAlignGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::vertAlignGroupClicked);


    colorToolBar = addToolBar(tr("Color"));
    colorToolBar->setObjectName("color");
    colorToolBar->addWidget(fontColorToolButton);
    colorToolBar->addWidget(fillColorToolButton);
    colorToolBar->addWidget(lineColorToolButton);

    lineThicknessButton = new QToolButton;
    lineThicknessButton->setIcon(createLineThicknesIcon(1));
    lineThicknessButton->setPopupMode(QToolButton::MenuButtonPopup);
    lineThicknessButton->setMenu(createLineThicknessMenu(SLOT(lineThicknessChanged()),
                                               0));
    thicknessAction = lineThicknessButton->menu()->defaultAction();
    connect(lineThicknessButton, &QToolButton::clicked,
            this, &MainWindow::lineThicknessButtonTriggered);

    colorToolBar->addWidget(lineThicknessButton);

    linePatternButton = new QToolButton;
    linePatternButton->setIcon(createLinePatternIcon(1));
    linePatternButton->setPopupMode(QToolButton::MenuButtonPopup);
    linePatternButton->setMenu(createLinePatternMenu(SLOT(linePatternChanged()),
                                               1));
    patternAction = linePatternButton->menu()->defaultAction();
    connect(linePatternButton, &QToolButton::clicked,
            this, &MainWindow::linePatternButtonTriggered);

    colorToolBar->addWidget(linePatternButton);

    pointerButton = new QToolButton;
    pointerButton->setCheckable(true);
    pointerButton->setChecked(true);
    pointerButton->setIcon(QIcon(":/images/tool-pointer.svg"));
    linePointerButton = new QToolButton;
    linePointerButton->setCheckable(true);
    linePointerButton->setIcon(QIcon(":/images/linepointer.png"));
    linePointerButton->setIcon(createArrowIcon(0));
    linePointerButton->setPopupMode(QToolButton::MenuButtonPopup);
    linePointerButton->setMenu(createArrowMenu(SLOT(lineArrowChanged()),
                                               0));
    arrowAction = linePointerButton->menu()->defaultAction();
    connect(linePointerButton, &QToolButton::clicked,
            this, &MainWindow::lineArrowButtonTriggered);

    textButton = new QToolButton;
    textButton->setCheckable(true);
    textButton->setIcon(QIcon(":/images/kdenlive-add-text-clip.svg"));

    pointerTypeGroup = new QButtonGroup(this);
    pointerTypeGroup->setExclusive(false);
    pointerTypeGroup->addButton(pointerButton, int(DiagramScene::MoveItem));
    pointerTypeGroup->addButton(linePointerButton, int(DiagramScene::InsertLine));
    pointerTypeGroup->addButton(textButton, int(DiagramScene::InsertText));
    connect(pointerTypeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::pointerGroupClicked);

    pointerToolbar = addToolBar(tr("Pointer type"));
    pointerToolbar->setObjectName("pointer");
    pointerToolbar->addWidget(pointerButton);
    pointerToolbar->addWidget(linePointerButton);
    pointerToolbar->addWidget(textButton);

    zoomToolbar = addToolBar(tr("Zoom"));
    zoomToolbar->setObjectName("zoom");
    zoomToolbar->addAction(zoomAction);
    zoomToolbar->addAction(zoomInAction);
    zoomToolbar->addAction(zoomOutAction);
    zoomToolbar->addAction(zoomFitAction);
}
/*!
 * \brief populate RecentFiles menu
 */
void MainWindow::populateRecentFiles()
{
    m_recentFilesMenu->clear();
    for(const QString &elem:m_recentFiles){
        QAction *act=new QAction(elem);
        connect(act,&QAction::triggered,this,&MainWindow::openRecentFile);
        m_recentFilesMenu->addAction(act);
    }
}

QWidget *MainWindow::createCellWidget(const QString &text,
                      int type,QButtonGroup *buttonGroup)
{
    QToolButton *button = new QToolButton;
    QString name=text;
    if(type==128){
        DiagramElement item(text, itemMenu);
        QIcon icon(item.image());
        button->setIcon(icon);
        button->setProperty("fn",text);
        name=item.getName();
    }else{
        if(type==256){
            auto *virtScene = new DiagramScene(itemMenu, this);
            virtScene->setSceneRect(QRectF(0, 0, 5000, 5000));
            virtScene->setGridVisible(false);
            virtScene->setCursorVisible(false);
            QFile file(text);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                return nullptr;
            }
            virtScene->load_json(&file);
            QRectF rect=virtScene->itemsBoundingRect(); // Bonding der Elemente in scene
            QRectF target(0,0,250,250);

            QPixmap pixmap(250, 250);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            virtScene->render(&painter,target,rect);
            button->setIcon(pixmap);
            button->setProperty("fn",text);
            QFileInfo fi(text);
            name=fi.baseName();
        }else{
            if(type>63){
                DiagramDrawItem item(static_cast<DiagramDrawItem::DiagramType>(type-64), itemMenu);
                item.setPos2(230,230);
                item.setEndPoint(QPointF(-1,2));
                QIcon icon(item.image());
                button->setIcon(icon);
            }else{
                DiagramItem item(static_cast<DiagramItem::DiagramType>(type), itemMenu);
                QIcon icon(item.image());
                button->setIcon(icon);
            }
        }
    }
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    buttonGroup->addButton(button, type);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(name), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

QMenu *MainWindow::createColorMenu(const char *slot, QColor defaultColor)
{
    QList<QColor> colors;
    colors << Qt::black << Qt::white << Qt::red << Qt::blue << Qt::yellow;
    QStringList names;
    names << tr("black") << tr("white") << tr("red") << tr("blue")
          << tr("yellow");

    QMenu *colorMenu = new QMenu(this);
    for (int i = 0; i < colors.count(); ++i) {
        QAction *action = new QAction(names.at(i), this);
        action->setData(colors.at(i));
        action->setIcon(createColorIcon(colors.at(i)));
        connect(action, SIGNAL(triggered()), this, slot);
        colorMenu->addAction(action);
        if (colors.at(i) == defaultColor)
            colorMenu->setDefaultAction(action);
    }
    return colorMenu;
}

QIcon MainWindow::createColorToolButtonIcon(const QString &imageFile, QColor color)
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPixmap image(imageFile);
    // Draw icon centred horizontally on button.
    QRect target(4, 0, 42, 43);
    QRect source=image.rect();
    painter.fillRect(QRect(0, 60, 50, 80), color);
    painter.drawPixmap(target, image, source);

    return QIcon(pixmap);
}

QIcon MainWindow::createColorIcon(QColor color)
{
    QPixmap pixmap(20, 20);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, 20, 20), color);

    return QIcon(pixmap);
}

/*!
 * \brief copy selected items
 */
void MainWindow::copyItems()
{
    m_scene->setMode(DiagramScene::CopyItem);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
}

/*!
 * \brief duplicate items
 * copy and shift a bit to right/down
 */
void MainWindow::duplicateItems()
{
    m_scene->duplicateItems();
}

void MainWindow::copyToClipboard()
{
    // TODO: only selected whould be exported to clipboard
    m_scene->setCursorVisible(false);
    m_scene->copyToBuffer();
    // save selected items to bytearray
    QJsonDocument doc=m_scene->create_json_save(true);
    QByteArray buffer(doc.toJson(QJsonDocument::Compact));
    // unselect everything
    m_scene->abort();
    bool gridVisible=m_scene->isGridVisible();
    m_scene->setGridVisible(false);

    QRectF rect=m_scene->itemsBoundingRect();
    rect.adjust(-1,-1,1,1);
    qreal w=rect.width();
    qreal h=rect.height();
    if(w>h){
        qreal target=w<1000 ? 1000 : w;
        h=target/w*h;
        w=1000;
    }else{
        qreal target=h<1000 ? 1000 : h;
        w=target/h*w;
        h=1000;
    }
    int width=int(w);
    int height=int(h);
    QPixmap pixmap(width,height);
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&painter,QRectF(),rect);
    painter.end();
    // copy image into mimedata
    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(pixmap);
    // add qdia data to mimedata
    mimeData->setData("application/qdia",buffer);
    mimeData->setData("application/qdia_id",m_id);
    // copy mimedata to clipboard
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(mimeData);
    m_scene->setCursorVisible(true);
    m_scene->setGridVisible(gridVisible);
}

void MainWindow::pasteFromClipboard()
{
    // check if we have a qdia mimedata
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QByteArray buffer;
    if (!clipboard->ownsClipboard() && mimeData->hasFormat("application/qdia")) {
        buffer = mimeData->data("application/qdia");
    }
    m_scene->pasteFromBuffer(buffer);
}

void MainWindow::groupItems()
{
    if (m_scene->selectedItems().isEmpty())
        return;

    QGraphicsItemGroup *groupItem=new QGraphicsItemGroup();
    const QRectF boundBox=m_scene->getTotalBoundary(m_scene->selectedItems());
    groupItem->setPos(boundBox.bottomLeft());
    const QList<QGraphicsItem*> lst=m_scene->selectedItems();
    for(QGraphicsItem *item:lst){
        item->setSelected(false);
    }
    for(QGraphicsItem *item:lst){
        if(item->parentItem()){
            continue; // already is part of "group"
        }
        groupItem->addToGroup(item);
    }
    m_scene->addItem(groupItem);
    groupItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    groupItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void MainWindow::ungroupItems()
{
    if (m_scene->selectedItems().isEmpty())
        return;

    foreach (QGraphicsItem *item, m_scene->selectedItems()) {
        if (item->type()==QGraphicsItemGroup::Type) {
            QGraphicsItemGroup *group = qgraphicsitem_cast<QGraphicsItemGroup*>(item);
            group->setSelected(false);
            QList<QGraphicsItem*>lst=group->childItems();
            m_scene->destroyItemGroup(group);
            for(auto *i:lst){
                i->setSelected(false);
            }
        }
    }
}
/*!
 * \brief store selection as user element
 * User elements are present in the tool box
 */
void MainWindow::makeElement()
{
    if (m_scene->selectedItems().isEmpty())
        return;

    // save selected in special path
#ifdef Q_OS_WIN
    QString elemenPath="%appdata%/Roaming/QDia/userElements";
#else
    QString elemenPath=QDir::homePath()+"/.config/QDia/userElements/";
#endif
    QDir dir(elemenPath);
    if(!dir.exists()){
        dir.mkdir(elemenPath);
    }
    fileSaveAs(true,elemenPath);
}
/*!
 * \brief tap a selected item to extract color/style/pen/etc.
 */
void MainWindow::tapItem()
{
    if (m_scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *item=m_scene->selectedItems().first();
    if(item->type()==QGraphicsItemGroup::Type) return; // needs to be a single item
    // check text item
    if(item->type()==DiagramTextItem::Type){
        DiagramTextItem *it=qgraphicsitem_cast<DiagramTextItem*>(item);
        m_textColor=it->defaultTextColor();
        fontColorToolButton->setIcon(createColorToolButtonIcon(":/images/textpointer.png", m_textColor));
        return;
    }
    if((item->type()==DiagramPathItem::Type)||(item->type()==DiagramSplineItem::Type)){
        auto *it=dynamic_cast<QGraphicsPathItem*>(item);
        m_lineColor=it->pen().color();
        lineColorToolButton->setIcon(createColorToolButtonIcon(
                                         ":/images/format-stroke-color.svg", m_lineColor));
        int w=it->pen().width();
        thicknessAction->setData(w);
        lineThicknessButton->setIcon(createLineThicknesIcon(w));

        return;
    }

    auto *it=dynamic_cast<DiagramItem*>(item);
    if(!it) return;
    m_lineColor=it->pen().color();
    lineColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-stroke-color.svg", m_lineColor));
    m_fillColor=it->brush().color();
    fillColorToolButton->setIcon(createColorToolButtonIcon(
                                     ":/images/format-fill-color.svg", m_fillColor));
    int w=it->pen().width();
    thicknessAction->setData(w);
    lineThicknessButton->setIcon(createLineThicknesIcon(w));
}

void MainWindow::activateShortcuts()
{
    foreach(QAction* item,listOfActions){
        item->setEnabled(true);
    }
    foreach(QShortcut* item,listOfShortcuts){
        item->setEnabled(true);
    }
}

void MainWindow::deactivateShortcuts()
{
    foreach(QAction* item,listOfActions){
        item->setEnabled(false);
    }
    foreach(QShortcut* item,listOfShortcuts){
        item->setEnabled(false);
    }
}

void MainWindow::print()
{
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        m_scene->render(&painter);
    }
}

void MainWindow::moveItems()
{
    m_scene->setMode(DiagramScene::MoveItems);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
}

void MainWindow::abort()
{
    m_scene->abort();
    pointerButton->setChecked(true);
    pointerGroupClicked(pointerButton);
}

void MainWindow::backoutOne()
{
    m_scene->backoutOne();
}
/*!
 * \brief abort triggered from scene
 * Basically activate the move button
 */
void MainWindow::abortFromScene()
{
    pointerButton->setChecked(true);
    pointerGroupClicked(pointerButton);
}
/*!
 * \brief insert dot a current cursor position
 */
void MainWindow::insertDot()
{
    QString fn=":/libs/analog/dot.json";
    m_scene->insertElementDirectly(fn);
}
/*!
 * \brief activate line mode from shortcut
 */
void MainWindow::switchToWire()
{
    m_scene->setArrow(0);
    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(false);
    linePointerButton->setIcon(createArrowIcon(0));
    linePointerButton->setChecked(true);
    m_scene->setMode(DiagramScene::InsertLine);
}
/*!
 * \brief activate text mode from shortcut
 */
void MainWindow::switchToText()
{
    textButton->setChecked(true);
    pointerGroupClicked(textButton);
}
/*!
 * \brief place rectangle from shortcut
 */
void MainWindow::switchToRect()
{
    switchToDrawItem(DiagramDrawItem::Rectangle);
}
/*!
 * \brief generalized direct inserting draw elements
 * \param type
 */
void MainWindow::switchToDrawItem(int type)
{
    QWidget *wgt=toolBox->widget(0); // basic shapes
    QGridLayout *layout=qobject_cast<QGridLayout *>(wgt->layout());
    // find right element
    QToolButton *bt;
    bool found=false;
    for(int i=0;i<layout->count();++i){
        wgt=dynamic_cast<QWidget *>(layout->itemAt(i)->widget());
        QLayout *lay=wgt->layout();
        bt=qobject_cast<QToolButton *>(lay->itemAt(0)->widget());
        QButtonGroup *buttonGroup=bt->group();
        if(buttonGroup->id(bt)-InsertDrawItemButton==type){
            found=true;
            break;
        }
    }
    if(!found) return; // button not found -> program error
    bt->setChecked(true);
    buttonGroupClicked(bt);
}
/*!
 * \brief exiting application
 * Check if modified content needs to be saved
 */
void MainWindow::fileExit()
{
    bool canQuit=true;
    recheck:
    int i=m_scene->getSnaphotPosition();
    if(i!=m_lastSavedSnapshot){
        int ret = QMessageBox::warning(this, tr("QDia"),
                                       tr("The document has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Save | QMessageBox::Discard
                                       | QMessageBox::Cancel,
                                       QMessageBox::Save);
        if(ret==QMessageBox::Save){
            fileSave();
            goto recheck;
        }
        if(ret==QMessageBox::Cancel){
            canQuit=false;
        }

    }
    if(canQuit){
        qApp->quit();
    }
}

/*!
 * \brief in selected text elements, search and replace text
 */
void MainWindow::searchAndReplaceTexts()
{
    // open a dialog which asks for text to search and text to replace
    if(searchDialog==nullptr){
        searchDialog=new SearchReplaceDialog(this);
    }
    if(searchDialog){
        connect(searchDialog,&SearchReplaceDialog::findNext,this,&MainWindow::findNextText);
        connect(searchDialog,&SearchReplaceDialog::replace,this,&MainWindow::replaceText);
        connect(searchDialog,&SearchReplaceDialog::replaceAll,this,&MainWindow::replaceAllText);
        searchDialog->show();
    }
}


/*!
 * \brief find next text matching to search string
 */
void MainWindow::findNextText()
{
    if(!searchDialog) return;
    const QString searchText=searchDialog->findText();
    if(searchText.isEmpty()) return;
    m_scene->findText(searchText);
}

/*!
 * \brief find next text matching to search string
 */
void MainWindow::replaceText()
{
    if(!searchDialog) return;
    const QString searchText=searchDialog->findText();
    const QString replaceText=searchDialog->replaceText();
    const bool success=m_scene->replaceText(searchText,replaceText,false);
    if(success){
        m_scene->takeSnapshot();
    }
}

/*!
 * \brief replace text in all selected
 */
void MainWindow::replaceAllText()
{
    if(!searchDialog) return;
    const QString searchText=searchDialog->findText();
    const QString replaceText=searchDialog->replaceText();
    const bool success=m_scene->replaceText(searchText,replaceText,true);
    if(success){
        m_scene->takeSnapshot();
    }
}
/*!
 * \brief show preferences dialog
 */
void MainWindow::showPreferences()
{
    // show preferences dialog
    if(preferencesDialog==nullptr){
        preferencesDialog=new PreferencesDialog(this);
    }
    if(preferencesDialog->exec()==QDialog::Accepted){
        const QString newStyleName=preferencesDialog->getStyle();
        if(newStyleName!=QApplication::style()->objectName()){
            QApplication::setStyle(QStyleFactory::create(newStyleName));
            configuration.style=newStyleName;
        }
    }
}

void MainWindow::exportImage()
{
    m_scene->setCursorVisible(false);
    m_scene->abort();
    bool gridVisible=m_scene->isGridVisible();
    m_scene->setGridVisible(false);
    QFileDialog::Options options;
    QString selectedFilter;
    QString path=m_lastPathImage.isEmpty() ? "" : m_lastPathImage+QDir::separator();
    QString baseName=m_fileName.isEmpty() ? "file" : QFileInfo(m_fileName).baseName();
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Export Diagram to ..."),
            path+baseName+".png",
            tr("Png (*.png);;Jpg (*.jpg);;SVG (*.svg);;Pdf (*.pdf);;Postscript (*.ps)"),
            &selectedFilter,
            options);
    if (!fileName.isEmpty()){

        if((selectedFilter=="Pdf (*.pdf)")or(selectedFilter=="Postscript (*.ps)")) {
            QRectF rect=m_scene->itemsBoundingRect(); // Bonding der Elemente in scene
            QPrinter printer;
            printer.setOutputFileName(fileName);
            QRectF size=printer.pageRect(QPrinter::Millimeter); // definiere Paper mit gleichen Aspectratio wie rect
            size.setHeight(size.width()*rect.height()/rect.width());
            //printer.setPageSize(size,QPrinter::Millimeter);
            //printer.setPageMargins(0,0,0,0,QPrinter::Millimeter);
            QPainter painter(&printer);// generate PDF/PS
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter,QRectF(),rect);
        }
        if((selectedFilter=="SVG (*.svg)")) {
            QRectF rect=m_scene->itemsBoundingRect(); // Bonding der Elemente in scene
            QSvgGenerator generator;
            generator.setFileName(fileName);
            generator.setSize(rect.size().toSize());
            QRectF target=rect;
            target.moveTo(0,0);
            generator.setViewBox(target);
            generator.setTitle(tr("qdiagram"));

            QPainter painter(&generator);// generate SVG
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter,target,rect);
        }
        if((selectedFilter=="Png (*.png)")or(selectedFilter=="Jpg (*.jpg)")){
            QRectF rect=m_scene->itemsBoundingRect(); // Bonding der Elemente in scene
            qreal w=rect.width();
            qreal h=rect.height();
            if(w>h){
                qreal target=w<1000 ? 1000 : w;
                h=target/w*h;
                w=target;
            }else{
                qreal target=h<1000 ? 1000 : h;
                w=target/h*w;
                h=target;
            }
            int width=int(w);
            int height=int(h);
            QPixmap pixmap(width,height);
            pixmap.fill();
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter,QRectF(),rect);
            painter.end();

            pixmap.save(fileName);
        }
        QFileInfo fi(fileName);
        m_lastPathImage= fi.absolutePath();

    }
    m_scene->setCursorVisible(true);
    m_scene->setGridVisible(gridVisible);
}
/*!
 * \brief Import diagram from file
 */
void MainWindow::importDiagram()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString path=m_lastPath.isEmpty() ? "" : m_lastPath+QDir::separator();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Diagram"),
                                                    path+"dia.json",
                                                    tr("QDiagram (*.qdia);;QDiagram old(*.json)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()){
        // reuse user element mechanism
        m_scene->setItemType(fileName);
        m_scene->setMode(DiagramScene::InsertUserElement);
    }
}

void MainWindow::zoomIn()
{
    zoom(1.4);
}

void MainWindow::zoomOut()
{
    zoom(0.7);
}

void MainWindow::zoom(const qreal factor)
{
    QPointF topLeft     = m_view->mapToScene( 0, 0 );
    QPointF bottomRight = m_view->mapToScene( m_view->viewport()->width() - 1, m_view->viewport()->height() - 1 );
    qreal width=bottomRight.x()-topLeft.x();
    qreal height=bottomRight.y()-topLeft.y();
    if((width/factor<=5000)&&(height/factor<=5000)){
        QTransform oldMatrix = m_view->transform();
        qreal newScale=oldMatrix.m11()*factor;
        m_view->resetTransform();
        m_view->translate(oldMatrix.dx(), oldMatrix.dy());
        m_view->scale(newScale, newScale);

        setGrid();
    }
}
/*!
 * \brief zoom with keeping the pointer at the same position
 * \param factor
 * \param pointer
 */
void MainWindow::zoomPointer(const qreal factor, QPointF pointer)
{
    QRectF r=m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
    if(r.contains(pointer)){
        r=r.translated(-pointer);
        r.setRect(r.x()/factor,r.y()/factor,r.width()/factor,r.height()/factor);
        r.translate(pointer);

        doZoomRect(r.topLeft(),r.bottomRight());
    }
}

void MainWindow::zoomRect()
{
    m_scene->setMode(DiagramScene::Zoom);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    setGrid();
}

void MainWindow::doZoomRect(QPointF p1,QPointF p2)
{
    QRectF r(p1,p2);
    m_view->fitInView(r.normalized(),Qt::KeepAspectRatio);
    setGrid();
}

void MainWindow::zoomFit()
{
    m_scene->setCursorVisible(false);
    m_view->fitInView(m_scene->itemsBoundingRect(),Qt::KeepAspectRatio);
    m_scene->setCursorVisible(true);
    setGrid();
}
/*!
 * \brief change grid by halving grid distance
 */
void MainWindow::changeGridFiner()
{
    qreal g=m_scene->grid();
    g/=2.;
    if(g<1) return; // limit grid to minimum of 1
    m_scene->setGrid(g);
    setGrid();
}
/*!
 * \brief change grid by doubling grid distance
 */
void MainWindow::changeGridCoarser()
{
    qreal g=m_scene->grid();
    g*=2.;
    if(g>200.) return; // limit grid to 160
    m_scene->setGrid(g);
    setGrid();
}
/*!
 * \brief show/hide grid
 * \param grid true:show
 */
void MainWindow::toggleGrid(bool grid)
{
    m_scene->setGridVisible(grid);
    QPointF topLeft     = m_view->mapToScene( 0, 0 );
    QPointF bottomRight = m_view->mapToScene( m_view->viewport()->width() - 1, m_view->viewport()->height() - 1 );
    m_scene->invalidate(topLeft.x(),topLeft.y(),bottomRight.x()-topLeft.x(),bottomRight.y()-topLeft.y());
    configuration.showGrid=grid;
}
/*!
 * \brief update grid painting after zoom etc
 */
void MainWindow::setGrid()
{
    if(m_scene->isGridVisible()){
        QPointF topLeft     = m_view->mapToScene( 0, 0 );
        QPointF bottomRight = m_view->mapToScene( m_view->viewport()->width() - 1, m_view->viewport()->height() - 1 );
        qreal width=bottomRight.x()-topLeft.x();
        qreal height=bottomRight.y()-topLeft.y();
        qreal zw=width;
        if(zw<height) zw=height;
        int n=int(zw)/int(m_scene->grid());
        int k=1;
        while(n/k>50)
        {
            k=k*2;
        }
        m_scene->setGridScale(k);
        m_view->resetCachedContent();
    }
}

void MainWindow::fileSaveAs(bool selectedItemsOnly,QString pathSuggestion)
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString path=m_lastPath.isEmpty() ? "" : m_lastPath+QDir::separator();
    if(!pathSuggestion.isEmpty()){
        path=pathSuggestion;
    }
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Diagram as ..."),
            path+"dia.qdia",
            tr("QDiagram (*.qdia);;QDiagram old(*.json)"),
            &selectedFilter,
            options);
    if (!fileName.isEmpty()){
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            QMessageBox::warning(this,tr("File operation error"),file.errorString());
        }
        else
        {
            if(m_scene->save_json(&file,selectedItemsOnly)){
                m_fileName=fileName;
                m_recentFiles.removeOne(m_fileName);
                m_recentFiles.prepend(m_fileName);
                populateRecentFiles();
                m_lastSavedSnapshot=m_scene->getSnaphotPosition();
            }
            file.close();
            if(file.error()){
                qDebug() << "Error: cannot write file "
                << file.fileName()
                << file.errorString();
            }else{
                setWindowFilePath(m_fileName);
            }
        }
        QFileInfo fi(fileName);
        m_lastPath= fi.absolutePath();
    }
}

void MainWindow::fileSave()
{
    if (!m_fileName.isEmpty()){
        QFile file(m_fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            QMessageBox::warning(this,tr("File operation error"),file.errorString());
        }
        else
        {
            m_scene->save_json(&file);
            m_recentFiles.removeOne(m_fileName);
            m_recentFiles.prepend(m_fileName);
            populateRecentFiles();
            m_lastSavedSnapshot=m_scene->getSnaphotPosition();
        }
    }else{
        fileSaveAs();
    }
}

void MainWindow::fileOpen()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString path=m_lastPath.isEmpty() ? "" : m_lastPath+QDir::separator();
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Load Diagram"),
            path+"dia.json",
            tr("QDiagram (*.qdia);;QDiagram old(*.json)"),
            &selectedFilter,
            options);
    if (!fileName.isEmpty()){
        openFile(fileName);
        m_recentFiles.removeOne(fileName);
        m_recentFiles.prepend(fileName);
        populateRecentFiles();
        QFileInfo fi(fileName);
        if(m_lastPath!=fi.absolutePath()){
            m_lastPathImage=m_lastPath; // assume to be exported in same folder
        }
        m_lastPath= fi.absolutePath();
    }
}
/*!
 * \brief open file
 * \param fileName
 * \return success
 */
bool MainWindow::openFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }
    abort(); // force defined state
    m_scene->clear();
    m_scene->load_json(&file);
    m_scene->takeSnapshot();
    m_fileName=fileName;
    setWindowFilePath(m_fileName);
    return true;
}
/*!
 * \brief open recent file
 * triggered from openrecent files menu
 */

void MainWindow::openRecentFile()
{
    QAction *act=qobject_cast<QAction*>(sender());
    QString fn=act->text();
    m_recentFiles.removeOne(fn);
    if(QFileInfo::exists(fn)){
        openFile(fn);
        m_recentFiles.prepend(fn);
    }
    populateRecentFiles();
}

QMenu *MainWindow::createArrowMenu(const char *slot, const int def)
{
    QStringList names;
        names << tr("Path") << tr("Start") << tr("End") << tr("StartEnd") << tr("Spline") << tr("Spline Start") << tr("Spline End") << tr("Spline StartEnd");
    QMenu *arrowMenu = new QMenu;
    for (int i = 0; i < names.count(); ++i) {
        QAction *action = new QAction(names.at(i), this);
        action->setData(i);
        action->setIcon(createArrowIcon(i));
        connect(action, SIGNAL(triggered()),
                this, slot);
        arrowMenu->addAction(action);
        if (i == def) {
            arrowMenu->setDefaultAction(action);
        }
    }
    return arrowMenu;
}

QIcon MainWindow::createArrowIcon(const int i)
{
    QPixmap pixmap(50, 80);
    if(i<4){
        DiagramPathItem* item=new DiagramPathItem(DiagramPathItem::DiagramType(i),nullptr,nullptr);
        pixmap=item->icon();
        delete item;
    }else{
        DiagramSplineItem* item=new DiagramSplineItem(DiagramSplineItem::DiagramType(i%4),nullptr,nullptr);
        pixmap=item->icon();
        delete item;
    }

    return QIcon(pixmap);
}
/*!
 * \brief create line thickness menu
 * \param slot to connect to
 * \param def default action
 * \return
 */
QMenu *MainWindow::createLineThicknessMenu(const char *slot, const int def)
{
    QList<int> th;
        th << 1 << 2 << 4;
    QMenu *thicknessMenu = new QMenu;
    for (int i = 0; i < th.count(); ++i) {
        QAction *action = new QAction(QString("%1").arg(th[i]), this);
        action->setData(th[i]);
        action->setIcon(createLineThicknesIcon(th[i]));
        connect(action, SIGNAL(triggered()),
                this, slot);
        thicknessMenu->addAction(action);
        if (i == def) {
            thicknessMenu->setDefaultAction(action);
        }
    }
    return thicknessMenu;
}

QIcon MainWindow::createLineThicknesIcon(const int i)
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    QPen pen(Qt::black);
    pen.setWidth(2*i);
    p.setPen(pen);

    p.drawLine(10,40,40,40);

    return QIcon(pixmap);
}

QMenu *MainWindow::createLinePatternMenu(const char *slot, const int def)
{
    QStringList names;
        names << tr("No Pen") << tr("Solid Line") << tr("Dash Line") << tr("Dot Line") << tr("Dash Dot Line") << tr("Dash Dot Dot Line");
    QMenu *patternMenu = new QMenu;
    for (int i = 0; i < names.count(); ++i) {
        QAction *action = new QAction(names.at(i), this);
        action->setData(i);
        action->setIcon(createLinePatternIcon(i));
        connect(action, SIGNAL(triggered()),
                this, slot);
        patternMenu->addAction(action);
        if (i == def) {
            patternMenu->setDefaultAction(action);
        }
    }
    return patternMenu;
}

QIcon MainWindow::createLinePatternIcon(const int i)
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    QPen pen(Qt::black);
    Qt::PenStyle style=static_cast<Qt::PenStyle>(i);
    pen.setStyle(style);
    p.setPen(pen);

    p.drawLine(10,40,40,40);

    return QIcon(pixmap);
}

void MainWindow::transformSelected(const QTransform transform, QList<QGraphicsItem *> items, bool forceOnGrid)
{
    if(items.isEmpty()) return;
    m_scene->filterSelectedChildItems(items);
    QRectF bound = m_scene->getTotalBoundary(items);
    QPointF pt=bound.center();
    if(forceOnGrid){
        // just use first element as it stays the same on repeated call on rotate
        pt=m_scene->onGrid(pt);
        QLineF diff(pt,m_rotationCenter);
        if(diff.length()<=1.5*m_scene->grid()){
            pt=m_rotationCenter;
        }else{
            m_rotationCenter=pt;
        }

    }

    transformItems(transform,items,pt);
    m_scene->takeSnapshot();
}
/*!
 * \brief perform the transformation of items around an anchor point
 * \param transform
 * \param items
 * \param abchorPoint
 */
void MainWindow::transformItems(const QTransform transform, QList<QGraphicsItem *> items, QPointF anchorPoint)
{
    // special treatment for selected draw/path/spline item (extra copy to show on top)
    if(items.size()==1){
        QGraphicsItem *item=items.first();
        if(item->type()==DiagramPathItem::Type){
            DiagramPathItem *it=qgraphicsitem_cast<DiagramPathItem*>(item);
            if(it){
                auto *partner=it->partnerItem();
                items.append(partner);
            }
        }
        if(item->type()==DiagramSplineItem::Type){
            DiagramSplineItem *it=qgraphicsitem_cast<DiagramSplineItem*>(item);
            auto *partner=it->partnerItem();
            if(partner){
                items.append(partner);
            }
        }
        if(item->type()==DiagramDrawItem::Type){
            DiagramDrawItem *it=qgraphicsitem_cast<DiagramDrawItem*>(item);
            auto *partner=it->partnerItem();
            if(partner){
                items.append(partner);
            }
        }
    }
    foreach( QGraphicsItem *item, items){
        if(!item) continue;
        // special treatment for texts
        if(item->type()==DiagramTextItem::Type && items.count()>1){
            // if only one item is selected, the anchor point is not moved
            if(transform==QTransform(-1,0,0,1,0,0)){
                // flipX, text not flipped but anchorpoint moved
                DiagramTextItem *it=qgraphicsitem_cast<DiagramTextItem*>(item);
                qreal dx=it->anchorPoint().x()-anchorPoint.x();
                it->setAnchorPoint(QPointF(it->anchorPoint().x()-2*dx,it->anchorPoint().y()));
                // move anchor point
                auto alignment=it->alignment();
                if(alignment.testFlag(Qt::AlignLeft)){
                    alignment.setFlag(Qt::AlignRight);
                    alignment.setFlag(Qt::AlignLeft,false);
                }else{
                    if(alignment.testFlag(Qt::AlignRight)){
                        alignment|=Qt::AlignLeft;
                        alignment.setFlag(Qt::AlignRight,false);
                    }
                }
                it->setAlignment(alignment);
                it->updateGeometry();
                continue;
            }
        }
        QTransform trans=item->transform();
        QPointF shift=item->pos()-anchorPoint;
        item->setTransform(trans*QTransform(1,0,0,1,shift.x(),shift.y())*transform*QTransform(1,0,0,1,-shift.x(),-shift.y()),false);
        // correct anchor point shift
        QTransform transform=item->transform();
        qreal mx=item->pos().x()+transform.dx();
        qreal my=item->pos().y()+transform.dy();
        transform*=QTransform::fromTranslate(-transform.dx(),-transform.dy());
        item->setPos(mx,my);
        item->setTransform(transform);
    }
}

void MainWindow::lineArrowButtonTriggered()
{
    int tp=arrowAction->data().toInt();
    m_scene->setArrow(tp%4);
    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(false);
    if(tp<4){
        m_scene->setMode(DiagramScene::InsertLine);
    }else{
        m_scene->setMode(DiagramScene::InsertSpline);
    }
}

void MainWindow::textAddButtonTriggered()
{
    m_scene->setMode(DiagramScene::InsertText);
}

void MainWindow::moveCursor(QPointF p)
{
    QPoint np=m_view->mapFromScene(p.toPoint());
    np=m_view->mapToGlobal(np);
    QCursor c;
    c.setPos(np);
}

void MainWindow::lineArrowChanged()
{
    arrowAction = qobject_cast<QAction *>(sender());
    linePointerButton->setIcon(createArrowIcon(arrowAction->data().toInt()));
    lineArrowButtonTriggered();
}

void MainWindow::lineThicknessChanged()
{
    thicknessAction=qobject_cast<QAction *>(sender());
    lineThicknessButton->setIcon(createLineThicknesIcon(thicknessAction->data().toInt()));
    lineThicknessButtonTriggered();
}

void MainWindow::linePatternChanged()
{
    patternAction=qobject_cast<QAction *>(sender());
    linePatternButton->setIcon(createLinePatternIcon(patternAction->data().toInt()));
    linePatternButtonTriggered();
}

/* TODO
 * color template ? / store custom colors
 ** show handlers on top when selected and below
 * click unselect (shift/? alt)
 * click again to select lower element
 * better handlers when zoomed out ?
 * cut line
 * manhattan routing
 * export wider to entail wider lines ?
 * user elements -> order ?
 * fix flip/rotate when moving/dragging several elements
 * manage user generated elements
 * tap style
 * Align ?
 * import xcircuit/drawio?
 * read xcircuit lps
 * show svg
 */
