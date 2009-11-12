#include "qVTKConnection.h"

#include "qVTKObjectEventsObserver.h"

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkCallbackCommand.h"

#include <QString>
#include <QTextStream>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qVTKConnection::qInternal
{
  enum
    {
    NO_ARGUMENTS = 0,
    ARG_UNKOWN,
    ARG_VTKOBJECT,
    ARG_VOID,
    ARG_VTKOBJECT_AND_VTKOBJECT,
    ARG_VOID_AND_VTKOBJECT,
    ARG_VTKOBJECT_ULONG_VOID_VOID
    };

  qInternal()
    {
    this->Callback = vtkSmartPointer<vtkCallbackCommand>::New();
    this->Callback->SetCallback(qVTKConnection::DoCallback);
    this->VTKObject  = 0;
    this->QtObject   = 0;
    this->VTKEvent = vtkCommand::NoEvent;
    this->Priority = 0.0;
    this->SlotType = NO_ARGUMENTS;
    this->Parent      = 0;
    this->Established = false;
    this->Blocked     = false;
    }

  ~qInternal()
    {
    if(this->VTKObject && this->Established)
      {
      this->VTKObject->RemoveObserver(this->Callback);
      //Qt takes care of disconnecting slots
      }
    }

  vtkSmartPointer<vtkCallbackCommand> Callback;
  vtkObject*                          VTKObject;
  qVTKObjectEventsObserver*           Parent;
  const QObject*                      QtObject;
  unsigned long                       VTKEvent;
  QString                             QtSlot;
  float                               Priority;
  int                                 SlotType;
  bool                                Established;
  bool                                Blocked;
};

//-----------------------------------------------------------------------------
qVTKConnection::qVTKConnection(qVTKObjectEventsObserver* parent):
  Superclass(parent)
{
  this->Internal = new qInternal();
  this->Internal->Callback->SetClientData(this);
  Q_ASSERT(parent);
  this->Internal->Parent = parent;
}

//-----------------------------------------------------------------------------
qVTKConnection::~qVTKConnection()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qVTKConnection::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo();
  qDebug() << "qVTKConnection:" << this << endl
           << " VTKObject:" << this->Internal->VTKObject->GetClassName()
             << "(" << this->Internal->VTKObject << ")" << endl
           << " Parent:" << this->Internal->Parent << endl
           << " QtObject:" << this->Internal->QtObject << endl
           << " VTKEvent:" << this->Internal->VTKEvent << endl
           << " QtSlot:" << this->Internal->QtSlot << endl
           << " SlotType:" << this->Internal->SlotType << endl
           << " Priority:" << this->Internal->Priority << endl
           << " Established:" << this->Internal->Established << endl
           << " Blocked:" << this->Internal->Blocked;
}

//-----------------------------------------------------------------------------
QString qVTKConnection::getShortDescription()
{
  return Self::getShortDescription(this->Internal->VTKObject, this->Internal->VTKEvent,
    this->Internal->QtObject, this->Internal->QtSlot);
}

//-----------------------------------------------------------------------------
QString qVTKConnection::getShortDescription(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot)
{
  QString ret;
  QTextStream ts( &ret );
  ts << (vtk_obj ? vtk_obj->GetClassName() : "NULL") << "/"
     << vtk_event << "/" << qt_obj << "/" << qt_slot;
  return ret;
}

