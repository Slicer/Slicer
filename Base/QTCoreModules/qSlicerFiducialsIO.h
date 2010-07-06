#ifndef __qSlicerFiducialsIO
#define __qSlicerFiducialsIO

// SlicerQt includes
#include "qSlicerIO.h"

//-----------------------------------------------------------------------------
class qSlicerFiducialsIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerFiducialsIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

#endif
