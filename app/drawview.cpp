#include "drawview.h"

DrawView::DrawView(QGraphicsScene *scene)
    :QGraphicsView(scene)
{
    m_hRuleBar = new QtRuleBar(RT_HORIZONTAL,this,this);
    m_vRuleBar = new QtRuleBar(RT_VERTICAL,this,this);
    box = new QtCornerBox(this);
    m_grid = new GridTool();
    setViewport(new QWidget);
}

void DrawView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter,rect);
    painter->save();
    painter->resetTransform();
    if( m_grid ){
        QRectF rc = transform().mapRect(sceneRect());
        m_grid->paintGrid(painter,rc.toRect());
    }
    painter->restore();
}

void DrawView::resizeEvent(QResizeEvent *event)
{
    this->setViewportMargins(RULER_SIZE,RULER_SIZE,0,0);
    m_hRuleBar->resize(this->size().width(),RULER_SIZE);
    m_hRuleBar->move(RULER_SIZE,0);
    m_vRuleBar->resize(RULER_SIZE,this->size().height());
    m_vRuleBar->move(0,RULER_SIZE);
    box->resize(RULER_SIZE,RULER_SIZE);
    box->move(0,0);
    QGraphicsView::resizeEvent(event);
}

void DrawView::scrollContentsBy(int dx, int dy)
{
    m_hRuleBar->updatePosition(dx,dy);
    m_vRuleBar->updatePosition(dx,dy);
    QGraphicsView::scrollContentsBy(dx,dy);
}

