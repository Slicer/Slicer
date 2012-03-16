/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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
class vtkCollection;
class qSlicerCoreIOManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreIOManager:public QObject
{
  Q_OBJECT;
public:
  qSlicerCoreIOManager(QObject* parent = 0);
  virtual ~qSlicerCoreIOManager();

  /// Return the file type associated with a \a file
  Q_INVOKABLE qSlicerIO::IOFileType fileType(const QString& file)const;
  Q_INVOKABLE QList<qSlicerIO::IOFileType> fileTypes(const QString& file)const;
  Q_INVOKABLE qSlicerIO::IOFileType fileTypeFromDescription(const QString& fileDescription)const;

  /// Return the file description associated with a \a file
  /// Usually the description is a short text of one or two words
  QStringList fileDescriptions(const QString& file)const;
  QStringList fileDescriptions(const qSlicerIO::IOFileType fileType)const;

  /// Return the file option associated with a \a file type
  qSlicerIOOptions* fileOptions(const QString& fileDescription)const;

  /// Load a list of nodes corresponding to \a fileType. A given \a fileType corresponds
  /// to a specific reader qSlicerIO.
  /// A map of qvariant allows to specify which \a parameters should be passed to the reader.
  /// The function return 0 if it fails.
  /// The map associated with most of the \a fileType should contains either
  /// fileName (QString or QStringList) or fileNames (QStringList).
  /// More specific parameters could also be set. For example, the volume reader qSlicerVolumesIO
  /// could also be called with the following parameters: LabelMap (bool), Center (bool)
  /// \note Make also sure the case of parameter name is respected
  /// \sa qSlicerIO::IOProperties, qSlicerIO::IOFileType
#if QT_VERSION < 0x040700
  Q_INVOKABLE virtual bool loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const QVariantMap& parameters,
                                     vtkCollection* loadedNodes = 0);
#else
  Q_INVOKABLE virtual bool loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes = 0);
#endif

  /// Utility function that loads a bunch of files. The "fileType" attribute should
  /// in the parameter map for each node to load.
  virtual bool loadNodes(const QList<qSlicerIO::IOProperties>& files,
                         vtkCollection* loadedNodes = 0);

  /// Load a list of node corresponding to \a fileType and return the first loaded node.
  /// This function is provided for convenience and is equivalent to call loadNodes
  /// with a vtkCollection parameter and retrieve the first element.
  vtkMRMLNode* loadNodesAndGetFirst(qSlicerIO::IOFileType fileType,
                                    const qSlicerIO::IOProperties& parameters);
   

  /// Load/import a scene corresponding to \a fileName
  /// This function is provided for convenience and is equivalent to call
  /// loadNodes function with qSlicerIO::SceneFile
  bool loadScene(const QString& fileName, bool clear = true);

  /// Attributes are typically:
  /// All: fileName[s] 
  /// Volume: LabelMap:bool, Center:bool, fileNames:QList<QString>...
  bool saveNodes(qSlicerIO::IOFileType fileType,
                 const qSlicerIO::IOProperties& parameters);

  /// Register the reader/writer \a io
  /// Note also that the IOManager takes ownership of \a io
  void registerIO(qSlicerIO* io);
  
protected:

  /// Returns the list of registered readers/writers
  const QList<qSlicerIO*>& ios()const;

  /// Returns the list of registered readers or writers associated with \a fileType
  QList<qSlicerIO*> ios(const qSlicerIO::IOFileType& fileType)const;
  qSlicerIO* io(const QString& ioDescription)const;

protected:
  QScopedPointer<qSlicerCoreIOManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreIOManager);
  Q_DISABLE_COPY(qSlicerCoreIOManager);
};

#endif

