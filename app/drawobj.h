#ifndef DRAWOBJ
#define DRAWOBJ

#include <qgraphicsitem.h>
#include <QtCore/QObject>
#include <QGraphicsSceneMouseEvent>
#include "sizehandle.h"
#include <QVector>
#include <QGraphicsScene>
#include <QList>
#include <QCursor>

template < typename AbstractType = QGraphicsItem >
class AbstractShapeItem : public AbstractType
{
public:
   explicit AbstractShapeItem(QGraphicsItem * parent = 0 )
        :AbstractType(parent)
    {
        m_pen.setColor(Qt::black);
        m_brush.setColor(Qt::white);
    }
    virtual ~AbstractShapeItem(){}
    virtual QString displayName () const { return QString("AbstractType");}
    virtual void resizeTo(int dir, const QPointF & point ){}
    virtual QRectF  rect() const { return m_localRect; }
    virtual void updateCoordinate () {}
    virtual void move( const QPointF & point ){}

    int handleCount() const { return m_handles.count()-1;}

    int collidesWithHandle( const QPointF & point ) const
    {
        const Handles::const_iterator hend =  m_handles.end();
        for (Handles::const_iterator it = m_handles.begin(); it != hend; ++it)
        {
            QPointF pt = (*it)->mapFromScene(point);
            if ((*it)->contains(pt) ){
                return (*it)->dir();
            }
        }
        return Handle_None;
    }

    QColor brush() const {return m_brush.color();}
    QPen   pen() const {return m_pen;}
    QColor penColor() const {return m_pen.color();}
    void   setPen(const QPen & pen ) { m_pen = pen;}
    void   setBrush( const QBrush & brush ) { m_brush = brush ; }

protected:
    virtual void updateGeometry(){}
    void setState(SelectionHandleState st)
    {
        const Handles::iterator hend =  m_handles.end();
        for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
            (*it)->setState(st);
    }

    QBrush m_brush;
    QPen   m_pen ;
    typedef QVector<SizeHandleRect*> Handles;
    Handles m_handles;
    QRectF m_localRect;
};

typedef  AbstractShapeItem< QGraphicsItem > AbstractBasicShape;

class GraphicsItem : public QObject,
        public AbstractShapeItem<QGraphicsItem>
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ penColor WRITE setPen )
    Q_PROPERTY(QColor brush READ brush WRITE setBrush )

public:
    GraphicsItem(QGraphicsItem * parent );
    enum {Type = UserType+1};
    int  type() const { return Type; }
    QRectF  rect() const { return m_localRect;}
signals:
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    void updateGeometry();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class GraphicsRectItem : public GraphicsItem
{
public:
    GraphicsRectItem(const QRect & rect ,QGraphicsItem * parent);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    virtual void resizeTo(int dir, const QPointF & point );
    virtual QRectF  rect() const {  return m_localRect;}
    void updateCoordinate();
    void move( const QPointF & point );
    QString displayName() const { return tr("rectangle"); }
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
protected:
    qreal m_width;
    qreal m_height;
};

class GraphicsRoundRectItem : public GraphicsRectItem
{
public:
    GraphicsRoundRectItem( const QRect & rect , QGraphicsItem *parent );
    QPainterPath shape() const;
    virtual void resizeTo(int dir, const QPointF & point );
    QString displayName() const { return tr("roundrect"); }
protected:
    void updateGeometry();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPoint m_roundness;
    float m_fRatio;
};

class GraphicsItemGroup : public QObject,
        public AbstractShapeItem <QGraphicsItemGroup>
{
    Q_OBJECT
public:
    enum {Type = UserType+2};
    int  type() const { return Type; }

    explicit GraphicsItemGroup(QGraphicsItem *parent = 0);
    ~GraphicsItemGroup();
    void updateCoordinate();

signals:
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};


class GraphicsEllipseItem : public GraphicsRectItem
{
public:
    GraphicsEllipseItem(const QRect & rect ,QGraphicsItem * parent);
    QPainterPath shape() const;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class GraphicsLineItem : public GraphicsRectItem
{
public:
    GraphicsLineItem(QGraphicsItem * parent );
    QPainterPath shape() const;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

};

class GraphicsPolygonItem : public GraphicsItem
{
public:
    GraphicsPolygonItem(QGraphicsItem * parent );
    QRectF boundingRect() const ;
    QPainterPath shape() const;
    virtual void addPoint( const QPointF & point ) ;
    virtual void resizeTo(int dir, const QPointF & point );
    void updateCoordinate ();
    virtual void endPoint(const QPointF & point );
protected:
    QRectF RecalcBounds();
    void updateGeometry();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPolygonF m_points;
    qreal m_width;
    qreal m_height;
};


class GraphicsBezierCurve : public GraphicsPolygonItem
{
public:
    GraphicsBezierCurve(QGraphicsItem * parent );
    QPainterPath shape() const;
    virtual void addPoint( const QPointF & point ) ;    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int m_index;
};

class GraphicsArcItem :public GraphicsPolygonItem
{
public:
    GraphicsArcItem(QGraphicsItem * parent);
    QPainterPath shape() const;
    virtual void addPoint( const QPointF & point ) ;
     void endPoint(const QPointF & point );
    virtual void resizeTo(int dir, const QPointF & point );
    QRectF boundingRect() const ;
    void updateCoordinate ();
protected:
    void updateGeometry();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    qreal m_Radius;
    qreal m_startAngle;
    qreal m_endAngle;
};

class GridTool : public QGraphicsItem
{
public:
    GridTool(const QSize &grid = QSize(3200,2400) , const QSize & space = QSize(20,20) );
    QRectF boundingRect() const ;
    QPainterPath shape() const;
    void paintGrid(QPainter *painter,const QRect & rect );
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QSize m_sizeGrid;
    QSize m_sizeGridSpace;
};

#endif // DRAWOBJ
