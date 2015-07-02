#include "drawscene.h"
#include<QGraphicsSceneMouseEvent>
#include<QGraphicsRectItem>
#include <QDebug>
#include <QKeyEvent>
#include "drawobj.h"
#include <vector>


DrawScene::DrawScene(QObject *parent)
    :QGraphicsScene(parent)
{
    m_view = NULL;
    m_dx=m_dy=0;
}

class BBoxSort
{
public:
    BBoxSort( QGraphicsItem * item , const QRectF & rect , AlignType alignType )
        :item_(item),box(rect),align(alignType)
    {
        min_ = alignType == HORZEVEN_ALIGN ? box.topLeft().x() : box.topLeft().y();
        max_ = alignType == HORZEVEN_ALIGN ? box.bottomRight().x() : box.bottomRight().y();
        extent_ = alignType == HORZEVEN_ALIGN ? box.width() : box.height();
        anchor =  min_*0.5 + max_ * 0.5;
    }
    qreal min() { return min_;}
    qreal max() { return max_;}
    qreal extent() { return extent_;}
    QGraphicsItem * item_;
    qreal anchor;
    qreal min_;
    qreal max_;
    qreal extent_;
    QRectF box;
    AlignType align ;
};

bool operator< (const BBoxSort &a, const BBoxSort &b)
{
    return (a.anchor < b.anchor);
}


void DrawScene::align(AlignType alignType)
{
    QGraphicsItem * firstItem = selectedItems().first();
    QRectF rectref = firstItem->mapRectToScene(firstItem->boundingRect());
    int nLeft, nRight, nTop, nBottom;
    nLeft=nRight=rectref.center().x();
    nTop=nBottom=rectref.center().y();
    QPointF pt = rectref.center();
    if ( alignType == HORZEVEN_ALIGN || alignType == VERTEVEN_ALIGN ){
        std::vector< BBoxSort  > sorted;
        foreach (QGraphicsItem *item , selectedItems()) {
            QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
            if ( g )
                continue;
            sorted.push_back(BBoxSort(item,item->mapRectToScene(item->boundingRect()),alignType));
        }
        //sort bbox by anchors
        std::sort(sorted.begin(), sorted.end());

        unsigned int len = sorted.size();
        bool changed = false;
        //overall bboxes span
        float dist = (sorted.back().max()-sorted.front().min());
        //space eaten by bboxes
        float span = 0;
        for (unsigned int i = 0; i < len; i++)
        {
            span += sorted[i].extent();
        }
        //new distance between each bbox
        float step = (dist - span) / (len - 1);
        float pos = sorted.front().min();
        for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
              it < sorted.end();
              ++it )
        {
            {
                QPointF t;
                if ( alignType == HORZEVEN_ALIGN )
                    t.setX( pos - it->min() );
                else
                    t.setY(pos - it->min());
                it->item_->moveBy(t.x(),t.y());
                changed = true;
            }
            pos += it->extent();
            pos += step;
        }

        return;
    }

    int i = 0;
    foreach (QGraphicsItem *item , selectedItems()) {
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( g )
            continue;
        QRectF rectItem = item->mapRectToScene( item->boundingRect() );
        QPointF ptNew = rectItem.center();
       switch ( alignType ){
        case UP_ALIGN:
            ptNew.setY(nTop + (rectItem.height()-rectref.height())/2);
            break;
        case HORZ_ALIGN:
            ptNew.setY(pt.y());
            break;
        case VERT_ALIGN:
            ptNew.setX(pt.x());
            break;
        case DOWN_ALIGN:
            ptNew.setY(nBottom-(rectItem.height()-rectref.height())/2);
            break;
        case LEFT_ALIGN:
            ptNew.setX(nLeft-(rectref.width()-rectItem.width())/2);
            break;
        case RIGHT_ALIGN:
            ptNew.setX(nRight+(rectref.width()-rectItem.width())/2);
            break;
        case CENTER_ALIGN:
            ptNew=pt;
            break;
       case ALL_ALIGN:
       {
           AbstractBasicShape * aitem = qgraphicsitem_cast<AbstractBasicShape*>(item);
           if ( aitem ){
               aitem->setWidth(rectref.width());
               aitem->setHeight(rectref.height());
           }
       }
           break;
        case WIDTH_ALIGN:
       {
            AbstractBasicShape * aitem = qgraphicsitem_cast<AbstractBasicShape*>(item);
            if ( aitem )
                aitem->setWidth(rectref.width());
       }
            break;

        case HEIGHT_ALIGN:
       {
            AbstractBasicShape * aitem = qgraphicsitem_cast<AbstractBasicShape*>(item);
            if ( aitem )
                aitem->setHeight(rectref.height());
       }
            break;
        }
        QPointF ptLast= rectItem.center();
        QPointF ptMove = ptNew - ptLast;
        item->moveBy(ptMove.x(),ptMove.y());
        i++;
    }

}

