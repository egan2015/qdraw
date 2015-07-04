#include "drawobj.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QCursor>
#include <QDebug>
#include <QGraphicsEffect>
#include <QMatrix4x4>
#include <QGraphicsTransform>
#include <cmath>

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    if (path == QPainterPath() || pen == Qt::NoPen)
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}


GraphicsItem::GraphicsItem(QGraphicsItem *parent)
    :AbstractShapeType<QGraphicsItem>(parent)
{
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//    effect->setBlurRadius(8);
//    setGraphicsEffect(effect);
//    setCacheMode(QGraphicsItem::ItemCoordinateCache);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}


void GraphicsItem::updatehandles()
{
    const QRectF &geom = this->boundingRect();

    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;;
        switch (hndl->dir()) {
        case LeftTop:
            hndl->move(geom.x() , geom.y() );
            break;
        case Top:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() );
            break;
        case RightTop:
            hndl->move(geom.x() + geom.width() , geom.y() );
            break;
        case Right:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() / 2 );
            break;
        case RightBottom:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() );
            break;
        case Bottom:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() + geom.height() );
            break;
        case LeftBottom:
            hndl->move(geom.x(), geom.y() + geom.height());
            break;
        case Left:
            hndl->move(geom.x(), geom.y() + geom.height() / 2);
            break;
        default:
            break;
        }
    }
}

void GraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}

QVariant GraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if ( change == QGraphicsItem::ItemSelectedHasChanged ) {
        setState(value.toBool() ? SelectionHandleActive : SelectionHandleOff);
    }
    return QGraphicsItem::itemChange(change, value);
}


GraphicsRectItem::GraphicsRectItem(const QRect & rect , bool isRound , QGraphicsItem *parent)
    :GraphicsItem(parent)
    ,m_isRound(isRound)
    ,m_fRatio(1/3.0)
{

    m_width = rect.width();
    m_height = rect.height();

    // handles
    m_handles.reserve(Left);
    for (int i = LeftTop; i <= Left; ++i) {
        SizeHandleRect *shr = new SizeHandleRect(this,i, this);
        m_handles.push_back(shr);
    }
    if( m_isRound ){
        m_fRatio = 1/3.0f;
        SizeHandleRect *shr = new SizeHandleRect(this, 8 , this);
        m_handles.push_back(shr);
    }
    updatehandles();
}

QRectF GraphicsRectItem::boundingRect() const
{
    return m_localRect;
}

QPainterPath GraphicsRectItem::shape() const
{
    QPainterPath path;
    double r;
    if(m_fRatio<=0)
       r=0;
    else {
        r = m_height * m_fRatio + 0.5;
    }
    if ( m_isRound )
        path.addRoundedRect(rect(),r,r);
    else
        path.addRect(rect());
    return path;
}

void GraphicsRectItem::resize(int dir, const QPointF & delta)
{
    QPointF local = mapFromParent(delta);
    QString dirName;

    QRect delta1 = this->rect().toRect();
    switch (dir) {
    case Right:
        dirName = "Rigth";
        delta1.setRight(local.x());
        break;
    case RightTop:
        dirName = "RightTop";
        delta1.setTopRight(local.toPoint());
        break;
    case RightBottom:
        dirName = "RightBottom";
        delta1.setBottomRight(local.toPoint());
        break;
    case LeftBottom:
        dirName = "LeftBottom";
        delta1.setBottomLeft(local.toPoint());
        break;
    case Bottom:
        dirName = "Bottom";
        delta1.setBottom(local.y());
        break;
    case LeftTop:
        dirName = "LeftTop";
        delta1.setTopLeft(local.toPoint());
        break;
    case Left:
        dirName = "Left";
        delta1.setLeft(local.x());
        break;
    case Top:
        dirName = "Top";
        delta1.setTop(local.y());
        break;
    case 8:
    {
        QRectF delta1 = rect();
        int y = local.y();
        if(y> delta1.center().y() )
            y = delta1.center().y();
        if(y<delta1.top())
            y=delta1.top();
        int H= delta1.height();
        if(H==0)
            H=1;
        m_fRatio= std::abs(((float)(delta1.top()-y)))/H;
    }
   default:
        break;
    }

    m_width = delta1.width();
    m_height = delta1.height();

    prepareGeometryChange();
    m_localRect = delta1;

    updatehandles();
}

void GraphicsRectItem::updateCoordinate()
{
    QPointF pt1,pt2,delta;
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(boundingRect().center());
    delta = pt1 - pt2;

    prepareGeometryChange();

    m_localRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);

    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(boundingRect().center());
    moveBy(-delta.x(),-delta.y());
    setTransform(transform().translate(-delta.x(),-delta.y()));

    updatehandles();

}

