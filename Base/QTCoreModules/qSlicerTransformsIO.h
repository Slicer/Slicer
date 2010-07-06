#ifndef __qSlicerTransformsIO
#define __qSlicerTransformsIO

// SlicerQt includes
#include "qSlicerIO.h"

//-----------------------------------------------------------------------------
class qSlicerTransformsIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerTransformsIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

#endif
