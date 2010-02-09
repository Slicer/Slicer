#include "qVTKObjectEventsObserver.h"

#include "qVTKConnection.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include <QStringList>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QDebug>

//-----------------------------------------------------------------------------
class qVTKObjectEventsObserverPrivate: public qCTKPrivate<qVTKObjectEventsObserver>
{
public:
  qVTKObjectEventsObserverPrivate();
  QString convertPointerToString(void* pointer);
  
  bool                            ParentSet;
  bool                            AllEnabled;
  bool                            AllBlocked;
  QHash<QString, qVTKConnection*> ConnectionMap;
};

//-----------------------------------------------------------------------------
qVTKObjectEventsObserver::qVTKObjectEventsObserver(QObject* _parent):Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qVTKObjectEventsObserver);
  QCTK_D(qVTKObjectEventsObserver);
  
  if (_parent)
    {
    d->ParentSet = true;
    }

  this->setProperty("QVTK_OBJECT", true);
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::setParent(QObject* _parent)
{
  Q_ASSERT(_parent);
  QCTK_D(qVTKObjectEventsObserver);
  
  if (_parent && !d->ParentSet)
    {
    this->Superclass::setParent(_parent);
    //qDebug() << "qVTKObjectEventsObserver::setParent:" << this->parent();
    d->ParentSet = true;
    }
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo();
  QCTK_D(qVTKObjectEventsObserver);
  qDebug() << "qVTKObjectEventsObserver:" << this << endl
           << " AllEnabled:" << d->AllEnabled << endl
           << " AllBlocked:" << d->AllBlocked << endl
           << " ParentSet:" << d->ParentSet << endl
           << " Parent:" << (this->parent()?this->parent()->objectName():"NULL") << endl
           << " Connection count:" << d->ConnectionMap.count();

  // Loop through all connection
  foreach (qVTKConnection* connection, d->ConnectionMap.values())
    {
    connection->printAdditionalInfo();
    }
}

