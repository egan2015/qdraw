#include "drawview.h"

DrawView::DrawView(QGraphicsScene *scene)
    :QGraphicsView(scene)
{
    m_scrollPos   = QPoint(0,0);
    m_hruler = new QtRuleBar(RT_HORIZONTAL,this,this);
    m_vruler = new QtRuleBar(RT_VERTICAL,this,this);
    box = new QtCornerBox(this);
    setViewport(new QWidget);
    m_zoomFactor = 1;
}

void DrawView::zoomIn()
{
    scale(1.2,1.2);
    m_zoomFactor+=1.2;
    updateRuler();
}

void DrawView::zoomOut()
{
    scale(1 / 1.2, 1 / 1.2);
    m_zoomFactor-=1.2;
    updateRuler();
}

void DrawView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pt = mapToScene(event->pos());
    m_hruler->updatePosition(event->pos());
    m_vruler->updatePosition(event->pos());
    emit positionChanged( pt.x() , pt.y() );
    QGraphicsView::mouseMoveEvent(event);
}

void DrawView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    this->setViewportMargins(RULER_SIZE-1,RULER_SIZE-1,0,0);
    m_hruler->resize(this->size().width(),RULER_SIZE);
    m_hruler->move(RULER_SIZE,0);
    m_vruler->resize(RULER_SIZE,this->size().height());
    m_vruler->move(0,RULER_SIZE);

    box->resize(RULER_SIZE,RULER_SIZE);
    box->move(0,0);
    updateRuler();
}

void DrawView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx,dy);
    updateRuler();
}

void DrawView::updateRuler()
{
    QRectF viewbox = this->rect();
    QPointF offset = mapFromScene(scene()->sceneRect().topLeft());
    double factor =  1./transform().m11();
    double lower_x = factor * ( viewbox.left()  - (m_scrollPos.x() + offset.x()) );
    double upper_x = factor * ( viewbox.right() - (m_scrollPos.x() + offset.x()) );

    double lower_y = factor * ( viewbox.bottom()  -   (m_scrollPos.y() + RULER_SIZE + offset.y()+1));
    double upper_y = factor * ( viewbox.top() - (m_scrollPos.y() + RULER_SIZE + offset.y()+1));

    m_hruler->setRange(lower_x,upper_x,upper_x - lower_x );
    m_vruler->setRange(lower_y,upper_y,upper_y - lower_y );
    m_hruler->update();
    m_vruler->update();    

}

