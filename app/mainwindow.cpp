#include "mainwindow.h"
#include <QtWidgets>
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"
#include <QDockWidget>
#include <QGraphicsItem>
#include <QMdiSubWindow>
#include "customproperty.h"

#include "drawobj.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createActions();
    createToolbars();
    createToolBox();
    createPropertyEditor();

    setStatusBar(new QStatusBar);


    scene = new DrawScene(this);
    scene->setSceneRect(QRectF(0 , 0 , 800, 600));

    connect(scene, SIGNAL(itemSelected(QGraphicsItem*)),
            this, SLOT(itemSelected(QGraphicsItem*)));


    view = new DrawView(scene);
    scene->setView(view);

    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    view->setBackgroundBrush(Qt::darkGray);

    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setBackgroundRole(QPalette::Dark);
    setCentralWidget(mdiArea);
    mdiArea->addSubWindow(view);
    mdiArea->tileSubWindows();


    setWindowTitle(tr("Qt Graphics Editor"));
    setUnifiedTitleAndToolBarOnMac(true);

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(updateUI()));
    m_timer.start(100);
    theControlledObject = NULL;

}

MainWindow::~MainWindow()
{

}

void MainWindow::createToolBox()
{
    QDockWidget *dock = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    listView = new QListView();
    listView->setViewMode(QListView::IconMode);
    listView->setStyleSheet(tr("QListView {background-color: mediumaquamarine;}"));
    toolBox = new QToolBox(dock);
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    toolBox->addItem(listView,tr("Graphics Library"));
    dock->setWidget(toolBox);
}

void MainWindow::createActions()
{
    // create align actions

    actionRight   = new QAction(QIcon(":/icons/align_right.png"),tr("align right"),this);
    actionLeft    = new QAction(QIcon(":/icons/align_left.png"),tr("align left"),this);
    actionVCenter = new QAction(QIcon(":/icons/align_vcenter.png"),tr("align vcenter"),this);
    actionHCenter = new QAction(QIcon(":/icons/align_hcenter.png"),tr("align hcenter"),this);
    actionTop     = new QAction(QIcon(":/icons/align_top.png"),tr("align top"),this);
    actionBottom  = new QAction(QIcon(":/icons/align_bottom.png"),tr("align bottom"),this);
    actionBringToFront = new QAction(QIcon(":/icons/bringtofront.png"),tr("bring to front"),this);
    actionSendToBack   = new QAction(QIcon(":/icons/sendtoback.png"),tr("send to back"),this);
    actionGroup        = new QAction(QIcon(":/icons/group.png"),tr("group"),this);
    actionUnGroup        = new QAction(QIcon(":/icons/ungroup.png"),tr("ungroup"),this);

    connect(actionBringToFront,SIGNAL(triggered()),this,SLOT(on_actionBringToFront_triggered()));
    connect(actionSendToBack,SIGNAL(triggered()),this,SLOT(on_actionSendToBack_triggered()));
    connect(actionRight,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(actionGroup,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(actionUnGroup,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));


    //create draw actions
    actionSelect = new QAction(QIcon(":/icons/arrow.png"),tr("select tool"),this);

    actionSelect->setCheckable(true);
    actionLine= new QAction(QIcon(":/icons/line.png"),tr("line tool"),this);
    actionLine->setCheckable(true);
    actionRect= new QAction(QIcon(":/icons/rectangle.png"),tr("rect tool"),this);
    actionRect->setCheckable(true);

    actionRoundRect =  new QAction(QIcon(":/icons/roundrect.png"),tr("roundrect tool"),this);
    actionRoundRect->setCheckable(true);
    actionEllipse= new QAction(QIcon(":/icons/ellipse.png"),tr("ellipse tool"),this);
    actionEllipse->setCheckable(true);
    actionPolygon= new QAction(QIcon(":/icons/polygon.png"),tr("polygon tool"),this);
    actionPolygon->setCheckable(true);
    actionPolyline= new QAction(QIcon(":/icons/polyline.png"),tr("polyline tool"),this);
    actionPolyline->setCheckable(true);
    actionBezier= new QAction(QIcon(":/icons/bezier.png"),tr("bezier tool"),this);
    actionBezier->setCheckable(true);

    actionRotate = new QAction(QIcon(":/icons/rotate.png"),tr("rotate tool"),this);
    actionRotate->setCheckable(true);

    drawActionGroup = new QActionGroup(this);
    drawActionGroup->addAction(actionSelect);
    drawActionGroup->addAction(actionLine);
    drawActionGroup->addAction(actionRect);
    drawActionGroup->addAction(actionRoundRect);
    drawActionGroup->addAction(actionEllipse);
    drawActionGroup->addAction(actionPolygon);
    drawActionGroup->addAction(actionPolyline);
    drawActionGroup->addAction(actionBezier);
    drawActionGroup->addAction(actionRotate);
    actionSelect->setChecked(true);


    connect(actionSelect,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionLine,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionRect,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionRoundRect,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionEllipse,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionPolygon,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionPolyline,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionBezier,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(actionRotate,SIGNAL(triggered()),this,SLOT(addShape()));

    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));
    this->addAction(deleteAction);
}

