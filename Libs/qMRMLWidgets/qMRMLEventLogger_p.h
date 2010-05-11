#ifndef __qMRMLEventLogger_p_h
#define __qMRMLEventLogger_p_h

/// Qt includes
#include <QHash>
#include <QObject>
#include <QString>

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLEventLogger.h"

/// VTK includes
#include <vtkWeakPointer.h>

class vtkMRMLScene;

//------------------------------------------------------------------------------
class qMRMLEventLoggerPrivate: public QObject, public ctkPrivate<qMRMLEventLogger>
{
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLEventLogger);

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
