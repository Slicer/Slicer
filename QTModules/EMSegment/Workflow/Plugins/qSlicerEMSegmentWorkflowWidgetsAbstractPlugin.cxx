/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerEMSegmentWorkflowWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetsAbstractPlugin::qSlicerEMSegmentWorkflowWidgetsAbstractPlugin()
{
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetsAbstractPlugin::group() const
{ 
  return "Slicer [EMSegment Workflow]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentWorkflowWidgetsAbstractPlugin::icon() const
{
  return QIcon(); 
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetsAbstractPlugin::toolTip() const
{ 
  return QString(); 
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetsAbstractPlugin::whatsThis() const
{
  return QString(); 
}