void GraphicsRectItem::move(const QPointF &point)
{
    moveBy(point.x(),point.y());
}

GraphicsItem *GraphicsRectItem::copy() const
{
    GraphicsRectItem * item = new GraphicsRectItem( rect().toRect(),m_isRound);
    item->setWidth(width());
    item->setHeight(height());
    item->setPos(pos().x()+10,pos().y()+10);
    item->setPen(pen());
    item->setBrush(brush());
    item->setTransform(transform());
    item->setTransformOriginPoint(transformOriginPoint());
    item->setRotation(rotation());
    item->setScale(scale());
    item->setZValue(zValue()+0.1);
    item->updateCoordinate();
    return item;
}

void GraphicsRectItem::updatehandles()
{
    const QRectF &geom = this->boundingRect();
    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;;
        switch (hndl->dir()) {
        case LeftTop:
            hndl->move(geom.x() , geom.y());
            break;
        case Top:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() );
            break;
        case RightTop:
            hndl->move(geom.x() + geom.width() , geom.y() );
            break;
        case Right:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() / 2 );
            break;
        case RightBottom:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() );
            break;
        case Bottom:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() + geom.height() );
            break;
        case LeftBottom:
            hndl->move(geom.x() , geom.y() + geom.height() );
            break;
        case Left:
            hndl->move(geom.x() , geom.y() + geom.height() / 2 );
            break;
        case 8:
         {
            hndl->move( geom.right() , geom.top() + geom.height() * m_fRatio );
         }
            break;
        default:
            break;
        }
    }

}


void GraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
/*
    QColor c = QColor(Qt::red);
    QLinearGradient result(rect().topLeft(), rect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);
 */
   painter->setPen(pen());
   painter->setBrush(brush());
   double r;
   if(m_fRatio<=0)
      r=0;
   else {
       r = m_height * m_fRatio + 0.5;
   }
   if ( m_isRound )
       painter->drawRoundedRect(rect(),r,r);
   else
       painter->drawRect(rect().toRect());

/*
    QPointF origin = m_localRect.center();
    painter->setBrush(Qt::blue);
    painter->drawEllipse(origin,6,6);

    QPointF pos2 = transformOriginPoint();
    painter->setBrush(Qt::black);
    painter->drawEllipse(pos2,6,6);
*/

}


GraphicsEllipseItem::GraphicsEllipseItem(const QRect &rect, QGraphicsItem *parent)
    :GraphicsRectItem(rect,parent)
{

}

QPainterPath GraphicsEllipseItem::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void GraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

//    QColor c = QColor(Qt::red);
//    c.setAlpha(160);
    QRectF rc = rect().normalized();
/*
    qreal radius = qMax(rc.width(),rc.height());
    QRadialGradient result(rc.center(),radius);
    result.setColorAt(0, c.light(200));
    result.setColorAt(0.5, c.dark(150));
    result.setColorAt(1, c);
    painter->setPen(pen());
    QBrush b(result);
    b.setStyle(Qt::RadialGradientPattern);
    painter->setBrush(b);
*/
    painter->setBrush(brush());
    painter->drawEllipse(rc);
}

GraphicsLineItem::GraphicsLineItem(QGraphicsItem *parent)
    :GraphicsRectItem(QRect(0,0,0,0),parent)
{
    // handles
    m_handles.reserve(Left);

    Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
        delete (*it);
    m_handles.clear();

    SizeHandleRect *shr = new SizeHandleRect(this,LeftTop, this);
    m_handles.push_back(shr);
    shr = new SizeHandleRect(this,RightBottom, this);
    m_handles.push_back(shr);

    updatehandles();
}

QPainterPath GraphicsLineItem::shape() const
{
    QPainterPath path;
    path.moveTo(rect().topLeft());
    path.lineTo(rect().bottomRight());
    return qt_graphicsItem_shapeFromPath(path,pen());
}

void GraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(pen());
    painter->drawLine(rect().topLeft(),rect().bottomRight());
}

GraphicsItemGroup::GraphicsItemGroup(QGraphicsItem *parent)
    :AbstractShapeType <QGraphicsItemGroup>(parent),
      m_parent(parent)
{
    // handles
    m_handles.reserve(Left);
    for (int i = LeftTop; i <= Left; ++i) {
        SizeHandleRect *shr = new SizeHandleRect(this, i, this);
        m_handles.push_back(shr);
    }
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}

