// QT includes
#include <QDebug>
#include <QRegExp>
#include <QString>
#include <QTextStream>

// qVTK includes 
#include "qVTKConnection.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

//-----------------------------------------------------------------------------
QString convertPointerToString(void* pointer)
{
  QString pointerAsString;
  QTextStream(&pointerAsString) << pointer;
  return pointerAsString;
}

//-----------------------------------------------------------------------------
class qVTKConnectionPrivate: public qCTKPrivate<qVTKConnection>
{
public:
  enum
    {
    ARG_UNKNOWN = 0,
    ARG_VTKOBJECT_AND_VTKOBJECT,
    ARG_VTKOBJECT_VOID_ULONG_VOID
    };

  typedef qVTKConnectionPrivate Self;
  qVTKConnectionPrivate();
  ~qVTKConnectionPrivate();

  void connect();
  void disconnect();

  /// 
  /// VTK Callback
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);

  /// 
  /// Called by 'DoCallback' to emit signal
  void Execute(vtkObject* vtk_obj, unsigned long vtk_event, void* client_data, void* call_data);

  vtkSmartPointer<vtkCallbackCommand> Callback;
  vtkObject*                          VTKObject;
  const QObject*                      QtObject;
  unsigned long                       VTKEvent;
  QString                             QtSlot;
  float                               Priority;
  int                                 SlotType;
  bool                                Connected;
  bool                                Blocked;
  QString                             Id;
  bool                                AboutToBeDeleted;
};

//-----------------------------------------------------------------------------
// qVTKConnectionPrivate methods

//-----------------------------------------------------------------------------
qVTKConnectionPrivate::qVTKConnectionPrivate()
{
  this->Callback    = vtkSmartPointer<vtkCallbackCommand>::New();
  this->Callback->SetCallback(qVTKConnectionPrivate::DoCallback);
  this->Callback->SetClientData(this);
  this->VTKObject   = 0;
  this->QtObject    = 0;
  this->VTKEvent    = vtkCommand::NoEvent;
  this->Priority    = 0.0;
  this->SlotType    = ARG_UNKNOWN;
  this->Connected   = false;
  this->Blocked     = false;
  this->Id          = convertPointerToString(this);
  this->AboutToBeDeleted = false;
}

//-----------------------------------------------------------------------------
qVTKConnectionPrivate::~qVTKConnectionPrivate()
{
  /*
  if(this->VTKObject && this->Connected)
    {
    this->VTKObject->RemoveObserver(this->Callback);
    //Qt takes care of disconnecting slots
    }
  */

}

//-----------------------------------------------------------------------------
void qVTKConnectionPrivate::connect()
{
  QCTK_P(qVTKConnection);
  
  if (this->Connected) 
    {
    qDebug() << "qVTKConnection already connected.";
    return; 
    }

  switch (this->SlotType)
    {
    case qVTKConnectionPrivate::ARG_VTKOBJECT_AND_VTKOBJECT:
      QObject::connect(p, SIGNAL(emitExecute(vtkObject*, vtkObject*)),
                       this->QtObject, this->QtSlot.toLatin1().data());
      break;
    case qVTKConnectionPrivate::ARG_VTKOBJECT_VOID_ULONG_VOID:
      QObject::connect(p, SIGNAL(emitExecute(vtkObject*, void*, unsigned long, void*)),
                       this->QtObject, this->QtSlot.toLatin1().data());
      break;
    default:
      Q_ASSERT(false);
      qCritical() << "Failed to connect - "
                  << "The slot (" << this->QtSlot <<  ") owned by "
                  << "QObject(" << this->QtObject->objectName() << ")"
                  << " seems to have a wrong signature.";
      break;
    }

  // Make a connection between this and the vtk object
  this->VTKObject->AddObserver(this->VTKEvent, this->Callback, this->Priority);

  // If necessary, observe vtk DeleteEvent
  if(this->VTKEvent != vtkCommand::DeleteEvent)
    {
    this->VTKObject->AddObserver(vtkCommand::DeleteEvent, this->Callback);
    }

  // Remove itself from its parent when vtkObject is deleted
  QObject::connect(this->QtObject, SIGNAL(destroyed(QObject*)), 
                   p, SLOT(deleteConnection()));

  this->Connected = true;
}

