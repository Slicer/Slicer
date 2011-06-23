/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qSlicer includes
#include "qSlicerIOOptionsWidget.h"

//------------------------------------------------------------------------------
qSlicerIOOptionsWidget::qSlicerIOOptionsWidget(QWidget* parentWidget)
  :qSlicerWidget(parentWidget)
{
}

//------------------------------------------------------------------------------
qSlicerIOOptionsWidget::~qSlicerIOOptionsWidget()
{
}

//------------------------------------------------------------------------------
bool qSlicerIOOptionsWidget::isValid()const
{
  return this->Properties.contains("fileName") || this->Properties.contains("fileNames");
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::setFileName(const QString& fileName)
{
  bool oldValid = this->isValid();
  // replace the old filename if any
  if (!fileName.isEmpty())
    {
    this->Properties["fileName"] = fileName;
    }
  else
    {
    this->Properties.remove("fileName");
    }
  this->Properties.remove("fileNames");
  bool newValid = this->isValid();
  if (oldValid != newValid)
    {
    emit this->validChanged(newValid);
    }
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  bool oldValid = this->isValid();
  if (fileNames.count())
    {
    this->Properties["fileNames"] = fileNames;
    }
  else
    {
    this->Properties.remove("fileNames");
    }
  this->Properties.remove("fileName");
  bool newValid = this->isValid();
  if (oldValid != newValid)
    {
    emit this->validChanged(newValid);
    }
}
