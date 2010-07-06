#ifndef __qSlicerScalarOverlayIO
#define __qSlicerScalarOverlayIO

// SlicerQt includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTCoreModulesExport.h"

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerScalarOverlayIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerScalarOverlayIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;
  virtual qSlicerIOOptions* options()const;

  virtual bool load(const IOProperties& properties);
};

#endif
