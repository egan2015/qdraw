#include "drawscene.h"
#include<QGraphicsSceneMouseEvent>
#include<QGraphicsRectItem>
#include <QDebug>
#include <QKeyEvent>
#include "drawobj.h"
#include <vector>
#include <QPainter>


GridTool::GridTool(const QSize & grid , const QSize & space )
    :m_sizeGrid(grid)
    ,m_sizeGridSpace(20,20)
{
}


void GridTool::paintGrid(QPainter *painter, const QRect &rect)
{
    QColor c(Qt::darkCyan);
    QPen p(c);
    p.setStyle(Qt::DashLine);
    p.setWidthF(0.2);
    painter->setPen(p);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing,false);

    painter->fillRect(rect,Qt::white);
    for (int x=rect.left() ;x <rect.right()  ;x+=(int)(m_sizeGridSpace.width())) {
        painter->drawLine(x,rect.top(),x,rect.bottom());

    }
    for (int y=rect.top();y<rect.bottom() ;y+=(int)(m_sizeGridSpace.height()))
    {
        painter->drawLine(rect.left(),y,rect.right(),y);
    }
    p.setStyle(Qt::SolidLine);
    p.setColor(Qt::black);
    painter->drawLine(rect.right(),rect.top(),rect.right(),rect.bottom());
    painter->drawLine(rect.left(),rect.bottom(),rect.right(),rect.bottom());

    //draw shadow
//    QColor c1(Qt::black);
//    painter->fillRect(QRect(rect.right()+1,rect.top()+2,2,rect.height()),c1.dark(200));
//    painter->fillRect(QRect(rect.left()+2,rect.bottom()+2,rect.width(),2),c1.dark(200));

    painter->restore();
}

class BBoxSort
{
public:
    BBoxSort( QGraphicsItem * item , const QRectF & rect , AlignType alignType )
        :item_(item),box(rect),align(alignType)
    {
        //topLeft
        min_ = alignType == HORZEVEN_ALIGN ? box.topLeft().x() : box.topLeft().y();
        //bottomRight
        max_ = alignType == HORZEVEN_ALIGN ? box.bottomRight().x() : box.bottomRight().y();
        //width or height
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

DrawScene::DrawScene(QObject *parent)
    :QGraphicsScene(parent)
{
    m_view = NULL;
    m_dx=m_dy=0;
    m_grid = new GridTool();
    QGraphicsItem * item = addRect(QRectF(0,0,0,0));
    item->setAcceptHoverEvents(true);

}

DrawScene::~DrawScene()
{
    delete m_grid;
}

void DrawScene::align(AlignType alignType)
{
    AbstractShape * firstItem = qgraphicsitem_cast<AbstractShape*>(selectedItems().first());
    if ( !firstItem ) return;
    QRectF rectref = firstItem->mapRectToScene(firstItem->boundingRect());
    int nLeft, nRight, nTop, nBottom;
    qreal width = firstItem->width();
    qreal height = firstItem->height();

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
                 emit itemMoved(it->item_,t);
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
           AbstractShape * aitem = qgraphicsitem_cast<AbstractShape*>(item);
           if ( aitem ){
               qreal fx = width / aitem->width();
               qreal fy = height / aitem->height();
               if ( fx == 1.0 && fy == 1.0 ) break;
               aitem->stretch(RightBottom,fx,fy,aitem->opposite(RightBottom));
               aitem->updateCoordinate();
               emit itemResize(aitem,RightBottom,QPointF(fx,fy));
           }
       }
           break;
        case WIDTH_ALIGN:
       {
            AbstractShape * aitem = qgraphicsitem_cast<AbstractShape*>(item);
            if ( aitem ){
                qreal fx = width / aitem->width();
                if ( fx == 1.0 ) break;
                aitem->stretch(Right,fx,1,aitem->opposite(Right));
                aitem->updateCoordinate();
                emit itemResize(aitem,Right,QPointF(fx,1));
            }
       }
            break;

        case HEIGHT_ALIGN:
       {
            AbstractShape * aitem = qgraphicsitem_cast<AbstractShape*>(item);
            if ( aitem ){

                qreal fy = height / aitem->height();
                if (fy == 1.0 ) break ;
                aitem->stretch(Bottom,1,fy,aitem->opposite(Bottom));
                aitem->updateCoordinate();
                emit itemResize(aitem,Bottom,QPointF(1,fy));
            }
       }
            break;
        }
        QPointF ptLast= rectItem.center();
        QPointF ptMove = ptNew - ptLast;
        if ( !ptMove.isNull()){
            item->moveBy(ptMove.x(),ptMove.y());
            emit itemMoved(item,ptMove);
        }
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

GraphicsItemGroup *DrawScene::createGroup(const QList<QGraphicsItem *> &items,bool isAdd)
{
    // Build a list of the first item's ancestors
    QList<QGraphicsItem *> ancestors;
    int n = 0;
    QPointF pt = items.first()->pos();
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
    if (!commonAncestor && isAdd )
        addItem(group);
    foreach (QGraphicsItem *item, items){
        item->setSelected(false);
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
             group->addToGroup(item);
    }
    group->updateCoordinate();
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

void DrawScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter,rect);
    painter->fillRect(sceneRect(),Qt::white);
    if( m_grid ){
        m_grid->paintGrid(painter,sceneRect().toRect());
    }
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

