/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital, based on a template developed by Jean-Christophe Fillion-Robin,
  Kitware Inc. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

#include "qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin.h"
#include "qMRMLAnnotationFiducialProjectionPropertyWidget.h"

//------------------------------------------------------------------------------
qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::createWidget(QWidget *_parent)
{
  qMRMLAnnotationFiducialProjectionPropertyWidget* _widget
    = new qMRMLAnnotationFiducialProjectionPropertyWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::domXml() const
{
  return "<widget class=\"qMRMLAnnotationFiducialProjectionPropertyWidget\" \
          name=\"MRMLAnnotationFiducialProjectionPropertyWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::includeFile() const
{
  return "qMRMLAnnotationFiducialProjectionPropertyWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLAnnotationFiducialProjectionPropertyWidgetPlugin
::name() const
{
  return "qMRMLAnnotationFiducialProjectionPropertyWidget";
}
