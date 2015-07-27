#include "drawobj.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QCursor>
#include <QDebug>
#include <QGraphicsEffect>
#include <QMatrix4x4>
#include <QGraphicsTransform>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <cmath>
#include "drawscene.h"

ShapeMimeData::ShapeMimeData(QList<QGraphicsItem *> items)
{
    foreach (QGraphicsItem *item , items ) {
       AbstractShape *sp = qgraphicsitem_cast<AbstractShape*>(item);
       m_items.append(sp->copy());
    }
}
ShapeMimeData::~ShapeMimeData()
{
    foreach (QGraphicsItem *item , m_items ) {
        delete item;
    }
    m_items.clear();
}

QList<QGraphicsItem *> ShapeMimeData::items() const
{
    return m_items;
}

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

static void qt_graphicsItem_highlightSelected(
    QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
        return;

    const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
        return;

    qreal itemPenWidth;
    switch (item->type()) {
        case QGraphicsEllipseItem::Type:
            itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
            break;
        case QGraphicsPathItem::Type:
            itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
            break;
        case QGraphicsPolygonItem::Type:
            itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
            break;
        case QGraphicsRectItem::Type:
            itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
            break;
        case QGraphicsSimpleTextItem::Type:
            itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
            break;
        case QGraphicsLineItem::Type:
            itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
            break;
        default:
            itemPenWidth = 1.0;
    }
    const qreal pad = itemPenWidth / 2;

    const qreal penWidth = 0; // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
        fgcolor.red()   > 127 ? 0 : 255,
        fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue()  > 127 ? 0 : 255);


    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(-pad, -pad, pad, pad));

    painter->setPen(QPen(QColor("lightskyblue"), 0, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(item->boundingRect().adjusted(-pad, -pad, pad, pad));

}

GraphicsItem::GraphicsItem(QGraphicsItem *parent)
    :AbstractShapeType<QGraphicsItem>(parent)
{
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//    effect->setBlurRadius(8);
//    setGraphicsEffect(effect);
//    setCacheMode(QGraphicsItem::ItemCoordinateCache);

    // handles
    m_handles.reserve(Left);
    for (int i = LeftTop; i <= Left; ++i) {
        SizeHandleRect *shr = new SizeHandleRect(this,i);
        m_handles.push_back(shr);
    }


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
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(parentItem());
        if (!g)
            setState(value.toBool() ? SelectionHandleActive : SelectionHandleOff);
        else{
            setSelected(false);
            return QVariant::fromValue<bool>(false);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}


GraphicsRectItem::GraphicsRectItem(const QRect & rect , bool isRound , QGraphicsItem *parent)
    :GraphicsItem(parent)
    ,m_isRound(isRound)
    ,m_fRatioX(1/10.0)
    ,m_fRatioY(1/3.0)
{

    m_width = rect.width();
    m_height = rect.height();
    m_initialRect = QRect(1,1,1,1);
    m_localRect = m_initialRect;
    if( m_isRound ){
        SizeHandleRect *shr = new SizeHandleRect(this, 9 , true);
        m_handles.push_back(shr);
        shr = new SizeHandleRect(this, 10 , true);
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
    double rx,ry;
    if(m_fRatioX<=0)
       rx=0;
    else {
        rx = m_width * m_fRatioX + 0.5;
    }
    if ( m_fRatioY <=0 )
        ry = 0;
    else
        ry = m_height * m_fRatioY + 0.5;
    if ( m_isRound )
        path.addRoundedRect(rect(),rx,ry);
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
    case 9:
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
        m_fRatioY = std::abs(((float)(delta1.top()-y)))/H;
    }
        break;
    case 10:
    {
        QRectF delta1 = rect();
        int x = local.x();
        if(x < delta1.center().x() )
            x = delta1.center().x();
        if(x>delta1.right())
            x=delta1.right();
        int W= delta1.width();
        if(W==0)
            W=1;
        m_fRatioX = std::abs(((float)(delta1.right()-x)))/W;
        break;
    }
   default:
        break;
    }

    m_width = delta1.normalized().width();
    m_height = delta1.normalized().height();

    prepareGeometryChange();
    m_localRect = delta1;
    updatehandles();
}

void GraphicsRectItem::stretch(int handle , double sx, double sy, const QPointF & origin)
{
    QTransform trans = transform() ;
    switch (handle) {
    case Right:
    case Left:
        sy = 1;
        break;
    case Top:
    case Bottom:
        sx = 1;
        break;
    default:
        break;
    }
    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());

    prepareGeometryChange();
    m_localRect = trans.mapRect(m_initialRect);
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}

