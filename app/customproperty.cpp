#include "customproperty.h"
#include <QtWidgets>
#include <qitemdelegate.h>

static
QIcon createColorIcon(const QColor & color )
{
    QRect r (0,0,100,10);
    QPixmap pixmap(100, 10);
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.setBrush(color);
    painter.drawRect(QRect(0,0,99,9));
    QIcon icon = pixmap;
    icon.paint(&painter,r,Qt::AlignHCenter|Qt::AlignVCenter);
    return icon;
}

class ColorItemDelegate :public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit ColorItemDelegate(QObject *parent );
    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};

ColorItemDelegate::ColorItemDelegate(QObject *parent)
    :QAbstractItemDelegate(parent)
{

}

void ColorItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

}

QSize ColorItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(80,10);
}

ShadeWidget::ShadeWidget(QWidget *parent)
    :QWidget(parent)
{

}

void ShadeWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawRect(rect().adjusted(1,1,-1,-1));
}

QtGradientEditor::QtGradientEditor(QWidget *parent)
    :QWidget(parent)
{
    QStringList colorList = QColor::colorNames();

    QVBoxLayout *layout = new QVBoxLayout();
    m_colorBegin = new QComboBox(this);
    m_colorMiddle = new QComboBox(this);
    m_colorEnd = new QComboBox(this);
    m_colorBegin->setIconSize(QSize(80,10));
    m_colorBegin->setToolTip(tr("Gradient Beginning color"));
    m_colorMiddle->setIconSize(QSize(80,10));
    m_colorMiddle->setToolTip(tr("Gradient middle color"));
    m_colorEnd->setIconSize(QSize(80,10));
    m_colorEnd->setToolTip(tr("Gradient ending color"));


    m_colorBegin->setItemDelegate(new ColorItemDelegate(m_colorBegin));

    for (int i = 0; i < colorList.count(); ++i) {
       m_colorBegin->addItem(createColorIcon(colorList.at(i)),NULL);
       m_colorMiddle->addItem(createColorIcon(colorList.at(i)),NULL);
       m_colorEnd->addItem(createColorIcon(colorList.at(i)),NULL);
    }

    layout->addWidget(m_colorBegin);
    layout->addWidget(m_colorMiddle);
    layout->addWidget(m_colorEnd);
    QVBoxLayout *layout1 = new QVBoxLayout();
    m_gradientMode = new QComboBox(this);
    m_shadewidget  = new ShadeWidget(this);

    layout1->addWidget(m_gradientMode);
    layout1->addWidget(m_shadewidget);
    QVBoxLayout *layout2 = new QVBoxLayout();
    m_midpoint = new QSlider(this);
    layout2->addWidget(m_midpoint);

    QHBoxLayout *ly = new QHBoxLayout();
    ly->addLayout(layout);
    ly->addLayout(layout2);
    ly->addLayout(layout1);

    setLayout(ly);
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