void DrawScene::mouseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch( mouseEvent->type() ){
    case QEvent::GraphicsSceneMousePress:
        QGraphicsScene::mousePressEvent(mouseEvent);
        break;
    case QEvent::GraphicsSceneMouseMove:
        QGraphicsScene::mouseMoveEvent(mouseEvent);
        break;
    case QEvent::GraphicsSceneMouseRelease:
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        break;
    }
}

GraphicsItemGroup *DrawScene::createGroup(const QList<QGraphicsItem *> &items)
{
    // Build a list of the first item's ancestors
    QList<QGraphicsItem *> ancestors;
    int n = 0;
    if (!items.isEmpty()) {
        QGraphicsItem *parent = items.at(n++);
        while ((parent = parent->parentItem()))
            ancestors.append(parent);
    }

    // Find the common ancestor for all items
    QGraphicsItem *commonAncestor = 0;
    if (!ancestors.isEmpty()) {
        while (n < items.size()) {
            int commonIndex = -1;
            QGraphicsItem *parent = items.at(n++);
            do {
                int index = ancestors.indexOf(parent, qMax(0, commonIndex));
                if (index != -1) {
                    commonIndex = index;
                    break;
                }
            } while ((parent = parent->parentItem()));

            if (commonIndex == -1) {
                commonAncestor = 0;
                break;
            }

            commonAncestor = ancestors.at(commonIndex);
        }
    }

    // Create a new group at that level
    GraphicsItemGroup *group = new GraphicsItemGroup(commonAncestor);
    if (!commonAncestor)
        addItem(group);
    foreach (QGraphicsItem *item, items){
        item->setSelected(false);
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
             group->addToGroup(item);
    }
    return group;
}

void DrawScene::destroyGroup(QGraphicsItemGroup *group)
{
    group->setSelected(false);
    foreach (QGraphicsItem *item, group->childItems()){
        item->setSelected(true);
        group->removeFromGroup(item);
    }
    removeItem(group);
    delete group;
}

void DrawScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

    DrawTool * tool = DrawTool::findTool( DrawTool::c_drawShape );
    if ( tool )
        tool->mousePressEvent(mouseEvent,this);
}

void DrawScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    DrawTool * tool = DrawTool::findTool( DrawTool::c_drawShape );
    if ( tool )
        tool->mouseMoveEvent(mouseEvent,this);
}

void DrawScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    DrawTool * tool = DrawTool::findTool( DrawTool::c_drawShape );
    if ( tool )
        tool->mouseReleaseEvent(mouseEvent,this);
}

void DrawScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvet)
{
    DrawTool * tool = DrawTool::findTool( DrawTool::c_drawShape );
    if ( tool )
        tool->mouseDoubleClickEvent(mouseEvet,this);

}

void DrawScene::keyPressEvent(QKeyEvent *e)
{
    qreal dx=0,dy=0;
    m_moved = false;
    switch( e->key())
    {
    case Qt::Key_Up:
        dx = 0;
        dy = -1;
        m_moved = true;
        break;
    case Qt::Key_Down:
        dx = 0;
        dy = 1;
        m_moved = true;
        break;
    case Qt::Key_Left:
        dx = -1;
        dy = 0;
        m_moved = true;
        break;
    case Qt::Key_Right:
        dx = 1;
        dy = 0;
        m_moved = true;
        break;
    }
    m_dx += dx;
    m_dy += dy;
    if ( m_moved )
    foreach (QGraphicsItem *item, selectedItems()) {
       item->moveBy(dx,dy);
    }
    QGraphicsScene::keyPressEvent(e);
}

void DrawScene::keyReleaseEvent(QKeyEvent *e)
{
    if (m_moved && selectedItems().count()>0)
    emit itemMoved(NULL,QPointF(m_dx,m_dy));
    m_dx=m_dy=0;
    QGraphicsScene::keyReleaseEvent(e);
}

