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
class QListWidget;
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

    bool openFile(const QString &fileName);

public slots:

    void newFile();
    void open();
    void save();
    DrawView *createMdiChild();
    void updateMenus();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);

    void addShape();
    void updateActions();
    void deleteItem();
    void itemSelected();
    void itemMoved(QGraphicsItem * item , const QPointF & oldPosition );
    void itemAdded(QGraphicsItem * item );
    void itemRotate(QGraphicsItem * item , const qreal oldAngle );
    void itemResize(QGraphicsItem * item , int handle , const QPointF& scale );
    void itemControl(QGraphicsItem * item , int handle , const QPointF & newPos , const QPointF& lastPos_ );

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

    void about();
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    void createActions();
    void createMenus();
    void createToolbars();
    void createPropertyEditor();
    void createToolBox();

    DrawView *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);


    QMenu *windowMenu;

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

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
    // file
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;

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
    QAction  * rotateAct;

    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    //property editor
    QDockWidget *dockProperty;
    ObjectController *propertyEditor;
    QObject *theControlledObject;

    QListWidget    *listView;

    QUndoStack *undoStack;
    QUndoView *undoView;
    // statusbar label
    QLabel *m_posInfo;
};

#endif // MAINWINDOW_H
