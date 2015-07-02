#include "drawtool.h"
#include "drawobj.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QtMath>
#include "drawobj.h"
#define PI 3.14

QList<DrawTool*> DrawTool::c_tools;
QPointF DrawTool::c_down;
QPointF DrawTool::c_last;
quint32 DrawTool::c_nDownFlags;

DrawShape DrawTool::c_drawShape = selection;

static SelectTool selectTool;
static RectTool   lineTool(line);
static RectTool   rectTool(rectangle);
static RectTool   roundRectTool(roundrect);
static RectTool   ellipseTool(ellipse);
static PolygonTool   arcTool(arc);
static PolygonTool polygonTool(polygon);
static PolygonTool bezierTool(bezier);

static RotationTool rotationTool;

enum SelectMode
{
    none,
    netSelect,
    move,
    size,
    rotate
};

SelectMode selectMode = none;

int nDragHandle = Handle_None;

static void setCursor(DrawScene * scene , const QCursor & cursor )
{
        QGraphicsView * view = scene->view();
        if (view)
            view->setCursor(cursor);
}

DrawTool::DrawTool(DrawShape shape)
{
    m_drawShape = shape ;
    m_hoverSizer = false;
    c_tools.push_back(this);
}

void DrawTool::mousePressEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    c_down = event->scenePos();
    c_last = event->scenePos();
}

void DrawTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    c_last = event->scenePos();
}

void DrawTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    if (event->scenePos() == c_down)
        c_drawShape = selection;
    setCursor(scene,Qt::ArrowCursor);
}

void DrawTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{

}

DrawTool *DrawTool::findTool(DrawShape drawShape)
{
    QList<DrawTool*>::const_iterator iter = c_tools.constBegin();
    for ( ; iter != c_tools.constEnd() ; ++iter ){
        if ((*iter)->m_drawShape == drawShape )
            return (*iter);
    }
    return 0;
}

SelectTool::SelectTool()
    :DrawTool(selection)
{
    m_lastSize.setHeight(0);
    m_lastSize.setWidth(0);
    dashRect = 0;
}

void SelectTool::mousePressEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mousePressEvent(event,scene);

    QPointF itemPoint;

    if (!m_hoverSizer)
      scene->mouseEvent(event);

    selectMode = none;
    QList<QGraphicsItem *> items = scene->selectedItems();
    AbstractBasicShape *item = 0;

    if ( items.count() == 1 )
        item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());

    if ( item != 0 ){

        nDragHandle = item->collidesWithHandle(event->scenePos());
        if ( nDragHandle != Handle_None)
             selectMode = size;
        else
            selectMode =  move;

        m_lastSize = item->boundingRect().size();
        itemPoint = item->mapFromScene(c_down);

        setCursor(scene,Qt::ClosedHandCursor);

    }else if ( items.count() > 1 )
        selectMode =  move;

    if( selectMode == none ){
        selectMode = netSelect;
        if ( scene->view() ){
            QGraphicsView * view = scene->view();
            view->setDragMode(QGraphicsView::RubberBandDrag);
        }

    }

    if ( selectMode == move && items.count() == 1 ){

        if (dashRect ){
            scene->removeItem(dashRect);
            delete dashRect;
            dashRect = 0;
        }

        dashRect = new QGraphicsPathItem(item->shape());
        dashRect->setPen(Qt::DashLine);
        dashRect->setPos(item->pos());
        dashRect->setTransformOriginPoint(item->transformOriginPoint());
        dashRect->setTransform(item->transform());
        dashRect->setRotation(item->rotation());
        dashRect->setScale(item->scale());
        initialPositions = item->pos();
        scene->addItem(dashRect);
    }
}

void SelectTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{

    DrawTool::mouseMoveEvent(event,scene);
    QList<QGraphicsItem *> items = scene->selectedItems();
    AbstractBasicShape * item = 0;
    if ( items.count() == 1 ){
        item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());
        if ( item != 0 ){
            if ( nDragHandle != Handle_None && selectMode == size ){
                QSizeF delta(c_last.x() - c_down.x() , c_last.y() - c_down.y());
                item->resize(nDragHandle,c_last);
            }
            else if(nDragHandle == Handle_None && selectMode == selection ){
                 int handle = item->collidesWithHandle(event->scenePos());
                 if ( handle != Handle_None){
                     setCursor(scene,Qt::OpenHandCursor/*item->getCursor(handle)*/);
                     m_hoverSizer = true;
                 }else{
                     setCursor(scene,Qt::ArrowCursor);
                     m_hoverSizer = false;
                 }
             }
        }
    }
    if ( selectMode == move ){
        if ( dashRect ){
            dashRect->setPos(initialPositions + c_last - c_down);
        }
    }
    if ( selectMode != size  && items.count() > 1)
    {
        scene->mouseEvent(event);

    }

}

void SelectTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mouseReleaseEvent(event,scene);

    QList<QGraphicsItem *> items = scene->selectedItems();
    if ( items.count() == 1 ){
        AbstractBasicShape * item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());
        if ( item != 0  && selectMode == move && c_last != c_down ){
             item->setPos(initialPositions + c_last - c_down);
             emit scene->itemMoved(item , c_last - c_down );
         }else if ( item !=0 && selectMode == size && c_last != c_down ){
            item->updateCoordinate();
        }
    }else if ( items.count() > 1 && selectMode == move && c_last != c_down )
        emit scene->itemMoved(NULL , c_last - c_down );


    if (selectMode == netSelect ){

        if ( scene->view() ){
            QGraphicsView * view = scene->view();
            view->setDragMode(QGraphicsView::NoDrag);
        }
    }

    if (dashRect ){
        scene->removeItem(dashRect);
        delete dashRect;
        dashRect = 0;
    }

    selectMode = none;
    nDragHandle = Handle_None;
    m_hoverSizer = false;
    scene->mouseEvent(event);

}

RotationTool::RotationTool()
    :DrawTool(rotation)
{
    lastAngle == 0;
    dashRect = 0;
}

void RotationTool::mousePressEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mousePressEvent(event,scene);


    if (!m_hoverSizer)
      scene->mouseEvent(event);

    QList<QGraphicsItem *> items = scene->selectedItems();
    if ( items.count() == 1 ){
        AbstractBasicShape * item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());
        if ( item != 0 ){
            nDragHandle = item->collidesWithHandle(event->scenePos());
            if ( nDragHandle !=Handle_None)
            {
                QPointF origin = item->mapToScene(item->boundingRect().center());

                qreal len_y = c_last.y() - origin.y();
                qreal len_x = c_last.x() - origin.x();

                qreal angle = atan2(len_y,len_x)*180/PI;

                lastAngle = angle;
                selectMode = rotate;

                if (dashRect ){
                    scene->removeItem(dashRect);
                    delete dashRect;
                    dashRect = 0;
                }

                dashRect = new QGraphicsPathItem(item->shape());
                dashRect->setPen(Qt::DashLine);
                dashRect->setPos(item->pos());
                dashRect->setTransformOriginPoint(item->transformOriginPoint());
                dashRect->setTransform(item->transform());
                dashRect->setRotation(item->rotation());
                dashRect->setScale(item->scale());
                scene->addItem(dashRect);

                setCursor(scene,QCursor((QPixmap(":/icons/rotate.png"))));
            }
            else{
                    c_drawShape = selection;
                    selectTool.mousePressEvent(event,scene);
                }
        }
    }
}

void RotationTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mouseMoveEvent(event,scene);

    QList<QGraphicsItem *> items = scene->selectedItems();
    if ( items.count() == 1 ){
        AbstractBasicShape * item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());
        if ( item != 0  && nDragHandle !=Handle_None && selectMode == rotate ){


             QPointF origin = item->mapToScene(item->boundingRect().center());

             qreal len_y = c_last.y() - origin.y();
             qreal len_x = c_last.x() - origin.x();

             qreal angle = atan2(len_y,len_x)*180/PI;

             angle = item->rotation() + int(angle - lastAngle) ;

             if ( angle > 360 )
                 angle -= 360;
             if ( angle < -360 )
                 angle+=360;

             if ( dashRect ){
                 dashRect->setRotation( angle );
             }

             setCursor(scene,QCursor((QPixmap(":/icons/rotate.png"))));
        }
        else if ( item )
        {
            int handle = item->collidesWithHandle(event->scenePos());
            if ( handle != Handle_None){
                setCursor(scene,QCursor((QPixmap(":/icons/rotate.png"))));
                m_hoverSizer = true;
            }else{
                setCursor(scene,Qt::ArrowCursor);
                m_hoverSizer = false;
            }
        }
    }
    scene->mouseEvent(event);
}

void RotationTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mouseReleaseEvent(event,scene);

    QList<QGraphicsItem *> items = scene->selectedItems();
    if ( items.count() == 1 ){
        AbstractBasicShape * item = qgraphicsitem_cast<AbstractBasicShape*>(items.first());
        if ( item != 0  && nDragHandle !=Handle_None && selectMode == rotate ){

             QPointF origin = item->mapToScene(item->boundingRect().center());

             QPointF delta = c_last - origin ;
             qreal len_y = c_last.y() - origin.y();
             qreal len_x = c_last.x() - origin.x();
             qreal angle = atan2(len_y,len_x)*180/PI,oldAngle = item->rotation();
             angle = item->rotation() + int(angle - lastAngle) ;

             if ( angle > 360 )
                 angle -= 360;
             if ( angle < -360 )
                 angle+=360;

             item->setRotation( angle );
             emit scene->itemRotate(item , oldAngle);
             qDebug()<<"rotate:"<<angle<<item->boundingRect();
        }
    }

    setCursor(scene,Qt::ArrowCursor);
    selectMode = none;
    nDragHandle = Handle_None;
    lastAngle = 0;
    m_hoverSizer = false;
    if (dashRect ){
        scene->removeItem(dashRect);
        delete dashRect;
        dashRect = 0;
    }
    scene->mouseEvent(event);

}

RectTool::RectTool(DrawShape drawShape)
    :DrawTool(drawShape)
{
    item = 0;
}

void RectTool::mousePressEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mousePressEvent(event,scene);

    scene->clearSelection();
    switch ( c_drawShape ){
    case rectangle:
        item = new GraphicsRectItem(QRect(0,0,0,0),NULL);
        break;
    case roundrect:
        item = new GraphicsRoundRectItem(QRect(0,0,0,0),NULL);
        break;
    case ellipse:
        item = new GraphicsEllipseItem(QRect(0,0,0,0),NULL);
        break;
    case line:
        item = new GraphicsLineItem(0);
        break;
    case arc:
        item = new GraphicsArcItem(0);
        break;
    }
    if ( item == 0) return;
    item->setPos(event->scenePos());
    scene->addItem(item);
    item->setSelected(true);

    selectMode = size;
    nDragHandle = RightBottom;

}

void RectTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    setCursor(scene,Qt::CrossCursor);

    selectTool.mouseMoveEvent(event,scene);
}

void RectTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    selectTool.mouseReleaseEvent(event,scene);

    if ( event->scenePos() == c_down ){

       if ( item != 0){
         item->setSelected(false);
         scene->removeItem(item);
         delete item ;
         item = 0;
       }
       qDebug()<<"RectTool removeItem:";
    }else if( item ){
        emit scene->itemAdded( item );
    }

    c_drawShape = selection;
}


PolygonTool::PolygonTool(DrawShape shape)
    :DrawTool(shape)
{
    item = NULL;
    m_nPoints = 0;
}

void PolygonTool::mousePressEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mousePressEvent(event,scene);
    if ( item == NULL ){
        if ( c_drawShape == polygon ){
        item = new GraphicsPolygonItem(NULL);
        }else if (c_drawShape == bezier ){
            item = new GraphicsBezierCurve(NULL);
        }else {
            item = new GraphicsArcItem(NULL);
        }
        item->setPos(event->scenePos());
        scene->addItem(item);
        initialPositions = c_down;
        item->addPoint(c_down);
        item->setSelected(true);
        m_nPoints++;

    }else if ( c_down == c_last ){
        /*
        if ( item != NULL )
        {
            scene->removeItem(item);
            delete item;
            item = NULL ;
            c_drawShape = selection;
            selectMode = none;
            return ;
        }
        */
    }
    item->addPoint(c_down+QPoint(1,0));
    m_nPoints++;
    selectMode = size ;
    nDragHandle = item->handleCount();
}

void PolygonTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mouseMoveEvent(event,scene);
    setCursor(scene,Qt::CrossCursor);
    selectTool.mouseMoveEvent(event,scene);
}

void PolygonTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    if ( c_drawShape == bezier && m_nPoints == 4 ){
        item->updateCoordinate();
        emit scene->itemAdded( item );
        item = NULL;
        selectMode = none;
        c_drawShape = selection;
        m_nPoints = 0;
    }else if (c_drawShape == arc && m_nPoints == 4 ){
        item->updateCoordinate();
        item->endPoint(event->scenePos());
        emit scene->itemAdded( item );
        item = NULL;
        selectMode = none;
        c_drawShape = selection;
        m_nPoints = 0;
    }

    DrawTool::mousePressEvent(event,scene);
}

void PolygonTool::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event, DrawScene *scene)
{
    DrawTool::mouseDoubleClickEvent(event,scene);
    item->endPoint(event->scenePos());
    item->updateCoordinate();
    emit scene->itemAdded( item );
    item = NULL;
    selectMode = none;
    c_drawShape = selection;
    m_nPoints = 0;
}
