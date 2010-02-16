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

  /// 
  /// Check if a connection has already been added
  bool containsConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot);

  /// 
  /// Return a reference toward the corresponding connection or 0 if doesn't exist
  qVTKConnection* findConnection(const QString& id);

  /// 
  /// Return a reference toward the corresponding connection or 0 if doesn't exist
  qVTKConnection* findConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot);

  /// 
  /// Return all the references that match the given parameters
  QList<qVTKConnection*> findConnections(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot);
  
  inline QList<qVTKConnection*> connections()const 
  {
    return qctk_p()->findChildren<qVTKConnection*>();
  }
  
  bool                            AllEnabled;
  bool                            AllBlocked;
};

//-----------------------------------------------------------------------------
// qVTKObjectEventsObserver methods

//-----------------------------------------------------------------------------
qVTKObjectEventsObserver::qVTKObjectEventsObserver(QObject* _parent):Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qVTKObjectEventsObserver);
  this->setProperty("QVTK_OBJECT", true);
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo();
  QCTK_D(qVTKObjectEventsObserver);
  qDebug() << "qVTKObjectEventsObserver:" << this << endl
           << " AllEnabled:" << d->AllEnabled << endl
           << " AllBlocked:" << d->AllBlocked << endl
           << " Parent:" << (this->parent()?this->parent()->objectName():"NULL") << endl
           << " Connection count:" << d->connections().count();

  // Loop through all connection
  foreach (qVTKConnection* connection, d->connections())
    {
    connection->printAdditionalInfo();
    }
}

//-----------------------------------------------------------------------------
bool qVTKObjectEventsObserver::allEnabled()const
{
  return qctk_d()->AllEnabled;
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::setAllEnabled(bool enable)
{
  QCTK_D(qVTKObjectEventsObserver);
  // FIXME: maybe a particular module has been enabled/disabled
  if (d->AllEnabled == enable) 
    { 
    return; 
    }
  // Loop through VTKQtConnections to enable/disable
  foreach(qVTKConnection* connection, d->connections())
    {
    connection->setEnabled(enable);
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
  QCTK_D(qVTKObjectEventsObserver);
  if (!qVTKConnection::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot))
    {
    qDebug() << "qVTKObjectEventsObserver::addConnection(...) - Invalid parameters - "
             << qVTKConnection::shortDescription(vtk_obj, vtk_event, qt_obj, qt_slot);
    return QString();
    }

  // Check if such event is already observed
  if (d->containsConnection(vtk_obj, vtk_event, qt_obj, qt_slot))
    {
    qWarning() << "qVTKObjectEventsObserver::addConnection - [vtkObject:" 
               << vtk_obj->GetClassName()
               << ", event:" << vtk_event << "]"
               << " is already connected with [qObject:" << qt_obj->objectName()
               << ", slot:" << qt_slot << "]";
    return QString();
    }

  // Instantiate a new connection, set its parameters and add it to the list
  qVTKConnection * connection = new qVTKConnection(this);
  connection->SetParameters(vtk_obj, vtk_event, qt_obj, qt_slot, priority);

  // If required, establish connection
  connection->setEnabled(d->AllEnabled);
  connection->setBlocked(d->AllBlocked);

  return connection->id();
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockAllConnections(bool block)
{
  QCTK_D(qVTKObjectEventsObserver);
  this->printAdditionalInfo();
  if (d->AllBlocked == block) { return; }

  foreach (qVTKConnection* connection, d->connections())
    {
    connection->setBlocked(block);
    }
  d->AllBlocked = block;
}

//-----------------------------------------------------------------------------
void qVTKObjectEventsObserver::blockConnection(const QString& id, bool blocked)
{
  QCTK_D(qVTKObjectEventsObserver);
  qVTKConnection* connection = d->findConnection(id);
  if (connection == 0)
    {
    return;
    }
  connection->setBlocked(blocked);
}

//-----------------------------------------------------------------------------
int qVTKObjectEventsObserver::blockConnection(bool block, vtkObject* vtk_obj,
  unsigned long vtk_event, const QObject* qt_obj)
{
  QCTK_D(qVTKObjectEventsObserver);
  if (!vtk_obj)
    {
    qDebug() << "qVTKObjectEventsObserver::blockConnectionRecursive"
             << "- Failed to " << (block?"block":"unblock") <<" connection"
             << "- vtkObject is NULL";
    return 0;
    }
  QList<qVTKConnection*> connections =
    d->findConnections(vtk_obj, vtk_event, qt_obj, 0);
  foreach (qVTKConnection* connection, connections)
    {
    connection->setBlocked(block);
    }
  return connections.size();
}

//-----------------------------------------------------------------------------
int qVTKObjectEventsObserver::removeConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot)
{
  QCTK_D(qVTKObjectEventsObserver);

  QList<qVTKConnection*> connections = 
    d->findConnections(vtk_obj, vtk_event, qt_obj, qt_slot);
  
  foreach (qVTKConnection* connection, connections)
    {
    connection->deleteConnection();
    }
  return connections.count();
}

//-----------------------------------------------------------------------------
// qVTKObjectEventsObserverPrivate methods

//-----------------------------------------------------------------------------
qVTKObjectEventsObserverPrivate::qVTKObjectEventsObserverPrivate()
{
  this->AllEnabled = true;
  this->AllBlocked = false;
}

//-----------------------------------------------------------------------------
bool qVTKObjectEventsObserverPrivate::containsConnection(vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, const char* qt_slot)
{
  return (this->findConnection(vtk_obj, vtk_event, qt_obj, qt_slot) != 0);
}

//-----------------------------------------------------------------------------
qVTKConnection* 
qVTKObjectEventsObserverPrivate::findConnection(const QString& id)
{
  foreach(qVTKConnection* connection, this->connections())
    {
    if (connection->id() == id)
      {
      return connection;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
qVTKConnection* 
qVTKObjectEventsObserverPrivate::findConnection(vtkObject* vtk_obj, unsigned long vtk_event,
                                         const QObject* qt_obj, const char* qt_slot)
{
  QList<qVTKConnection*> foundConnections =
    this->findConnections(vtk_obj, vtk_event, qt_obj, qt_slot);

  return foundConnections.size() ? foundConnections[0] : 0;
}

//-----------------------------------------------------------------------------
QList<qVTKConnection*> 
qVTKObjectEventsObserverPrivate::findConnections(
  vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, const char* qt_slot)
{
  bool all_info = true;
  if(qt_slot == NULL || qt_obj == NULL || vtk_event == vtkCommand::NoEvent)
    {
    all_info = false;
    }

  QList<qVTKConnection*> foundConnections;
  // Loop through all connection
  foreach (qVTKConnection* connection, this->connections())
    {
    if (connection->isEqual(vtk_obj, vtk_event, qt_obj, qt_slot)) 
      {
      foundConnections.append(connection); 
      if (all_info)
        {
        break;
        }
      }
    }
  return foundConnections;
}
