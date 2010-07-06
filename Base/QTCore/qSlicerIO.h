#ifndef __qSlicerIO_h
#define __qSlicerIO_h

// Qt includes
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

// CTK includes
#include <ctkPimpl.h>

// QtCore includes
#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLScene;
class qSlicerIOOptions;
class qSlicerIOPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIO : public QObject
{
  Q_OBJECT
public:
  explicit qSlicerIO(QObject* parent = 0);
  virtual ~qSlicerIO();

  typedef int IOFileType; 
  enum IOFileTypes
  {
    NoFile = 0,
    SceneFile = 1, 
    VolumeFile = 2,
    TransformFile = 3,
    ModelFile = 4,
    ScalarOverlayFile = 5,
    ColorTableFile = 7,
    FiducialListFile = 8,
    UserFile = 32,
  };

  typedef QMap<QString, QVariant> IOProperties;

  virtual QString description()const = 0;
  virtual IOFileType fileType()const = 0;
  virtual QString extensions()const;
  bool canLoadFile(const QString& file)const;
  virtual qSlicerIOOptions* options()const;

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;
  
  virtual bool load(const IOProperties& properties);
  virtual bool save(const IOProperties& properties);

  QStringList loadedNodes()const;
  QStringList savedNodes()const;
  
protected:
  void setLoadedNodes(const QStringList& nodes);
  void setSavedNodes(const QStringList& nodes);
  
private:
  CTK_DECLARE_PRIVATE(qSlicerIO);
};

#endif
