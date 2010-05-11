#ifndef __qSlicerXcedeCatalogIO
#define __qSlicerXcedeCatalogIO

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIO.h"

class qSlicerXcedeCatalogIOPrivate;

//-----------------------------------------------------------------------------
class qSlicerXcedeCatalogIO: public qSlicerIO
{
  Q_OBJECT
public: 
  qSlicerXcedeCatalogIO(QObject* parent = 0);
  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QString extensions()const;

  virtual bool load(const IOProperties& properties);
private:
  CTK_DECLARE_PRIVATE(qSlicerXcedeCatalogIO);
};

#endif
