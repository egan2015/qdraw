#ifndef DRAWVIEW_H
#define DRAWVIEW_H
#include <QGraphicsView>

#include "rulebar.h"
#include "drawobj.h"

class GridTool;
class DrawView : public QGraphicsView
{
public:
    DrawView(QGraphicsScene *scene);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void scrollContentsBy(int dx, int dy) Q_DECL_OVERRIDE;
    QtRuleBar *m_hRuleBar;
    QtRuleBar *m_vRuleBar;
    QtCornerBox * box;    
    GridTool *m_grid;
};

#endif // DRAWVIEW_H
