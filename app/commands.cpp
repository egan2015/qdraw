#include "commands.h"

MoveShapeCommand::MoveShapeCommand(QGraphicsScene *graphicsScene, const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myItem = 0;
    myItems = graphicsScene->selectedItems();
    myGraphicsScene = graphicsScene;
    myDelta = delta;
    bMoved = true;
}

MoveShapeCommand::MoveShapeCommand(QGraphicsItem * item, const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myGraphicsScene = 0;
    myItem = item;
    myDelta = delta;
    bMoved = true;
}

//! [2]
void MoveShapeCommand::undo()
{
    if ( myItem )
        myItem->moveBy(-myDelta.x(),-myDelta.y());
    else if( myItems.count() > 0 ){
        foreach (QGraphicsItem *item, myItems) {
           item->moveBy(-myDelta.x(),-myDelta.y());
        }
    }
    setText(QObject::tr("Move %1,%2")
        .arg(-myDelta.x()).arg(-myDelta.y()));
    bMoved = false;
}
//! [2]

//! [3]
void MoveShapeCommand::redo()
{
    if ( !bMoved ){
        if ( myItem ){
            myItem->moveBy(myDelta.x(),myDelta.y());
            myItem->scene()->update();
        }else if( myItems.count() > 0 ){
            foreach (QGraphicsItem *item, myItems) {
               item->moveBy(myDelta.x(),myDelta.y());
            }
            myGraphicsScene->update();
        }
    }
    setText(QObject::tr("Move %1,%2")
        .arg(myDelta.x()).arg(myDelta.y()));
}
//! [3]
//! [4]
RemoveShapeCommand::RemoveShapeCommand(QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myGraphicsScene = scene;
    items = myGraphicsScene->selectedItems();
    setText(QObject::tr("Delete %1").arg(items.count()));
}

RemoveShapeCommand::~RemoveShapeCommand()
{

}
//! [4]

//! [5]
void RemoveShapeCommand::undo()
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
void RemoveShapeCommand::redo()
{
    foreach (QGraphicsItem *item, items) {
        QGraphicsItemGroup *g = dynamic_cast<QGraphicsItemGroup*>(item->parentItem());
        if ( !g )
            myGraphicsScene->removeItem(item);
    }
}
//! [6]