//-----------------------------------------------------------------------------
void qVTKConnectionPrivate::disconnect()
{
  QCTK_P(qVTKConnection);
  
  if (!this->Connected) 
    { 
    Q_ASSERT(this->Connected);
    return; 
    }

  this->VTKObject->RemoveObserver(this->Callback);

  switch (this->SlotType)
    {
    case qVTKConnectionPrivate::ARG_VTKOBJECT_AND_VTKOBJECT:
      QObject::disconnect(p, SIGNAL(emitExecute(vtkObject*, vtkObject*)),
                          this->QtObject,this->QtSlot.toLatin1().data());
      break;
    case qVTKConnectionPrivate::ARG_VTKOBJECT_VOID_ULONG_VOID:
      QObject::disconnect(p, SIGNAL(emitExecute(vtkObject*, void*, unsigned long, void*)),
                          this->QtObject, this->QtSlot.toLatin1().data());
      break;
    default:
      Q_ASSERT(false);
      qCritical() << "Failed to disconnect - "
                  << "The slot (" << this->QtSlot <<  ") owned by "
                  << "QObject(" << this->QtObject->objectName() << ")"
                  << " seems to have a wrong signature.";
      break;
    }

  QObject::disconnect(this->QtObject, SIGNAL(destroyed(QObject*)),
                      p, SLOT(deleteConnection()));

  this->Connected = false;
}

//-----------------------------------------------------------------------------
// qVTKConnection methods

//-----------------------------------------------------------------------------
qVTKConnection::qVTKConnection(QObject* _parent):
  Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qVTKConnection);
}

//-----------------------------------------------------------------------------
qVTKConnection::~qVTKConnection()
{
  this->setEnabled(false);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qVTKConnection, QString, id, Id);

//-----------------------------------------------------------------------------
void qVTKConnection::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo();

  QCTK_D(qVTKConnection);
  
  qDebug() << "qVTKConnection:" << this << endl
           << "Id:" << d->Id << endl
           << " VTKObject:" << d->VTKObject->GetClassName()
             << "(" << d->VTKObject << ")" << endl
           << " QtObject:" << d->QtObject << endl
           << " VTKEvent:" << d->VTKEvent << endl
           << " QtSlot:" << d->QtSlot << endl
           << " SlotType:" << d->SlotType << endl
           << " Priority:" << d->Priority << endl
           << " Connected:" << d->Connected << endl
           << " Blocked:" << d->Blocked;
}

//-----------------------------------------------------------------------------
QString qVTKConnection::shortDescription()
{
  QCTK_D(qVTKConnection);
  
  return Self::shortDescription(d->VTKObject, d->VTKEvent, d->QtObject, d->QtSlot);
}

//-----------------------------------------------------------------------------
QString qVTKConnection::shortDescription(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot)
{
  QString ret;
  QTextStream ts( &ret );
  ts << (vtk_obj ? vtk_obj->GetClassName() : "NULL") << " "
     << vtk_event << " " << qt_obj << " " << qt_slot;
  return ret;
}

//-----------------------------------------------------------------------------
bool qVTKConnection::ValidateParameters(vtkObject* vtk_obj, unsigned long vtk_event,
                                        const QObject* qt_obj, QString qt_slot)
{
  Q_UNUSED(vtk_event);
  if (!vtk_obj)
    {
    return false;
    }
  if (!qt_obj)
    {
    return false;
    }
  if (qt_slot.isEmpty())
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void qVTKConnection::SetParameters(vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, QString qt_slot, float priority)
{
  QCTK_D(qVTKConnection);
  
  if (!Self::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot)) 
    { 
    return; 
    }

  d->VTKObject = vtk_obj;
  d->QtObject = qt_obj;
  d->VTKEvent = vtk_event;
  d->QtSlot = qt_slot;
  d->Priority = priority;

  if (qt_slot.contains(QRegExp(QString("\\( ?vtkObject ?\\* ?, ?vtkObject ?\\* ?\\)"))))
    {
    d->SlotType = qVTKConnectionPrivate::ARG_VTKOBJECT_AND_VTKOBJECT;
    }
  else
    {
    d->SlotType = qVTKConnectionPrivate::ARG_VTKOBJECT_VOID_ULONG_VOID;
    }
}

