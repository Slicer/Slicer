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

#include "qSlicerMouseModeToolBarPlugin.h"
#include "qSlicerMouseModeToolBar.h"

qSlicerMouseModeToolBarPlugin::qSlicerMouseModeToolBarPlugin(QObject* parent)
  : QObject(parent)
{
}

QWidget *qSlicerMouseModeToolBarPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerMouseModeToolBar* widget = new qSlicerMouseModeToolBar(parentWidget);
  return widget;
}

QString qSlicerMouseModeToolBarPlugin::domXml() const
{
  return "<widget class=\"qSlicerMouseModeToolBar\" \
          name=\"SlicerMouseModeToolBar\">\n"
          "</widget>\n";
}

QString qSlicerMouseModeToolBarPlugin::includeFile() const
{
  return "qSlicerMouseModeToolBar.h";
}

bool qSlicerMouseModeToolBarPlugin::isContainer() const
{
  return false;
}

QString qSlicerMouseModeToolBarPlugin::name() const
{
  return "qSlicerMouseModeToolBar";
}
