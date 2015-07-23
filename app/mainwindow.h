#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "drawscene.h"
#include "objectcontroller.h"
#include "drawview.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
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
class QListView;
class QStatusBar;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
class QUndoStack;
class QUndoView;
QT_END_NAMESPACE

class QtVariantProperty;
class QtProperty;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void addShape();
    void updateActions();
    void deleteItem();
    void itemSelected();
    void itemMoved(QGraphicsItem * item , const QPointF & oldPosition );
    void itemAdded(QGraphicsItem * item );
    void itemRotate(QGraphicsItem * item , const qreal oldAngle );
    void on_actionBringToFront_triggered();
    void on_actionSendToBack_triggered();
    void on_aglin_triggered();
    void zoomIn();
    void zoomOut();
    void on_group_triggered();
    void on_unGroup_triggered();
    void on_func_test_triggered();
    void on_copy();
    void on_paste();
    void on_cut();
    void dataChanged();
    void positionChanged(int x, int y );
private:
    void createActions();
    void createMenus();
    void createToolbars();
    void createPropertyEditor();
    void createToolBox();


    QMdiArea *mdiArea;
    // update ui
    QTimer      m_timer;
    // toolbox
    QToolBox *toolBox;
    // edit toolbar;
    QToolBar * editToolBar;
    // align toolbar
    QToolBar * alignToolBar;
    QAction  * rightAct;
    QAction  * leftAct;
    QAction  * vCenterAct;
    QAction  * hCenterAct;
    QAction  * upAct;
    QAction  * downAct;
    QAction  * horzAct;
    QAction  * vertAct;
    QAction  * heightAct;
    QAction  * widthAct;
    QAction  * allAct;
    QAction  * bringToFrontAct;
    QAction  * sendToBackAct;

    QAction  * funcAct;

    QAction  * groupAct;
    QAction  * unGroupAct;

    // edit action
    QAction  * deleteAct;
    QAction  * undoAct;
    QAction  * redoAct;
    QAction  * copyAct;
    QAction  * pasteAct;
    QAction  * cutAct;
    QAction  * zoomInAct;
    QAction  * zoomOutAct;

    // drawing toolbar
    QToolBar * drawToolBar;
    QActionGroup * drawActionGroup;
    QAction  * selectAct;
    QAction  * lineAct;
    QAction  * rectAct;
    QAction  * roundRectAct;
    QAction  * ellipseAct;
    QAction  * polygonAct;
    QAction  * polylineAct;
    QAction  * bezierAct;
    QAction  * arcAct;
    QAction  * rotateAct;

    //property editor
    ObjectController *propertyEditor;
    QObject *theControlledObject;

    DrawView  *view;
    DrawScene      *scene;
    QListView      *listView;

    QUndoStack *undoStack;
    QUndoView *undoView;
    // statusbar label
    QLabel *m_posInfo;
};

#endif // MAINWINDOW_H
