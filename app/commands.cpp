#include "commands.h"

MoveCommand::MoveCommand(QGraphicsItem *item, const QPointF &oldPos, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myItem = item;
    myOldPos = oldPos;
    newPos = item->pos();
}

bool MoveCommand::mergeWith(const QUndoCommand *command)
{
    const MoveCommand *moveCommand = static_cast<const MoveCommand *>(command);
    QGraphicsItem *item = moveCommand->myItem;

    if (myItem != item)
    return false;

    newPos = item->pos();
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));

    return true;
}
//! [2]
void MoveCommand::undo()
{
    myItem->setPos(myOldPos);
    myItem->scene()->update();
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));
}
//! [2]

//! [3]
void MoveCommand::redo()
{
    myItem->setPos(newPos);
    setText(QObject::tr("Move %1")
        .arg(createCommandString(myItem, newPos)));
}
//! [3]
//! [4]
DeleteCommand::DeleteCommand(QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myGraphicsScene = scene;
    items = myGraphicsScene->selectedItems();
    setText(QObject::tr("Delete %1").arg(items.count()));
}
//! [4]

//! [5]
void DeleteCommand::undo()
{
    foreach (QGraphicsItem *item, items) {
        myGraphicsScene->addItem(item);
    }
    myGraphicsScene->update();

}
//! [5]

//! [6]
void DeleteCommand::redo()
{
    foreach (QGraphicsItem *item, items) {
        myGraphicsScene->removeItem(item);
    }
}
//! [6]

//! [7]
AddCommand::AddCommand(QGraphicsItem *item,
                       QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;

    myGraphicsScene = scene;
    myDiagramItem = item;
    initialPosition = item->pos();
    scene->update();
    ++itemCount;
    setText(QObject::tr("Add %1")
        .arg(createCommandString(myDiagramItem, initialPosition)));
}
//! [7]

AddCommand::~AddCommand()
{
    if (!myDiagramItem->scene())
        delete myDiagramItem;
}

//! [8]
void AddCommand::undo()
{
    myGraphicsScene->removeItem(myDiagramItem);
    myGraphicsScene->update();
}
//! [8]

//! [9]
void AddCommand::redo()
{
    myGraphicsScene->addItem(myDiagramItem);
    myDiagramItem->setPos(initialPosition);
    myGraphicsScene->clearSelection();
    myGraphicsScene->update();
}

QString createCommandString(QGraphicsItem *item, const QPointF &pos)
{
    return QObject::tr("Item at (%1, %2)")
        .arg(pos.x()).arg(pos.y());
}
