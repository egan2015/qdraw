#ifndef DRAWTOOL
#define DRAWTOOL


#include "drawobj.h"
#include "drawscene.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
class QAbstractGraphicsShapeItem;
QT_END_NAMESPACE

class DrawScene;
class GraphicsItem;
class GraphicsPolygonItem;
enum DrawShape
{
    selection ,
    rotation  ,
    line ,
    rectangle ,
    roundrect ,
    ellipse ,
    bezier,
    polygon,
    polyline,
};

class DrawTool
{
public:
    DrawTool( DrawShape shape );
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene );
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event ,DrawScene *scene );
    DrawShape m_drawShape;
    bool m_hoverSizer;

    static DrawTool * findTool( DrawShape drawShape );
    static QList<DrawTool*> c_tools;
    static QPointF c_down;
    static quint32 c_nDownFlags;
    static QPointF c_last;
    static DrawShape c_drawShape;
};

class SelectTool : public DrawTool
{
public:
    SelectTool();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene );
    QPointF initialPositions;
    QPointF opposite_;
    QGraphicsPathItem * dashRect;
    GraphicsItemGroup * selLayer;
};

class  RotationTool : public DrawTool
{
public:
    RotationTool();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene );
    qreal lastAngle;
    QGraphicsPathItem * dashRect;
};

class RectTool : public DrawTool
{
public:
    RectTool(DrawShape drawShape);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene );
    GraphicsItem * item;
};

class PolygonTool : public DrawTool
{
public:
    PolygonTool(DrawShape shape );
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene ) ;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event , DrawScene * scene );
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event ,DrawScene *scene );
    GraphicsPolygonItem * item;
    int m_nPoints;
    QPointF initialPositions;

};

#endif // DRAWTOOL

