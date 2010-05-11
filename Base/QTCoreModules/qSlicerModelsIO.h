#ifndef __qSlicerModelsIO
#define __qSlicerModelsIO

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIO.h"

//-----------------------------------------------------------------------------
class qSlicerModelsIO: public qSlicerIO
{
  Q_OBJECT
public: 
  qSlicerModelsIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

//-----------------------------------------------------------------------------
class qSlicerScalarOverlayIO: public qSlicerIO
{
  Q_OBJECT
public: 
  qSlicerScalarOverlayIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
};

#endif
