#include "customproperty.h"
#include <QtWidgets>
#include <qlistview.h>
#include <qpainter.h>
#include <qevent.h>
#include <QLinearGradient>

static
QIcon createColorIcon(const QColor & color )
{
    QRect r (0,0,60,10);
    QPixmap pixmap(60, 10);
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.setBrush(color);
    painter.drawRect(QRect(0,0,59,9));
    QIcon icon = pixmap;
    icon.paint(&painter,r,Qt::AlignHCenter|Qt::AlignVCenter);
    return icon;
}

static
QIcon createGradientIcon( int type , const QColor & c )
{
    QPixmap pixmap(40, 20);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);

    QBrush brush;
    if ( type == 0 ){
        brush = c.dark(150);
    }else if ( type == 1){
        QLinearGradient result(0,0,40,0);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.light(200));
        result.setColorAt(1, c.dark(150));
        brush = result;
    }else if ( type == 2){
        QLinearGradient result(0,0,0,20);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.light(200));
        result.setColorAt(1, c.dark(150));
        brush = result;
    }else if ( type == 3 ){
        QLinearGradient result(0,0,40,20);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.dark(200));
        result.setColorAt(1, c.dark(250));
        brush = result;
    }else if ( type == 4 ){
        QLinearGradient result(40,20,20,0);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.dark(200));
        result.setColorAt(1, c.dark(250));
        brush = result;
    }else if ( type == 5 ){
        QLinearGradient result(40,20,20,0);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.dark(200));
        result.setColorAt(1, c.dark(250));
        brush = result;
    }else if ( type == 6 ){
        QLinearGradient result(40,20,20,0);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.dark(200));
        result.setColorAt(1, c.dark(250));
        brush = result;
    }else if ( type == 7 ){
        QRadialGradient result(20,10,20);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.light(200));
        result.setColorAt(1, c.dark(150));
        brush = result;
    } else {
        QConicalGradient result(20,10,45);
        result.setColorAt(0, c.dark(150));
        result.setColorAt(0.5, c.light(200));
        result.setColorAt(1, c.dark(150));
        brush = result;

    }
    painter.setBrush(brush);
    painter.drawRect(QRect(0,0,40,20));

    QIcon icon = pixmap;
    return icon;
}

ColorButton::ColorButton(QWidget *parent)
    :QPushButton(parent),
      m_color(Qt::white)
{
    setFixedHeight(20);
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter painter (this);
    QRect r = rect().adjusted(2,2,-2,-2);
    painter.setPen(Qt::black);
    painter.setBrush(m_color);
    painter.drawRect(r.adjusted(1,1,-1,-1));
}

ShadeWidget::ShadeWidget(QWidget *parent)
    :QWidget(parent)
{
    m_colorBegin = m_colorMiddle = m_colorEnd = Qt::white;
    m_type = 0;
    m_midpoint = 50 ;

    QPixmap pm(20, 20);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
    pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
    pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
    pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
    pmp.end();
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush(pm));
    setAutoFillBackground(true);
    setPalette(pal);


}

void ShadeWidget::colorChanged(const QColor &begin, const QColor &middle, const QColor &end)
{
    m_colorBegin = begin;
    m_colorMiddle = middle;
    m_colorEnd = end;

    update();
}

void ShadeWidget::positionChanged(int position)
{
    m_midpoint = position;
     update();
}

void ShadeWidget::typeChanged(int type)
{
    m_type = type;
     update();
}

void ShadeWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect r;
    int h = rect().height() - 10;
    qreal pos = m_midpoint / 100.0;
    r.setRect(-h/2,-h/2,h,h);
    p.drawRect(rect().adjusted(1,1,-1,-1));
    QBrush brush;
    if ( m_type == 0 ){
        brush = m_colorBegin.dark(150);
    }else if ( m_type == 1){
        QLinearGradient result(r.topLeft(),r.topRight());
        result.setColorAt(0, m_colorBegin);
        result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(1, m_colorEnd);
        brush = result;
    } else if ( m_type == 2){
        QLinearGradient result(r.topLeft(),r.bottomLeft());
        result.setColorAt(0, m_colorBegin);
        result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(1, m_colorEnd);
        brush = result;
    } else if ( m_type == 3 ){
        QLinearGradient result(r.topLeft(),r.bottomRight());
        result.setColorAt(0, m_colorBegin);
        //result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(pos, m_colorEnd);
        brush = result;
    }else if ( m_type == 4 ){
        QLinearGradient result(r.bottomLeft(),r.topRight());
        result.setColorAt(0, m_colorBegin);
        //result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(pos, m_colorEnd);
        brush = result;
    }else if ( m_type == 5){
        QLinearGradient result(r.topRight(),r.bottomLeft());
        result.setColorAt(0, m_colorBegin);
        //result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(pos, m_colorEnd);
        brush = result;
    }else if ( m_type == 6){
        QLinearGradient result(r.bottomRight(),r.topLeft());
        result.setColorAt(0, m_colorBegin);
        //result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(pos, m_colorEnd);
        brush = result;
    }else if ( m_type == 7){
        QRadialGradient result(r.center(),h);
        result.setColorAt(0, m_colorBegin);
        result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(1, m_colorEnd);
        brush = result;
    } else {
        QConicalGradient result(r.center(),45);
        result.setColorAt(0, m_colorBegin);
        result.setColorAt(pos, m_colorMiddle);
        result.setColorAt(1, m_colorEnd);
        brush = result;

    }

    p.save();
    p.translate(rect().center().x(),rect().center().y());
    p.setBrush(brush);
    p.drawRect(r);
    p.restore();
}

QStringList colorList = QColor::colorNames();

QtGradientEditor::QtGradientEditor(QWidget *parent)
    :QWidget(parent)
{

    QVBoxLayout *layout = new QVBoxLayout();
    m_colorBegin = new ColorButton(this);
    m_colorMiddle = new ColorButton(this);
    m_colorEnd = new ColorButton(this);
    m_colorBegin->setToolTip(tr("Gradient Beginning color"));
    m_colorMiddle->setToolTip(tr("Gradient middle color"));
    m_colorEnd->setToolTip(tr("Gradient ending color"));


    layout->addWidget(m_colorBegin);
    layout->addWidget(m_colorMiddle);
    layout->addWidget(m_colorEnd);
    QVBoxLayout *layout1 = new QVBoxLayout();
    m_gradientMode = new QComboBox(this);
    m_gradientMode->setToolTip(tr("Gradient mode"));
    m_shadewidget  = new ShadeWidget(this);

    m_gradientMode->setIconSize(QSize(40,20));
    for ( int i= 0 ; i < 9 ; ++i ){
        m_gradientMode->addItem(createGradientIcon(i,colorList[18]),NULL);
    }
    layout1->addWidget(m_gradientMode);
    layout1->addWidget(m_shadewidget);

    QVBoxLayout *layout2 = new QVBoxLayout();
    m_midpoint = new QSlider(this);
    m_midpoint->setRange(0,100);
    m_midpoint->setValue(50);
    m_midpoint->setToolTip(tr("Set Middle Position (0~100)"));
    layout2->addWidget(m_midpoint);

    QHBoxLayout *ly = new QHBoxLayout();
    ly->addLayout(layout);
    ly->addLayout(layout2);
    ly->addLayout(layout1);

    setLayout(ly);

    connect(m_colorBegin,SIGNAL(clicked(bool)),this,SLOT(clicked()));
    connect(m_colorMiddle,SIGNAL(clicked(bool)),this,SLOT(clicked()));
    connect(m_colorEnd,SIGNAL(clicked(bool)),this,SLOT(clicked()));
    connect(this,SIGNAL(colorChanged(QColor,QColor,QColor)),m_shadewidget,SLOT(colorChanged(QColor,QColor,QColor)));
    connect(m_gradientMode,SIGNAL(currentIndexChanged(int)),m_shadewidget,SLOT(typeChanged(int)));
    connect(m_midpoint,SIGNAL(valueChanged(int)),m_shadewidget,SLOT(positionChanged(int)));
    setFixedHeight(150);
    setFixedWidth(300);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint );
    setWindowTitle(tr("GradientEditor"));
}

void QtGradientEditor::colorChanged(int)
{

}

