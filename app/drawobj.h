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
#include <QMimeData>

class ShapeMimeData : public QMimeData
{
    Q_OBJECT
public:
    ShapeMimeData( QList<QGraphicsItem * > items);
    ~ShapeMimeData();
    QList<QGraphicsItem *> items() const ;
private:
    QList<QGraphicsItem * > m_items;
};


template < typename BaseType = QGraphicsItem >
class AbstractShapeType : public BaseType
{
public:
   explicit AbstractShapeType(QGraphicsItem * parent = 0 )
        :BaseType(parent)
    {
        m_pen=QPen(Qt::NoPen);
        m_brush= QBrush(QColor(rand() % 32 * 8, rand() % 32 * 8, rand() % 32 * 8));
        m_width = m_height = 0;
    }
    virtual ~AbstractShapeType(){}
    virtual QString displayName () const { return QString("AbstractType");}
    virtual void resize(int dir, const QPointF & delta ){ Q_UNUSED(dir);Q_UNUSED(delta);}    
    virtual void stretch( int  , double  , double  , const QPointF & ) {}
    virtual QRectF  rect() const { return m_localRect; }
    virtual void updateCoordinate () {}
    virtual void move( const QPointF & point ){Q_UNUSED(point);}
    virtual QGraphicsItem * copy() const { return NULL;}
    virtual int handleCount() const { return m_handles.size();}

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

    virtual QPointF opposite( int handle ) {
        QPointF pt;
        switch (handle) {
        case Right:
            pt = m_handles.at(Left-1)->pos();
            break;
        case RightTop:
            pt = m_handles[LeftBottom-1]->pos();
            break;
        case RightBottom:
            pt = m_handles[LeftTop-1]->pos();
            break;
        case LeftBottom:
            pt = m_handles[RightTop-1]->pos();
            break;
        case Bottom:
            pt = m_handles[Top-1]->pos();
            break;
        case LeftTop:
            pt = m_handles[RightBottom-1]->pos();
            break;
        case Left:
            pt = m_handles[Right-1]->pos();
            break;
        case Top:
            pt = m_handles[Bottom-1]->pos();
            break;
         }
        return pt;
    }

    QColor brushColor() const {return m_brush.color();}
    QBrush brush() const {return m_brush;}
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

typedef  AbstractShapeType< QGraphicsItem > AbstractShape;

class GraphicsItem : public QObject,
        public AbstractShapeType<QGraphicsItem>
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ penColor WRITE setPen )
    Q_PROPERTY(QColor brush READ brushColor WRITE setBrush )
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
    GraphicsRectItem(const QRect & rect , bool isRound = false ,QGraphicsItem * parent = 0 );
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void resize(int dir, const QPointF & delta);
    void stretch(int handle , double sx , double sy , const QPointF & origin);
    QRectF  rect() const {  return m_localRect;}
    void updateCoordinate();
    void move( const QPointF & point );
    QGraphicsItem *copy () const ;

    QString displayName() const { return tr("rectangle"); }
protected:
    void updatehandles();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool m_isRound;
    qreal m_fRatioY;
    qreal m_fRatioX;
    QRectF m_initialRect;
};

class GraphicsEllipseItem :public GraphicsRectItem
{
public:
    GraphicsEllipseItem(QGraphicsItem * parent = 0);
    QPainterPath shape() const;
    void resize(int dir, const QPointF & delta );
    QRectF boundingRect() const ;
    void updateCoordinate ();
    QGraphicsItem *copy() const;
    void stretch( int handle , double sx , double sy , const QPointF & origin );
     QString displayName() const { return tr("ellipse"); }
protected:
    void updatehandles();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int   m_startAngle;
    int   m_spanAngle;
};

class GraphicsItemGroup : public QObject,
        public AbstractShapeType <QGraphicsItemGroup>
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ penColor WRITE setPen )
    Q_PROPERTY(QColor brush READ brushColor WRITE setBrush )
    Q_PROPERTY(qreal  width READ width WRITE setWidth )
    Q_PROPERTY(qreal  height READ height WRITE setHeight )
    Q_PROPERTY(QPointF  position READ pos WRITE setPos )

public:
    enum {Type = UserType+2};
    int  type() const { return Type; }
    explicit GraphicsItemGroup(QGraphicsItem *parent = 0);
    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);
    QRectF boundingRect() const;
    ~GraphicsItemGroup();

    QString displayName() const { return tr("group"); }

    QGraphicsItem *copy () const ;
    void resize(int dir, const QPointF & delta);
    void stretch( int handle , double sx , double sy , const QPointF & origin );
    void updateCoordinate();
signals:
    void selectedChange(QGraphicsItem *item);

protected:
    GraphicsItemGroup * createGroup(const QList<QGraphicsItem *> &items) const;
    QList<QGraphicsItem *> copyChildItems() const;
    void updatehandles();
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QGraphicsItem * m_parent;
    QRectF itemsBoundingRect;
    QRectF m_initialRect;
};

class GraphicsPolygonItem : public GraphicsItem
{
public:
    GraphicsPolygonItem(QGraphicsItem * parent = 0);
    QRectF boundingRect() const ;
    QPainterPath shape() const;
    virtual void addPoint( const QPointF & point ) ;
    virtual void endPoint(const QPointF & point );
    void resize(int dir, const QPointF & delta);
    void stretch( int handle , double sx , double sy , const QPointF & origin );
    void updateCoordinate ();
     QGraphicsItem *copy() const;
protected:
    void updatehandles();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPolygonF m_points;
    QPolygonF m_initialPoints;
};

class GraphicsLineItem : public GraphicsPolygonItem
{
public:
    GraphicsLineItem(QGraphicsItem * parent = 0);
    QPainterPath shape() const;
    QGraphicsItem *copy() const;
    void addPoint( const QPointF & point ) ;
    void endPoint(const QPointF & point );
    virtual QPointF opposite( int handle ) {
        QPointF pt;
        return pt;
    }
    void updateCoordinate() {}
    int handleCount() const { return m_handles.size() + Left;}
protected:
    void updatehandles();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

};

class GraphicsBezier : public GraphicsPolygonItem
{
public:
    GraphicsBezier(bool bbezier = true , QGraphicsItem * parent = 0);
    QPainterPath shape() const;
    QGraphicsItem *copy() const;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    bool m_isBezier;
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
