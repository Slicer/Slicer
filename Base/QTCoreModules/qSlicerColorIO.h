#ifndef __qSlicerColorIO
#define __qSlicerColorIO

// SlicerQt includes
#include "qSlicerIO.h"

//-----------------------------------------------------------------------------
class qSlicerColorIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerColorIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

#endif
