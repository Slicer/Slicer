#ifndef __qVTKObject_h
#define __qVTKObject_h 

/// qVTK includes
#include "qVTKObjectEventsObserver.h"

/// QT includes
#include <QString>
#include <QDebug>

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_ADD_CONNECTION_METHOD                               \
QString qvtkConnect(vtkObject* vtk_obj, unsigned long vtk_event,        \
  const QObject* qt_obj, const char* qt_slot)                           \
{                                                                       \
  MyQVTK.setParent( this );                                             \
  return MyQVTK.addConnection(vtk_obj, vtk_event,                       \
    qt_obj, qt_slot);                                                   \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_RECONNECT_METHOD                                    \
QString qvtkReconnect(vtkObject* old_vtk_obj, vtkObject* vtk_obj,       \
  unsigned long vtk_event, const QObject* qt_obj, const char* qt_slot)  \
{                                                                       \
  MyQVTK.setParent( this );                                             \
  return MyQVTK.addConnection(old_vtk_obj, vtk_obj, vtk_event,          \
    qt_obj, qt_slot);                                                   \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_BLOCK_CONNECTION_METHOD                              \
void qvtkBlock(vtkObject* vtk_obj, unsigned long vtk_event,              \
  const QObject* qt_obj)                                                 \
{                                                                        \
  MyQVTK.setParent( this );                                              \
  MyQVTK.blockConnection(true, vtk_obj, vtk_event, qt_obj);              \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_BLOCK_CONNECTION_METHOD2                             \
void qvtkBlock(const QString& id, bool blocked)                          \
{                                                                        \
  MyQVTK.setParent( this );                                              \
  MyQVTK.blockConnection(id, blocked);                                   \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_BLOCKALL_CONNECTION_METHOD                          \
void qvtkBlockAll()                                                     \
{                                                                       \
  MyQVTK.setParent( this );                                             \
  MyQVTK.blockAllConnection(true, true);                                \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_UNBLOCK_CONNECTION_METHOD                             \
void qvtkUnblock(vtkObject* vtk_obj, unsigned long vtk_event,             \
  const QObject* qt_obj)                                                  \
{                                                                         \
  MyQVTK.setParent( this );                                               \
  MyQVTK.blockConnection(false, vtk_obj, vtk_event, qt_obj);              \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_UNBLOCKALL_CONNECTION_METHOD                        \
void qvtkUnblockAll()                                                   \
{                                                                       \
  MyQVTK.setParent( this );                                             \
  MyQVTK.blockAllConnection(false, true);                               \
}

//-----------------------------------------------------------------------------
#define QVTK_OBJECT_REMOVE_CONNECTION_METHOD                            \
void qvtkDisconnect(vtkObject* vtk_obj, unsigned long vtk_event,        \
  const QObject* qt_obj, const char* qt_slot)                           \
{                                                                       \
  MyQVTK.setParent( this );                                             \
  MyQVTK.removeConnection(vtk_obj, vtk_event,                           \
    qt_obj, qt_slot);                                                   \
}


//-----------------------------------------------------------------------------
#define QVTK_OBJECT                             \
protected:                                      \
  QVTK_OBJECT_ADD_CONNECTION_METHOD             \
  QVTK_OBJECT_RECONNECT_METHOD                  \
  QVTK_OBJECT_BLOCK_CONNECTION_METHOD           \
  QVTK_OBJECT_BLOCK_CONNECTION_METHOD2          \
  QVTK_OBJECT_BLOCKALL_CONNECTION_METHOD        \
  QVTK_OBJECT_UNBLOCK_CONNECTION_METHOD         \
  QVTK_OBJECT_UNBLOCKALL_CONNECTION_METHOD      \
  QVTK_OBJECT_REMOVE_CONNECTION_METHOD          \
private:                                        \
  qVTKObjectEventsObserver MyQVTK;


#endif
