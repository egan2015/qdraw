#ifndef COMMANDS
#define COMMANDS

#include <QUndoCommand>
#include "drawscene.h"

class MoveShapeCommand : public QUndoCommand
{
public:
    MoveShapeCommand(QGraphicsScene *graphicsScene, const QPointF & delta ,
                QUndoCommand * parent = 0);
    MoveShapeCommand(QGraphicsItem * item, const QPointF & delta , QUndoCommand * parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QGraphicsScene *myGraphicsScene;
    QGraphicsItem  *myItem;
    QList<QGraphicsItem *> myItems;
    QPointF myDelta;
    bool bMoved;
};

class ResizeShapeCommand : public QUndoCommand
{
public:
    enum { Id = 1234, };
    ResizeShapeCommand(QGraphicsItem * item ,
                       int handle,
                       const QPointF& scale,
                       QUndoCommand *parent = 0 );
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE { return Id; }
private:
    QGraphicsItem  *myItem;
    int handle_;
    int opposite_;
    QPointF scale_;
    bool bResized;
};

class ControlShapeCommand : public QUndoCommand
{
public:
    enum { Id = 1235, };
    ControlShapeCommand(QGraphicsItem * item ,
                       int handle,
                       const QPointF& newPos,
                       const QPointF& lastPos,
                       QUndoCommand *parent = 0 );
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE { return Id; }

private:
    QGraphicsItem  *myItem;
    int handle_;
    QPointF lastPos_;
    QPointF newPos_;
    bool bControled;
};


class RotateShapeCommand : public QUndoCommand
{
public:
    RotateShapeCommand(QGraphicsItem *item , const qreal oldAngle ,
                QUndoCommand * parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QGraphicsItem *myItem;
    qreal myOldAngle;
    qreal newAngle;
};

class RemoveShapeCommand : public QUndoCommand
{
public:
    explicit RemoveShapeCommand(QGraphicsScene *graphicsScene, QUndoCommand *parent = 0);
    ~RemoveShapeCommand();
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QList<QGraphicsItem *> items;
    QGraphicsScene *myGraphicsScene;
};

class GroupShapeCommand : public QUndoCommand
{
public:
    explicit GroupShapeCommand( QGraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                           QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    QGraphicsItemGroup * myGroup;
    QGraphicsScene *myGraphicsScene;
    bool b_undo;
};

class UnGroupShapeCommand : public QUndoCommand
{
public:
    explicit UnGroupShapeCommand( QGraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                             QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    QGraphicsItemGroup * myGroup;
    QGraphicsScene *myGraphicsScene;
};

class AddShapeCommand : public QUndoCommand
{
public:
    AddShapeCommand(QGraphicsItem *item , QGraphicsScene *graphicsScene,
               QUndoCommand *parent = 0);
    ~AddShapeCommand();

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QGraphicsItem *myDiagramItem;
    QGraphicsScene *myGraphicsScene;
    QPointF initialPosition;
};

QString createCommandString(QGraphicsItem *item, const QPointF &point);

#endif // COMMANDS

