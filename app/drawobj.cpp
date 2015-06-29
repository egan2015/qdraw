#include "drawobj.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QCursor>
#include <QDebug>
#include <QGraphicsEffect>
#include <QMatrix4x4>
#include <QGraphicsTransform>
#include <math.h>

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
    :GraphicsBasicItem(parent)
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

void GraphicsItem::updateGeometry()
{
    const QRectF &geom = this->boundingRect();

    const int w = SELECTION_HANDLE_SIZE;
    const int h = SELECTION_HANDLE_SIZE;

    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;;
        switch (hndl->dir()) {
        case SizeHandleRect::LeftTop:
            hndl->move(geom.x() , geom.y() );
            break;
        case SizeHandleRect::Top:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() );
            break;
        case SizeHandleRect::RightTop:
            hndl->move(geom.x() + geom.width() , geom.y() );
            break;
        case SizeHandleRect::Right:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() / 2 );
            break;
        case SizeHandleRect::RightBottom:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() );
            break;
        case SizeHandleRect::Bottom:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() + geom.height() );
            break;
        case SizeHandleRect::LeftBottom:
            hndl->move(geom.x(), geom.y() + geom.height());
            break;
        case SizeHandleRect::Left:
            hndl->move(geom.x(), geom.y() + geom.height() / 2);
            break;
        default:
            break;
        }
    }
}

void GraphicsItem::setState(SelectionHandleState st)
{
    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
        (*it)->setState(st);
}

SizeHandleRect::Direction GraphicsItem::hitTest(const QPointF &point) const
{
    const Handles::const_iterator hend =  m_handles.end();
    for (Handles::const_iterator it = m_handles.begin(); it != hend; ++it)
    {
        if ((*it)->hitTest(point) ){
            return (*it)->dir();
        }
    }
    return SizeHandleRect::None;
}

Qt::CursorShape GraphicsItem::getCursor(SizeHandleRect::Direction dir)
{
    switch (dir) {
    case SizeHandleRect::Right:
        return Qt::SizeHorCursor;
    case SizeHandleRect::RightTop:
        return Qt::SizeBDiagCursor;
    case SizeHandleRect::RightBottom:
        return Qt::SizeFDiagCursor;
    case SizeHandleRect::LeftBottom:
        return Qt::SizeBDiagCursor;
    case SizeHandleRect::Bottom:
        return Qt::SizeVerCursor;
    case SizeHandleRect::LeftTop:
        return Qt::SizeFDiagCursor;
    case SizeHandleRect::Left:
        return Qt::SizeHorCursor;
    case SizeHandleRect::Top:
        return Qt::SizeVerCursor;
    default:
        break;
    }
    return Qt::ArrowCursor;
}

void GraphicsItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{

}

void GraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

}

QVariant GraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if ( change == QGraphicsItem::ItemSelectedHasChanged ) {
        qDebug()<<" Item Selected : " << value.toString();
        setState(value.toBool() ? SelectionHandleActive : SelectionHandleOff);
        if( value.toBool())
            emit selectedChange(this);
    }else if ( change == QGraphicsItem::ItemRotationHasChanged ){
        qDebug()<<"Item Rotation Changed:" << value.toString();
    }else if ( change == QGraphicsItem::ItemTransformOriginPointHasChanged ){
        QPointF newPos=boundingRect().center();

        qDebug()<<"ItemTransformOriginPointHasChanged:" << value.toPointF() << newPos;
    }
    return QGraphicsItem::itemChange(change, value);
}


GraphicsRectItem::GraphicsRectItem(const QRect & rect ,QGraphicsItem *parent)
    :GraphicsItem(parent)
    ,m_width(rect.width())
    ,m_height(rect.height())
{

    // handles
    m_handles.reserve(SizeHandleRect::None);
    for (int i = SizeHandleRect::LeftTop; i <= SizeHandleRect::Left; ++i) {
        SizeHandleRect *shr = new SizeHandleRect(this, static_cast<SizeHandleRect::Direction>(i), this);
        m_handles.push_back(shr);
    }
    updateGeometry();
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}

QRectF GraphicsRectItem::boundingRect() const
{
    return m_localRect;
}

QPainterPath GraphicsRectItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return qt_graphicsItem_shapeFromPath(path,pen());
}

void GraphicsRectItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{
    QPointF local = mapFromParent(point);
    QString dirName;

    const QRectF &geom = this->boundingRect();
    QRect delta = this->rect().toRect();
    switch (dir) {
    case SizeHandleRect::Right:
        dirName = "Rigth";
        delta.setRight(local.x());
        break;
    case SizeHandleRect::RightTop:
        dirName = "RightTop";
        delta.setTopRight(local.toPoint());
        break;
    case SizeHandleRect::RightBottom:
        dirName = "RightBottom";
        delta.setBottomRight(local.toPoint());
        break;
    case SizeHandleRect::LeftBottom:
        dirName = "LeftBottom";
        delta.setBottomLeft(local.toPoint());
        break;
    case SizeHandleRect::Bottom:
        dirName = "Bottom";
        delta.setBottom(local.y());
        break;
    case SizeHandleRect::LeftTop:
        dirName = "LeftTop";
        delta.setTopLeft(local.toPoint());
        break;
    case SizeHandleRect::Left:
        dirName = "Left";
        delta.setLeft(local.x());
        break;
    case SizeHandleRect::Top:
        dirName = "Top";
        delta.setTop(local.y());
        break;
   default:
        break;
    }

    m_width = delta.width();
    m_height = delta.height();

    prepareGeometryChange();
    m_localRect = delta;

    updateGeometry();
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

    updateGeometry();

}

void GraphicsRectItem::move(const QPointF &point)
{
   moveBy(point.x(),point.y());
}


void GraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor c = QColor(Qt::red);
 /*
    QLinearGradient result(rect().topLeft(), rect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);
 */
   painter->setPen(pen());
   painter->setBrush(brush());
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
    return qt_graphicsItem_shapeFromPath(path,pen());
}

void GraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor c = QColor(Qt::red);
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
    m_handles.reserve(SizeHandleRect::None);

    Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
        delete (*it);
    m_handles.clear();

    SizeHandleRect *shr = new SizeHandleRect(this,SizeHandleRect::LeftTop, this);
    m_handles.push_back(shr);
    shr = new SizeHandleRect(this,SizeHandleRect::RightBottom, this);
    m_handles.push_back(shr);

    updateGeometry();
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
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
    QColor c = QColor(Qt::red);
    painter->setPen(pen());
    painter->drawLine(rect().topLeft(),rect().bottomRight());
}



GraphicsItemGroup::GraphicsItemGroup(QGraphicsItem *parent)
    :GraphicsItem(parent)
{
    // handles
    m_handles.reserve(SizeHandleRect::None);
    for (int i = SizeHandleRect::LeftTop; i <= SizeHandleRect::Left; ++i) {
        SizeHandleRect *shr = new SizeHandleRect(this, static_cast<SizeHandleRect::Direction>(i), this);
        m_handles.push_back(shr);
    }
    updateGeometry();
    setHandlesChildEvents(true);
}

GraphicsItemGroup::~GraphicsItemGroup()
{

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

    item->setFlag(QGraphicsItem::ItemIsMovable, false);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);


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
    //item->d_func()->setIsMemberOfGroup(true);
    prepareGeometryChange();
    itemsBoundingRect |= itemTransform.mapRect(item->boundingRect() | item->childrenBoundingRect());
    updateCoordinate();
    update();
}

void GraphicsItemGroup::removeFromGroup(QGraphicsItem *item)
{
    if (!item) {
        qWarning("QGraphicsItemGroup::removeFromGroup: cannot remove null item");
        return;
    }

    QGraphicsItem *newParent = parentItem();

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

    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

//    item->d_func()->setIsMemberOfGroup(item->group() != 0);

    // ### Quite expensive. But removeFromGroup() isn't called very often.
    prepareGeometryChange();
    itemsBoundingRect = childrenBoundingRect();
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
    updateGeometry();
}

QRectF GraphicsItemGroup::boundingRect() const
{
    return itemsBoundingRect;
}

void GraphicsItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

#if 0
    QPointF origin = mapFromScene(pos());

    QPointF origin1 =transformOriginPoint();

    QColor c1 = QColor(Qt::blue);
  //  c1.setAlpha(180);
    painter->setBrush(c1);
    painter->drawEllipse(origin.x() - 3 , origin.y() - 3 ,6,6);


    QColor c2 = QColor(Qt::green);
  //  c2.setAlpha(180);
    painter->setBrush(c2);
    painter->drawEllipse(origin1.x() - 3 , origin1.y() - 3 ,6,6);
#endif

}

bool GraphicsItemGroup::isObscuredBy(const QGraphicsItem *item) const
{
    return GraphicsItem::isObscuredBy(item);
}

QPainterPath GraphicsItemGroup::opaqueArea() const
{
    return GraphicsItem::opaqueArea() ;
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
    SizeHandleRect *shr = new SizeHandleRect(this, static_cast<SizeHandleRect::Direction>(m_index), this);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
    m_index++;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updateGeometry();
}