void GraphicsRectItem::updateCoordinate()
{
    QPointF pt1,pt2,delta;
    if (parentItem()==NULL)
    {
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
    m_width = m_localRect.width();
    m_height = m_localRect.height();

    m_initialRect = m_localRect;
}

void GraphicsRectItem::move(const QPointF &point)
{
    moveBy(point.x(),point.y());
}

QGraphicsItem *GraphicsRectItem::copy() const
{
    GraphicsRectItem * item = new GraphicsRectItem( rect().toRect(),m_isRound);
    item->m_width = width();
    item->m_height = height();
    item->setPos(pos().x(),pos().y());
    item->setPen(pen());
    item->setBrush(brush());
    item->setTransform(transform());
    item->setTransformOriginPoint(transformOriginPoint());
    item->setRotation(rotation());
    item->setScale(scale());
    item->setZValue(zValue()+0.1);
    item->m_fRatioY = m_fRatioY;
    item->m_fRatioX = m_fRatioX;
    item->updateCoordinate();
    return item;
}

void GraphicsRectItem::updatehandles()
{
    const QRectF &geom = this->boundingRect();
    GraphicsItem::updatehandles();
    if ( m_isRound ){
        m_handles[8]->move( geom.right() , geom.top() + geom.height() * m_fRatioY );
        m_handles[9]->move( geom.right() - geom.width() * m_fRatioX , geom.top());
    }
}


void GraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

   painter->setPen(pen());
   painter->setBrush(brush());
   double rx,ry;
   if(m_fRatioX<=0)
      rx=0;
   else {
       rx = m_width * m_fRatioX + 0.5;
   }
   if ( m_fRatioY <=0 )
       ry = 0;
   else
       ry = m_height * m_fRatioY + 0.5;
   if ( m_isRound )
       painter->drawRoundedRect(rect(),rx,ry);
   else
       painter->drawRect(rect().toRect());


   if (option->state & QStyle::State_Selected)
       qt_graphicsItem_highlightSelected(this, painter, option);
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

QGraphicsItem *GraphicsEllipseItem::copy() const
{
    GraphicsEllipseItem * item = new GraphicsEllipseItem( rect().toRect());
    item->m_width = width();
    item->m_height = height();
    item->setPos(pos().x(),pos().y());
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

void GraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor c = brush();
    QRectF rc = rect();

    qreal radius = qMax(rc.width(),rc.height());
    QRadialGradient result(rc.center(),radius);
    result.setColorAt(0, c.light(200));
    result.setColorAt(0.5, c.dark(150));
    result.setColorAt(1, c);
    painter->setPen(pen());
    QBrush b(result);
    b.setStyle(Qt::RadialGradientPattern);
    painter->setBrush(b);

//    painter->setBrush(brush());
    painter->drawEllipse(rc);

    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);

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

    SizeHandleRect *shr = new SizeHandleRect(this,LeftTop);
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

QGraphicsItem *GraphicsLineItem::copy() const
{
    GraphicsLineItem * item = new GraphicsLineItem();
    item->m_width = width();
    item->m_height = height();
    item->m_localRect = m_localRect;
    item->setPos(pos().x(),pos().y());
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
        SizeHandleRect *shr = new SizeHandleRect(this, i);
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
    m_initialRect = itemsBoundingRect;
    m_width = itemsBoundingRect.width();
    m_height = itemsBoundingRect.height();
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
    m_initialRect = itemsBoundingRect;
    m_width = itemsBoundingRect.width();
    m_height = itemsBoundingRect.height();
}

QRectF GraphicsItemGroup::boundingRect() const
{
    return itemsBoundingRect;
}

GraphicsItemGroup::~GraphicsItemGroup()
{

}

QGraphicsItem *GraphicsItemGroup::copy() const
{
    GraphicsItemGroup *item = 0;
    QList<QGraphicsItem*> copylist = copyChildItems();
    item = createGroup(copylist);
    item->setPos(pos().x(),pos().y());
    item->setPen(pen());
    item->setBrush(brush());
    item->setTransform(transform());
    item->setTransformOriginPoint(transformOriginPoint());
    item->setRotation(rotation());
    item->setScale(scale());
    item->setZValue(zValue()+0.1);
    item->updateCoordinate();
    item->m_width = m_width;
    item->m_height = m_height;
    return item;
}

void GraphicsItemGroup::resize(int dir, const QPointF &delta)
{
    QPointF local = mapFromParent(delta);
    if ( dir < Left ) return ;
    if ( dir == 9 ) {

    }
    updatehandles();
}

void GraphicsItemGroup::stretch(int handle, double sx, double sy, const QPointF &origin)
{
    QTransform trans ;
    switch (handle) {
    case Right:
    case Left:
        sy = 1;
        break;
    case Top:
    case Bottom:
        sx = 1;
        break;
    default:
        break;
    }
    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());


    foreach (QGraphicsItem *item , childItems()) {
         AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(item);
         if (ab && !qgraphicsitem_cast<SizeHandleRect*>(ab))
             ab->stretch(handle,sx,sy,ab->mapFromParent(origin));
    }

    prepareGeometryChange();
    itemsBoundingRect = trans.mapRect(m_initialRect);
    m_width = itemsBoundingRect.width();
    m_height = itemsBoundingRect.height();
    updatehandles();
}

