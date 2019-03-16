/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __vtkSlicerPlotsLogic_h
#define __vtkSlicerPlotsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// Plots includes
#include "vtkSlicerPlotsModuleLogicExport.h"

class vtkAbstractArray;
class vtkMRMLTableNode;
class vtkMRMLPlotChartNode;
class vtkMRMLPlotSeriesNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
/// \brief Slicer logic class for double array manipulation
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the double array nodes
class VTK_SLICER_PLOTS_MODULE_LOGIC_EXPORT vtkSlicerPlotsLogic
  : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPlotsLogic *New();
  vtkTypeMacro(vtkSlicerPlotsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Returns ID of the layout that is similar to current layout but also contains a table view
  static int GetLayoutWithPlot(int currentLayout);

  /// Create a deep copy of a \a source and add it to the current scene.
  /// \sa GetMRMLScene()
  vtkMRMLPlotSeriesNode* CloneSeries(vtkMRMLPlotSeriesNode* source, const char *name);

  /// Show chart in view layout.
  /// Switches to a layout that contains a plot and propagates
  void ShowChartInLayout(vtkMRMLPlotChartNode* chartNode);

  /// Finds the first plot chart that contains the specified series
  vtkMRMLPlotChartNode* GetFirstPlotChartForSeries(vtkMRMLPlotSeriesNode* seriesNode);

protected:
  vtkSlicerPlotsLogic();
  ~vtkSlicerPlotsLogic() override;

private:
  vtkSlicerPlotsLogic(const vtkSlicerPlotsLogic&) = delete;
  void operator=(const vtkSlicerPlotsLogic&) = delete;
};

#endif
