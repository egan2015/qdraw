#include "mainwindow.h"
#include <QtWidgets>
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"
#include <QDockWidget>
#include <QGraphicsItem>
#include <QMdiSubWindow>
#include <QUndoStack>
#include "customproperty.h"
#include "drawobj.h"
#include "commands.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    undoStack = new QUndoStack(this);
    undoView = new QUndoView(undoStack);
    undoView->setWindowTitle(tr("Command List"));
    undoView->setAttribute(Qt::WA_QuitOnClose, false);

    QDockWidget *dock = new QDockWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    dock->setWidget(undoView);


    createActions();
    createToolbars();
    createToolBox();
    createPropertyEditor();

 /*
    m_posInfo = new QLabel(tr("x,y"));
    m_posInfo->setMinimumSize(m_posInfo->sizeHint());
    m_posInfo->setAlignment(Qt::AlignHCenter);
    statusBar()->addWidget(m_posInfo);
*/

    funcAct = new QAction(tr("func test"),this);
    connect(funcAct,SIGNAL(triggered()),this,SLOT(on_func_test_triggered()));
    menuBar()->addMenu(tr("Func"))->addAction(funcAct);

    scene = new DrawScene(this);

    QRectF rc = QRectF(0 , 0 , 800, 600);

    scene->setSceneRect(rc);
    qDebug()<<rc.bottomLeft()<<rc.size() << rc.topLeft();

    connect(scene, SIGNAL(selectionChanged()),
            this, SLOT(itemSelected()));

    connect(scene,SIGNAL(itemAdded(QGraphicsItem*)),
            this, SLOT(itemAdded(QGraphicsItem*)));
    connect(scene,SIGNAL(itemMoved(QGraphicsItem*,QPointF)),
            this,SLOT(itemMoved(QGraphicsItem*,QPointF)));
    connect(scene,SIGNAL(itemRotate(QGraphicsItem*,qreal)),
            this,SLOT(itemRotate(QGraphicsItem*,qreal)));

    view = new DrawView(scene);
    scene->setView(view);
   // view->setTransform(view->transform().scale(1.0,-1.0));
    connect(view,SIGNAL(positionChanged(int,int)),this,SLOT(positionChanged(int,int)));

    view->setRenderHint(QPainter::Antialiasing);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    scene->setBackgroundBrush(Qt::darkGray);

    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    mdiArea->addSubWindow(view);
    mdiArea->tileSubWindows();


    setWindowTitle(tr("Qt Drawing"));
    setUnifiedTitleAndToolBarOnMac(true);


    connect(QApplication::clipboard(),SIGNAL(dataChanged()),this,SLOT(dataChanged()));
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(updateActions()));
    m_timer.start(500);
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

    rightAct   = new QAction(QIcon(":/icons/align_right.png"),tr("align right"),this);
    leftAct    = new QAction(QIcon(":/icons/align_left.png"),tr("align left"),this);
    vCenterAct = new QAction(QIcon(":/icons/align_vcenter.png"),tr("align vcenter"),this);
    hCenterAct = new QAction(QIcon(":/icons/align_hcenter.png"),tr("align hcenter"),this);
    upAct      = new QAction(QIcon(":/icons/align_top.png"),tr("align top"),this);
    downAct    = new QAction(QIcon(":/icons/align_bottom.png"),tr("align bottom"),this);
    horzAct    = new QAction(QIcon(":/icons/align_horzeven.png"),tr("align horzeven"),this);
    vertAct    = new QAction(QIcon(":/icons/align_verteven.png"),tr("align verteven"),this);
    heightAct  = new QAction(QIcon(":/icons/align_height.png"),tr("align height"),this);
    widthAct   = new QAction(QIcon(":/icons/align_width.png"),tr("align width"),this);
    allAct     = new QAction(QIcon(":/icons/align_all.png"),tr("align width and height"),this);

    bringToFrontAct = new QAction(QIcon(":/icons/bringtofront.png"),tr("bring to front"),this);
    sendToBackAct   = new QAction(QIcon(":/icons/sendtoback.png"),tr("send to back"),this);
    groupAct        = new QAction(QIcon(":/icons/group.png"),tr("group"),this);
    unGroupAct        = new QAction(QIcon(":/icons/ungroup.png"),tr("ungroup"),this);

    connect(bringToFrontAct,SIGNAL(triggered()),this,SLOT(on_actionBringToFront_triggered()));
    connect(sendToBackAct,SIGNAL(triggered()),this,SLOT(on_actionSendToBack_triggered()));
    connect(rightAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(leftAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(vCenterAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(hCenterAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(upAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(downAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));

    connect(horzAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(vertAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(heightAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(widthAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));
    connect(allAct,SIGNAL(triggered()),this,SLOT(on_aglin_triggered()));

    connect(groupAct,SIGNAL(triggered()),this,SLOT(on_group_triggered()));
    connect(unGroupAct,SIGNAL(triggered()),this,SLOT(on_unGroup_triggered()));


    //create draw actions
    selectAct = new QAction(QIcon(":/icons/arrow.png"),tr("select tool"),this);

    selectAct->setCheckable(true);
    lineAct = new QAction(QIcon(":/icons/line.png"),tr("line tool"),this);
    lineAct->setCheckable(true);
    rectAct = new QAction(QIcon(":/icons/rectangle.png"),tr("rect tool"),this);
    rectAct->setCheckable(true);

    roundRectAct =  new QAction(QIcon(":/icons/roundrect.png"),tr("roundrect tool"),this);
    roundRectAct->setCheckable(true);
    ellipseAct = new QAction(QIcon(":/icons/ellipse.png"),tr("ellipse tool"),this);
    ellipseAct->setCheckable(true);
    polygonAct = new QAction(QIcon(":/icons/polygon.png"),tr("polygon tool"),this);
    polygonAct->setCheckable(true);
    polylineAct = new QAction(QIcon(":/icons/polyline.png"),tr("polyline tool"),this);
    polylineAct->setCheckable(true);
    bezierAct= new QAction(QIcon(":/icons/bezier.png"),tr("bezier tool"),this);
    bezierAct->setCheckable(true);

    rotateAct = new QAction(QIcon(":/icons/rotate.png"),tr("rotate tool"),this);
    rotateAct->setCheckable(true);

    drawActionGroup = new QActionGroup(this);
    drawActionGroup->addAction(selectAct);
    drawActionGroup->addAction(lineAct);
    drawActionGroup->addAction(rectAct);
    drawActionGroup->addAction(roundRectAct);
    drawActionGroup->addAction(ellipseAct);
    drawActionGroup->addAction(polygonAct);
    drawActionGroup->addAction(polylineAct);
    drawActionGroup->addAction(bezierAct);
    drawActionGroup->addAction(rotateAct);
    selectAct->setChecked(true);


    connect(selectAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(lineAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(rectAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(roundRectAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(ellipseAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(polygonAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(polylineAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(bezierAct,SIGNAL(triggered()),this,SLOT(addShape()));
    connect(rotateAct,SIGNAL(triggered()),this,SLOT(addShape()));

    deleteAct = new QAction(tr("&Delete"), this);
    deleteAct->setShortcut(tr("Delete"));

    undoAct = undoStack->createUndoAction(this,tr("undo"));
    undoAct->setIcon(QIcon(":/icons/undo.png"));
    undoAct->setShortcuts(QKeySequence::Undo);

    redoAct = undoStack->createRedoAction(this,tr("redo"));
    redoAct->setIcon(QIcon(":/icons/redo.png"));
    redoAct->setShortcuts(QKeySequence::Redo);

    zoomInAct = new QAction(QIcon(":/icons/zoomin.png"),tr("zoomIn"),this);
    zoomOutAct = new QAction(QIcon(":/icons/zoomout.png"),tr("zoomOut"),this);

    copyAct = new QAction(QIcon(":/icons/copy.png"),tr("copy"),this);
    copyAct->setShortcut(QKeySequence::Copy);
    pasteAct = new QAction(QIcon(":/icons/paste.png"),tr("paste"),this);
    pasteAct->setShortcut(QKeySequence::Paste);
    cutAct = new QAction(QIcon(":/icons/cut.png"),tr("cut"),this);
    cutAct->setShortcut(QKeySequence::Cut);
    pasteAct->setEnabled(false);

    connect(copyAct,SIGNAL(triggered()),this,SLOT(on_copy()));
    connect(pasteAct,SIGNAL(triggered()),this,SLOT(on_paste()));
    connect(cutAct,SIGNAL(triggered()),this,SLOT(on_cut()));

    connect(zoomInAct , SIGNAL(triggered()),this,SLOT(zoomIn()));
    connect(zoomOutAct , SIGNAL(triggered()),this,SLOT(zoomOut()));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteItem()));
    this->addAction(deleteAct);
}

void MainWindow::createMenus()
{

}

void MainWindow::createToolbars()
{
    // create edit toolbar
    editToolBar = addToolBar(tr("edit"));
    editToolBar->setIconSize(QSize(24,24));
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(cutAct);

    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    editToolBar->addAction(zoomInAct);
    editToolBar->addAction(zoomOutAct);

    // create draw toolbar
    drawToolBar = addToolBar(tr("drawing"));
    drawToolBar->setIconSize(QSize(24,24));
    drawToolBar->addAction(selectAct);
    drawToolBar->addAction(lineAct);
    drawToolBar->addAction(rectAct);
    drawToolBar->addAction(roundRectAct);
    drawToolBar->addAction(ellipseAct);
    drawToolBar->addAction(polygonAct);
    drawToolBar->addAction(polylineAct);
    drawToolBar->addAction(bezierAct);
    drawToolBar->addAction(rotateAct);

    // create align toolbar
    alignToolBar = addToolBar(tr("align"));
    alignToolBar->setIconSize(QSize(24,24));
    alignToolBar->addAction(upAct);
    alignToolBar->addAction(downAct);
    alignToolBar->addAction(rightAct);
    alignToolBar->addAction(leftAct);
    alignToolBar->addAction(vCenterAct);
    alignToolBar->addAction(hCenterAct);

    alignToolBar->addAction(horzAct);
    alignToolBar->addAction(vertAct);
    alignToolBar->addAction(heightAct);
    alignToolBar->addAction(widthAct);
    alignToolBar->addAction(allAct);

    alignToolBar->addAction(bringToFrontAct);
    alignToolBar->addAction(sendToBackAct);
    alignToolBar->addAction(groupAct);
    alignToolBar->addAction(unGroupAct);
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
    if ( sender() == selectAct )
        DrawTool::c_drawShape = selection;
    else if (sender() == lineAct )
        DrawTool::c_drawShape = line;
    else if ( sender() == rectAct )
        DrawTool::c_drawShape = rectangle;
    else if ( sender() == roundRectAct )
        DrawTool::c_drawShape = roundrect;
    else if ( sender() == ellipseAct )
        DrawTool::c_drawShape = ellipse ;
    else if ( sender() == polygonAct )
        DrawTool::c_drawShape = polygon;
    else if ( sender() == bezierAct )
        DrawTool::c_drawShape = bezier ;
    else if (sender() == rotateAct )
        DrawTool::c_drawShape = rotation;
    else if (sender() == polylineAct )
        DrawTool::c_drawShape = polyline;

    if ( sender() != selectAct && sender() != rotateAct ){
        scene->clearSelection();
    }
}

void MainWindow::updateActions()
{
    selectAct->setChecked(DrawTool::c_drawShape == selection);
    lineAct->setChecked(DrawTool::c_drawShape == line);
    rectAct->setChecked(DrawTool::c_drawShape == rectangle);
    roundRectAct->setChecked(DrawTool::c_drawShape == roundrect);
    ellipseAct->setChecked(DrawTool::c_drawShape == ellipse);
    bezierAct->setChecked(DrawTool::c_drawShape == bezier);
    rotateAct->setChecked(DrawTool::c_drawShape == rotation);
    polygonAct->setChecked(DrawTool::c_drawShape == polygon);
    polylineAct->setChecked(DrawTool::c_drawShape == polyline );
    undoAct->setEnabled(undoStack->canUndo());
    redoAct->setEnabled(undoStack->canRedo());

    bringToFrontAct->setEnabled(scene->selectedItems().count() > 0);
    sendToBackAct->setEnabled(scene->selectedItems().count() > 0);
    groupAct->setEnabled( scene->selectedItems().count() > 1);
    unGroupAct->setEnabled(scene->selectedItems().count() > 0 &&
                              dynamic_cast<GraphicsItemGroup*>(scene->selectedItems().first()));

    leftAct->setEnabled(scene->selectedItems().count() > 1);
    rightAct->setEnabled(scene->selectedItems().count() > 1);
    leftAct->setEnabled(scene->selectedItems().count() > 1);
    vCenterAct->setEnabled(scene->selectedItems().count() > 1);
    hCenterAct->setEnabled(scene->selectedItems().count() > 1);
    upAct->setEnabled(scene->selectedItems().count() > 1);
    downAct->setEnabled(scene->selectedItems().count() > 1);

    heightAct->setEnabled(scene->selectedItems().count() > 1);
    widthAct->setEnabled(scene->selectedItems().count() > 1);
    allAct->setEnabled(scene->selectedItems().count()>1);
    horzAct->setEnabled(scene->selectedItems().count() > 2);
    vertAct->setEnabled(scene->selectedItems().count() > 2);

    copyAct->setEnabled(scene->selectedItems().count() > 0);
    cutAct->setEnabled(scene->selectedItems().count() > 0);
}

void MainWindow::itemSelected()
{
    if ( scene->selectedItems().count() > 0
         && scene->selectedItems().first()->isSelected())
    {
        QGraphicsItem *item = scene->selectedItems().first();
        theControlledObject = dynamic_cast<QObject*>(item);

    }
    if ( theControlledObject ){
        propertyEditor->setObject(theControlledObject);
    }
}

void MainWindow::itemMoved(QGraphicsItem *item, const QPointF &oldPosition)
{
    Q_UNUSED(item);
    if ( item ){
        QUndoCommand *moveCommand = new MoveCommand(item, oldPosition);
        undoStack->push(moveCommand);
    }else{
        QUndoCommand *moveCommand = new MoveCommand(scene, oldPosition);
        undoStack->push(moveCommand);
    }
}

void MainWindow::itemAdded(QGraphicsItem *item)
{
    QUndoCommand *addCommand = new AddCommand(item, scene);
    undoStack->push(addCommand);
}

void MainWindow::itemRotate(QGraphicsItem *item, const qreal oldAngle)
{
    QUndoCommand *rotateCommand = new RotateCommand(item , oldAngle);
    undoStack->push(rotateCommand);
}

void MainWindow::deleteItem()
{
    if (scene->selectedItems().isEmpty())
        return;

    QUndoCommand *deleteCommand = new DeleteCommand(scene);
    undoStack->push(deleteCommand);

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
    if ( sender() == rightAct )
    {
        scene->align(RIGHT_ALIGN);
    }else if ( sender() == leftAct){
        scene->align(LEFT_ALIGN);
    }else if ( sender() == upAct ){
        scene->align(UP_ALIGN);
    }else if ( sender() == downAct ){
        scene->align(DOWN_ALIGN);
    }else if ( sender() == vCenterAct ){
        scene->align(VERT_ALIGN);
    }else if ( sender() == hCenterAct)
    {
        scene->align(HORZ_ALIGN);
    }else if ( sender() == heightAct )
        scene->align(HEIGHT_ALIGN);
    else if ( sender()==widthAct )
        scene->align(WIDTH_ALIGN);
    else if ( sender() == horzAct )
        scene->align(HORZEVEN_ALIGN);
    else if ( sender() == vertAct )
        scene->align(VERTEVEN_ALIGN);
    else if ( sender () == allAct )
        scene->align(ALL_ALIGN);
}

void MainWindow::zoomIn()
{
    view->zoomIn();
}

void MainWindow::zoomOut()
{
    view->zoomOut();
}

void MainWindow::on_group_triggered()
{
    //QGraphicsItemGroup
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    // Create a new group at that level
    if ( selectedItems.count() < 2) return;
    GraphicsItemGroup *group = scene->createGroup(selectedItems);
    QUndoCommand *groupCommand = new GroupCommand(group,scene);
    undoStack->push(groupCommand);
}

void MainWindow::on_unGroup_triggered()
{
    QGraphicsItem *selectedItem = scene->selectedItems().first();
    GraphicsItemGroup * group = dynamic_cast<GraphicsItemGroup*>(selectedItem);
    if ( group ){
        QUndoCommand *unGroupCommand = new UnGroupCommand(group,scene);
        undoStack->push(unGroupCommand);
    }
}

void MainWindow::on_func_test_triggered()
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


        QtGradientEditor * editor = new QtGradientEditor(NULL);
        editor->show();


}

void MainWindow::on_copy()
{
    ShapeMimeData * data = new ShapeMimeData( scene->selectedItems() );
    QApplication::clipboard()->setMimeData(data);
}

void MainWindow::on_paste()
{
    QMimeData * mp = const_cast<QMimeData *>(QApplication::clipboard()->mimeData()) ;
    ShapeMimeData * data = dynamic_cast< ShapeMimeData*>( mp );
    if ( data ){
        scene->clearSelection();
        foreach (QGraphicsItem * item , data->items() ) {
            AbstractShape *sp = qgraphicsitem_cast<AbstractShape*>(item);
            QGraphicsItem * copy = sp->copy();
            if ( copy ){
                copy->setSelected(true);
                copy->moveBy(10,10);
                QUndoCommand *addCommand = new AddCommand(copy, scene);
                undoStack->push(addCommand);
            }
        }
    }
}

void MainWindow::on_cut()
{
    QList<QGraphicsItem *> copylist ;
    foreach (QGraphicsItem *item , scene->selectedItems()) {
        AbstractShape *sp = qgraphicsitem_cast<AbstractShape*>(item);
        QGraphicsItem * copy = sp->copy();
        if ( copy )
            copylist.append(copy);
    }
    QUndoCommand *deleteCommand = new DeleteCommand(scene);
    undoStack->push(deleteCommand);
    if ( copylist.count() > 0 ){
        ShapeMimeData * data = new ShapeMimeData( copylist );
        QApplication::clipboard()->setMimeData(data);
    }
}

void MainWindow::dataChanged()
{
    pasteAct->setEnabled(true);
}

void MainWindow::positionChanged(int x, int y)
{
   char buf[255];
   sprintf(buf,"%d,%d",x,y);
   statusBar()->showMessage(buf);
}