void GraphicsBezierCurve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

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


GraphicsBasicItem::GraphicsBasicItem(QGraphicsItem *parent)
    :QGraphicsObject(parent)
    ,m_pen(Qt::black)
    ,m_brush(Qt::NoBrush)
{
    m_pen.setColor(Qt::black);
    m_brush.setColor(Qt::white);
}

GraphicsBasicItem::GraphicsBasicItem(const QString &name, QGraphicsItem *parent)
    :QGraphicsObject(parent)
    ,m_pen(Qt::black)
    ,m_brush(Qt::NoBrush)
{
    m_pen.setColor(Qt::black);
    m_brush.setColor(Qt::white);
}

GraphicsBasicItem::~GraphicsBasicItem()
{

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
    m_endAngle = 360;
}

QPainterPath GraphicsArcItem::shape() const
{
    QPainterPath path;

    if ( m_points.count() == 2  ){
        path.addEllipse(m_points.at(0),m_Radius,m_Radius);
        return path;
    }

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
    if ( m_points.count() > 3) {
        m_points.remove(3);
        delete m_handles.at(3);
        m_handles.remove(3);
    }
}

void GraphicsArcItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{
    if ( dir == 0 ) return;
    GraphicsPolygonItem::resizeTo( dir , point);
    QPointF local = mapFromScene(point);
    if ( m_points.count() ==  2 ){
        qreal rx = abs(local.x() - m_points.at(0).x());
        qreal ry = abs(local.y() - m_points.at(0).y());
        qreal r  = qMax(rx,ry);
        m_Radius = r;
        qreal len_y = local.y() - m_points.at(0).y();
        qreal len_x = local.x() - m_points.at(0).x();
        m_startAngle = -atan2(len_y,len_x)*180/3.1416;
        qDebug() <<" change radius:" << m_Radius << " " << m_startAngle;
    }else if ( m_points.count() > 2){
            qreal startAngle,endAngle;
            qreal len_y = m_points.at(1).y() - m_points.at(0).y();
            qreal len_x = m_points.at(1).x() - m_points.at(0).x();
            m_startAngle = -atan2(len_y,len_x)*180/3.1416;
            len_y = m_points.at(2).y() - m_points.at(0).y();
            len_x = m_points.at(2).x() - m_points.at(0).x();
            m_endAngle = -atan2(len_y,len_x)*180/3.1416;
            qDebug() <<" change angle:" << m_startAngle << " " << m_endAngle;
    }
    prepareGeometryChange();
    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);
    updateGeometry();
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
    updateGeometry();
}

void GraphicsArcItem::updateGeometry()
{
    GraphicsPolygonItem::updateGeometry();
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
    QPainterPath path;
/*
    if ( m_points.count() == 2  ){
        path.addEllipse(m_points.at(0),m_Radius,m_Radius);
        painter->drawPath(path);
        return ;
    }
*/
    painter->drawArc(m_localRect, m_startAngle * 16 , (m_endAngle - m_startAngle) * 16);
}


GraphicsRoundRectItem::GraphicsRoundRectItem(const QRect &rect, QGraphicsItem *parent)
    :GraphicsRectItem(rect,parent),
     m_roundness(16,16)
{
    m_fRatio = 1/3;
    SizeHandleRect *shr = new SizeHandleRect(this, static_cast<SizeHandleRect::Direction>(SizeHandleRect::Extend), this);
    m_handles.push_back(shr);
}

QPainterPath GraphicsRoundRectItem::shape() const
{
    QPainterPath path;
    path.addRoundRect(rect(),m_roundness.x(),m_roundness.y());
    return path;

}

void GraphicsRoundRectItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{
    if ( dir == SizeHandleRect::Extend ){
        QPointF local = mapFromScene(point);
        QRectF rc = rect();
        rc.normalized();
        if (local.x() > rc.right() - 1)
            local.setX(rc.right() - 1);

        else if (local.x() < rc.left() + rc.width() / 2)

            local.setX(rc.left() + rc.width() / 2);

        if (local.y() > rc.bottom() - 1)

            local.setY( rc.bottom() - 1 );

        else if (local.y() < rc.top() + rc.height() / 2)

            local.setY( rc.top() + rc.height() / 2);

        m_roundness.setX( 2 * (rc.right() - local.x()));

        m_roundness.setY( 2 * (rc.bottom() - local.y()));

    }

    GraphicsRectItem::resizeTo(dir,point);
}