//-----------------------------------------------------------------------------
bool qVTKObjectEventsObserver::allEnabled()
{
  return qctk_d()->AllEnabled;
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::setAllEnabled(bool enable)
{
  QCTK_D(qVTKObjectEventsObserver);
  QList<qVTKConnection*> list = d->ConnectionMap.values();
  this->enableAll(list, enable);
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::enableAll(QList<qVTKConnection*>& connectionList, bool enable)
{
  QCTK_D(qVTKObjectEventsObserver);
  
  if (d->AllEnabled == enable) { return; }

  // Loop through VTKQtConnections to enable/disable
  foreach(qVTKConnection* connection, connectionList)
    {
    connection->SetEstablished(enable);
    }
  d->AllEnabled = enable;
}

//-----------------------------------------------------------------------------
QString qVTKObjectEventsObserver::addConnection(vtkObject* old_vtk_obj, vtkObject* vtk_obj,
  unsigned long vtk_event, const QObject* qt_obj, const char* qt_slot, float priority)
{
  QString connectionId; 
  if (old_vtk_obj)
    {
    // Check that old_object and new_object are the same type
    if (vtk_obj && !vtk_obj->IsA(old_vtk_obj->GetClassName()))
      {
      qCritical() << "Old vtkObject (type:" << old_vtk_obj->GetClassName() << ") to disconnect and "
                  << "the new VtkObject (type:" << vtk_obj->GetClassName() << ") to connect"
                  << "should have the same type.";
      return connectionId;
      }
    // Disconnect old vtkObject
    this->removeConnection(old_vtk_obj, vtk_event, qt_obj, qt_slot);
    }
  if (vtk_obj)
    {
    connectionId = this->addConnection(vtk_obj, vtk_event, qt_obj, qt_slot, priority);
    }
  return connectionId; 
}

//-----------------------------------------------------------------------------
QString qVTKObjectEventsObserver::addConnection(vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, const char* qt_slot, float priority)
{
  Q_ASSERT(vtk_obj);
  QCTK_D(qVTKObjectEventsObserver);
  if (!qVTKConnection::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot))
    {
    qCritical() << "Can't establish a vtkQtConnection - Invalid parameters - "
                << qVTKConnection::getShortDescription(vtk_obj, vtk_event, qt_obj, qt_slot);
    return QString();
    }

  // Check if such event is already observed
  if (this->containsConnection(vtk_obj, vtk_event, qt_obj, qt_slot))
    {
    qWarning() << " [vtkObject:" << vtk_obj->GetClassName()
               << ", event:" << vtk_event << "]"
               << "already connected with [qObject:" << qt_obj->objectName()
               << ", slot:" << qt_slot << "]";
    return QString();
    }

  // Instantiate a new connection, set its parameters and add it to the list
  qVTKConnection * connection = new qVTKConnection(this);
  connection->SetParameters(vtk_obj, vtk_event, qt_obj, qt_slot, priority);
  d->ConnectionMap[connection->GetId()] = connection;

  // If required, establish connection
  connection->SetEstablished(d->AllEnabled);

  return connection->GetId();
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockAllConnection(bool block, bool recursive)
{
  QCTK_D(qVTKObjectEventsObserver);
  qDebug() << "blockAllConnection-recursive:" << recursive;
  this->printAdditionalInfo();
  if (d->AllBlocked == block) { return; }

  foreach (qVTKConnection* connection, d->ConnectionMap.values())
    {
    connection->SetBlocked(block);
    }
//   if (recursive)
//     {
//     this->blockAllConnectionFromChildren(block);
//     }
  d->AllBlocked = block;
}

// //-----------------------------------------------------------------------------
// void qVTKObjectEventsObserver::blockAllConnectionFromChildren(bool block)
// {
//   QObject* qt_obj = this->parent();
//   Q_ASSERT(qt_obj);
//   if (!qt_obj) { return; }
//
//   foreach(QObject* o, qt_obj->children())
//     {
//     if (o != qt_obj)
//       {
//       if (o->metaObject()->indexOfProperty("QVTK_OBJECT"))
//         {
//         QMetaObject::invokeMethod(o, "blockAllConnection", Qt::DirectConnection,
//                         Q_ARG(bool, block),
//                         Q_ARG(bool, false) // False means no recursion
//                         );
//         }
//       }
//     }
// }

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockConnection(bool block, vtkObject* vtk_obj,
  unsigned long vtk_event, const QObject* qt_obj)
{
  int hit = this->blockConnectionRecursive(block, vtk_obj, vtk_event, qt_obj);
  if (!hit)
    {
    qCritical() << "Failed to " << (block?"block":"unblock") << " unexisting connection ("
                << qVTKConnection::getShortDescription(vtk_obj, vtk_event, qt_obj) << ")" << endl;
    }
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockConnection(const QString& id, bool blocked)
{
  QCTK_D(qVTKObjectEventsObserver);
  QHash<QString, qVTKConnection*>::const_iterator i = d->ConnectionMap.constFind(id);
  if (i != d->ConnectionMap.constEnd())
    {
    qVTKConnection* connection = i.value();
    Q_ASSERT(connection);
    connection->SetBlocked(blocked);
    }
}

//-----------------------------------------------------------------------------
int qVTKObjectEventsObserver::blockConnectionRecursive(bool block, vtkObject* vtk_obj,
  unsigned long vtk_event, const QObject* qt_obj)
{
  Q_ASSERT(vtk_obj);
  QCTK_D(qVTKObjectEventsObserver);
  if (!vtk_obj)
    {
    qCritical() << "qVTKObjectEventsObserver - Failed to " << (block?"block":"unblock")
                <<" connection - vtkObject is NULL";
    return 0;
    }
  bool all_info = true;
  if(qt_obj == NULL || vtk_event == vtkCommand::NoEvent)
    {
    all_info = false;
    }

  int hit = 0;
  foreach (qVTKConnection* connection, d->ConnectionMap.values())
    {
    //qDebug() << "blockConnection(" << block << ") - " << connection->getShortDescription();
    if (connection->IsEqual(vtk_obj, vtk_event, qt_obj, 0))
      {
      //qDebug() << "----> "<< (block?"blocked":"unblocked") << " <----";
      connection->SetBlocked(block);
      hit++;
      if (all_info){ break; }
      }
    }
//   if (!hit)
//     {
//     return this->blockConnectionFromChildren(block, vtk_obj, vtk_event, qt_obj);
//     }
  return hit;
}

// //-----------------------------------------------------------------------------
// int qVTKObjectEventsObserver::blockConnectionFromChildren(bool block, vtkObject* vtk_obj,
//     unsigned long vtk_event, const QObject* qt_obj)
// {
//   QObject* qt_parent_obj = this->parent();
//   Q_ASSERT(qt_parent_obj);
//   if (!qt_parent_obj) { return 0; }
//
//   qDebug() << "qt_parent_obj:" << qt_parent_obj->objectName();
//
//   foreach(QObject* o, qt_parent_obj->children())
//     {
//     if (o != qt_obj)
//       {
//       if (o->metaObject()->indexOfProperty("QVTK_OBJECT") > 0)
//         {
//         //qDebug() << "blockConnectionFromChildren:" << o->objectName();
//         int hit = 0;
//         bool status = QMetaObject::invokeMethod(o, "blockConnectionRecursive", Qt::DirectConnection,
//                         Q_RETURN_ARG(int, hit),
//                         Q_ARG(bool, block),
//                         Q_ARG(vtkObject*, vtk_obj),
//                         Q_ARG(unsigned long, vtk_event),
//                         Q_ARG(const QObject*, qt_obj)
//                         );
//         Q_ASSERT(status);
//         if (!status)
//           {
//           qDebug("Failed to invoke slot: blockConnectionRecursive");
//           }
//         if (hit > 0)
//           {
//           return hit;
//           }
//         }
//       }
//     }
//   return 0;
// }

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::removeConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot)
{
  Q_ASSERT(vtk_obj);
  QCTK_D(qVTKObjectEventsObserver);
  if (!vtk_obj)
    {
    qCritical() << "qVTKObjectEventsObserver - Failed to remove connection - vtkObject is NULL";
    return;
    }

  bool all_info = true;
  if(qt_slot == NULL || qt_obj == NULL || vtk_event == vtkCommand::NoEvent)
    {
    all_info = false;
    }

  QStringList connectionToRemove;
  foreach (qVTKConnection* connection, d->ConnectionMap.values())
    {
    if (connection->IsEqual(vtk_obj, vtk_event, qt_obj, qt_slot))
      {
      connectionToRemove << connection->GetId();
      if (all_info){ break; }
      }
    }

  // Remove connections
  foreach(const QString& id, connectionToRemove)
    {
    qVTKConnection* connection = d->ConnectionMap[id];
    Q_ASSERT(connection);
    this->removeConnection(connection);
    }
}

//-----------------------------------------------------------------------------
bool qVTKObjectEventsObserver::containsConnection(vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, const char* qt_slot)
{
  return (this->findConnection(vtk_obj, vtk_event, qt_obj, qt_slot) !=0);
}

//-----------------------------------------------------------------------------
qVTKConnection* qVTKObjectEventsObserver::findConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot)
{
  if (!qVTKConnection::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot)) { return 0; }

  QCTK_D(qVTKObjectEventsObserver);
  
  // Loop through all connection
  foreach (qVTKConnection* connection, d->ConnectionMap.values())
    {
    if (connection->IsEqual(vtk_obj, vtk_event, qt_obj, qt_slot)) { return connection; }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::removeConnection(qVTKConnection* connection)
{
  QCTK_D(qVTKObjectEventsObserver);
  // there is no need to remove the connection if the connection itself says that
  // it is about to get deleted. We trust the connection :-)
  if (!connection || connection->isAboutToBeDeleted())
    {
    return; 
    }
  connection->SetEstablished(false);
  d->ConnectionMap.remove(connection->GetId());
  delete connection;
}

//-----------------------------------------------------------------------------
// qVTKObjectEventsObserverPrivate methods

//-----------------------------------------------------------------------------
qVTKObjectEventsObserverPrivate::qVTKObjectEventsObserverPrivate()
{
  this->AllEnabled = true;
  this->AllBlocked = false;
  this->ParentSet = false;
}

