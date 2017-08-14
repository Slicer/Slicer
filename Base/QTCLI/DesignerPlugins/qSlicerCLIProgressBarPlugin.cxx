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

#include "qSlicerCLIProgressBarPlugin.h"
#include "qSlicerCLIProgressBar.h"

//-----------------------------------------------------------------------------
qSlicerCLIProgressBarPlugin::qSlicerCLIProgressBarPlugin(QObject* parent)
  : QObject(parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerCLIProgressBarPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerCLIProgressBar* widget = new qSlicerCLIProgressBar(parentWidget);
  return widget;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIProgressBarPlugin::domXml() const
{
  return "<widget class=\"qSlicerCLIProgressBar\" \
          name=\"CLIProgressBar\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerCLIProgressBarPlugin::includeFile() const
{
  return "qSlicerCLIProgressBar.h";
}

//-----------------------------------------------------------------------------
bool qSlicerCLIProgressBarPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIProgressBarPlugin::name() const
{
  return "qSlicerCLIProgressBar";
}
