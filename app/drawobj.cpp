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
       m_items.append(sp->duplicate());
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

    /*
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(4);
    setGraphicsEffect(effect);
   */
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


QPixmap GraphicsItem::image() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    setPen(QPen(Qt::black));
    setBrush(Qt::white);
    QStyleOptionGraphicsItem *styleOption = new QStyleOptionGraphicsItem;
//    painter.translate(m_localRect.center().x(),m_localRect.center().y());
    paint(&painter,styleOption);
    delete styleOption;
    return pixmap;
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

bool GraphicsItem::readBaseAttributes(QXmlStreamReader *xml)
{
    qreal x = xml->attributes().value(tr("x")).toDouble();
    qreal y = xml->attributes().value(tr("y")).toDouble();
    m_width = xml->attributes().value("width").toDouble();
    m_height = xml->attributes().value("height").toDouble();
    setZValue(xml->attributes().value("z").toDouble());
    setRotation(xml->attributes().value("rotate").toDouble());
    setPos(x,y);
    return true;
}

bool GraphicsItem::writeBaseAttributes(QXmlStreamWriter *xml)
{
    xml->writeAttribute(tr("rotate"),QString("%1").arg(rotation()));
    xml->writeAttribute(tr("x"),QString("%1").arg(pos().x()));
    xml->writeAttribute(tr("y"),QString("%1").arg(pos().y()));
    xml->writeAttribute(tr("z"),QString("%1").arg(zValue()));
    xml->writeAttribute(tr("width"),QString("%1").arg(m_width));
    xml->writeAttribute(tr("height"),QString("%1").arg(m_height));
    return true;
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
    /*
    else if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right()-boundingRect().width()/2, qMax(newPos.x(), rect.left()+boundingRect().width()/2)));
            newPos.setY(qMin(rect.bottom()-boundingRect().height()/2, qMax(newPos.y(), rect.top()+boundingRect().height()/2)));
            return newPos;
        }
    }
    */
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
    m_initialRect = rect;
    m_localRect = m_initialRect;
    m_localRect = rect;
    m_originPoint = QPointF(0,0);
    if( m_isRound ){
        SizeHandleRect *shr = new SizeHandleRect(this, 9 , true);
        m_handles.push_back(shr);
        shr = new SizeHandleRect(this, 10 , true);
        m_handles.push_back(shr);
        //shr = new SizeHandleRect(this, 11 , true);
        //m_handles.push_back(shr);
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

void GraphicsRectItem::control(int dir, const QPointF & delta)
{
    QPointF local = mapFromParent(delta);
    switch (dir) {
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
    case 11:
    {
//        setTransform(transform().translate(-local.x(),-local.y()));
//        setTransformOriginPoint(local.x(),local.y());
//        setTransform(transform().translate(local.x(),local.y()));
        m_originPoint = local;
    }
        break;
   default:
        break;
    }
    prepareGeometryChange();
    updatehandles();
}

void GraphicsRectItem::stretch(int handle , double sx, double sy, const QPointF & origin)
{
    QTransform trans  ;
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

    opposite_ = origin;

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

    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(m_localRect.center());
    delta = pt1 - pt2;

    if (!parentItem() ){
        prepareGeometryChange();
        m_localRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);
        m_width = m_localRect.width();
        m_height = m_localRect.height();
        setTransform(transform().translate(delta.x(),delta.y()));
        setTransformOriginPoint(m_localRect.center());
        moveBy(-delta.x(),-delta.y());
        setTransform(transform().translate(-delta.x(),-delta.y()));
        opposite_ = QPointF(0,0);
        updatehandles();
    }
    m_initialRect = m_localRect;
}

void GraphicsRectItem::move(const QPointF &point)
{
    moveBy(point.x(),point.y());
}

QGraphicsItem *GraphicsRectItem::duplicate() const
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

bool GraphicsRectItem::loadFromXml(QXmlStreamReader * xml )
{
    m_isRound = (xml->name() == tr("roundrect"));
    if ( m_isRound ){
        m_fRatioX = xml->attributes().value(tr("rx")).toDouble();
        m_fRatioY = xml->attributes().value(tr("ry")).toDouble();
    }
    readBaseAttributes(xml);
    updateCoordinate();
    xml->skipCurrentElement();
    return true;
}

bool GraphicsRectItem::saveToXml(QXmlStreamWriter * xml)
{
    if ( m_isRound ){
        xml->writeStartElement(tr("roundrect"));
        xml->writeAttribute(tr("rx"),QString("%1").arg(m_fRatioX));
        xml->writeAttribute(tr("ry"),QString("%1").arg(m_fRatioY));
    }
    else
        xml->writeStartElement(tr("rect"));

    writeBaseAttributes(xml);
    xml->writeEndElement();
    return true;
}

void GraphicsRectItem::updatehandles()
{
    const QRectF &geom = this->boundingRect();
    GraphicsItem::updatehandles();
    if ( m_isRound ){
        m_handles[8]->move( geom.right() , geom.top() + geom.height() * m_fRatioY );
        m_handles[9]->move( geom.right() - geom.width() * m_fRatioX , geom.top());
        //m_handles[10]->move(m_originPoint.x(),m_originPoint.y());
    }
}

static
QRectF RecalcBounds(const QPolygonF&  pts)
{
    QRectF bounds(pts[0], QSize(0, 0));
    for (int i = 1; i < pts.count(); ++i)
    {
        if (pts[i].x() < bounds.left())
            bounds.setLeft(pts[i].x());
        if (pts[i].x() > bounds.right())
            bounds.setRight(pts[i].x());
        if (pts[i].y() < bounds.top())
            bounds.setTop(pts[i].y());
        if (pts[i].y() > bounds.bottom())
            bounds.setBottom (pts[i].y());
    }
    bounds = bounds.normalized();
    return bounds;
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

   painter->setPen(Qt::blue);
   painter->drawLine(QLine(QPoint(opposite_.x()-6,opposite_.y()),QPoint(opposite_.x()+6,opposite_.y())));
   painter->drawLine(QLine(QPoint(opposite_.x(),opposite_.y()-6),QPoint(opposite_.x(),opposite_.y()+6)));


   if (option->state & QStyle::State_Selected)
       qt_graphicsItem_highlightSelected(this, painter, option);
/*

   QPolygonF pts;
   pts<<m_localRect.topLeft()<<m_localRect.topRight()<<m_localRect.bottomRight()<<m_localRect.bottomLeft();
   pts = mapToScene(pts);
   QRectF bound = RecalcBounds(pts);


   qDebug()<<m_localRect<<bound;
    pts.clear();
   pts<<bound.topLeft()<<bound.topRight()<<bound.bottomRight()<<bound.bottomLeft();
   pts = mapFromScene(pts);
   if ( scene() ){
   painter->save();
   painter->setPen(Qt::blue);
   painter->setBrush(Qt::NoBrush);
   painter->drawPolygon(pts);
   painter->restore();
   }
*/
}

GraphicsLineItem::GraphicsLineItem(QGraphicsItem *parent)
    :GraphicsPolygonItem(parent)
{
    m_pen = QPen(Qt::black);
    // handles
    m_handles.reserve(Left);

    Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it)
       delete (*it);
    m_handles.clear();
}