void GraphicsItemGroup::updateCoordinate()
{
    QPointF pt1,pt2,delta;
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(boundingRect().center());
    delta = pt1 - pt2;
 //   itemsBoundingRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);
    m_initialRect = itemsBoundingRect;

//    setTransform(transform().translate(delta.x(),delta.y()));
//    setTransformOriginPoint(boundingRect().center());
//    moveBy(-delta.x(),-delta.y());
  //  setTransform(transform().translate(-delta.x(),-delta.y()));


    foreach (QGraphicsItem *item , childItems()) {
         AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(item);
         if (ab && !qgraphicsitem_cast<SizeHandleRect*>(ab))
             ab->updateCoordinate();
    }

    updatehandles();

}

GraphicsItemGroup *GraphicsItemGroup::createGroup(const QList<QGraphicsItem *> &items) const
{
    // Build a list of the first item's ancestors
    QList<QGraphicsItem *> ancestors;
    int n = 0;
    if (!items.isEmpty()) {
        QGraphicsItem *parent = items.at(n++);
        while ((parent = parent->parentItem()))
            ancestors.append(parent);
    }

    // Find the common ancestor for all items
    QGraphicsItem *commonAncestor = 0;
    if (!ancestors.isEmpty()) {
        while (n < items.size()) {
            int commonIndex = -1;
            QGraphicsItem *parent = items.at(n++);
            do {
                int index = ancestors.indexOf(parent, qMax(0, commonIndex));
                if (index != -1) {
                    commonIndex = index;
                    break;
                }
            } while ((parent = parent->parentItem()));

            if (commonIndex == -1) {
                commonAncestor = 0;
                break;
            }

            commonAncestor = ancestors.at(commonIndex);
        }
    }
    // Create a new group at that level
    GraphicsItemGroup *group = new GraphicsItemGroup(commonAncestor);
    foreach (QGraphicsItem *item, items){
        item->setSelected(false);
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
             group->addToGroup(item);
    }
    return group;
}

QList<QGraphicsItem *> GraphicsItemGroup::copyChildItems() const
{
    QList<QGraphicsItem*> copylist ;
    foreach (QGraphicsItem * shape , childItems() ) {
        AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(shape);
        if ( ab && !qgraphicsitem_cast<SizeHandleRect*>(ab)){
            QGraphicsItem * cp = ab->copy();
            //if ( !cp->scene() )
            //    scene()->addItem(cp);
            copylist.append(cp);
        }
    }
    return copylist;
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
        case 9:
            hndl->move(transformOriginPoint().x(),transformOriginPoint().y());
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
//    Q_UNUSED(option);
    Q_UNUSED(widget);
//    Q_UNUSED(painter);
    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}

GraphicsBezierCurve::GraphicsBezierCurve(QGraphicsItem *parent)
    :GraphicsPolygonItem(parent)
{
}

QPainterPath GraphicsBezierCurve::shape() const
{
    QPainterPath path;
    path.moveTo(m_points.at(0));
    int i=1;
    while (i + 2 < m_points.size()) {
        path.cubicTo(m_points.at(i), m_points.at(i+1), m_points.at(i+2));
        i += 3;
    }
    while (i < m_points.size()) {
        path.lineTo(m_points.at(i));
        ++i;
    }

    return path;
}

/*
void GraphicsBezierCurve::addPoint(const QPointF &point)
{
    m_points.append(mapFromScene(point));
    SizeHandleRect *shr = new SizeHandleRect(this, Left + m_index, true);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
    m_index++;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}
*/

