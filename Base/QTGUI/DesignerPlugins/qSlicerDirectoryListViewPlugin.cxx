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
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

#include "qSlicerDirectoryListViewPlugin.h"
#include "qSlicerDirectoryListView.h"

qSlicerDirectoryListViewPlugin::qSlicerDirectoryListViewPlugin(QObject* parent)
  : QObject(parent)
{
}

QWidget *qSlicerDirectoryListViewPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerDirectoryListView* widget = new qSlicerDirectoryListView(parentWidget);
  return widget;
}

QString qSlicerDirectoryListViewPlugin::domXml() const
{
  return "<widget class=\"qSlicerDirectoryListView\" \
          name=\"SlicerDirectoryListView\">\n"
          "</widget>\n";
}

QString qSlicerDirectoryListViewPlugin::includeFile() const
{
  return "qSlicerDirectoryListView.h";
}

bool qSlicerDirectoryListViewPlugin::isContainer() const
{
  return false;
}

QString qSlicerDirectoryListViewPlugin::name() const
{
  return "qSlicerDirectoryListView";
}