//-----------------------------------------------------------------------------
void qVTKConnection::setEnabled(bool enable)
{
  QCTK_D(qVTKConnection);
  
  if (d->Connected == enable)
    {
    return;
    }

  if (enable)
    {
    d->connect();
    }
  else
    {
    d->disconnect();
    }
}

//-----------------------------------------------------------------------------
bool qVTKConnection::isEnabled()const
{
  QCTK_D(const qVTKConnection);
  return d->Connected;
}

//-----------------------------------------------------------------------------
void qVTKConnection::setBlocked(bool block)
{
  QCTK_D(qVTKConnection);
  d->Blocked = block;
}

//-----------------------------------------------------------------------------
bool qVTKConnection::isBlocked()const
{
  QCTK_D(const qVTKConnection);
  return d->Blocked;
}

//-----------------------------------------------------------------------------
bool qVTKConnection::isEqual(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot)const
{
  QCTK_D(const qVTKConnection);
  
  if (d->VTKObject != vtk_obj)
    {
    return false;
    }
  if (vtk_event != vtkCommand::NoEvent && d->VTKEvent != vtk_event)
    {
    return false;
    }
  if (qt_obj && d->QtObject != qt_obj)
    {
    return false;
    }
  if (!qt_slot.isEmpty() && 
      (QString(d->QtSlot).remove(' ').compare(
        QString(qt_slot).remove(' ')) != 0))
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void qVTKConnectionPrivate::DoCallback(vtkObject* vtk_obj, unsigned long event,
                                 void* client_data, void* call_data)
{
  qVTKConnectionPrivate* conn = static_cast<qVTKConnectionPrivate*>(client_data);
  Q_ASSERT(conn);
  conn->Execute(vtk_obj, event, client_data, call_data);
}

//-----------------------------------------------------------------------------
// callback from VTK to emit signal
void qVTKConnectionPrivate::Execute(vtkObject* vtk_obj, unsigned long vtk_event,
  void* client_data, void* call_data)
{
  QCTK_P(qVTKConnection);
  
  Q_ASSERT(this->Connected);
  if (this->Blocked) 
    { 
    return; 
    }
  if (vtk_event == vtkCommand::DeleteEvent)
    {
    this->AboutToBeDeleted = true;
    }

  if(vtk_event != vtkCommand::DeleteEvent ||
     (vtk_event == vtkCommand::DeleteEvent && this->VTKEvent == vtkCommand::DeleteEvent))
    {
    vtkObject* callDataAsVtkObject = 0;
    switch (this->SlotType)
      {
      case qVTKConnectionPrivate::ARG_VTKOBJECT_AND_VTKOBJECT:
        if (this->VTKEvent == vtk_event)
          {
          callDataAsVtkObject = reinterpret_cast<vtkObject*>( call_data );
          if (!callDataAsVtkObject)
            {
            qCritical() << "The VTKEvent(" << this->VTKEvent<< ") triggered by vtkObject("
              << this->VTKObject->GetClassName() << ") "
              << "doesn't return data of type vtkObject." << endl
              << "The slot (" << this->QtSlot <<  ") owned by "
              << "QObject(" << this->QtObject->objectName() << ")"
              << " may be incorrect.";
            }
          emit p->emitExecute(vtk_obj, callDataAsVtkObject);
          }
        break;
      case qVTKConnectionPrivate::ARG_VTKOBJECT_VOID_ULONG_VOID:
        emit p->emitExecute(vtk_obj, call_data, vtk_event, client_data);
        break;
      default:
        // Should never reach
        qCritical() << "Unknown SlotType:" << this->SlotType;
        return;
        break;
      }
    }

  if(vtk_event == vtkCommand::DeleteEvent)
    {
    this->AboutToBeDeleted = false;
    p->deleteConnection();
    }
}

void qVTKConnection::deleteConnection()
{
  QCTK_D(qVTKConnection);
  if (d->AboutToBeDeleted)
    {
    return;
    }
  delete this;
}
