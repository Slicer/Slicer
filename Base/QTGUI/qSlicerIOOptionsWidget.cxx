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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// qSlicer includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerIOOptionsWidget.h"

//------------------------------------------------------------------------------
qSlicerIOOptionsWidget::qSlicerIOOptionsWidget(QWidget* parentWidget)
  : qSlicerWidget(parentWidget)
{
}

//------------------------------------------------------------------------------
qSlicerIOOptionsWidget
::qSlicerIOOptionsWidget(qSlicerIOOptionsPrivate* pimpl, QWidget* parentWidget)
  : qSlicerWidget(parentWidget)
  , qSlicerIOOptions(pimpl)
{
}
//------------------------------------------------------------------------------
qSlicerIOOptionsWidget::~qSlicerIOOptionsWidget() = default;

//------------------------------------------------------------------------------
bool qSlicerIOOptionsWidget::isValid()const
{
  Q_D(const qSlicerIOOptions);
  return d->Properties.contains("fileName") || d->Properties.contains("fileNames");
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::updateValid()
{
  Q_D(const qSlicerIOOptions);
  bool wasValid = d->ArePropertiesValid;
  this->Superclass::updateValid();
  if (wasValid != d->ArePropertiesValid)
    {
    emit this->validChanged(d->ArePropertiesValid);
    }
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::setFileName(const QString& fileName)
{
  Q_D(qSlicerIOOptions);
  // replace the old filename if any
  if (!fileName.isEmpty())
    {
    d->Properties["fileName"] = fileName;
    }
  else
    {
    d->Properties.remove("fileName");
    }
  d->Properties.remove("fileNames");
  this->updateValid();
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  Q_D(qSlicerIOOptions);
  /*
  if (fileNames.count())
    {
    this->Properties["fileNames"] = fileNames;
    }
  else
    {
    this->Properties.remove("fileNames");
    }
  this->Properties.remove("fileName");
  */
  d->Properties["fileName"] = fileNames;
  this->updateValid();
}

//------------------------------------------------------------------------------
void qSlicerIOOptionsWidget::updateGUI(const qSlicerIO::IOProperties& ioProperties)
{
  Q_UNUSED(ioProperties);
  // derived classes should implement update of GUI based on provided properties
}
