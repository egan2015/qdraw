#ifndef CUSTOMPROPERTY
#define CUSTOMPROPERTY

#include <qtpropertymanager.h>
#include <qteditorfactory.h>
QT_BEGIN_NAMESPACE
class QComboBox;
class QSlider;
QT_END_NAMESPACE

class ShadeWidget : public QWidget
{
    Q_OBJECT
public:
    ShadeWidget(QWidget *parent);
protected:
    void paintEvent(QPaintEvent * event);
};

static QIcon createColorIcon(const QColor & color );

class QtGradientEditor :public QWidget
{
    Q_OBJECT
public:
    QtGradientEditor( QWidget * parent );
private:
    QComboBox *m_gradientMode;
    QComboBox *m_colorBegin;
    QComboBox *m_colorMiddle;
    QComboBox *m_colorEnd;
    QSlider   *m_midpoint;
    ShadeWidget * m_shadewidget;
};

class QtPenPropertyManager : public QtAbstractPropertyManager
{
    Q_OBJECT
public:
    QtPenPropertyManager(QObject * parent = 0);
    ~QtPenPropertyManager();

    QtIntPropertyManager *subIntPropertyManager() const;
    QtEnumPropertyManager *subEnumPropertyManager() const;
    QPen value(const QtProperty *property) const;
public Q_SLOTS:
    void setValue(QtProperty *property, const QPen &val);
    void slotIntChanged(QtProperty *, int);
    void slotEnumChanged(QtProperty *, int);
    void slotPropertyDestroyed(QtProperty *);

Q_SIGNALS:
    void valueChanged(QtProperty *property, const QPen &val);
protected:
    QString valueText(const QtProperty *property) const;
    QIcon valueIcon(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);

private:
    typedef QMap<const QtProperty *, QPen> PropertyValueMap;
    PropertyValueMap m_values;

    QtIntPropertyManager *m_intPropertyManager;
    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToWidth;
    QMap<const QtProperty *, QtProperty *> m_propertyToStyle;
    QMap<const QtProperty *, QtProperty *> m_propertyToCapStyle;
    QMap<const QtProperty *, QtProperty *> m_propertyToJoinStyle;

    QMap<const QtProperty *, QtProperty *> m_widthToProperty;
    QMap<const QtProperty *, QtProperty *> m_styleToProperty;
    QMap<const QtProperty *, QtProperty *> m_capStyleToProperty;
    QMap<const QtProperty *, QtProperty *> m_joinStyleToProperty;

    Q_DISABLE_COPY(QtPenPropertyManager)

};

#endif // CUSTOMPROPERTY