QPainterPath GraphicsLineItem::shape() const
{
    QPainterPath path;
    if ( m_points.size() > 1 ){
        path.moveTo(m_points.at(0));
        path.lineTo(m_points.at(1));
    }
    return qt_graphicsItem_shapeFromPath(path,pen());
}

QGraphicsItem *GraphicsLineItem::duplicate() const
{
    GraphicsLineItem * item = new GraphicsLineItem();
    item->m_width = width();
    item->m_height = height();
    item->m_points = m_points;
    item->m_initialPoints = m_initialPoints;
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

void GraphicsLineItem::addPoint(const QPointF &point)
{
    m_points.append(mapFromScene(point));
    int dir = m_points.count();
    SizeHandleRect *shr = new SizeHandleRect(this, dir+Left, dir == 1 ? false : true);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
}


void GraphicsLineItem::endPoint(const QPointF &point)
{
    Q_UNUSED(point);
    int nPoints = m_points.count();
    if( nPoints > 2 && (m_points[nPoints-1] == m_points[nPoints-2] ||
        m_points[nPoints-1].x() - 1 == m_points[nPoints-2].x() &&
        m_points[nPoints-1].y() == m_points[nPoints-2].y())){
        delete m_handles[ nPoints-1];
        m_points.remove(nPoints-1);
        m_handles.resize(nPoints-1);
    }
    m_initialPoints = m_points;
}

QPointF GraphicsLineItem::opposite(int handle)
{
    QPointF pt;
    switch (handle) {
    case Right:
    case Left:
    case Top:
    case LeftTop:
    case RightTop:
        pt = m_handles[1]->pos();
        break;
    case RightBottom:
    case LeftBottom:
    case Bottom:
        pt = m_handles[0]->pos();
        break;
     }
    return pt;
}

void GraphicsLineItem::stretch(int handle, double sx, double sy, const QPointF &origin)
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
    m_localRect = m_points.boundingRect();
    m_width = m_localRect.width();
    m_height = m_localRect.height();
    updatehandles();
}