void GraphicsItemGroup::addToGroup(QGraphicsItem *item)
{
    if (!item) {
        qWarning("QGraphicsItemGroup::addToGroup: cannot add null item");
        return;
    }
    if (item == this) {
        qWarning("QGraphicsItemGroup::addToGroup: cannot add a group to itself");
        return;
    }

    // COMBINE
    bool ok;
    QTransform itemTransform = item->itemTransform(this, &ok);

    if (!ok) {
        qWarning("QGraphicsItemGroup::addToGroup: could not find a valid transformation from item to group coordinates");
        return;
    }

    QTransform newItemTransform(itemTransform);
    item->setPos(mapFromItem(item, 0, 0));
    item->setParentItem(this);

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        newItemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied with itemTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform*> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    newItemTransform *= m.toTransform().inverted();
    newItemTransform.translate(origin.x(), origin.y());
    newItemTransform.rotate(-item->rotation());
    newItemTransform.scale(1/item->scale(), 1/item->scale());
    newItemTransform.translate(-origin.x(), -origin.y());

    // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

    item->setTransform(newItemTransform);
//    item->d_func()->setIsMemberOfGroup(true);
    prepareGeometryChange();
    itemsBoundingRect |= itemTransform.mapRect(item->boundingRect() | item->childrenBoundingRect());
    update();
}

void GraphicsItemGroup::removeFromGroup(QGraphicsItem *item)
{
    if (!item) {
        qWarning("QGraphicsItemGroup::removeFromGroup: cannot remove null item");
        return;
    }

    QGraphicsItem *newParent = m_parent;

    // COMBINE
    bool ok;
    QTransform itemTransform;
    if (newParent)
        itemTransform = item->itemTransform(newParent, &ok);
    else
        itemTransform = item->sceneTransform();

    QPointF oldPos = item->mapToItem(newParent, 0, 0);
    item->setParentItem(newParent);
    item->setPos(oldPos);

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        itemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied
    // with itemTransform() or sceneTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform*> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    itemTransform *= m.toTransform().inverted();
    itemTransform.translate(origin.x(), origin.y());
    itemTransform.rotate(-item->rotation());
    itemTransform.scale(1 / item->scale(), 1 / item->scale());
    itemTransform.translate(-origin.x(), -origin.y());

    // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

    item->setTransform(itemTransform);
//    item->d_func()->setIsMemberOfGroup(item->group() != 0);

    // ### Quite expensive. But removeFromGroup() isn't called very often.
    prepareGeometryChange();
    itemsBoundingRect = childrenBoundingRect();
}

QRectF GraphicsItemGroup::boundingRect() const
{
    return itemsBoundingRect;
}

GraphicsItemGroup::~GraphicsItemGroup()
{

}

void GraphicsItemGroup::updateCoordinate()
{
    QPointF pt1,pt2,delta;
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(boundingRect().center());
    delta = pt1 - pt2;
    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(boundingRect().center());
    moveBy(-delta.x(),-delta.y());
    updatehandles();
}

void GraphicsItemGroup::updatehandles()
{
    const QRectF &geom = this->boundingRect();

    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;;
        switch (hndl->dir()) {
        case LeftTop:
            hndl->move(geom.x() , geom.y() );
            break;
        case Top:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() );
            break;
        case RightTop:
            hndl->move(geom.x() + geom.width() , geom.y() );
            break;
        case Right:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() / 2 );
            break;
        case RightBottom:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() );
            break;
        case Bottom:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() + geom.height() );
            break;
        case LeftBottom:
            hndl->move(geom.x(), geom.y() + geom.height());
            break;
        case Left:
            hndl->move(geom.x(), geom.y() + geom.height() / 2);
            break;
        default:
            break;
        }
    }
}

QVariant GraphicsItemGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if ( change == QGraphicsItem::ItemSelectedHasChanged ) {
        setState(value.toBool() ? SelectionHandleActive : SelectionHandleOff);
        if( value.toBool()){
            updateCoordinate();
        }
    }
    return QGraphicsItemGroup::itemChange(change, value);
}

void GraphicsItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    Q_UNUSED(painter);
}

GraphicsBezierCurve::GraphicsBezierCurve(QGraphicsItem *parent)
    :GraphicsPolygonItem(parent)
{
    m_index = 0;
}

QPainterPath GraphicsBezierCurve::shape() const
{
    QPainterPath path;
    path.moveTo(m_points.at(0));
    if ( m_points.count() ==2 )
        path.quadTo(m_points.at(0),m_points.at(1));
    else if (m_points.count() == 3) {
        path.cubicTo(m_points.at(0),m_points.at(1),m_points.at(2));
    } else if ( m_points.count() == 4 ){
        path.cubicTo(m_points.at(1),m_points.at(2),m_points.at(3));
    }
    return path;
}

