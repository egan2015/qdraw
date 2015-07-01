#ifndef DRAWSCENE
#define DRAWSCENE

#include <QGraphicsScene>
#include "drawtool.h"
#include "drawobj.h"

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
class QKeyEvent;
QT_END_NAMESPACE


enum AlignType
{
    UP_ALIGN=0,
    HORZ_ALIGN,
    VERT_ALIGN,
    DOWN_ALIGN,
    LEFT_ALIGN,
    RIGHT_ALIGN,
    CENTER_ALIGN,
    HORZEVEN_ALIGN,
    VERTEVEN_ALIGN,
    WIDTH_ALIGN,
    HEIGHT_ALIGN,
    ALL_ALIGN
};

class GraphicsItemGroup;

class DrawScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DrawScene(QObject *parent = 0);
    void setView(QGraphicsView * view ) { m_view = view ; }
    QGraphicsView * view() { return m_view; }
    void align(AlignType alignType );
    void mouseEvent(QGraphicsSceneMouseEvent *mouseEvent );
    GraphicsItemGroup * createGroup(const QList<QGraphicsItem *> &items);
    void destroyGroup(QGraphicsItemGroup *group);
signals:
    void itemMoved( QGraphicsItem * item , const QPointF & oldPosition );
    void itemRotate(QGraphicsItem * item , const qreal oldAngle );
    void itemAdded(QGraphicsItem * item );

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvet) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    QGraphicsView * m_view;

    qreal m_dx;
    qreal m_dy;
    bool  m_moved;
};

#endif // DRAWSCENE

