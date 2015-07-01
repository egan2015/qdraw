#ifndef COMMANDS
#define COMMANDS

#include <QUndoCommand>
#include "drawscene.h"

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(QGraphicsScene *graphicsScene, const QPointF & delta ,
                QUndoCommand * parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    QGraphicsScene *myGraphicsScene;
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

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QList<QGraphicsItem *> items;
    QGraphicsScene *myGraphicsScene;
};

class GroupCommand : public QUndoCommand
{
public:
    explicit GroupCommand( QGraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                           QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    QGraphicsItemGroup * myGroup;
    QGraphicsScene *myGraphicsScene;
    bool b_undo;
};

class UnGroupCommand : public QUndoCommand
{
public:
    explicit UnGroupCommand( QGraphicsItemGroup * group, QGraphicsScene *graphicsScene,
                             QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
private:
    QList<QGraphicsItem *> items;
    QGraphicsItemGroup * myGroup;
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

