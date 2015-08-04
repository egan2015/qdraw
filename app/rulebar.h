#ifndef RULEBAR
#define RULEBAR

#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QFrame;
class QPaintEvent;
class QPainter;
class QGraphicsView;
QT_END_NAMESPACE

#define RT_VERTICAL	  0
#define RT_HORIZONTAL 1

// hint information
#define RW_VSCROLL	  1
#define RW_HSCROLL	  2
#define RW_POSITION   3
#define RULER_SIZE    16

typedef struct {
    int   uMessage;
    QPoint ScrollPos;
    QPoint Pos;
    QSize  DocSize;
    float  fZoomFactor;
}RULER_INFO;

class QtCornerBox : public QWidget
{
    Q_OBJECT
public:
    explicit QtCornerBox(QWidget * parent );
protected:
    void paintEvent(QPaintEvent *);
};

class QtRuleBar : public QWidget
{
    Q_OBJECT
public:
    explicit QtRuleBar(Qt::Orientation direction, QGraphicsView * view, QWidget * parent = 0  );
    void setRange( double lower , double upper , double max_size );
    void updatePosition( const QPoint & pos );
protected:
    void paintEvent(QPaintEvent *event);
    void drawTicker(QPainter * painter);
    void drawPos(QPainter * painter) ;
    Qt::Orientation   m_direction;
    QPoint m_lastPos;
    QColor m_faceColor;
    QGraphicsView * m_view;

    double m_lower;
    double m_upper;
    double m_maxsize;
};

#endif // RULEBAR

