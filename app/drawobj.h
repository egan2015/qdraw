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
#include <vector>

template < typename AbstractType = QGraphicsItem >
class AbstractShapeItem : public AbstractType
{
public:
   explicit AbstractShapeItem(QGraphicsItem * parent = 0 )
        :AbstractType(parent)
    {
        m_pen.setColor(Qt::black);
        m_brush.setColor(Qt::white);
        m_width = m_height = 0;
    }
    virtual ~AbstractShapeItem(){}
    virtual QString displayName () const { return QString("AbstractType");}
    virtual void resize(int dir, const QPointF & delta ){}
    virtual QRectF  rect() const { return m_localRect; }
    virtual void updateCoordinate () {}
    virtual void move( const QPointF & point ){}

    int handleCount() const { return m_handles.size()-1;}

    int collidesWithHandle( const QPointF & point ) const
    {
        const Handles::const_reverse_iterator hend =  m_handles.rend();
        for (Handles::const_reverse_iterator it = m_handles.rbegin(); it != hend; ++it)
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
    qreal  width() const { return m_width ; }
    void   setWidth( qreal width )
    {
        m_width = width ;
        updateCoordinate();
    }
    qreal  height() const {return m_height;}
    void   setHeight ( qreal height )
    {
        m_height = height ;
        updateCoordinate();
    }

protected:
    virtual void updatehandles(){}
    void setState(SelectionHandleState st)
    {
        const Handles::iterator hend =  m_handles.end();
        for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
            (*it)->setState(st);
    }

    QBrush m_brush;
    QPen   m_pen ;
    typedef std::vector<SizeHandleRect*> Handles;
    Handles m_handles;
    QRectF m_localRect;
    qreal m_width;
    qreal m_height;

};

typedef  AbstractShapeItem< QGraphicsItem > AbstractBasicShape;

class GraphicsItem : public QObject,
        public AbstractShapeItem<QGraphicsItem>
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ penColor WRITE setPen )
    Q_PROPERTY(QColor brush READ brush WRITE setBrush )
    Q_PROPERTY(qreal  width READ width WRITE setWidth )
    Q_PROPERTY(qreal  height READ height WRITE setHeight )
    Q_PROPERTY(QPointF  position READ pos WRITE setPos )

public:
    GraphicsItem(QGraphicsItem * parent );
    enum {Type = UserType+1};
    int  type() const { return Type; }

signals:
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    void updatehandles();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class GraphicsRectItem : public GraphicsItem
{
public:
    GraphicsRectItem(const QRect & rect ,QGraphicsItem * parent);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    virtual void resize(int dir, const QPointF & delta);
    virtual QRectF  rect() const {  return m_localRect;}
    void updateCoordinate();
    void move( const QPointF & point );
    QString displayName() const { return tr("rectangle"); }
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class GraphicsRoundRectItem : public GraphicsRectItem
{
public:
    GraphicsRoundRectItem( const QRect & rect , QGraphicsItem *parent );
    QPainterPath shape() const;
    virtual void resize(int dir, const QPointF & delta);
    QString displayName() const { return tr("roundrect"); }
protected:
    void updatehandles();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPoint m_roundness;
    qreal m_fRatio;
};

class GraphicsItemGroup : public QObject,
        public AbstractShapeItem <QGraphicsItemGroup>
{
    Q_OBJECT
public:
    enum {Type = UserType+2};
    int  type() const { return Type; }
    explicit GraphicsItemGroup(QGraphicsItem *parent = 0);
    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);
    QRectF boundingRect() const;
    ~GraphicsItemGroup();
    void updateCoordinate();
signals:
    void selectedChange(QGraphicsItem *item);

protected:
    void updatehandles();
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QGraphicsItem * m_parent;
    QRectF itemsBoundingRect;
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
    virtual void resize(int dir, const QPointF & delta);
    void updateCoordinate ();
    virtual void endPoint(const QPointF & point );
protected:
    void updatehandles();
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
    virtual void resize(int dir, const QPointF & delta );
    QRectF boundingRect() const ;
    void updateCoordinate ();
protected:
    void updatehandles();

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
