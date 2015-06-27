#include "rulebar.h"
#include <QGraphicsView>

QtRuleBar::QtRuleBar(int type , QGraphicsView * view, QWidget *parent)
    :QFrame(parent),
    m_DocSize(QSize(3000,2000)),
    m_view(view),
    m_faceColor(0xE0, 0xFF, 0xE0)
{
    m_rulerType   = type;
    m_scrollPos   = QPoint(0,0);
    m_lastPos     = QPoint(0,0);
    m_fZoomFactor = 1;
    setFont(QFont("Times New Roman",10,10));
}

void QtRuleBar::updatePosition(int dx, int dy )
{
    m_scrollPos -= QPoint(dx,dy);
    update();
}

void QtRuleBar::updateRuler(const QRect &localRect, float fZoomFactor)
{

    m_DocSize = localRect.size();
    m_fZoomFactor = fZoomFactor;

}

void QtRuleBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect rulerRect = rect();

    if (m_rulerType==RT_HORIZONTAL)
    {
        rulerRect.setRight((int)(m_DocSize.width()*m_fZoomFactor));
    }
    else //(m_rulerType==RT_VERTICAL)
    {
        rulerRect.setBottom((int)(m_DocSize.height()*m_fZoomFactor));
    }

    painter.fillRect(rulerRect,m_faceColor);
    if ( m_rulerType == RT_HORIZONTAL ){
        painter.drawLine(rulerRect.bottomLeft(),rulerRect.bottomRight());
//        painter.translate(rulerRect.width()/2 , 0);
//        rulerRect.translate(-rulerRect.width()/2 , 0);
    }
    else{
        painter.drawLine(rulerRect.topRight(),rulerRect.bottomRight());
//        painter.translate(0,rulerRect.height()/2 );
//        rulerRect.translate(0,-rulerRect.height()/2 );
    }
    DrawTicker(&painter, rulerRect, 50);
    DrawTicker(&painter, rulerRect, 10, 10, false);
    DrawTicker(&painter, rulerRect, 5, 12, false);



}

void QtRuleBar::DrawTicker(QPainter *painter, const QRect &rulerRect, int nFactor, int nBegin, bool bShowPos)
{
    int nSize  = (m_rulerType == RT_HORIZONTAL)?rulerRect.width() : rulerRect.height();
    int nTick  = (int)(nFactor*m_fZoomFactor);

    for (int i=0;  i<= nSize/nFactor; i++)
    {
        QString text = QString(tr("%1")).arg(i*nFactor);

        if (m_rulerType==RT_HORIZONTAL)
        {
            QPoint pt1,pt2;
            pt1.setX(nTick*i-m_scrollPos.x());
            pt1.setY(rulerRect.top()+nBegin);
            pt2.setX(nTick*i-m_scrollPos.x());
            pt2.setY(rulerRect.bottom());
            painter->drawLine(pt1,pt2);
            if (bShowPos )
                painter->drawText(nTick*i- m_scrollPos.x() + 2,
                                  rulerRect.top(),
                                  nFactor,
                                  RULER_SIZE,
                                  Qt::AlignLeft|Qt::AlignTop,text);
        }
        else //(m_rulerType==RT_VERTICAL)
        {
            painter->drawLine(rulerRect.left()+nBegin,
                              nTick*i-m_scrollPos.y(),
                              rulerRect.right() ,
                              nTick*i-m_scrollPos.y());

            if (bShowPos){
                QFontMetrics fm(font());
                int w = fm.width(text);
                QRect textRect(0,0,nFactor,RULER_SIZE);
                painter->save();
                painter->translate(rulerRect.left() , nTick*(i+1)-m_scrollPos.y()+2);
                painter->rotate(-90);
                painter->drawText(textRect,Qt::AlignTop | Qt::AlignRight,text);
                painter->restore();
            }
        }
    }
}

void QtRuleBar::DrawCursorPos(QPoint NewPos)
{

}

QtCornerBox::QtCornerBox(QWidget *parent)
    :QFrame(parent)
{
    setFrameShape(QFrame::Box);
}

void QtCornerBox::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(),QColor(0xE0, 0xFF, 0xE0));
    painter.setPen(Qt::DashLine);
    painter.drawLine(rect().center().x(),rect().top(),rect().center().x(),rect().bottom());
    painter.drawLine(rect().left(),rect().center().y(),rect().right(),rect().center().y());
//    painter.drawLine(rect().topLeft(),rect().topRight());
//    painter.drawLine(rect().topLeft(),rect().bottomLeft());
    painter.drawLine(rect().topRight(),rect().bottomRight());
    painter.drawLine(rect().bottomLeft(),rect().bottomRight());

}
