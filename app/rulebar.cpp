#include "rulebar.h"
#include <QGraphicsView>

#define MINIMUM_INCR   5

typedef struct
{
  double ruler_scale[16];
  int    subdivide[5];
}SPRulerMetric;

// Ruler metric for general use.
static SPRulerMetric const ruler_metric_general = {
  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000 },
  { 1, 5, 10, 50, 100 }
};

// Ruler metric for inch scales.
static SPRulerMetric const ruler_metric_inches = {
  { 1, 2, 4,  8, 16, 32,  64, 128, 256,  512, 1024, 2048, 4096, 8192, 16384, 32768 },
  { 1, 2,  4,  8,  16 }
};


QtRuleBar::QtRuleBar(Qt::Orientation direction, QGraphicsView * view, QWidget *parent)
    :QWidget(parent),
    m_view(view),
    m_faceColor(0xFF, 0xFF, 0xFF)
{
    m_lower = m_upper = m_maxsize = 0;
    m_lastPos = QPoint(0,0);
    m_direction   = direction;
    QFont f(font());
    f.setBold(false);
    f.setPixelSize(10);
    setFont(f);
}

void QtRuleBar::setRange(double lower, double upper, double max_size)
{
    m_lower = lower;
    m_upper = upper;
    m_maxsize = max_size;
}

void QtRuleBar::updatePosition(const QPoint &pos)
{
    m_lastPos = pos;
    update();
}

void QtRuleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QRect rulerRect = rect();
    painter.fillRect(rulerRect,m_faceColor);

    if ( m_direction == Qt::Horizontal ){
        painter.drawLine(rulerRect.bottomLeft(),rulerRect.bottomRight());
    }
    else{
        painter.drawLine(rulerRect.topRight(),rulerRect.bottomRight());
    }

    drawTicker(&painter);
    drawPos(&painter);
}

void QtRuleBar::drawTicker(QPainter *painter)
{
    int             i;
    int             width, height;
    int             length, ideal_length;
    double          lower = m_lower , upper = m_upper; /* Upper and lower limits, in ruler units */
    double          increment;    /* Number of pixels per unit */
    uint            scale;        /* Number of units per major unit */
    double          start, end, cur;
    char            unit_str[32];
    char            digit_str[2] = { '\0', '\0' };
    int             digit_height;
    int             digit_offset;
    int             text_size;
    int             pos;
    double          max_size = m_maxsize;
    SPRulerMetric    ruler_metric = ruler_metric_general; /* The metric to use for this unit system */
    QRect allocation = this->rect();
    QFontMetrics fm(font());
    digit_height = fm.height() ;
    digit_offset = 0;
    if (m_direction==Qt::Horizontal){
        width = allocation.width();
        height = allocation.height();
    }else{
        width = allocation.height();
        height = allocation.width();
    }
    if ( (upper - lower) == 0 ) return ;
    increment = (double) width / (upper - lower);

    scale = ceil (max_size);
    sprintf (unit_str, "%d", scale );
    text_size = strlen (unit_str) * digit_height + 1;
    for (scale = 0; scale < sizeof (ruler_metric.ruler_scale) /
         sizeof(ruler_metric.ruler_scale[0]); scale++)
        if (ruler_metric.ruler_scale[scale] * fabs (increment) > 2 * text_size)
            break;
    if (scale == sizeof (ruler_metric.ruler_scale))
        scale = sizeof (ruler_metric.ruler_scale) - 1;
    length = 0;

    for (i = sizeof (ruler_metric.subdivide) /
         sizeof( ruler_metric.subdivide[0] ) - 1; i >= 0; i--){
        double subd_incr;
        if (scale == 1 && i == 1 )
            subd_incr = 1.0 ;
        else
            subd_incr = ((double)ruler_metric.ruler_scale[scale] /
                         (double)ruler_metric.subdivide[i]);
        if (subd_incr * fabs (increment) <= MINIMUM_INCR)
            continue;

        ideal_length = height / (i + 1) - 1;

        if (ideal_length > ++length)
            length = ideal_length;
        if (lower < upper){
            start = floor (lower / subd_incr) * subd_incr;
            end   = ceil  (upper / subd_incr) * subd_incr;
        }else
        {
            start = floor (upper / subd_incr) * subd_incr;
            end   = ceil  (lower / subd_incr) * subd_incr;
        }
        int tick_index = 0;
        for (cur = start; cur <= end; cur += subd_incr){
            pos = int(qRound((cur - lower) * increment + 1e-12));
            if (m_direction==Qt::Horizontal){
                QRect rt(pos,height-length,1,length);
                painter->drawLine(rt.topLeft(),rt.bottomLeft());
            }else{
                QRect rt(height-length,pos,length,1);
                painter->drawLine(rt.topLeft(),rt.topRight());
            }
            double label_spacing_px = fabs(increment*(double)ruler_metric.ruler_scale[scale]/ruler_metric.subdivide[i]);
            if (i == 0 &&
                    (label_spacing_px > 6*digit_height || tick_index%2 == 0 || cur == 0) &&
                    (label_spacing_px > 3*digit_height || tick_index%4 == 0 || cur == 0))
            {
                if (qAbs((int)cur) >= 2000 && (((int) cur)/1000)*1000 == ((int) cur))
                    sprintf (unit_str, "%dk", ((int) cur)/1000);
                else
                    sprintf (unit_str, "%d", (int) cur);
               if (m_direction==Qt::Horizontal){
                   int w = fm.width(unit_str);
                   painter->drawText(pos + 2,
                                     allocation.top(),
                                     w,
                                     RULER_SIZE,
                                     Qt::AlignLeft|Qt::AlignTop,unit_str);
               } else{
#if 0
                   int w = fm.width("u") + 2;
                   int start = cur < 0 ? 1 : 0 ;
                   if ( start == 1 ){
                       QRect textRect(-w/2,-digit_height/2,w,digit_height);
                       painter->save();
                       painter->translate(4, pos + w/2 + 2 );
                       painter->rotate(90);
                       painter->drawText(textRect,
                                         Qt::AlignRight,QString(unit_str[0]));
                       painter->restore();
                   }
                   for (int j = start; j < (int) strlen (unit_str); j++){
                       digit_str[0] = unit_str[j];
                       painter->drawText(1,
                                         pos + digit_height * j,
                                         w,
                                         digit_height,
                                         Qt::AlignLeft|Qt::AlignTop,QString(digit_str[0]));
                   }
#else
                   int w = fm.width(unit_str);
                   QRect textRect(-w/2,-RULER_SIZE/2,w,RULER_SIZE);
                   painter->save();
                   painter->translate(4, pos + w/2+2);
                   painter->rotate(90);
                   painter->drawText(textRect,Qt::AlignRight,unit_str);
                   painter->restore();
#endif
               }
            }
           tick_index++;
        }
    }
}

