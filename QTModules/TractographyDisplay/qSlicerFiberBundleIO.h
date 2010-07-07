#ifndef __qSlicerFiberBundleIO
#define __qSlicerFiberBundleIO

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIO.h"

//-----------------------------------------------------------------------------
class qSlicerFiberBundleIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerFiberBundleIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

#endif
