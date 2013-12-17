#ifndef __qMRMLEventLogger_p_h
#define __qMRMLEventLogger_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

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
class qMRMLEventLoggerPrivate: public QObject
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLEventLogger);
protected:
  qMRMLEventLogger* const q_ptr;
public:
  qMRMLEventLoggerPrivate(qMRMLEventLogger& object);
  typedef QObject Superclass;

  void init();

  void setMRMLScene(vtkMRMLScene* scene);

private:
  vtkWeakPointer<vtkMRMLScene> MRMLScene;

  QList<QString>          EventToListen;
  QHash<QString, QString> EventNameToConnectionIdMap;

  bool ConsoleOutputEnabled;
};

#endif