void GraphicsRoundRectItem::updateGeometry()
{
    const QRectF &geom = this->boundingRect();

    const int w = SELECTION_HANDLE_SIZE;
    const int h = SELECTION_HANDLE_SIZE;

    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;;
        switch (hndl->dir()) {
        case SizeHandleRect::LeftTop:
            hndl->move(geom.x() , geom.y());
            break;
        case SizeHandleRect::Top:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() );
            break;
        case SizeHandleRect::RightTop:
            hndl->move(geom.x() + geom.width() , geom.y() );
            break;
        case SizeHandleRect::Right:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() / 2 );
            break;
        case SizeHandleRect::RightBottom:
            hndl->move(geom.x() + geom.width() , geom.y() + geom.height() );
            break;
        case SizeHandleRect::Bottom:
            hndl->move(geom.x() + geom.width() / 2 , geom.y() + geom.height() );
            break;
        case SizeHandleRect::LeftBottom:
            hndl->move(geom.x() , geom.y() + geom.height() );
            break;
        case SizeHandleRect::Left:
            hndl->move(geom.x() , geom.y() + geom.height() / 2 );
            break;
        case SizeHandleRect::Extend:
            hndl->move( geom.bottomRight().x() - m_roundness.x() / 2, geom.bottomRight().y() - m_roundness.y()/2);
            break;
        default:
            break;
        }
    }
}

void GraphicsRoundRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor c = QColor(Qt::red);

    /*
    QLinearGradient result(rect().topLeft(), rect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);
    */
    painter->setBrush(brush());

    painter->setPen(pen());

    painter->drawRoundRect(rect(),m_roundness.x(),m_roundness.y());

}


GraphicsPolygonItem::GraphicsPolygonItem(QGraphicsItem *parent)
    :GraphicsItem(parent)
{
    // handles
    m_handles.reserve(SizeHandleRect::None);

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
    SizeHandleRect *shr = new SizeHandleRect(this, static_cast<SizeHandleRect::Direction>(dir), this);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();//RecalcBounds();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updateGeometry();
}

void GraphicsPolygonItem::resizeTo(SizeHandleRect::Direction dir, const QPointF &point)
{
    QPointF pt = mapFromScene(point);
    m_points[(int)dir] = pt;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();//RecalcBounds();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updateGeometry();
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

    updateGeometry();

}

void GraphicsPolygonItem::endPoint(const QPointF & point)
{
    int nPoints = m_points.count();
    if( nPoints > 2 && m_points[nPoints-1] == m_points[nPoints-2] ||
        m_points[nPoints-1].x() - 1 == m_points[nPoints-2].x() &&
        m_points[nPoints-1].y() == m_points[nPoints-2].y()){
        delete m_handles[nPoints-1];
        m_points.remove(nPoints-1);
        m_handles.remove(nPoints-1);
    }
}

QRectF GraphicsPolygonItem::RecalcBounds()
{
    QRectF bounds(m_points[0],QSize(0,0));
    for (int i = 1; i < m_points.count(); ++i)
    {
        if (m_points[i].x() < bounds.left())
            bounds.setLeft(m_points[i].x());
        if (m_points[i].x() > bounds.right())
            bounds.setRight(m_points[i].x());
        if (m_points[i].y() < bounds.top())
            bounds.setTop(m_points[i].y());
        if (m_points[i].y() > bounds.bottom())
            bounds.setBottom(m_points[i].y());
    }
    return bounds;
}

void GraphicsPolygonItem::updateGeometry()
{
    const Handles::iterator hend =  m_handles.end();
    const int w = SELECTION_HANDLE_SIZE;
    const int h = SELECTION_HANDLE_SIZE;

    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
        SizeHandleRect *hndl = *it;
        int idx = (int)hndl->dir();
        hndl->move(m_points[idx].x() ,m_points[idx].y() );
    }
}

void GraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor c = QColor(Qt::red);

    /*
    QLinearGradient result(rect().topLeft(), rect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);
*/
    painter->setBrush(brush());

    painter->setPen(pen());
    painter->drawPolygon(m_points);


    /*
    painter->setBrush(Qt::NoBrush);

    painter->setPen(Qt::DashLine);
    painter->setPen(Qt::green);
    painter->drawRect(m_points.boundingRect());

    QPointF pos = m_points.boundingRect().center();
    painter->setBrush(Qt::green);
    painter->drawEllipse(pos,6,6);


    QPointF pos2 = transformOriginPoint();
    painter->setBrush(Qt::black);
    painter->drawEllipse(pos2,6,6);
    */
}