void MainWindow::createMenus()
{

}

void MainWindow::createToolbars()
{
    // create draw toolbar
    drawToolBar = addToolBar(tr("drawing"));
    drawToolBar->setIconSize(QSize(24,24));
    drawToolBar->addAction(actionSelect);
    drawToolBar->addAction(actionLine);
    drawToolBar->addAction(actionRect);
    drawToolBar->addAction(actionRoundRect);
    drawToolBar->addAction(actionEllipse);
    drawToolBar->addAction(actionPolygon);
    drawToolBar->addAction(actionPolyline);
    drawToolBar->addAction(actionBezier);
    drawToolBar->addAction(actionRotate);

    // create align toolbar
    alignToolBar = addToolBar(tr("align"));
    alignToolBar->setIconSize(QSize(24,24));
    alignToolBar->addAction(actionRight);
    alignToolBar->addAction(actionLeft);
    alignToolBar->addAction(actionVCenter);
    alignToolBar->addAction(actionHCenter);
    alignToolBar->addAction(actionTop);
    alignToolBar->addAction(actionBottom);
    alignToolBar->addAction(actionBringToFront);
    alignToolBar->addAction(actionSendToBack);
    alignToolBar->addAction(actionGroup);
    alignToolBar->addAction(actionUnGroup);
}

void MainWindow::createPropertyEditor()
{
    QDockWidget *dock = new QDockWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    propertyEditor = new ObjectController(this);
    dock->setWidget(propertyEditor);
}

void MainWindow::addShape()
{
    if ( sender() == actionSelect )
        DrawTool::c_drawShape = selection;
    else if (sender() == actionLine )
        DrawTool::c_drawShape = line;
    else if ( sender() == actionRect )
        DrawTool::c_drawShape = rectangle;
    else if ( sender() == actionRoundRect )
        DrawTool::c_drawShape = roundrect;
    else if ( sender() == actionEllipse )
        DrawTool::c_drawShape = ellipse ;
    else if ( sender() == actionPolygon )
        DrawTool::c_drawShape = polygon;
    else if ( sender() == actionBezier )
        DrawTool::c_drawShape = bezier ;
    else if (sender() == actionRotate )
        DrawTool::c_drawShape = rotation;
}

