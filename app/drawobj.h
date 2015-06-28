#ifndef DRAWOBJ
#define DRAWOBJ

#include <qgraphicsitem.h>
#include <QtCore/QObject>
#include <QGraphicsSceneMouseEvent>
#include "drawtool.h"
#include "sizehandle.h"
#include <QVector>

class GraphicsBasicItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ penColor WRITE setPen )
    Q_PROPERTY(QColor brush READ brush WRITE setBrush )
public:
    explicit GraphicsBasicItem(QGraphicsItem * parent);
    explicit GraphicsBasicItem(const QString &name ,QGraphicsItem *parent );
    virtual ~GraphicsBasicItem();

    QColor brush() const {return m_brush.color();}
    QPen   pen() const {return m_pen;}
    QColor penColor() const {return m_pen.color();}
    void   setPen(const QPen & pen ) { m_pen = pen;}
    void   setBrush( const QBrush & brush ) { m_brush = brush ; }

protected:
    QBrush m_brush;
    QPen   m_pen ;
};

class GraphicsItem : public GraphicsBasicItem
{
    Q_OBJECT
public:
    GraphicsItem(QGraphicsItem * parent );
    enum {Type = UserType+1};
    int  type() const { return Type; }
    virtual SizeHandleRect::Direction  hitTest( const QPointF & point ) const;
    virtual void resizeTo(SizeHandleRect::Direction dir, const QPointF & point );
    virtual Qt::CursorShape getCursor(SizeHandleRect::Direction dir );
    virtual QRectF  rect() const { return m_localRect;}
    virtual void updateCoordinate () {}
    virtual void move( const QPointF & point ){}
    int  getHandleCount() const { return m_handles.count();}
signals:
    void selectedChange(QGraphicsItem *item);
protected:
    virtual void updateGeometry();
    void setState(SelectionHandleState st);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    typedef QVector<SizeHandleRect*> Handles;
    Handles m_handles;
    QRectF m_localRect;
};

class GraphicsRectItem : public GraphicsItem
{
public:
    GraphicsRectItem(const QRect & rect ,QGraphicsItem * parent);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    virtual void resizeTo(SizeHandleRect::Direction dir, const QPointF & point );
    virtual QRectF  rect() const {  return m_localRect;}
    void updateCoordinate();
    void move( const QPointF & point );

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
    virtual void resizeTo(SizeHandleRect::Direction dir, const QPointF & point );
protected:
    void updateGeometry();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPoint m_roundness;
    float m_fRatio;
};

class GraphicsItemGroup : public GraphicsItem
{

public:
    explicit GraphicsItemGroup(QGraphicsItem *parent = 0);
    ~GraphicsItemGroup();
    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);
    void updateCoordinate();

    QRectF boundingRect() const;
    bool isObscuredBy(const QGraphicsItem *item) const;
    QPainterPath opaqueArea() const;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
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
    virtual void resizeTo(SizeHandleRect::Direction dir, const QPointF & point );    
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
    virtual void resizeTo(SizeHandleRect::Direction dir, const QPointF & point );
    QRectF boundingRect() const ;
    void updateCoordinate ();
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    qreal m_Radius;
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
