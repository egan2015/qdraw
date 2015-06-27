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

class SizeHandleRect :public QGraphicsRectItem
{
public:
    enum Direction { None = -1 , LeftTop , Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left , Extend };

    SizeHandleRect(QGraphicsItem* parent , Direction d, QGraphicsItem *resizable);

    Direction dir() const  { return m_dir; }
    void updateCursor();
    void setState(SelectionHandleState st);
    bool hitTest( const QPointF & point );
    void move(qreal x, qreal y );

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const Direction m_dir;
    QGraphicsItem *m_resizable;
    SelectionHandleState m_state;
    QColor borderColor;
};


#endif // SIZEHANDLE

