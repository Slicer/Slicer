/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QVariant>

// Slicer includes
#include "qSlicerRelativePathMapper.h"
#include "qSlicerCoreApplication.h"

//-----------------------------------------------------------------------------
class qSlicerRelativePathMapperPrivate
{
public:
  qSlicerRelativePathMapperPrivate();
  QByteArray PropertyName;
};

// --------------------------------------------------------------------------
qSlicerRelativePathMapperPrivate::qSlicerRelativePathMapperPrivate() {}

// --------------------------------------------------------------------------
// qSlicerRelativePathMapper methods

// --------------------------------------------------------------------------
qSlicerRelativePathMapper::qSlicerRelativePathMapper(QObject* targetObject,
                                                     const QByteArray& property,
                                                     const QByteArray& signal)
  : QObject(targetObject)
  , d_ptr(new qSlicerRelativePathMapperPrivate)
{
  Q_ASSERT(!property.isEmpty());
  Q_ASSERT(targetObject != nullptr);
  Q_D(qSlicerRelativePathMapper);
  d->PropertyName = property;
  if (!signal.isEmpty())
  {
    if (QString(this->targetObject()->property(this->propertyName()).typeName()).compare("QStringList") == 0)
    {
      // Property is a QStringList
      connect(targetObject, signal, this, SLOT(emitPathsChanged()));
    }
    else
    {
      connect(targetObject, signal, this, SLOT(emitPathChanged()));
    }
  }
}

// --------------------------------------------------------------------------
qSlicerRelativePathMapper::~qSlicerRelativePathMapper() {}

// --------------------------------------------------------------------------
QByteArray qSlicerRelativePathMapper::propertyName() const
{
  Q_D(const qSlicerRelativePathMapper);
  return d->PropertyName;
}

// --------------------------------------------------------------------------
QObject* qSlicerRelativePathMapper::targetObject() const
{
  return this->parent();
}

// --------------------------------------------------------------------------
QString qSlicerRelativePathMapper::path() const
{
  Q_D(const qSlicerRelativePathMapper);
  return this->targetObject()->property(this->propertyName()).toString();
}

// --------------------------------------------------------------------------
QStringList qSlicerRelativePathMapper::paths() const
{
  Q_D(const qSlicerRelativePathMapper);
  return this->targetObject()->property(this->propertyName()).toStringList();
}

// --------------------------------------------------------------------------
QString qSlicerRelativePathMapper::relativePath() const
{
  Q_D(const qSlicerRelativePathMapper);
  return qSlicerCoreApplication::application()->toSlicerHomeRelativePath(this->path());
}

// --------------------------------------------------------------------------
QStringList qSlicerRelativePathMapper::relativePaths() const
{
  Q_D(const qSlicerRelativePathMapper);
  return qSlicerCoreApplication::application()->toSlicerHomeRelativePaths(this->paths());
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::setPath(const QString& newPath)
{
  Q_D(qSlicerRelativePathMapper);
  if (this->path() == newPath)
  {
    return;
  }
  this->targetObject()->setProperty(this->propertyName(), QVariant(newPath));
  this->emitPathChanged();
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::setPaths(const QStringList& newPaths)
{
  Q_D(qSlicerRelativePathMapper);
  if (this->paths() == newPaths)
  {
    return;
  }
  this->targetObject()->setProperty(this->propertyName(), QVariant(newPaths));
  this->emitPathsChanged();
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::setRelativePath(const QString& newRelativePath)
{
  Q_D(qSlicerRelativePathMapper);
  this->setPath(qSlicerCoreApplication::application()->toSlicerHomeAbsolutePath(newRelativePath));
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::setRelativePaths(const QStringList& newRelativePaths)
{
  Q_D(qSlicerRelativePathMapper);
  this->setPaths(qSlicerCoreApplication::application()->toSlicerHomeAbsolutePaths(newRelativePaths));
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::emitPathChanged()
{
  emit pathChanged(this->path());
  emit relativePathChanged(this->relativePath());
}

// --------------------------------------------------------------------------
void qSlicerRelativePathMapper::emitPathsChanged()
{
  emit pathsChanged(this->paths());
  emit relativePathsChanged(this->relativePaths());
}
