#include "drawview.h"
#include <QSvgGenerator>

//http://www.w3.org/TR/SVG/Overview.html

DrawView::DrawView(QGraphicsScene *scene)
    :QGraphicsView(scene)
{
    m_hruler = new QtRuleBar(RT_HORIZONTAL,this,this);
    m_vruler = new QtRuleBar(RT_VERTICAL,this,this);
    box = new QtCornerBox(this);
    setViewport(new QWidget);

    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;

}

void DrawView::zoomIn()
{
    scale(1.2,1.2);
    updateRuler();
}

void DrawView::zoomOut()
{
    scale(1 / 1.2, 1 / 1.2);
    updateRuler();
}

void DrawView::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("drawing%1.svg").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
}

bool DrawView::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Qt Drawing"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    setCurrentFile(fileName);

    return true;
}

bool DrawView::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool DrawView::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);

}

bool DrawView::saveFile(const QString &fileName)
{
/*
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Qt Drawing"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
*/
    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(QSize(800, 600));
    generator.setTitle(tr("SVG Generator Example Drawing"));
    generator.setDescription(tr("An SVG drawing created by the SVG Generator "
                                "Example provided with Qt."));
//![configure SVG generator]
//![begin painting]
    QPainter painter;
    painter.begin(&generator);
//![begin painting]
//!
    scene()->clearSelection();
    scene()->render(&painter);
//![end painting]
    painter.end();
//![end painting]

    setCurrentFile(fileName);
    return true;
}

QString DrawView::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void DrawView::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void DrawView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pt =mapToScene(event->pos());
    m_hruler->updatePosition(event->pos());
    m_vruler->updatePosition(event->pos());
    emit positionChanged( pt.x() , pt.y() );
    QGraphicsView::mouseMoveEvent(event);
}

void DrawView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    this->setViewportMargins(RULER_SIZE-1,RULER_SIZE-1,0,0);
    m_hruler->resize(this->size().width()- RULER_SIZE - 1,RULER_SIZE);
    m_hruler->move(RULER_SIZE,0);
    m_vruler->resize(RULER_SIZE,this->size().height() - RULER_SIZE - 1);
    m_vruler->move(0,RULER_SIZE);

    box->resize(RULER_SIZE,RULER_SIZE);
    box->move(0,0);
    updateRuler();
}

void DrawView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx,dy);
    updateRuler();
}

void DrawView::updateRuler()
{
    if ( scene() == 0) return;
    QRectF viewbox = this->rect();
    QPointF offset = mapFromScene(scene()->sceneRect().topLeft());
    double factor =  1./transform().m11();
    double lower_x = factor * ( viewbox.left()  - offset.x() );
    double upper_x = factor * ( viewbox.right() -RULER_SIZE- offset.x() -1 );
    m_hruler->setRange(lower_x,upper_x,upper_x - lower_x );
    m_hruler->update();

    double lower_y = factor * ( viewbox.top() - offset.y());
    double upper_y = factor * ( viewbox.bottom() - RULER_SIZE - offset.y() -1);

    m_vruler->setRange(lower_y,upper_y,upper_y - lower_y );
    m_vruler->update();

    //   qDebug()<<viewbox<<QPoint(lower_x,upper_x) << QPoint(lower_y,upper_y) << offset;
}

bool DrawView::maybeSave()
{
    if (isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("MDI"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
                     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void DrawView::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString DrawView::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();

}

