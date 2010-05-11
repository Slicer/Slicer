#ifndef __qVTKObjectEventsObserver_h
#define __qVTKObjectEventsObserver_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QObject>
#include <QList>

/// VTK includes
#include <vtkCommand.h>

#include "qVTKWidgetsExport.h"

class qVTKConnection;
class vtkObject;
class qVTKObjectEventsObserverPrivate;

class QVTK_WIDGETS_EXPORT qVTKObjectEventsObserver : public QObject
{
Q_OBJECT

public:
  typedef QObject Superclass;
  explicit qVTKObjectEventsObserver(QObject* parent = 0);
  virtual ~qVTKObjectEventsObserver(){}

  virtual void printAdditionalInfo();

  /// 
  /// Enable / Disable all qVTKConnections
  void setAllEnabled( bool enable );
  bool allEnabled();

  /// 
  /// Set QVTK parent object
  void setParent(QObject* parent);

  /// 
  /// Add a connection, an Id allowing to uniquely identify the connection is also returned
  QString addConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);
  QString addConnection(vtkObject* old_vtk_obj, vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);

  /// 
  /// Block/Unblock a connection.
  void blockConnection(bool block, vtkObject* vtk_obj,
    unsigned long vtk_event, const QObject* qt_obj);
  void blockConnection(const QString& id, bool blocked);

  /// 
  /// Remove a connection
  void removeConnection(vtkObject* vtk_obj, unsigned long vtk_event = vtkCommand::NoEvent,
    const QObject* qt_obj = NULL, const char* qt_slot = NULL);

public slots:
  /// 
  /// Blocks recursive all qVTKConnection from this object and the QObject parent children
  void blockAllConnection( bool block, bool recursive);

  /// 
  /// Blocks recursive all qVTKConnection from the QObject parent children
///   void blockAllConnectionFromChildren( bool block );

protected:

  /// 
  /// Enable/Disable all qVTKConnections
  void enableAll(QList<qVTKConnection*>& connectionList, bool enable);

  /// 
  /// Check if a connection has already been added
  bool containsConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot);

  /// 
  /// Return a reference toward the corresponding connection or NULL if doesn't exist
  qVTKConnection* findConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot);

  friend class qVTKConnection;
  void removeConnection(qVTKConnection* connection);

  /// 
  /// Loop through all children until the invokation of 'blockConnection' returns true
  /// or until all children have been reviewed
///   int blockConnectionFromChildren(bool block, vtkObject* vtk_obj,
///     unsigned long vtk_event, const QObject* qt_obj);

protected slots:
  /// 
  /// Block/Unblock a connection.
  /// Return the number of blocked connection. Zero if any.
  int blockConnectionRecursive(bool block, vtkObject* vtk_obj,
    unsigned long vtk_event, const QObject* qt_obj);

private:
  CTK_DECLARE_PRIVATE(qVTKObjectEventsObserver);

};

#endif
