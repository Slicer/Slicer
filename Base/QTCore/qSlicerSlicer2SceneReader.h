#ifndef __qSlicerSlicer2SceneReader
#define __qSlicerSlicer2SceneReader

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIO.h"

class qSlicerSlicer2SceneReaderPrivate;

//-----------------------------------------------------------------------------
class qSlicerSlicer2SceneReader: public qSlicerIO
{
  Q_OBJECT
public: 
  qSlicerSlicer2SceneReader(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
private:
  CTK_DECLARE_PRIVATE(qSlicerSlicer2SceneReader);
};

#endif