bool GraphicsLineItem::loadFromXml(QXmlStreamReader *xml)
{
    readBaseAttributes(xml);
    while(xml->readNextStartElement()){
        if (xml->name()=="point"){
            qreal x = xml->attributes().value("x").toDouble();
            qreal y = xml->attributes().value("y").toDouble();
            m_points.append(QPointF(x,y));
            int dir = m_points.count();
            SizeHandleRect *shr = new SizeHandleRect(this, dir+Left, dir == 1 ? false : true);
            m_handles.push_back(shr);
            xml->skipCurrentElement();
        }else
            xml->skipCurrentElement();
    }
    updatehandles();
    return true;
}

bool GraphicsLineItem::saveToXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("line");
    writeBaseAttributes(xml);
    for ( int i = 0 ; i < m_points.count();++i){
        xml->writeStartElement("point");
        xml->writeAttribute("x",QString("%1").arg(m_points[i].x()));
        xml->writeAttribute("y",QString("%1").arg(m_points[i].y()));
        xml->writeEndElement();
    }
    xml->writeEndElement();
    return true;
}

void GraphicsLineItem::updatehandles()
{
    for ( int i = 0 ; i < m_points.size() ; ++i ){
        m_handles[i]->move(m_points[i].x() ,m_points[i].y() );
    }
}

void GraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(pen());
    if ( m_points.size() > 1)
    painter->drawLine(m_points.at(0),m_points.at(1));
}

GraphicsItemGroup::GraphicsItemGroup(QGraphicsItem *parent)
    :AbstractShapeType <QGraphicsItemGroup>(parent),
      m_parent(parent)
{
    itemsBoundingRect = QRectF();
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


QRectF GraphicsItemGroup::boundingRect() const
{
    return itemsBoundingRect;
}

GraphicsItemGroup::~GraphicsItemGroup()
{

}

bool GraphicsItemGroup::loadFromXml(QXmlStreamReader *xml)
{
//    qDebug()<<"GraphicsItemGroup::loadFromXml";
    return true;
}

bool GraphicsItemGroup::saveToXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("group");
    xml->writeAttribute(tr("x"),QString("%1").arg(pos().x()));
    xml->writeAttribute(tr("y"),QString("%1").arg(pos().y()));
    xml->writeAttribute(tr("rotate"),QString("%1").arg(rotation()));

    foreach (QGraphicsItem * item , childItems()) {
        removeFromGroup(item);
        AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(item);
        if ( ab &&!qgraphicsitem_cast<SizeHandleRect*>(ab)){
            ab->updateCoordinate();
            ab->saveToXml(xml);
        }
        addToGroup(item);
    }
    xml->writeEndElement();
    return true;
}

QGraphicsItem *GraphicsItemGroup::duplicate() const
{
    GraphicsItemGroup *item = 0;
    QList<QGraphicsItem*> copylist = duplicateItems();
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

void GraphicsItemGroup::control(int dir, const QPointF &delta)
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

    foreach (QGraphicsItem *item , childItems()) {
         AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(item);
         if (ab && !qgraphicsitem_cast<SizeHandleRect*>(ab)){
             ab->stretch(handle,sx,sy,ab->mapFromParent(origin));
         }
    }

    trans.translate(origin.x(),origin.y());
    trans.scale(sx,sy);
    trans.translate(-origin.x(),-origin.y());

    prepareGeometryChange();
    itemsBoundingRect = trans.mapRect(m_initialRect);
    m_width = itemsBoundingRect.width();
    m_height = itemsBoundingRect.height();
    updatehandles();

}

