#ifndef COMMANDS
#define COMMANDS

#include <QUndoCommand>
#include "drawscene.h"

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(QGraphicsScene *graphicsScene, const QPointF & delta ,
                QUndoCommand * parent = 0);
    MoveCommand(QGraphicsItem * item, const QPointF & delta , QUndoCommand * parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QGraphicsScene *myGraphicsScene;
    QGraphicsItem  *myItem;
    QList<QGraphicsItem *> myItems;
    QPointF myDelta;
    bool bMoved;
};

class RotateCommand : public QUndoCommand
{
public:
    RotateCommand(QGraphicsItem *item , const qreal oldAngle ,
                QUndoCommand * parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QGraphicsItem *myItem;
    qreal myOldAngle;
    qreal newAngle;
};

class DeleteCommand : public QUndoCommand
{
public:
    explicit DeleteCommand(QGraphicsScene *graphicsScene, QUndoCommand *parent = 0);
    ~DeleteCommand();
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QList<QGraphicsItem *> items;
    QGraphicsScene *myGraphicsScene;
};

class GroupCommand : public QUndoCommand
{
public:
    explicit GroupCommand( GraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                           QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    GraphicsItemGroup * myGroup;
    QGraphicsScene *myGraphicsScene;
    bool b_undo;
};

class UnGroupCommand : public QUndoCommand
{
public:
    explicit UnGroupCommand( GraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                             QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    GraphicsItemGroup * myGroup;
    QGraphicsScene *myGraphicsScene;
};

class AddCommand : public QUndoCommand
{
public:
    AddCommand(QGraphicsItem *item , QGraphicsScene *graphicsScene,
               QUndoCommand *parent = 0);
    ~AddCommand();

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QGraphicsItem *myDiagramItem;
    QGraphicsScene *myGraphicsScene;
    QPointF initialPosition;
};

QString createCommandString(QGraphicsItem *item, const QPointF &point);

#endif // COMMANDS

