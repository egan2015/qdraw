#ifndef DRAWVIEW_H
#define DRAWVIEW_H
#include <QGraphicsView>

#include "rulebar.h"
#include "drawobj.h"

class QMouseEvent;

class DrawView : public QGraphicsView
{
    Q_OBJECT
public:
    DrawView(QGraphicsScene *scene);
    void zoomIn();
    void zoomOut();
signals:
    void positionChanged(int x , int y );
protected:
    void mouseMoveEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void scrollContentsBy(int dx, int dy) Q_DECL_OVERRIDE;
    void updateRuler();
    QtRuleBar *m_hruler;
    QtRuleBar *m_vruler;
    QtCornerBox * box;    
    QPoint m_scrollPos;
    double m_zoomFactor;
};

#endif // DRAWVIEW_H
