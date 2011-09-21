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

#include "qMRMLAnnotationROIWidgetPlugin.h"
#include "qMRMLAnnotationROIWidget.h"

//-----------------------------------------------------------------------------
qMRMLAnnotationROIWidgetPlugin::qMRMLAnnotationROIWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLAnnotationROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLAnnotationROIWidget* _widget = new qMRMLAnnotationROIWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLAnnotationROIWidget\" \
          name=\"MRMLAnnotationROIWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationROIWidgetPlugin::includeFile() const
{
  return "qMRMLAnnotationROIWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLAnnotationROIWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationROIWidgetPlugin::name() const
{
  return "qMRMLAnnotationROIWidget";
}
