#include "sizehandle.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <qdebug.h>
#include <QtWidgets>

SizeHandleRect::SizeHandleRect(QGraphicsItem* parent , int d, QGraphicsItem *resizable)
    :QGraphicsRectItem(-SELECTION_HANDLE_SIZE/2,
                       -SELECTION_HANDLE_SIZE/2,
                       SELECTION_HANDLE_SIZE,
                       SELECTION_HANDLE_SIZE,parent)
    ,m_dir(d)
    ,m_resizable(resizable)
    ,m_state(SelectionHandleOff)
    ,borderColor("white")
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
    hide();
}


void SizeHandleRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setPen(Qt::SolidLine);
    painter->setBrush(QBrush(borderColor));

    painter->setRenderHint(QPainter::Antialiasing,false);
    if ( m_dir > Left )
    {
        painter->setBrush(Qt::darkBlue);
        //painter->drawEllipse(rect());
    }
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
    m_state = st;
}

void SizeHandleRect::move(qreal x, qreal y)
{   
    setPos(x,y);
}






