#ifndef __qVTKConnection_h
#define __qVTKConnection_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QObject>
#include <QVector>

#include "qVTKWidgetsExport.h"

class qVTKObjectEventsObserver;
class vtkObject;
class qVTKConnectionPrivate;

class QVTK_WIDGETS_EXPORT qVTKConnection : public QObject
{
Q_OBJECT

public:
  typedef QObject Superclass;
  explicit qVTKConnection(qVTKObjectEventsObserver* parent);
  virtual ~qVTKConnection(){}

  /// 
  virtual void printAdditionalInfo();
  QString getShortDescription();
  static QString getShortDescription(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot = "");

  /// 
  void SetParameters(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot, float priority);

  /// 
  static bool ValidateParameters(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot);

  /// 
  void SetEstablished(bool enable);

  /// 
  void SetBlocked(bool block);

  /// 
  bool IsEqual(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot);

  /// 
  int GetSlotType()const;

  /// 
  /// Return a string uniquely identifying the connection within the current process
  QString GetId()const; 

  /// 
  /// VTK Callback
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);

  /// 
  /// Called by 'DoCallback' to emit signal
  void Execute(vtkObject* vtk_obj, unsigned long vtk_event, void* client_data, void* call_data);

  /// 
  /// The flag is set to true in Execute if the vtkObject emitted a DeleteEvent event.
  /// This tells that the object will delete itself at the end of Execute(). Knowing
  /// that information can be useful to prevent a multiple deletion of the same object
  bool isAboutToBeDeleted() const;

signals:
  /// 
  /// The qt signal emited by the VTK Callback
  /// The signal corresponding to the slot will be emited
  void emitExecute(vtkObject* caller, vtkObject* call_data);
  /// Note: even if the signal has a signature with 4 args, you can
  /// connect it to a slot with less arguments as long as the types of the 
  /// argument are matching:
  /// connect(obj1,SIGNAL(signalFunc(A,B,C,D)),obj2,SLOT(slotFunc(A)));
  void emitExecute(vtkObject* caller, void* call_data, unsigned long vtk_event, void* client_data);

protected slots:
  void deleteConnection();

protected:
  void EstablishConnection();
  void BreakConnection();


private:
  CTK_DECLARE_PRIVATE(qVTKConnection);
};

#endif
