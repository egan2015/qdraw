#include "commands.h"

MoveCommand::MoveCommand(QGraphicsItem *item, const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myItem = item;
    myDelta = delta;
    bMoved = true;
}

//! [2]
void MoveCommand::undo()
{
    myItem->moveBy(-myDelta.x(),-myDelta.y());

    setText(QObject::tr("Move %1,%2")
        .arg(-myDelta.x()).arg(-myDelta.y()));
    bMoved = false;
}
//! [2]

//! [3]
void MoveCommand::redo()
{
    if ( !bMoved ){
        myItem->moveBy(myDelta.x(),myDelta.y());
        myItem->scene()->update();
    }
    setText(QObject::tr("Move %1,%2")
        .arg(myDelta.x()).arg(myDelta.y()));
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

DeleteCommand::~DeleteCommand()
{

}
//! [4]

//! [5]
void DeleteCommand::undo()
{
    foreach (QGraphicsItem *item, items) {
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
            myGraphicsScene->addItem(item);
    }
    myGraphicsScene->update();

}
//! [5]

//! [6]
void DeleteCommand::redo()
{
    foreach (QGraphicsItem *item, items) {
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
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
    if ( myDiagramItem->scene() == NULL )
        myGraphicsScene->addItem(myDiagramItem);
    myDiagramItem->setPos(initialPosition);
    myGraphicsScene->update();
}

QString createCommandString(QGraphicsItem *item, const QPointF &pos)
{
    return QObject::tr("Item at (%1, %2)")
        .arg(pos.x()).arg(pos.y());
}


RotateCommand::RotateCommand(QGraphicsItem *item, const qreal oldAngle, QUndoCommand *parent)
    :QUndoCommand(parent)
{
    myItem = item;
    myOldAngle = oldAngle;
    newAngle = item->rotation();
    setText(QObject::tr("Rotate %1").arg(newAngle));
}

void RotateCommand::undo()
{
    myItem->setRotation(myOldAngle);
    myItem->scene()->update();
}

void RotateCommand::redo()
{
    myItem->setRotation(newAngle);
    myItem->update();
}


GroupCommand::GroupCommand(QGraphicsItemGroup * group,
                           QGraphicsScene *graphicsScene,
                           QUndoCommand *parent)
: QUndoCommand(parent)
{
    myGraphicsScene = graphicsScene;
    myGroup = group;
    items = group->childItems();
    b_undo = false;
    setText(QObject::tr("Group %1").arg(items.count()));
}

void GroupCommand::undo()
{
    myGroup->setSelected(false);
    QList<QGraphicsItem*> plist = myGroup->childItems();
    foreach (QGraphicsItem *item, plist){
        item->setSelected(true);
        myGroup->removeFromGroup(item);
    }
    myGraphicsScene->removeItem(myGroup);
    myGraphicsScene->update();
    b_undo = true;
}

void GroupCommand::redo()
{
    if (b_undo){
        foreach (QGraphicsItem *item, items){
            item->setSelected(false);
            QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
            if ( !g )
                myGroup->addToGroup(item);
        }
    }
    myGroup->setSelected(true);
    if ( myGroup->scene() == NULL )
        myGraphicsScene->addItem(myGroup);
    myGraphicsScene->update();
}


UnGroupCommand::UnGroupCommand(QGraphicsItemGroup *group,
                               QGraphicsScene *graphicsScene,
                               QUndoCommand *parent)
    :QUndoCommand(parent)
{
    myGraphicsScene = graphicsScene;
    myGroup = group;
    items = group->childItems();
    setText(QObject::tr("UnGroup %1").arg(items.count()));
}

void UnGroupCommand::undo()
{
    foreach (QGraphicsItem *item, items){
        item->setSelected(false);
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
             myGroup->addToGroup(item);
    }
    myGroup->setSelected(true);
    if ( myGroup->scene() == NULL )
        myGraphicsScene->addItem(myGroup);
    myGraphicsScene->update();
}

void UnGroupCommand::redo()
{
    myGroup->setSelected(false);
    foreach (QGraphicsItem *item, myGroup->childItems()){
        item->setSelected(true);
        myGroup->removeFromGroup(item);
    }
    myGraphicsScene->removeItem(myGroup);
    myGraphicsScene->update();
}