void GraphicsBezierCurve::addPoint(const QPointF &point)
{
    m_points.append(mapFromScene(point));
    SizeHandleRect *shr = new SizeHandleRect(this, m_index, this);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
    m_index++;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}

void GraphicsBezierCurve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    painter->setPen(pen());
    painter->setBrush(brush());
    path.moveTo(m_points.at(0));
    if ( m_points.count() ==2 )
        path.quadTo(m_points.at(0),m_points.at(1));
    else if (m_points.count() == 3) {
        path.cubicTo(m_points.at(0),m_points.at(1),m_points.at(2));
    } else if ( m_points.count() == 4 ){
        path.cubicTo(m_points.at(1),m_points.at(2),m_points.at(3));
    }

    painter->drawPath(path);
}


GridTool::GridTool(const QSize & grid , const QSize & space )
    :QGraphicsItem(NULL)
    ,m_sizeGrid(grid)
    ,m_sizeGridSpace(20,20)
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
}

QRectF GridTool::boundingRect() const
{

    return QRectF(0,
                  0,
                  m_sizeGrid.width(),
                  m_sizeGrid.height());
}

QPainterPath GridTool::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void GridTool::paintGrid(QPainter *painter, const QRect &rect)
{
    QColor c(Qt::darkCyan);
    QPen p(c);
    p.setStyle(Qt::DashLine);
    p.setWidthF(0.2);
    painter->setPen(p);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing,false);

    painter->fillRect(rect,Qt::white);
    for (int x=rect.left() ;x <rect.right()  ;x+=(int)(m_sizeGridSpace.width())) {
        painter->drawLine(x,rect.top(),x,rect.bottom());

    }
    for (int y=rect.top();y<rect.bottom() ;y+=(int)(m_sizeGridSpace.height()))
    {
        painter->drawLine(rect.left(),y,rect.right(),y);
    }
    painter->drawLine(rect.right(),rect.top(),rect.right(),rect.bottom());
    painter->drawLine(rect.left(),rect.bottom(),rect.right(),rect.bottom());
    painter->restore();
}

void GridTool::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor c(Qt::darkCyan);
    painter->setPen(c);

    for (int x=boundingRect().left() ;x < boundingRect().right()  ;x+=(int)(m_sizeGridSpace.width())) {
        for (int y=boundingRect().top();y<boundingRect().bottom() ;y+=(int)(m_sizeGridSpace.height()))
        {
            painter->drawPoint(x,y);
        }
    }
}


GraphicsArcItem::GraphicsArcItem(QGraphicsItem *parent)
    :GraphicsPolygonItem(parent)
{
    m_Radius = 0;
    m_startAngle = 0;
    m_endAngle = 0;
}

QPainterPath GraphicsArcItem::shape() const
{
    QPainterPath path;
    path.moveTo(m_points.at(0));
    path.arcTo(m_localRect,m_startAngle,m_endAngle-m_startAngle);
    path.closeSubpath();
    return path;
}

void GraphicsArcItem::addPoint(const QPointF &point)
{
    if ( m_points.count() > 3 ) return;
    GraphicsPolygonItem::addPoint( point );
}

void GraphicsArcItem::endPoint(const QPointF &point)
{
    Q_UNUSED(point)
    if ( m_points.count() > 3) {
        m_points.remove(3);
        delete m_handles.at(3);
        m_handles.resize(3);
    }
}

void GraphicsArcItem::resize(int dir, const QPointF & delta)
{
    QPointF local = mapFromScene(delta);
    switch( dir )
    {
    case 0:
        break;
    case 1:
    case 2:
    {
        m_points[(int)dir] = local;
        qreal rx = local.x() - m_points.at(0).x();
        qreal ry = local.y() - m_points.at(0).y();
        qreal r  = sqrt(rx*rx+ry*ry);
        m_Radius = r;
        qreal len_y = local.y() - m_points.at(0).y();
        qreal len_x = local.x() - m_points.at(0).x();
        m_startAngle = -atan2(len_y,len_x)*180/3.1416;
       if ( m_points.count() > 2){
            qreal len_y = m_points.at(1).y() - m_points.at(0).y();
            qreal len_x = m_points.at(1).x() - m_points.at(0).x();
            m_startAngle = -atan2(len_y,len_x)*180/3.1416;
            len_y = m_points.at(2).y() - m_points.at(0).y();
            len_x = m_points.at(2).x() - m_points.at(0).x();
            m_endAngle = -atan2(len_y,len_x)*180/3.1416;

            if ( m_startAngle > m_endAngle )
                m_startAngle-=360;
            if ( m_endAngle < m_startAngle ){
                qreal tmp = m_endAngle;
                m_endAngle = m_startAngle;
                m_startAngle = tmp;
            }
            if ( std::abs(m_endAngle-m_startAngle) > 360 ){
                m_startAngle = 0;
                m_endAngle = 360;
            }
        }
    }
    break;
    }

    prepareGeometryChange();
    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);
    updatehandles();
}