QGraphicsItem *GraphicsBezierCurve::copy() const
{
    GraphicsBezierCurve * item = new GraphicsBezierCurve( );
    item->m_width = width();
    item->m_height = height();
    item->m_points = m_points;

    for ( int i = 0 ; i < m_points.size() ; ++i ){
        item->m_handles.push_back(new SizeHandleRect(item,Left+i+1,true));
    }
    item->setPos(pos().x(),pos().y());
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

void GraphicsBezierCurve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    painter->setPen(pen());
    painter->setBrush(brush());
    path.moveTo(m_points.at(0));

    int i=1;
    while (i + 2 < m_points.size()) {
        path.cubicTo(m_points.at(i), m_points.at(i+1), m_points.at(i+2));
        i += 3;
    }
    while (i < m_points.size()) {
        path.lineTo(m_points.at(i));
        ++i;
    }
    painter->drawPath(path);

   if (option->state & QStyle::State_Selected){
       painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
       painter->setBrush(Qt::NoBrush);
       painter->drawPolyline(m_points);
    }

   if (option->state & QStyle::State_Selected)
       qt_graphicsItem_highlightSelected(this, painter, option);
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
    :GraphicsRectItem(QRect(0,0,0,0),parent)
{
    m_Radius = 0;
    m_startAngle = 20;
    m_spanAngle  = 380;
    SizeHandleRect *shr = new SizeHandleRect(this, 9 , true);
    m_handles.push_back(shr);
    shr = new SizeHandleRect(this, 10 , true);
    m_handles.push_back(shr);

}

QPainterPath GraphicsArcItem::shape() const
{
    QPainterPath path;
    int startAngle = m_startAngle <= m_spanAngle ? m_startAngle : m_spanAngle;
    int endAngle = m_startAngle >= m_spanAngle ? m_startAngle : m_spanAngle;
    if(endAngle - startAngle > 360)
        endAngle = startAngle + 360;

    if (m_localRect.isNull())
        return path;
    if ((endAngle - startAngle) % 360 != 0 ) {
        path.moveTo(m_localRect.center());
        path.arcTo(m_localRect, startAngle, endAngle - startAngle);
    } else {
        path.addEllipse(m_localRect);
    }
    path.closeSubpath();
    return path;
}

void GraphicsArcItem::resize(int dir, const QPointF & delta)
{
    QPointF local = mapFromScene(delta);
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
    case 9:
    {
        qreal len_y = local.y() - m_localRect.center().y();
        qreal len_x = local.x() - m_localRect.center().x();
        m_startAngle = -atan2(len_y,len_x)*180/3.1416;
    }
        break;
    case 10:
    {
        qreal len_y = local.y() - m_localRect.center().y();
        qreal len_x = local.x() - m_localRect.center().x();
        m_spanAngle = -atan2(len_y,len_x)*180/3.1416;
        break;
    }
   default:
        break;
    }
    ;
    m_width = delta1.normalized().width();
    m_height = delta1.normalized().height();
    m_Radius = qMax(m_width/2,m_height/2);
    prepareGeometryChange();
    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);
    if ( m_startAngle > m_spanAngle )
        m_startAngle-=360;
    if ( m_spanAngle < m_startAngle ){
        qreal tmp = m_spanAngle;
        m_spanAngle = m_startAngle;
        m_startAngle = tmp;
    }

    if ( std::abs(m_spanAngle-m_startAngle) > 360 ){
        m_startAngle = 20;
        m_spanAngle = 380;
    }

    updatehandles();
}

QRectF GraphicsArcItem::boundingRect() const
{
    return shape().controlPointRect();
}

void GraphicsArcItem::updateCoordinate()
{

    QPointF pt1,pt2,delta;
    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(m_localRect.center());
    delta = pt1 - pt2;

    prepareGeometryChange();

    m_Radius = qMax(m_width/2,m_height/2);
    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);
    m_initialRect = m_localRect;
    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(m_localRect.center());
    moveBy(-delta.x(),-delta.y());
    setTransform(transform().translate(-delta.x(),-delta.y()));
    updatehandles();
}

QGraphicsItem *GraphicsArcItem::copy() const
{
    GraphicsArcItem * item = new GraphicsArcItem( );
    item->m_width = width();
    item->m_height = height();

    item->m_startAngle = m_startAngle;
    item->m_spanAngle   = m_spanAngle;
    item->m_Radius = m_Radius;

    item->setPos(pos().x(),pos().y());
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

void GraphicsArcItem::stretch(int handle, double sx, double sy, const QPointF &origin)
{
    QTransform trans;
    switch (handle) {
    case Right:
    case Left:
        sy = 1;
        break;
    case Top:
    case Bottom:
        sx = 1;
        break;
    default:
        break;
    }
    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());

    prepareGeometryChange();
    m_localRect = trans.mapRect(m_initialRect);
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    m_Radius = qMax(m_width/2,m_height/2);
    m_localRect = QRectF(-m_Radius,-m_Radius,m_Radius*2,m_Radius*2);
    if ( m_startAngle > m_spanAngle )
        m_startAngle-=360;
    if ( m_spanAngle < m_startAngle ){
        qreal tmp = m_spanAngle;
        m_spanAngle = m_startAngle;
        m_startAngle = tmp;
    }

    if ( std::abs(m_spanAngle-m_startAngle) > 360 ){
        m_startAngle = 20;
        m_spanAngle = 380;
    }

    updatehandles();
}