void QtRuleBar::drawPos(QPainter *painter)
{
   int  x, y;
   int  width, height;
   int  bs_width, bs_height;
   QRect allocation = this->rect();
   double position;
   double lower = m_lower;
   double upper = m_upper;
   if (m_direction==Qt::Horizontal){
       width = allocation.width();
       height = allocation.height();
       bs_width = height / 2 + 2 ;
       bs_width |= 1;  /* make sure it's odd */
       bs_height = bs_width / 2 + 1;
       position = lower + (upper - lower) * m_lastPos.x() / allocation.width();
   }else{
       width = allocation.height();
       height = allocation.width();
       bs_height = width / 2 + 2 ;
       bs_height |= 1;  /* make sure it's odd */
       bs_width = bs_height / 2 + 1;
       position = lower + (upper - lower) *  m_lastPos.y() / allocation.height() ;
   }
   if ((bs_width > 0) && (bs_height > 0)){
       double increment;
       if (m_direction==Qt::Horizontal){
           increment = (double) width / (upper - lower);
           x = qRound ((position - lower) * increment) + bs_width / 2 - 1;
           y = (height + bs_height) / 2 ;
           painter->drawLine(m_lastPos.x(),0, m_lastPos.x() , height);
       }else{
           increment = (double) height / (upper - lower);
           x = (width + bs_width) / 2 ;
           y = qRound ((position - lower) * increment) + (bs_height) / 2 - 1;
           painter->drawLine(0 , m_lastPos.y() , width , m_lastPos.y());
       }
   }
}

QtCornerBox::QtCornerBox(QWidget *parent)
    :QWidget(parent)
{
}

void QtCornerBox::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.fillRect(rect(),QColor(0xFF, 0xFF, 0xFF));

    painter.setPen(Qt::DashLine);
    painter.drawLine(rect().center().x(),rect().top(),rect().center().x(),rect().bottom());
    painter.drawLine(rect().left(),rect().center().y(),rect().right(),rect().center().y());

    painter.drawLine(rect().topRight(),rect().bottomRight());
    painter.drawLine(rect().bottomLeft(),rect().bottomRight());
}