//! [7]
AddShapeCommand::AddShapeCommand(QGraphicsItem *item,
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

AddShapeCommand::~AddShapeCommand()
{
    if (!myDiagramItem->scene())
        delete myDiagramItem;
}

//! [8]
void AddShapeCommand::undo()
{
    myGraphicsScene->removeItem(myDiagramItem);
    myGraphicsScene->update();
}
//! [8]

//! [9]
void AddShapeCommand::redo()
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


RotateShapeCommand::RotateShapeCommand(QGraphicsItem *item, const qreal oldAngle, QUndoCommand *parent)
    :QUndoCommand(parent)
{
    myItem = item;
    myOldAngle = oldAngle;
    newAngle = item->rotation();
    setText(QObject::tr("Rotate %1").arg(newAngle));
}

void RotateShapeCommand::undo()
{
    myItem->setRotation(myOldAngle);
    myItem->scene()->update();
}

void RotateShapeCommand::redo()
{
    myItem->setRotation(newAngle);
    myItem->update();
}


GroupShapeCommand::GroupShapeCommand(QGraphicsItemGroup * group,
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

void GroupShapeCommand::undo()
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

void GroupShapeCommand::redo()
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


UnGroupShapeCommand::UnGroupShapeCommand(QGraphicsItemGroup *group,
                               QGraphicsScene *graphicsScene,
                               QUndoCommand *parent)
    :QUndoCommand(parent)
{
    myGraphicsScene = graphicsScene;
    myGroup = group;
    items = group->childItems();
    setText(QObject::tr("UnGroup %1").arg(items.count()));
}

void UnGroupShapeCommand::undo()
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

void UnGroupShapeCommand::redo()
{
    myGroup->setSelected(false);
    foreach (QGraphicsItem *item, myGroup->childItems()){
        item->setSelected(true);
        myGroup->removeFromGroup(item);
    }
    myGraphicsScene->removeItem(myGroup);
    myGraphicsScene->update();
}


ResizeShapeCommand::ResizeShapeCommand(QGraphicsItem *item,
                                       int handle,
                                       const QPointF& scale,
                                       QUndoCommand *parent)
{
    myItem = item;
    handle_ = handle;
    scale_  = QPointF(scale) ;
    bResized = true;
}

void ResizeShapeCommand::undo()
{

    AbstractShape * item = qgraphicsitem_cast<AbstractShape*>(myItem);
    if ( item ){
        item->stretch(handle_,1./scale_.x(),1./scale_.y(),item->opposite(handle_));
        item->updateCoordinate();
        item->update();
    }
    bResized = false;
    setText(QObject::tr("Undo Resize %1,%2")
        .arg(1./scale_.x()).arg(1./scale_.y()));

}

void ResizeShapeCommand::redo()
{
    if ( !bResized ){
        AbstractShape * item = qgraphicsitem_cast<AbstractShape*>(myItem);
        if ( item ){
            item->stretch(handle_,scale_.x(),scale_.y(),item->opposite(handle_));
            item->updateCoordinate();
            item->update();
        }
    }
    setText(QObject::tr("Redo Resize %1,%2")
        .arg(scale_.x()).arg(scale_.y()));

}
bool ResizeShapeCommand::mergeWith(const QUndoCommand *command)
{
    if (command->id() != ResizeShapeCommand::Id )
        return false;

    const ResizeShapeCommand *cmd = static_cast<const ResizeShapeCommand *>(command);

    QGraphicsItem *item = cmd->myItem;

    if (myItem != item)
        return false;
    if ( cmd->handle_ != handle_ )
        return false;

    handle_ = cmd->handle_;
    scale_ = cmd->scale_;
    setText(QObject::tr(" mergeWith Resize %1,%2,%3")
        .arg(scale_.x()).arg(scale_.y()).arg(handle_));

    return true;
}

ControlShapeCommand::ControlShapeCommand(QGraphicsItem *item,
                                       int handle,
                                       const QPointF& newPos,
                                       const QPointF& lastPos,
                                       QUndoCommand *parent)
{
    myItem = item;
    handle_ = handle;
    lastPos_  = QPointF(lastPos) ;
    newPos_ = QPointF(newPos);
    bControled = true;
}

void ControlShapeCommand::undo()
{

    AbstractShape * item = qgraphicsitem_cast<AbstractShape*>(myItem);
    if ( item ){
        item->control(handle_,lastPos_);
        item->updateCoordinate();
        item->update();
    }
    bControled = false;
    setText(QObject::tr("Undo Control %1,%2")
        .arg(lastPos_.x()).arg(lastPos_.y()));

}

void ControlShapeCommand::redo()
{
    if ( !bControled ){
        AbstractShape * item = qgraphicsitem_cast<AbstractShape*>(myItem);
        if ( item ){
            item->control(handle_,newPos_);
            item->updateCoordinate();
            item->update();
        }
    }
    setText(QObject::tr("Redo Control %1,%2")
        .arg(newPos_.x()).arg(newPos_.y()));

}
bool ControlShapeCommand::mergeWith(const QUndoCommand *command)
{
    if (command->id() != ControlShapeCommand::Id )
        return false;

    const ControlShapeCommand *cmd = static_cast<const ControlShapeCommand *>(command);
    QGraphicsItem *item = cmd->myItem;

    if (myItem != item )
        return false;
    if ( cmd->handle_ != handle_ )
        return false;
    handle_ = cmd->handle_;
    lastPos_ = cmd->lastPos_;
    newPos_  = cmd->newPos_;
    setText(QObject::tr(" mergeWith Control %1,%2,%3")
        .arg(newPos_.x()).arg(newPos_.y()).arg(handle_));

    return true;
}