void GraphicsArcItem::updatehandles()
{
    GraphicsItem::updatehandles();
    qreal x = m_Radius * cos( -m_startAngle * 3.1416 / 180 );
    qreal y = m_Radius * sin( -m_startAngle * 3.1416 / 180);
    m_handles.at(8)->move(x,y);
    x = m_Radius * cos( -m_spanAngle * 3.1416 / 180);
    y = m_Radius * sin(-m_spanAngle * 3.1416 / 180);
    m_handles.at(9)->move(x,y);
}

void GraphicsArcItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(pen());
    painter->setBrush(brush());
    int startAngle = m_startAngle <= m_spanAngle ? m_startAngle : m_spanAngle;
    int endAngle = m_startAngle >= m_spanAngle ? m_startAngle : m_spanAngle;
    if(endAngle - startAngle > 360)
        endAngle = startAngle + 360;

    if (qAbs(endAngle-startAngle) % (360) == 0)
        painter->drawEllipse(m_localRect);
    else
        painter->drawPie(m_localRect, startAngle * 16 , (endAngle-startAngle) * 16);


    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}

GraphicsPolygonItem::GraphicsPolygonItem(QGraphicsItem *parent)
    :GraphicsItem(parent)
{
    // handles
    m_points.clear();

}

QRectF GraphicsPolygonItem::boundingRect() const
{
    return shape().controlPointRect();
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
    int dir = m_points.count();
    SizeHandleRect *shr = new SizeHandleRect(this, dir+Left, true);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
}

void GraphicsPolygonItem::resize(int dir, const QPointF &delta)
{
    QPointF pt = mapFromScene(delta);
    if ( dir <= Left ) return ;
    m_points[dir - Left -1] = pt;
    prepareGeometryChange();
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    m_initialPoints = m_points;
    updatehandles();
}

void GraphicsPolygonItem::stretch(int handle, double sx, double sy, const QPointF &origin)
{
    QTransform trans;
    switch (handle) {
    case Right:
    case Left:
        sy = 1;
        break;
    case Top:
    case Bottom:
        sx = 1;
        break;
    default:
        break;
    }
    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());

    prepareGeometryChange();
    m_points = trans.map(m_initialPoints);
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}

void GraphicsPolygonItem::updateCoordinate()
{

    QPointF pt1,pt2,delta;
    QPolygonF pts = mapToScene(m_points);
    if (parentItem()==NULL)
    {
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
    }
    m_initialPoints = m_points;
    updatehandles();

}

void GraphicsPolygonItem::endPoint(const QPointF & point)
{
    Q_UNUSED(point);
    int nPoints = m_points.count();
    if( nPoints > 2 && (m_points[nPoints-1] == m_points[nPoints-2] ||
        m_points[nPoints-1].x() - 1 == m_points[nPoints-2].x() &&
        m_points[nPoints-1].y() == m_points[nPoints-2].y())){
        delete m_handles[Left + nPoints-1];
        m_points.remove(nPoints-1);
        m_handles.resize(Left + nPoints-1);
    }
    m_initialPoints = m_points;
}

QGraphicsItem *GraphicsPolygonItem::copy() const
{
    GraphicsPolygonItem * item = new GraphicsPolygonItem( );
    item->m_width = width();
    item->m_height = height();
    item->m_points = m_points;

    for ( int i = 0 ; i < m_points.size() ; ++i ){
        item->m_handles.push_back(new SizeHandleRect(item,Left+i+1,true));
    }

    item->setPos(pos().x(),pos().y());
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

void GraphicsPolygonItem::updatehandles()
{
    GraphicsItem::updatehandles();

    for ( int i = 0 ; i < m_points.size() ; ++i ){
        m_handles[Left+i]->move(m_points[i].x() ,m_points[i].y() );
    }
}

void GraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor c = brush();
    QLinearGradient result(boundingRect().topLeft(), boundingRect().topRight());
    result.setColorAt(0, c.dark(150));
    result.setColorAt(0.5, c.light(200));
    result.setColorAt(1, c.dark(150));
    painter->setBrush(result);

    painter->setPen(pen());
    painter->drawPolygon(m_points);

    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}