void QtGradientEditor::clicked()
{
    ColorButton * button = dynamic_cast<ColorButton*>(sender());

    bool ok = false;
    QRgb oldRgba = button->value().rgba();
    QRgb newRgba = QColorDialog::getRgba(oldRgba, &ok, this);
    if (ok && newRgba != oldRgba) {
        button->setValue(QColor::fromRgba(newRgba));
        button->update();
        if ( button == m_colorBegin ){
            m_gradientMode->clear();
            for ( int i= 0 ; i < 9 ; ++i ){
                m_gradientMode->addItem(createGradientIcon(i,m_colorBegin->value()),NULL);
            }
        }
        emit colorChanged(m_colorBegin->value(),m_colorMiddle->value(),m_colorEnd->value());
    }
}

QtPenPropertyManager::QtPenPropertyManager(QObject *parent)
    :QtAbstractPropertyManager(parent)
{
    m_intPropertyManager = new QtIntPropertyManager(this);
    connect(m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));

    connect(m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

QtPenPropertyManager::~QtPenPropertyManager()
{
    clear();
    delete m_intPropertyManager;
    delete m_enumPropertyManager;
}

QtIntPropertyManager *QtPenPropertyManager::subIntPropertyManager() const
{
    return m_intPropertyManager;
}

QtEnumPropertyManager *QtPenPropertyManager::subEnumPropertyManager() const
{
    return m_enumPropertyManager;
}

QPen QtPenPropertyManager::value(const QtProperty *property) const
{
    return m_values.value(property,QPen());
}

static int capStyleToEnum( int val )
{
    switch(val){
    case 0x00:
        return 0;
    case 0x10:
        return 1;
    case 0x20:
        return 2;
    }
    return 0;
}

static int enumToCapStyle( int val ){
    switch(val){
    case 0:
        return 0;
    case 1:
        return 0x10;
    case 2:
        return 0x20;
    }
    return 0;
}

static int joinStyleToEnum( int val ){
    switch(val){
    case 0x00:
        return 0;
    case 0x40:
        return 1;
    case 0x80:
        return 2;
    case 0x100:
        return 3;
    }
    return 0;
}

static int enumToJoinStyle( int val ){
    switch(val){
    case 0:
        return 0x00;
    case 1:
        return 0x40;
    case 2:
        return 0x80;
    case 3:
        return 0x100;
    }
    return 0;
}

void QtPenPropertyManager::setValue(QtProperty *property, const QPen &val)
{
    const PropertyValueMap::iterator it = m_values.find(property);
    if (it == m_values.end())
        return;

    const QPen oldVal = it.value();
    if (oldVal == val)
        return;

    it.value() = val;

    m_intPropertyManager->setValue(m_propertyToWidth[property],val.width());
    m_enumPropertyManager->setValue(m_propertyToStyle[property],val.style());
    m_enumPropertyManager->setValue(m_propertyToCapStyle[property],capStyleToEnum(val.capStyle()));
    m_enumPropertyManager->setValue(m_propertyToJoinStyle[property],joinStyleToEnum(val.joinStyle()));

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

void QtPenPropertyManager::slotIntChanged(QtProperty * property, int value)
{
    if (QtProperty *prop = m_widthToProperty.value(property,0)){
        QPen p = m_values[prop];
        p.setWidth(value);
        setValue(prop,p);
    }
}

void QtPenPropertyManager::slotEnumChanged(QtProperty * property, int value)
{
    if ( QtProperty * prop = m_styleToProperty.value(property,0)){
        QPen p = m_values[prop];
        p.setStyle((Qt::PenStyle)value);
        setValue(prop,p);
    }else if ( QtProperty * prop = m_capStyleToProperty.value(property,0)){
        QPen p = m_values[prop];
        p.setCapStyle((Qt::PenCapStyle)enumToCapStyle(value));
        setValue(prop,p);
    }else if ( QtProperty * prop = m_joinStyleToProperty.value(property,0)){
        QPen p = m_values[prop];
        p.setJoinStyle((Qt::PenJoinStyle)enumToJoinStyle(value));
        setValue(prop,p);
    }
}

void QtPenPropertyManager::slotPropertyDestroyed(QtProperty * property)
{
    if (QtProperty * prop = m_widthToProperty.value(property,0)){
        m_propertyToWidth[prop] = 0;
        m_widthToProperty.remove(property);
    }else if ( QtProperty * prop = m_styleToProperty.value(property,0)){
        m_propertyToStyle[prop] = 0;
        m_styleToProperty.remove(property);
    }else if ( QtProperty * prop = m_capStyleToProperty.value(property,0)){
        m_propertyToCapStyle[prop] = 0;
        m_capStyleToProperty.remove(property);
    }else if ( QtProperty * prop = m_joinStyleToProperty.value(property,0)){
        m_propertyToJoinStyle[prop] = 0;
        m_joinStyleToProperty.remove(property);
    }
}

QString QtPenPropertyManager::valueText(const QtProperty *property) const
{
    const PropertyValueMap::const_iterator it = m_values.constFind(property);
    if ( it == m_values.constEnd() )
        return QString();
    switch(it.value().style()){
    case Qt::SolidLine:
        return QString(tr("SolidLine"));
    case Qt::DashLine:
        return QString(tr("DashLine"));
    case Qt::NoPen:
        return QString(tr("NoPen"));
    case Qt::DashDotDotLine:
        return QString(tr("DashDotDotLine"));
    case Qt::DashDotLine:
        return QString(tr("DashDotLine"));
    case Qt::CustomDashLine:
        return QString(tr("CustomDashLine"));
    case Qt::DotLine:
        return tr("Dotline");
    }
    return QString();
}

QIcon QtPenPropertyManager::valueIcon(const QtProperty *property) const
{
    return QIcon();
}

void QtPenPropertyManager::initializeProperty(QtProperty *property)
{
    QPen val;
    m_values[property] = val;

    QtProperty * widthProp = m_intPropertyManager->addProperty();
    widthProp->setPropertyName(tr("Width"));
    m_intPropertyManager->setValue(widthProp,val.width());
    m_intPropertyManager->setMinimum(widthProp,1);

    m_propertyToWidth[property] = widthProp;
    m_widthToProperty[widthProp] = property;

    property->addSubProperty(widthProp);

    QtProperty *styleProp = m_enumPropertyManager->addProperty();
    styleProp->setPropertyName(tr("Style"));
    QStringList styleNames;
    styleNames<<tr("NoPen")<<tr("SolidLine")<<tr("DashLine")<<tr("DotLine")
              <<tr("DashDotLine")<<tr("DashDotDotLine")<<tr("CustomDashLine");
    m_enumPropertyManager->setEnumNames(styleProp,styleNames);
    m_enumPropertyManager->setValue(styleProp,val.style());

    m_propertyToStyle[property] = styleProp;
    m_styleToProperty[styleProp] = property;

    property->addSubProperty(styleProp);

    QtProperty *capStyleProp = m_enumPropertyManager->addProperty();
    capStyleProp->setPropertyName(tr("CapStyle"));
    QStringList capStyleNames;
    capStyleNames<<tr("FlatCap")<<tr("SquareCap")<<tr("RoundCap");
    m_enumPropertyManager->setEnumNames(capStyleProp,capStyleNames);
    m_enumPropertyManager->setValue(capStyleProp,val.capStyle());

    m_propertyToCapStyle[property] = capStyleProp;
    m_capStyleToProperty[capStyleProp] = property;
    property->addSubProperty(capStyleProp);

    QtProperty *joinStyleProp = m_enumPropertyManager->addProperty();
    joinStyleProp->setPropertyName(tr("JoinStyle"));
    QStringList joinStyleNames;
    joinStyleNames<<tr("MiterJoin")<<tr("BevelJoin")<<tr("RoundJoin")<<tr("SvgMiterJoin");
    m_enumPropertyManager->setEnumNames(joinStyleProp,joinStyleNames);
    m_enumPropertyManager->setValue(joinStyleProp,val.joinStyle());

    m_propertyToJoinStyle[property] = joinStyleProp;
    m_joinStyleToProperty[joinStyleProp] = property;
    property->addSubProperty(joinStyleProp);
}

void QtPenPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *Prop = m_propertyToWidth[property];
    if (Prop) {
        m_widthToProperty.remove(Prop);
        delete Prop;
    }
    m_propertyToWidth.remove(property);

    Prop = m_propertyToStyle[property];
    if (Prop) {
        m_styleToProperty.remove(Prop);
        delete Prop;
    }
    m_propertyToStyle.remove(property);

    Prop = m_propertyToCapStyle[property];
    if (Prop) {
        m_capStyleToProperty.remove(Prop);
        delete Prop;
    }
    m_propertyToCapStyle.remove(property);

    Prop = m_propertyToJoinStyle[property];
    if (Prop) {
        m_joinStyleToProperty.remove(Prop);
        delete Prop;
    }
    m_propertyToJoinStyle.remove(property);
}
