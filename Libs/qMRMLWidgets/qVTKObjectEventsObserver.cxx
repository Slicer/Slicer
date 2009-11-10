#include "qVTKObjectEventsObserver.h" 

#include "qVTKConnection.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include <QVector>
#include <QMutableVectorIterator>
#include <QVariant>
#include <QList>
#include <QDebug>

//-----------------------------------------------------------------------------
class qVTKObjectEventsObserver::qInternal
{
public:
  qInternal()
    {
    this->AllEnabled = true; 
    this->AllBlocked = false; 
    this->ParentSet = false; 
    }
  bool                           ParentSet; 
  bool                           AllEnabled; 
  bool                           AllBlocked; 
  QVector<qVTKConnection*>       ConnectionList;
};

//-----------------------------------------------------------------------------
qVTKObjectEventsObserver::qVTKObjectEventsObserver(QObject* parent):
  Superclass(parent)
{
  if (parent)
    {
    this->Internal->ParentSet = true; 
    }
    
  this->Internal = new qInternal;
  
  this->setProperty("QVTK_OBJECT", true);
}

//-----------------------------------------------------------------------------
qVTKObjectEventsObserver::~qVTKObjectEventsObserver()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::setParent(QObject* parent)
{
  Q_ASSERT(parent);
  if (parent && !this->Internal->ParentSet)
    {
    this->Superclass::setParent(parent);
    //qDebug() << "qVTKObjectEventsObserver::setParent:" << this->parent(); 
    this->Internal->ParentSet = true; 
    }
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo();
  qDebug() << "qVTKObjectEventsObserver:" << this << endl
           << " AllEnabled:" << this->Internal->AllEnabled << endl
           << " AllBlocked:" << this->Internal->AllBlocked << endl
           << " ParentSet:" << this->Internal->ParentSet << endl
           << " Parent:" << (this->parent()?this->parent()->objectName():"NULL") << endl
           << " Connection count:" << this->Internal->ConnectionList.count();
           
  // Loop through all connection
  foreach (qVTKConnection* connection, this->Internal->ConnectionList)
    {
    connection->printAdditionalInfo(); 
    }
}

//-----------------------------------------------------------------------------
bool qVTKObjectEventsObserver::allEnabled()
{
  return this->Internal->AllEnabled; 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::setAllEnabled(bool enable)
{
  this->enableAll(this->Internal->ConnectionList, enable); 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::enableAll(QVector<qVTKConnection*>& connectionList, bool enable)
{
  if (this->Internal->AllEnabled == enable) { return; }
  
  // Loop through VTKQtConnections to enable/disable
  foreach(qVTKConnection* connection, connectionList)
    {
    connection->SetEstablished(enable); 
    }
  this->Internal->AllEnabled = enable; 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::addConnection(vtkObject* old_vtk_obj, vtkObject* vtk_obj, 
  unsigned long vtk_event, const QObject* qt_obj, const char* qt_slot, float priority)
{
  if (old_vtk_obj)
    {
    // Check that old_object and new_object are the same type
    if (vtk_obj && !vtk_obj->IsA(old_vtk_obj->GetClassName()))
      {
      qCritical() << "Old vtkObject (type:" << old_vtk_obj->GetClassName() << ") to disconnect and "
                  << "the new VtkObject (type:" << vtk_obj->GetClassName() << ") to connect" 
                  << "should have the same type."; 
      return; 
      }
    // Disconnect old vtkObject
    this->removeConnection(old_vtk_obj, vtk_event, qt_obj, qt_slot); 
    }
  if (vtk_obj)
    {
    this->addConnection(vtk_obj, vtk_event, qt_obj, qt_slot, priority); 
    }
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::addConnection(vtkObject* vtk_obj, unsigned long vtk_event, 
  const QObject* qt_obj, const char* qt_slot, float priority)
{
  Q_ASSERT(vtk_obj);
  if (!qVTKConnection::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot))
    { 
    qCritical() << "Can't establish a vtkQtConnection - Invalid parameters - " 
                << qVTKConnection::getShortDescription(vtk_obj, vtk_event, qt_obj, qt_slot); 
    return;
    }
  
  // Check if such event is already observed
  if (this->containsConnection(vtk_obj, vtk_event, qt_obj, qt_slot))
    {
    qWarning() << " [vtkObject:" << vtk_obj->GetClassName() 
               << ", event:" << vtk_event << "]"
               << "already connected with [qObject:" << qt_obj->objectName() 
               << ", slot:" << qt_slot << "]"; 
    return; 
    }
  
  // Instanciate a new connection, set its parameters and add it to the list
  qVTKConnection * connection = new qVTKConnection(this); 
  connection->SetParameters(vtk_obj, vtk_event, qt_obj, qt_slot, priority);
  this->Internal->ConnectionList.append(connection); 
  
  // If required, establish connection
  connection->SetEstablished(this->Internal->AllEnabled); 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockAllConnection(bool block, bool recursive)
{
  qDebug() << "blockAllConnection-recursive:" << recursive; 
  this->printAdditionalInfo(); 
  if (this->Internal->AllBlocked == block) { return; }
  
  foreach (qVTKConnection* connection, this->Internal->ConnectionList)
    {
    connection->SetBlocked(block);
    }
//   if (recursive)
//     {
//     this->blockAllConnectionFromChildren(block); 
//     }
  this->Internal->AllBlocked = block; 
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
int qVTKObjectEventsObserver::blockConnectionRecursive(bool block, vtkObject* vtk_obj, 
  unsigned long vtk_event, const QObject* qt_obj)
{
  Q_ASSERT(vtk_obj); 
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
  foreach (qVTKConnection* connection, this->Internal->ConnectionList)
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
  
  QMutableVectorIterator<qVTKConnection*> i(this->Internal->ConnectionList);
  while (i.hasNext())
    {
    i.next();
    qVTKConnection * connection = i.value(); 
    //qDebug() << "this->Internal->MRMLTransformNode - InLoop(" << vtk_obj << ") =" 
    //         << connection->getShortDescription(); 
    if (connection->IsEqual(vtk_obj, vtk_event, qt_obj, qt_slot))
      {
      connection->SetEstablished(false); 
      delete connection;
      i.remove(); 
      if (all_info){ i.toBack(); }
      //qDebug() << "----> REMOVED"; 
      }
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

  // Loop through all connection 
  foreach (qVTKConnection* connection, this->Internal->ConnectionList)
    {
    if (connection->IsEqual(vtk_obj, vtk_event, qt_obj, qt_slot)) { return connection; }
    }
  return 0; 
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::removeConnection(qVTKConnection* connection)
{
  if (!connection) { return; }
  delete connection; 
  this->Internal->ConnectionList.remove(
    this->Internal->ConnectionList.indexOf(connection) ); 
}
