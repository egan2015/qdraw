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

class QtCornerBox : public QFrame
{
    Q_OBJECT
public:
    explicit QtCornerBox(QWidget * parent );
protected:
    void paintEvent(QPaintEvent *);
};

class QtRuleBar : public QFrame
{
    Q_OBJECT
public:
    explicit QtRuleBar(int type , QGraphicsView * view, QWidget * parent = 0  );

    void updatePosition(int dx, int dy );
    void updateRuler( const QRect & localRect, float fZoomFactor);
protected:
    void paintEvent(QPaintEvent *event);

    void DrawTicker(QPainter* painter, const QRect &rulerRect, int nFactor, int nBegin = 0, bool bShowPos = true);
    void DrawCursorPos(QPoint NewPos);
    int   m_rulerType;
    QPoint m_scrollPos;
    QPoint m_lastPos;
    QSize  m_DocSize;
    qreal  m_fZoomFactor;
    QColor m_faceColor;
    QGraphicsView * m_view;
};

#endif // RULEBAR

