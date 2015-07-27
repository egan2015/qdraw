#ifndef SIZEHANDLE
#define SIZEHANDLE

#include <QGraphicsRectItem>
#include <QList>

QT_BEGIN_NAMESPACE
class QColor;
class QFocusEvent;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneHoverEvent;
QT_END_NAMESPACE


enum { SELECTION_HANDLE_SIZE = 6, SELECTION_MARGIN = 10 };
enum SelectionHandleState { SelectionHandleOff, SelectionHandleInactive, SelectionHandleActive };
enum { Handle_None = 0 , LeftTop , Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left };

class SizeHandleRect :public QGraphicsRectItem
{
public:

    SizeHandleRect(QGraphicsItem* parent , int d , bool control = false );
    int dir() const  { return m_dir; }
    void setState(SelectionHandleState st);
    void move(qreal x, qreal y );
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e );
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const int m_dir;
    bool   m_controlPoint;
    SelectionHandleState m_state;
    QColor borderColor;
};


#endif // SIZEHANDLE

