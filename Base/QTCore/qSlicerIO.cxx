/// Qt includes
#include <QFileInfo>
#include <QRegExp>

/// QtCore includes
#include "qSlicerIO.h"

/// MRML includes
#include <vtkMRMLScene.h>

/// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerIOPrivate: public ctkPrivate<qSlicerIO>
{
public:
  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  
  QStringList LoadedNodes;
  QStringList SavedNodes;
};

//----------------------------------------------------------------------------
qSlicerIO::qSlicerIO(QObject* _parent)
  :QObject(_parent)
{
  CTK_INIT_PRIVATE(qSlicerIO);
}

//----------------------------------------------------------------------------
qSlicerIO::~qSlicerIO()
{
}

/*
//----------------------------------------------------------------------------
vtkStdString qSlicerIO::description()const
{
  // FIXME: abstract method ?
  return "File";
}

//----------------------------------------------------------------------------
IOFileType qSlicerIO::fileType()const
{
  // FIXME: abstract method ?
  return NoFile;
}
*/

//----------------------------------------------------------------------------
QString qSlicerIO::extensions()const
{
  return "*.*";
}

//----------------------------------------------------------------------------
bool qSlicerIO::canLoadFile(const QString& fileName)const
{
  QFileInfo file(fileName);
  if (!file.isFile() || 
      !file.isReadable() || 
      file.suffix().contains('~'))
    {
    return false;
    }
  QStringList ext = this->extensions().split(' ');
  foreach(QString extension, ext)
    {
    QRegExp regExp(extension, Qt::CaseInsensitive, QRegExp::Wildcard);
    Q_ASSERT(regExp.isValid());
    if (regExp.exactMatch(file.absoluteFilePath()))
      {
      return true;
      }
    }
    return false;
}

//----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerIO::options()const
{
  return 0;
}

//----------------------------------------------------------------------------
void qSlicerIO::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qSlicerIO);
  d->MRMLScene = scene;
}

//----------------------------------------------------------------------------
vtkMRMLScene* qSlicerIO::mrmlScene()const
{
  CTK_D(const qSlicerIO);
  return d->MRMLScene;
}

//----------------------------------------------------------------------------
bool qSlicerIO::load(const IOProperties& properties)
{
  CTK_D(qSlicerIO);
  d->LoadedNodes.clear();
  return false;
}

//----------------------------------------------------------------------------
bool qSlicerIO::save(const IOProperties& properties)
{
  return false;
}

//----------------------------------------------------------------------------
void qSlicerIO::setLoadedNodes(const QStringList& nodes)
{
  CTK_D(qSlicerIO);
  d->LoadedNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerIO::loadedNodes()const
{
  CTK_D(const qSlicerIO);
  return d->LoadedNodes;
}

//----------------------------------------------------------------------------
void qSlicerIO::setSavedNodes(const QStringList& nodes)
{
  CTK_D(qSlicerIO);
  d->SavedNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerIO::savedNodes()const
{
  CTK_D(const qSlicerIO);
  return d->SavedNodes;
}
