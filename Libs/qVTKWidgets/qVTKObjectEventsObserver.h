#ifndef __qVTKObjectEventsObserver_h
#define __qVTKObjectEventsObserver_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QObject>
#include <QList>
#include <QString>

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
  bool allEnabled()const;

  /// 
  /// Add a connection, an Id allowing to uniquely identify the connection is also returned
  QString addConnection(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);

  ///
  /// Utility function that remove a connection on old_vtk_obj and add a connection
  /// to vtk_obj (same event, object, slot, priority)
  QString addConnection(vtkObject* old_vtk_obj, vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);

  ///
  /// Utility function that remove a connection on old_vtk_obj and add a connection
  /// to vtk_obj (same event, object, slot, priority)
  QString reconnection(vtkObject* vtk_obj, unsigned long vtk_event,
                       const QObject* qt_obj, const char* qt_slot, 
                       float priority = 0.0);

  /// 
  /// Remove a connection
  int removeConnection(vtkObject* vtk_obj, unsigned long vtk_event = vtkCommand::NoEvent,
                       const QObject* qt_obj = 0, const char* qt_slot = 0);

  /// 
  /// Remove all the connections
  inline int removeAllConnections();

  ///
  /// Temporarilly block all the connection
  void blockAllConnections(bool block);
  
  /// 
  /// Block/Unblock a connection.
  int blockConnection(bool block, vtkObject* vtk_obj,
                      unsigned long vtk_event, const QObject* qt_obj);
  void blockConnection(const QString& id, bool blocked);

private:
  QCTK_DECLARE_PRIVATE(qVTKObjectEventsObserver);

};

int qVTKObjectEventsObserver::removeAllConnections()
{
  return this->removeConnection(0);
}

#endif
