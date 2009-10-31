#ifndef __qVTKObjectEventsObserver_h
#define __qVTKObjectEventsObserver_h 

#include <QObject>
#include <QVector>

#include "vtkCommand.h"

#include "qMRMLWidgetsWin32Header.h"
 
class qVTKConnection; 
class vtkObject; 

class QMRML_WIDGETS_EXPORT qVTKObjectEventsObserver : public QObject
{
Q_OBJECT

public:
  typedef QObject Superclass;
  qVTKObjectEventsObserver(QObject* parent = 0);
  virtual ~qVTKObjectEventsObserver();
  
  virtual void printAdditionalInfo();
  
  // Description:
  // Enable / Disable all qVTKConnections
  void setAllEnabled( bool enable ); 
  bool allEnabled(); 
  
  // Description:
  // Set QVTK parent object
  void setParent(QObject* parent);
  
  // Description:
  // Add a connection
  void addConnection(vtkObject* vtk_obj, unsigned long vtk_event, 
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);
  void addConnection(vtkObject* old_vtk_obj, vtkObject* vtk_obj, unsigned long vtk_event, 
    const QObject* qt_obj, const char* qt_slot, float priority = 0.0);
  
  // Description:
  // Block/Unblock a connection. 
  void blockConnection(bool block, vtkObject* vtk_obj, 
    unsigned long vtk_event, const QObject* qt_obj); 
  
  // Description:
  // Remove a connection
  void removeConnection(vtkObject* vtk_obj, unsigned long vtk_event = vtkCommand::NoEvent,
    const QObject* qt_obj = NULL, const char* qt_slot = NULL);

public slots:
  // Description:
  // Blocks recursive all qVTKConnection from this object and the QObject parent children
  void blockAllConnection( bool block, bool recursive);
  
  // Description:
  // Blocks recursive all qVTKConnection from the QObject parent children
//   void blockAllConnectionFromChildren( bool block );

protected:

  // Description:
  // Enable/Disable all qVTKConnections
  void enableAll(QVector<qVTKConnection*>& connectionList, bool enable);
   
  // Description:
  // Check if a connection has already been added
  bool containsConnection(vtkObject* vtk_obj, unsigned long vtk_event, 
    const QObject* qt_obj, const char* qt_slot);
   
  // Description:
  // Return a reference toward the corresponding connection or NULL if doesn't exist
  qVTKConnection* findConnection(vtkObject* vtk_obj, unsigned long vtk_event, 
    const QObject* qt_obj, const char* qt_slot); 
    
  friend class qVTKConnection;
  void removeConnection(qVTKConnection* connection);
    
  // Description:
  // Loop through all children until the invokation of 'blockConnection' returns true
  // or until all children have been reviewed
//   int blockConnectionFromChildren(bool block, vtkObject* vtk_obj, 
//     unsigned long vtk_event, const QObject* qt_obj); 

protected slots:
  // Description:
  // Block/Unblock a connection. 
  // Return the number of blocked connection. Zero if any.
  int blockConnectionRecursive(bool block, vtkObject* vtk_obj, 
    unsigned long vtk_event, const QObject* qt_obj);

private:
  class qInternal;
  qInternal* Internal; 
  
};

#endif
