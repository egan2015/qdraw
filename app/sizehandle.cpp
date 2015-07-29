#include "sizehandle.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <qdebug.h>
#include <QtWidgets>

SizeHandleRect::SizeHandleRect(QGraphicsItem* parent , int d, bool control)
    :QGraphicsRectItem(-SELECTION_HANDLE_SIZE/2,
                       -SELECTION_HANDLE_SIZE/2,
                       SELECTION_HANDLE_SIZE,
                       SELECTION_HANDLE_SIZE,parent)
    ,m_dir(d)
    ,m_controlPoint(control)
    ,m_state(SelectionHandleOff)
    ,borderColor("white")
{
    this->setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
    hide();
}


void SizeHandleRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setPen(Qt::SolidLine);
    painter->setBrush(QBrush(borderColor));

    painter->setRenderHint(QPainter::Antialiasing,false);

    if ( m_controlPoint  )
    {
        painter->setPen(QPen(Qt::red,Qt::SolidLine));
        painter->setBrush(Qt::green);
        painter->drawEllipse(rect().center(),3,3);
    }else
        painter->drawRect(rect());
    painter->restore();
}


void SizeHandleRect::setState(SelectionHandleState st)
{
    if (st == m_state)
        return;
    switch (st) {
    case SelectionHandleOff:
        hide();
        break;
    case SelectionHandleInactive:
    case SelectionHandleActive:
        show();
        break;
    }
    borderColor = Qt::white;
    m_state = st;
}

void SizeHandleRect::move(qreal x, qreal y)
{   
    setPos(x,y);
}

void SizeHandleRect::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    borderColor = Qt::blue;
    update();
    QGraphicsRectItem::hoverEnterEvent(e);
}

void SizeHandleRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    borderColor = Qt::white;
    update();
    QGraphicsRectItem::hoverLeaveEvent(e);
}







