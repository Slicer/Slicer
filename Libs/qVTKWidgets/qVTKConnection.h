#ifndef __qVTKConnection_h
#define __qVTKConnection_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>
#include <QVector>

#include "qVTKWidgetsWin32Header.h"

class qVTKObjectEventsObserver;
class vtkObject;
class qVTKConnectionPrivate;

class QVTK_WIDGETS_EXPORT qVTKConnection : public QObject
{
Q_OBJECT

public:
  typedef qVTKConnection Self;
  typedef QObject Superclass;
  explicit qVTKConnection(qVTKObjectEventsObserver* parent);
  virtual ~qVTKConnection(){}

  // Description:
  virtual void printAdditionalInfo();
  QString getShortDescription();
  static QString getShortDescription(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot = "");

  // Description:
  void SetParameters(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot, float priority);

  // Description:
  static bool ValidateParameters(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot);

  // Description:
  void SetEstablished(bool enable);

  // Description:
  void SetBlocked(bool block);

  // Description:
  bool IsEqual(vtkObject* vtk_obj, unsigned long vtk_event,
    const QObject* qt_obj, QString qt_slot);

  // Description:
  int GetSlotType()const;

  // Description:
  // VTK Callback
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);

  // Description:
  // Called by 'DoCallback' to emit signal
  void Execute(vtkObject* vtk_obj, unsigned long vtk_event, void* client_data, void* call_data);


signals:
  // Description:
  // The qt signal emited by the VTK Callback
  // The signal corresponding to the slot will be emited
  void emitExecute();
  void emitExecute(void* call_data);
  void emitExecute(vtkObject* call_data);
  void emitExecute(void* call_data, vtkObject* caller);
  void emitExecute(vtkObject* call_data, vtkObject* caller);
  void emitExecute(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);

protected slots:
  void deleteConnection();

protected:
  void EstablishConnection();
  void BreakConnection();

private:
  QCTK_DECLARE_PRIVATE(qVTKConnection);
};

#endif