//-----------------------------------------------------------------------------
bool qVTKConnection::ValidateParameters(vtkObject* vtk_obj, unsigned long /*vtk_event*/,
  const QObject* qt_obj, QString qt_slot)
{
  if (!vtk_obj)
    {
    qCritical() << "vtkObject is NULL";
    return false;
    }
  if (!qt_obj)
    {
    qCritical() << "QObject is NULL";
    return false;
    }
  if (qt_slot.isEmpty())
    {
    qCritical() << "QtSlot is NULL";
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void qVTKConnection::SetParameters(vtkObject* vtk_obj, unsigned long vtk_event,
  const QObject* qt_obj, QString qt_slot, float priority)
{
  if (!Self::ValidateParameters(vtk_obj, vtk_event, qt_obj, qt_slot)) { return; }

  this->Internal->VTKObject = vtk_obj;
  this->Internal->QtObject = qt_obj;
  this->Internal->VTKEvent = vtk_event;
  this->Internal->QtSlot = qt_slot;
  this->Internal->Priority = priority;

  int strSize = qt_slot.count();
  if (qt_slot.at(strSize-1) == '(' && qt_slot.at(strSize) == ')')
    {
    this->Internal->SlotType = qInternal::NO_ARGUMENTS;
    }
  else if (qt_slot.contains("(vtkObject*)"))
    {
    this->Internal->SlotType = qInternal::ARG_VTKOBJECT;
    }
  else if (qt_slot.contains("(void*)"))
    {
    this->Internal->SlotType = qInternal::ARG_VOID;
    }
  else if (qt_slot.contains("(vtkObject*,vtkObject*)"))
    {
    this->Internal->SlotType = qInternal::ARG_VTKOBJECT_AND_VTKOBJECT;
    }
  else if (qt_slot.contains("(void*,vtkObject*)"))
    {
    this->Internal->SlotType = qInternal::ARG_VOID_AND_VTKOBJECT;
    }
  else if (qt_slot.contains("(vtkObject*,unsigned long,void*,void*)"))
    {
    this->Internal->SlotType = qInternal::ARG_VTKOBJECT_ULONG_VOID_VOID;
    }
  else
    {
    this->Internal->SlotType = qInternal::ARG_UNKOWN;
    qCritical() << "The slot (" << this->Internal->QtSlot <<  ") owned by "
            << "QObject(" << this->Internal->QtObject->objectName() << ")"
            << " seems to have a wrong signature.";
    }
}

//-----------------------------------------------------------------------------
void qVTKConnection::SetEstablished(bool enable)
{
  if (this->Internal->Established == enable)
    {
    qCritical() << "Failed to " << (enable ? "establish" : "break")
                << " an already " << (enable ? "established" : "broken") << " connection";
    return;
    }

  if (enable)
    {
    this->EstablishConnection();
    }
  else
    {
    this->BreakConnection();
    }
}

//-----------------------------------------------------------------------------
void qVTKConnection::SetBlocked(bool block)
{
  if (this->Internal->Blocked == block)
    {
    qCritical() << "Failed to " << (block ? "block" : "unblock")
                << " an already " << (block ? "blocked" : "unblocked") << " connection";
    return;
    }
  this->Internal->Blocked = block;
}

#define QVTK_EMPTY_ARG
#define QVTK_OBJECT_INTERNAL_CONNECT1(ARG1)      \
QObject::connect(                               \
  this, SIGNAL(emitExecute(ARG1)),              \
  this->Internal->QtObject,                     \
  this->Internal->QtSlot.toLatin1().data());

#define QVTK_OBJECT_INTERNAL_CONNECT2(ARG1,ARG2)   \
QObject::connect(                                  \
  this, SIGNAL(emitExecute(ARG1, ARG2)),           \
  this->Internal->QtObject,                        \
  this->Internal->QtSlot.toLatin1().data());

#define QVTK_OBJECT_INTERNAL_CONNECT3(ARG1,ARG2,ARG3)  \
QObject::connect(                                      \
  this, SIGNAL(emitExecute(ARG1, ARG2, ARG3)),         \
  this->Internal->QtObject,                            \
  this->Internal->QtSlot.toLatin1().data());

#define QVTK_OBJECT_INTERNAL_CONNECT4(ARG1,ARG2,ARG3,ARG4)   \
QObject::connect(                                            \
  this, SIGNAL(emitExecute(ARG1, ARG2, ARG3, ARG4)),         \
  this->Internal->QtObject,                                  \
  this->Internal->QtSlot.toLatin1().data());

//-----------------------------------------------------------------------------
void qVTKConnection::EstablishConnection()
{
  if (this->Internal->Established) { return; }

  switch (this->Internal->SlotType)
    {
    case qInternal::NO_ARGUMENTS:
      QVTK_OBJECT_INTERNAL_CONNECT1(QVTK_EMPTY_ARG);
      break;
    case qInternal::ARG_VTKOBJECT:
      QVTK_OBJECT_INTERNAL_CONNECT1(vtkObject*);
      break;
    case qInternal::ARG_VOID:
      QVTK_OBJECT_INTERNAL_CONNECT1(void*);
      break;
    case qInternal::ARG_VTKOBJECT_AND_VTKOBJECT:
      QVTK_OBJECT_INTERNAL_CONNECT2(vtkObject*,vtkObject*);
      break;
    case qInternal::ARG_VOID_AND_VTKOBJECT:
      QVTK_OBJECT_INTERNAL_CONNECT2(void*,vtkObject*);
      break;
    case qInternal::ARG_VTKOBJECT_ULONG_VOID_VOID:
      QVTK_OBJECT_INTERNAL_CONNECT4(vtkObject*,unsigned long,void*,void*);
      break;
    default:
      Q_ASSERT(false);
      qCritical() << "Failed to establish qVTKConnection - "
            << "The slot (" << this->Internal->QtSlot <<  ") owned by "
            << "QObject(" << this->Internal->QtObject->objectName() << ")"
            << " seems to have a wrong signature.";
      break;
    }

  // Make a connection between this and the vtk object
  this->Internal->VTKObject->AddObserver(this->Internal->VTKEvent,
    this->Internal->Callback, this->Internal->Priority);

  // If necessary, observe vtk DeleteEvent
  if(this->Internal->VTKEvent != vtkCommand::DeleteEvent)
    {
    this->Internal->VTKObject->AddObserver(
      vtkCommand::DeleteEvent, this->Internal->Callback);
    }

  // Remove itself from its parent when vtkObject is deleted
  QObject::connect(
    this->Internal->QtObject, SIGNAL(destroyed(QObject*)),
    this, SLOT(deleteConnection()));

  this->Internal->Established = true;
}

#undef QVTK_EMPTY_ARG
#undef QVTK_OBJECT_INTERNAL_CONNECT1
#undef QVTK_OBJECT_INTERNAL_CONNECT2
#undef QVTK_OBJECT_INTERNAL_CONNECT3
#undef QVTK_OBJECT_INTERNAL_CONNECT4

//-----------------------------------------------------------------------------
void qVTKConnection::BreakConnection()
{
  Q_ASSERT(this->Internal->Established);
  if (!this->Internal->Established) { return; }

  this->Internal->VTKObject->RemoveObserver(this->Internal->Callback);

  QObject::disconnect(
    this, SIGNAL(emitExecute(void*, vtkObject*)),
    this->Internal->QtObject,
    this->Internal->QtSlot.toLatin1().data());

  QObject::disconnect(
    this->Internal->QtObject,
    SIGNAL(destroyed(QObject*)),
    this, SLOT(deleteConnection()));

  this->Internal->Established = false;
}

//-----------------------------------------------------------------------------
bool qVTKConnection::IsEqual(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot)
{
  if (this->Internal->VTKObject != vtk_obj)
    {
    return false;
    }
  if (vtk_event != vtkCommand::NoEvent && this->Internal->VTKEvent != vtk_event)
    {
    return false;
    }
  if (qt_obj && this->Internal->QtObject != qt_obj)
    {
    return false;
    }
  if (!qt_slot.isEmpty() && (this->Internal->QtSlot.compare(qt_slot)!=0))
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int qVTKConnection::GetSlotType()
{
  return this->Internal->SlotType;
}

//-----------------------------------------------------------------------------
void qVTKConnection::DoCallback(vtkObject* vtk_obj, unsigned long event,
                                 void* client_data, void* call_data)
{
  qVTKConnection* conn = static_cast<qVTKConnection*>(client_data);
  Q_ASSERT(conn);
  conn->Execute(vtk_obj, event, client_data, call_data);
}

//-----------------------------------------------------------------------------
// callback from VTK to emit signal
void qVTKConnection::Execute(vtkObject* vtk_obj, unsigned long vtk_event,
  void* client_data, void* call_data)
{
  if (this->Internal->Blocked) { return; }
  if(vtk_event != vtkCommand::DeleteEvent ||
     (vtk_event == vtkCommand::DeleteEvent && this->Internal->VTKEvent == vtkCommand::DeleteEvent))
    {
    vtkObject* callDataAsVtkObject = 0;
    switch (this->Internal->SlotType)
      {
      case qInternal::NO_ARGUMENTS:
        if (this->Internal->VTKEvent == vtk_event)
          {
          emit this->emitExecute();
          }
        break;
      case qInternal::ARG_VTKOBJECT:
        callDataAsVtkObject = reinterpret_cast<vtkObject*>( call_data );
        if (!callDataAsVtkObject)
          {
          qCritical() << "The VTKEvent (" << this->Internal->VTKEvent<< ") triggered by vtkObject ("
            << this->Internal->VTKObject->GetClassName() << ") "
            << "doesn't return data of type vtkObject." << endl
            << "The slot (" << this->Internal->QtSlot <<  ") owned by "
            << "QObject(" << this->Internal->QtObject->objectName() << ")"
            << " may be incorrect.";
          }
        if (this->Internal->VTKEvent == vtk_event)
          {
          emit this->emitExecute( callDataAsVtkObject );
          }
        break;
      case qInternal::ARG_VOID:
        if (this->Internal->VTKEvent == vtk_event)
          {
          emit this->emitExecute(call_data);
          }
        break;
      case qInternal::ARG_VTKOBJECT_AND_VTKOBJECT:
        if (this->Internal->VTKEvent == vtk_event)
          {
          callDataAsVtkObject = reinterpret_cast<vtkObject*>( call_data );
          if (!callDataAsVtkObject)
            {
            qCritical() << "The VTKEvent(" << this->Internal->VTKEvent<< ") triggered by vtkObject("
              << this->Internal->VTKObject->GetClassName() << ") "
              << "doesn't return data of type vtkObject." << endl
              << "The slot (" << this->Internal->QtSlot <<  ") owned by "
              << "QObject(" << this->Internal->QtObject->objectName() << ")"
              << " may be incorrect.";
            }
          emit this->emitExecute( callDataAsVtkObject, vtk_obj);
          }
        break;
      case qInternal::ARG_VOID_AND_VTKOBJECT:
        if (this->Internal->VTKEvent == vtk_event)
          {
          emit this->emitExecute(call_data, vtk_obj);
          }
        break;
      case qInternal::ARG_VTKOBJECT_ULONG_VOID_VOID:
        emit this->emitExecute(vtk_obj, vtk_event, client_data, call_data);
        break;
      default:
        // Should never reach
        qCritical() << "Unknown SlotType:" << this->Internal->SlotType;
        return;
        break;
      }
    }

  if(vtk_event == vtkCommand::DeleteEvent)
    {
    //qDebug("--------------------------> qVTKConnection::Execute - removeConnection <--------------------------");
    this->Internal->Parent->removeConnection( this->Internal->VTKObject, this->Internal->VTKEvent,
      this->Internal->QtObject, this->Internal->QtSlot.toLatin1().data());
    }
}

//-----------------------------------------------------------------------------
void qVTKConnection::deleteConnection()
{
  this->Internal->Parent->removeConnection( this );
}