QRectF GraphicsArcItem::boundingRect() const
{
    return m_localRect;
}

void GraphicsArcItem::updateCoordinate()
{

    QPointF pt1,pt2,delta;
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(m_points.at(0));
    delta = pt1 - pt2;

    prepareGeometryChange();

    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);

    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(boundingRect().center());
    moveBy(-delta.x(),-delta.y());
    setTransform(transform().translate(-delta.x(),-delta.y()));
    updatehandles();
}

void GraphicsArcItem::updatehandles()
{
    GraphicsPolygonItem::updatehandles();
    if ( m_points.count() > 2 ){
        qreal x = m_Radius * cos( -m_startAngle * 3.1416 / 180 );
        qreal y = m_Radius * sin( -m_startAngle * 3.1416 / 180);
        m_handles.at(1)->move(x,y);

        x = m_Radius * cos( -m_endAngle * 3.1416 / 180);
        y = m_Radius * sin(-m_endAngle * 3.1416 / 180);
        m_handles.at(2)->move(x,y);
    }
}

void GraphicsArcItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    qreal startAngle = m_startAngle <= m_endAngle ? m_startAngle : m_endAngle;
    qreal endAngle = m_startAngle >= m_endAngle ? m_startAngle : m_endAngle;

    painter->setPen(pen());
    painter->setBrush(brush());

    if(endAngle - startAngle > 360)
        endAngle = startAngle + 360;

    path.moveTo(m_points.at(0));
    path.arcTo(m_localRect,startAngle,endAngle-startAngle);
    path.closeSubpath();

    painter->drawPath(path);
//    painter->drawArc(m_localRect, startAngle * 16 , (endAngle - startAngle) * 16);
}

GraphicsPolygonItem::GraphicsPolygonItem(QGraphicsItem *parent)
    :GraphicsItem(parent)
{
    // handles
    m_handles.reserve(Left);

    Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
        delete (*it);
    m_handles.clear();

    m_points.clear();

}

QRectF GraphicsPolygonItem::boundingRect() const
{
    return m_points.boundingRect();
}

QPainterPath GraphicsPolygonItem::shape() const
{
    QPainterPath path;
    path.addPolygon(m_points);
    path.closeSubpath();
    return path;
}

void GraphicsPolygonItem::addPoint(const QPointF &point)
{
    m_points.append(mapFromScene(point));
    int dir = m_points.count() - 1 ;
    SizeHandleRect *shr = new SizeHandleRect(this, dir, this);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
}

void GraphicsPolygonItem::resize(int dir, const QPointF &delta)
{
    QPointF pt = mapFromScene(delta);
    m_points[dir] = pt;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}

void GraphicsPolygonItem::updateCoordinate()
{

    QPointF pt1,pt2,delta;
    QPolygonF pts = mapToScene(m_points);
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(boundingRect().center());
    delta = pt1 - pt2;

    for (int i = 0; i < pts.count() ; ++i )
        pts[i]+=delta;

    prepareGeometryChange();

    m_points = mapFromScene(pts);

    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(boundingRect().center());
    moveBy(-delta.x(),-delta.y());
    setTransform(transform().translate(-delta.x(),-delta.y()));

    updatehandles();

}

void GraphicsPolygonItem::endPoint(const QPointF & point)
{
    Q_UNUSED(point);
    int nPoints = m_points.count();
    if( nPoints > 2 && (m_points[nPoints-1] == m_points[nPoints-2] ||
        m_points[nPoints-1].x() - 1 == m_points[nPoints-2].x() &&
        m_points[nPoints-1].y() == m_points[nPoints-2].y())){
        delete m_handles[nPoints-1];
        m_points.remove(nPoints-1);
        m_handles.resize(nPoints-1);
    }
}

void GraphicsPolygonItem::updatehandles()
{
    const Handles::iterator hend =  m_handles.end();

    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;
        int idx = (int)hndl->dir();
        hndl->move(m_points[idx].x() ,m_points[idx].y() );
    }
}

void GraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor c = brush();
    QLinearGradient result(rect().topLeft(), rect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);

    painter->setPen(pen());
    painter->drawPolygon(m_points);

}