void MainWindow::updateUI()
{
    switch (DrawTool::c_drawShape ) {
    case selection:
        actionSelect->setChecked(true);
        break;
    case line:
        actionLine->setChecked(true);
        break;
    case rectangle:
        actionRect->setChecked(true);
        break;
    case roundrect:
        actionRoundRect->setChecked(true);
        break;
    case ellipse:
        actionEllipse->setChecked(true);
        break;
    case bezier:
        actionBezier->setChecked(true);
        break;
    case rotation:
        actionRotate->setChecked(true);
        break;
    case polygon:
        actionPolygon->setChecked(true);
        break;
    default:
        break;
    }

    if (scene->selectedItems().count() > 0 ){
        actionBringToFront->setEnabled(true);
        actionSendToBack->setEnabled(true);
        if ( scene->selectedItems().count() > 1 )
            actionGroup->setEnabled(true);
        else
            actionGroup->setEnabled(false);
        GraphicsItemGroup * group = dynamic_cast<GraphicsItemGroup*>(scene->selectedItems().first());
        if ( group )
            actionUnGroup->setEnabled(true);
        else
            actionUnGroup->setEnabled(false);
    }else{
        actionBringToFront->setEnabled(false);
        actionSendToBack->setEnabled(false);
        actionGroup->setEnabled(false);
        actionUnGroup->setEnabled(false);
    }
}

void MainWindow::itemSelected(QGraphicsItem *item)
{
    theControlledObject = dynamic_cast<QObject*>(item);
    if ( theControlledObject ){
        propertyEditor->setObject(theControlledObject);
    }

}

void MainWindow::deleteItem()
{
    if (scene->selectedItems().isEmpty())
        return;

    QList<QGraphicsItem *> overlapItems = scene->selectedItems();

    foreach (QGraphicsItem *item, overlapItems) {
        scene->removeItem(item);
        delete item;
    }
}

void MainWindow::on_actionBringToFront_triggered()
{
    if (scene->selectedItems().isEmpty())
        return;
    QGraphicsItem *selectedItem = scene->selectedItems().first();

    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();
    qreal zValue = 0;
    foreach (QGraphicsItem *item, overlapItems) {
        if (item->zValue() >= zValue && item->type() == GraphicsItem::Type)
            zValue = item->zValue() + 0.1;
    }
    selectedItem->setZValue(zValue);


}
void MainWindow::on_actionSendToBack_triggered()
{
    if (scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *selectedItem = scene->selectedItems().first();
    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    foreach (QGraphicsItem *item, overlapItems) {
        if (item->zValue() <= zValue && item->type() == GraphicsItem::Type)
            zValue = item->zValue() - 0.1;
    }
    selectedItem->setZValue(zValue);
}

void MainWindow::on_aglin_triggered()
{

/*
   QtPenPropertyManager *penPropertyManager = new QtPenPropertyManager();
   QtProperty * property = penPropertyManager->addProperty("pen");

   QtTreePropertyBrowser *editor = new QtTreePropertyBrowser();
   editor->setFactoryForManager(penPropertyManager->subIntPropertyManager(),new QtSpinBoxFactory());
   editor->setFactoryForManager(penPropertyManager->subEnumPropertyManager(),new QtEnumEditorFactory());

   editor->addProperty(property);

   QPen pen;
   pen.setWidth(10);
   pen.setCapStyle(Qt::RoundCap);
   pen.setJoinStyle(Qt::SvgMiterJoin);
   penPropertyManager->setValue(property,pen);

   editor->show();
*/
    if ( sender() == actionRight )
    {
        view->fitInView(scene->sceneRect());
    }else if ( sender() == actionGroup ){
        //QGraphicsItemGroup test
        QList<QGraphicsItem *> selectedItems = scene->selectedItems();
        GraphicsItemGroup * group = new GraphicsItemGroup();
        foreach (QGraphicsItem *item, selectedItems) {
            item->setSelected(false);
            group->addToGroup(item);
        }
        group->setSelected(true);
        scene->addItem(group);
    } else if ( sender() == actionUnGroup ){
        QGraphicsItem *selectedItem = scene->selectedItems().first();
        GraphicsItemGroup * group = dynamic_cast<GraphicsItemGroup*>(selectedItem);
        if ( group ){
            group->setSelected(false);
            QList<QGraphicsItem *> overlapItems = group->childItems();
            foreach (QGraphicsItem *item, overlapItems) {
                group->removeFromGroup(item);
                item->setSelected(true);
            }
            scene->removeItem(group);
            delete group;
        }
    }
}
