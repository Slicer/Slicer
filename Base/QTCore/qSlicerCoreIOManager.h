/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCoreIOManager_h
#define __qSlicerCoreIOManager_h

// Qt includes
#include <QObject>
#include <QMap>

// CTK includes
#include <ctkPimpl.h>

// QtCore includes
#include <qSlicerIO.h>
#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLNode;
class vtkMRMLScene; 
class vtkCollection;
class qSlicerCoreIOManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreIOManager:public QObject
{
  Q_OBJECT;
public:
  qSlicerCoreIOManager(QObject* parent = 0);
  virtual ~qSlicerCoreIOManager();

  ///
  /// Return the file type associated with a \a file
  qSlicerIO::IOFileType fileType(const QString& file)const;

  ///
  /// Return the file description associated with a \a file
  /// Usually the description is a short text of one or two words
  QString fileDescription(const QString& file)const;

  ///
  /// Return the file option associated with a \a file
  inline qSlicerIOOptions* fileOptions(const QString& file)const;
  
  ///
  /// Return the file option associated with a \a file type
  qSlicerIOOptions* fileOptions(const qSlicerIO::IOFileType& fileType)const;

  ///
  /// Load a list of nodes corresponding to \a fileType. A given \a fileType corresponds
  /// to a specific reader qSlicerIO.
  /// A map of qvariant allows to specify which \a parameters should be passed to the reader.
  /// The function return 0 if it fails.
  ///
  /// The map associated with most of the \a fileType should contains either
  /// fileName (QString) or filenames (QStringList).
  /// More specific parameters could also be set. For example, the volume reader qSlicerVolumesIO
  /// could also be called with the following parameters: LabelMap (bool), Center (bool)
  ///
  /// Note: Make also sure the case of parameter name is respected
  ///
  /// \sa qSlicerIO::IOProperties, qSlicerIO::IOFileType
  bool loadNodes(const qSlicerIO::IOFileType& fileType,
                 const qSlicerIO::IOProperties& parameters,
                 vtkCollection* loadedNodes = 0);


  ///
  /// Load a list of node corresponding to \a fileType and return the first loaded node.
  /// This function is provided for convenience and is equivalent to call loadNodes
  /// with a vtkCollection parameter and retrieve the first element.
  vtkMRMLNode* loadNodesAndGetFirst(qSlicerIO::IOFileType fileType,
                                    const qSlicerIO::IOProperties& parameters);
   

  /// 
  /// Load/import a scene corresponding to \a fileName
  /// This function is provided for convenience and is equivalent to call
  /// loadNodes function with qSlicerIO::SceneFile
  bool loadScene(const QString& fileName, bool clear = true);

  ///
  /// attributes are typically: 
  /// All: fileName[s] 
  /// Volume: LabelMap:bool, Center:bool, fileNames:QList<QString>...
  bool saveNodes(qSlicerIO::IOFileType fileType,
                 const qSlicerIO::IOProperties& parameters);
  
  ///
  /// Register the reader/writer \a io
  /// Note also that the IOManager takes ownership of \a io
  void registerIO(qSlicerIO* io);
  
protected:
  ///
  /// Returns the list of registered readers/writers
  const QList<qSlicerIO*>& ios()const;

  ///
  /// Returns the list of registered readers or writers associated with \a fileType
  QList<qSlicerIO*> ios(const qSlicerIO::IOFileType& fileType)const;
  
private:
  CTK_DECLARE_PRIVATE(qSlicerCoreIOManager);
};

qSlicerIOOptions* qSlicerCoreIOManager::fileOptions(const QString& file)const
{
  return this->fileOptions(this->fileType(file));
}

#endif

