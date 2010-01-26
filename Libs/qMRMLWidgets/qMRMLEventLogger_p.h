#ifndef __qMRMLEventLogger_p_h
#define __qMRMLEventLogger_p_h

#include "qMRMLEventLogger.h"

// qVTK includes
#include <qVTKObject.h>

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

// VTK includes
#include <vtkWeakPointer.h>

class vtkMRMLScene;

//------------------------------------------------------------------------------
class qMRMLEventLoggerPrivate: public QObject, public qCTKPrivate<qMRMLEventLogger>
{
  QVTK_OBJECT
public:
  QCTK_DECLARE_PUBLIC(qMRMLEventLogger);

  typedef QObject Superclass;
  qMRMLEventLoggerPrivate();

  void init();

  void setMRMLScene(vtkMRMLScene* scene); 

private:
  vtkWeakPointer<vtkMRMLScene> MRMLScene;

  QList<QString>          EventToListen;
  QHash<QString, QString> EventNameToConnectionIdMap;
};

#endif