void GraphicsItemGroup::updateCoordinate()
{

    QPointF pt1,pt2,delta;
    if (itemsBoundingRect.isNull() )
        itemsBoundingRect = QGraphicsItemGroup::boundingRect();

    pt1 = mapToScene(transformOriginPoint());
    pt2 = mapToScene(itemsBoundingRect.center());
    delta = pt1 - pt2;
    m_initialRect = itemsBoundingRect;
    m_width = itemsBoundingRect.width();
    m_height = itemsBoundingRect.height();
//    itemsBoundingRect = QRectF(-m_width/2,-m_height/2,m_width,m_height);
    setTransform(transform().translate(delta.x(),delta.y()));
    setTransformOriginPoint(itemsBoundingRect.center());
    moveBy(-delta.x(),-delta.y());
 //   setTransform(transform().translate(-delta.x(),-delta.y()));

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

QList<QGraphicsItem *> GraphicsItemGroup::duplicateItems() const
{
    QList<QGraphicsItem*> copylist ;
    foreach (QGraphicsItem * shape , childItems() ) {
        AbstractShape * ab = qgraphicsitem_cast<AbstractShape*>(shape);
        if ( ab && !qgraphicsitem_cast<SizeHandleRect*>(ab)){
            QGraphicsItem * cp = ab->duplicate();
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
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(parentItem());
        if (!g)
            setState(value.toBool() ? SelectionHandleActive : SelectionHandleOff);
        else{
            setSelected(false);
            return QVariant::fromValue<bool>(false);
        }
        if( value.toBool()){
            updateCoordinate();
        }
    }
    /*
    else if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right()-boundingRect().width()/2, qMax(newPos.x(), rect.left()+boundingRect().width()/2)));
            newPos.setY(qMin(rect.bottom()-boundingRect().height()/2, qMax(newPos.y(), rect.top()+boundingRect().height()/2)));
            return newPos;
        }
    }
    */

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

GraphicsBezier::GraphicsBezier(bool bbezier,QGraphicsItem *parent)
    :GraphicsPolygonItem(parent)
    ,m_isBezier(bbezier)
{
    m_brush = QBrush(Qt::NoBrush);
}

QPainterPath GraphicsBezier::shape() const
{
    QPainterPath path;
    path.moveTo(m_points.at(0));
    int i=1;
    while (m_isBezier && ( i + 2 < m_points.size())) {
        path.cubicTo(m_points.at(i), m_points.at(i+1), m_points.at(i+2));
        i += 3;
    }
    while (i < m_points.size()) {
        path.lineTo(m_points.at(i));
        ++i;
    }

    return qt_graphicsItem_shapeFromPath(path,pen());
}

QGraphicsItem *GraphicsBezier::duplicate() const
{
    GraphicsBezier * item = new GraphicsBezier( );
    item->m_width = width();
    item->m_height = height();
    item->m_points = m_points;
    item->m_isBezier = m_isBezier;
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

bool GraphicsBezier::loadFromXml(QXmlStreamReader *xml)
{
    m_isBezier = (xml->name() == tr("bezier"));
    return GraphicsPolygonItem::loadFromXml(xml);
}

bool GraphicsBezier::saveToXml(QXmlStreamWriter *xml)
{
    if ( m_isBezier )
        xml->writeStartElement("bezier");
    else
        xml->writeStartElement("polyline");

    writeBaseAttributes(xml);

    for ( int i = 0 ; i < m_points.count();++i){
        xml->writeStartElement("point");
        xml->writeAttribute("x",QString("%1").arg(m_points[i].x()));
        xml->writeAttribute("y",QString("%1").arg(m_points[i].y()));
        xml->writeEndElement();
    }
    xml->writeEndElement();

    return true;
}

void GraphicsBezier::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    painter->setPen(pen());
    painter->setBrush(brush());
    path.moveTo(m_points.at(0));

    int i=1;
    while (m_isBezier && ( i + 2 < m_points.size())) {
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


GraphicsEllipseItem::GraphicsEllipseItem(const QRect & rect ,QGraphicsItem *parent)
    :GraphicsRectItem(rect,parent)
{
    m_startAngle = 40;
    m_spanAngle  = 400;
    SizeHandleRect *shr = new SizeHandleRect(this, 9 , true);
    m_handles.push_back(shr);
    shr = new SizeHandleRect(this, 10 , true);
    m_handles.push_back(shr);
    updatehandles();
}

QPainterPath GraphicsEllipseItem::shape() const
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

void GraphicsEllipseItem::control(int dir, const QPointF & delta)
{
    QPointF local = mapFromScene(delta);

    switch (dir) {
    case 9:
    {
        qreal len_y = local.y() - m_localRect.center().y();
        qreal len_x = local.x() - m_localRect.center().x();
        m_startAngle = -atan2(len_y,len_x)*180/M_PI;
    }
        break;
    case 10:
    {
        qreal len_y = local.y() - m_localRect.center().y();
        qreal len_x = local.x() - m_localRect.center().x();
        m_spanAngle = -atan2(len_y,len_x)*180/M_PI;
        break;
    }
   default:
        break;
    }
    prepareGeometryChange();
    if ( m_startAngle > m_spanAngle )
        m_startAngle-=360;
    if ( m_spanAngle < m_startAngle ){
        qreal tmp = m_spanAngle;
        m_spanAngle = m_startAngle;
        m_startAngle = tmp;
    }

    if ( qAbs(m_spanAngle-m_startAngle) > 360 ){
        m_startAngle = 40;
        m_spanAngle = 400;
    }
    updatehandles();
}

QRectF GraphicsEllipseItem::boundingRect() const
{
    return shape().controlPointRect();
}

QGraphicsItem *GraphicsEllipseItem::duplicate() const
{
    GraphicsEllipseItem * item = new GraphicsEllipseItem( m_localRect.toRect() );
    item->m_width = width();
    item->m_height = height();
    item->m_startAngle = m_startAngle;
    item->m_spanAngle   = m_spanAngle;

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

bool GraphicsEllipseItem::loadFromXml(QXmlStreamReader *xml)
{
    m_startAngle = xml->attributes().value("startAngle").toInt();
    m_spanAngle  = xml->attributes().value("spanAngle").toInt();
    readBaseAttributes(xml);
    xml->skipCurrentElement();
    updateCoordinate();
    return true;
}

bool GraphicsEllipseItem::saveToXml(QXmlStreamWriter * xml)
{
    xml->writeStartElement(tr("ellipse"));
    xml->writeAttribute("startAngle",QString("%1").arg(m_startAngle));
    xml->writeAttribute("spanAngle",QString("%1").arg(m_spanAngle));

    writeBaseAttributes(xml);
    xml->writeEndElement();
    return true;
}


void GraphicsEllipseItem::updatehandles()
{
    GraphicsItem::updatehandles();
    QRectF local = QRectF(-m_width/2,-m_height/2,m_width,m_height);
    QPointF delta = local.center() - m_localRect.center();

    qreal x = (m_width/2) * cos( -m_startAngle * M_PI / 180 );
    qreal y = (m_height/2) * sin( -m_startAngle * M_PI / 180);

    m_handles.at(8)->move(x-delta.x(),y-delta.y());
    x = (m_width/2) * cos( -m_spanAngle * M_PI / 180);
    y = (m_height/2) * sin(-m_spanAngle * M_PI / 180);
    m_handles.at(9)->move(x-delta.x(),y-delta.y());
}

void GraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QColor c = brushColor();
    QRectF rc = m_localRect;

    qreal radius = qMax(rc.width(),rc.height());

/*
    QConicalGradient  result(rc.center(),-45);

    QColor niceBlue(150, 150, 200);
    result.setColorAt(0.0, c.dark(200));
    result.setColorAt(0.2, niceBlue);
    result.setColorAt(0.5, c.light(120));
    result.setColorAt(1.0, c.dark(200));
*/
    painter->setPen(pen());
    QBrush b(c);
    painter->setBrush(b);

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
    m_pen = QPen(Qt::black);
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
    return qt_graphicsItem_shapeFromPath(path,pen());
}

void GraphicsPolygonItem::addPoint(const QPointF &point)
{
    m_points.append(mapFromScene(point));
    int dir = m_points.count();
    SizeHandleRect *shr = new SizeHandleRect(this, dir+Left, true);
    shr->setState(SelectionHandleActive);
    m_handles.push_back(shr);
}

void GraphicsPolygonItem::control(int dir, const QPointF &delta)
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
    m_localRect = m_points.boundingRect();
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
        m_localRect = m_points.boundingRect();
        m_width = m_localRect.width();
        m_height = m_localRect.height();

        setTransform(transform().translate(delta.x(),delta.y()));
        //setTransformOriginPoint(boundingRect().center());
        moveBy(-delta.x(),-delta.y());
        setTransform(transform().translate(-delta.x(),-delta.y()));
        updatehandles();
    }
    m_initialPoints = m_points;

}

bool GraphicsPolygonItem::loadFromXml(QXmlStreamReader *xml)
{
    readBaseAttributes(xml);
    while(xml->readNextStartElement()){
        if (xml->name()=="point"){
            qreal x = xml->attributes().value("x").toDouble();
            qreal y = xml->attributes().value("y").toDouble();
            m_points.append(QPointF(x,y));
            int dir = m_points.count();
            SizeHandleRect *shr = new SizeHandleRect(this, dir+Left, true);
            m_handles.push_back(shr);
            xml->skipCurrentElement();
        }else
            xml->skipCurrentElement();
    }
    updateCoordinate();
    return true;
}

bool GraphicsPolygonItem::saveToXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("polygon");
    writeBaseAttributes(xml);
    for ( int i = 0 ; i < m_points.count();++i){
        xml->writeStartElement("point");
        xml->writeAttribute("x",QString("%1").arg(m_points[i].x()));
        xml->writeAttribute("y",QString("%1").arg(m_points[i].y()));
        xml->writeEndElement();
    }
    xml->writeEndElement();
    return true;
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

QGraphicsItem *GraphicsPolygonItem::duplicate() const
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

    QColor c = brushColor();
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


