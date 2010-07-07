// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerFiberBundleIO.h"

// Logic includes
#include "vtkSlicerFiberBundleLogic.h"

// MRML includes
#include <vtkMRMLFiberBundleNode.h>

//-----------------------------------------------------------------------------
qSlicerFiberBundleIO::qSlicerFiberBundleIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerFiberBundleIO::description()const
{
  return "FiberBundle";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerFiberBundleIO::fileType()const
{
  return qSlicerIO::DTIFile;
}

//-----------------------------------------------------------------------------
QString qSlicerFiberBundleIO::extensions()const
{
  return "*.*";
}

//-----------------------------------------------------------------------------
bool qSlicerFiberBundleIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QStringList fileNames;
  if (properties.contains("suffix"))
    {
    QStringList suffixList = properties["suffix"].toStringList();
    suffixList.removeDuplicates();
    // here filename describes a directory
    Q_ASSERT(QFileInfo(fileName).isDir());
    QDir dir(fileName);
    // suffix should be of style: *.png
    fileNames = dir.entryList(suffixList);
    }
  else
    {
    fileNames << fileName;
    }

  vtkSlicerFiberBundleLogic* fiberBundleLogic =
    vtkSlicerFiberBundleLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
      ->module("tractographydisplay")->logic());
  Q_ASSERT(fiberBundleLogic);

  QStringList nodes;
  foreach(QString file, fileNames)
    {
    vtkMRMLFiberBundleNode* node = fiberBundleLogic->AddFiberBundle(file.toLatin1().data(), 1);
    if (node)
      {
      nodes << node->GetID();
      }
    }
  this->setLoadedNodes(nodes);

  return nodes.size() > 0;
}
