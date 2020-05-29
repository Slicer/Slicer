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

#include "qMRMLSceneFactoryWidgetPlugin.h"
#include "qMRMLSceneFactoryWidget.h"

// qMRML includes

// Qt includes

// --------------------------------------------------------------------------
qMRMLSceneFactoryWidgetPlugin::qMRMLSceneFactoryWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSceneFactoryWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSceneFactoryWidget* _widget = new qMRMLSceneFactoryWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSceneFactoryWidget\" \
          name=\"MRMLSceneFactoryWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::includeFile() const
{
  return "qMRMLSceneFactoryWidgetPlugin.h";
}

// --------------------------------------------------------------------------
bool qMRMLSceneFactoryWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::name() const
{
  return "qMRMLSceneFactoryWidget";
}
