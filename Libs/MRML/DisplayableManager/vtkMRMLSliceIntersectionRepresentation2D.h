/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionRepresentation2D.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkMRMLSliceIntersectionRepresentation2D
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkMRMLSliceIntersectionRepresentation2D_h
#define vtkMRMLSliceIntersectionRepresentation2D_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidgetRepresentation.h"

#include "vtkMRMLSliceNode.h"

class vtkMRMLApplicationLogic;
class vtkMRMLModelDisplayNode;
class vtkMRMLSliceLogic;

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkTextProperty;
class vtkLeaderActor2D;
class vtkTextMapper;
class vtkTransform;
class vtkActor2D;

class SliceIntersectionDisplayPipeline;


class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceIntersectionRepresentation2D : public vtkMRMLAbstractWidgetRepresentation
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLSliceIntersectionRepresentation2D *New();

  //@{
  /**
   * Standard methods for instances of this class.
   */
  vtkTypeMacro(vtkMRMLSliceIntersectionRepresentation2D, vtkMRMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  vtkMRMLSliceNode* GetSliceNode();

  void AddIntersectingSliceLogic(vtkMRMLSliceLogic* sliceLogic);
  void RemoveIntersectingSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateIntersectingSliceNodes();
  void RemoveAllIntersectingSliceNodes();

  //@{
  /**
   * Methods to make this class behave as a vtkProp.
   */
  void GetActors2D(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  //@}

  void SetMRMLApplicationLogic(vtkMRMLApplicationLogic*);
  vtkGetObjectMacro(MRMLApplicationLogic, vtkMRMLApplicationLogic);

  double* GetSliceIntersectionPoint();

  void TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix);

protected:
  vtkMRMLSliceIntersectionRepresentation2D();
  ~vtkMRMLSliceIntersectionRepresentation2D() override;

  SliceIntersectionDisplayPipeline* GetDisplayPipelineFromSliceLogic(vtkMRMLSliceLogic* sliceLogic);

  static void SliceNodeModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  void SliceNodeModified(vtkMRMLSliceNode* sliceNode);
  void SliceModelDisplayNodeModified(vtkMRMLModelDisplayNode* sliceNode);

  void UpdateSliceIntersectionDisplay(SliceIntersectionDisplayPipeline *pipeline);

  double GetSliceRotationAngleRad(int eventPos[2]);

  // The internal transformation matrix
  vtkTransform *CurrentTransform;
  vtkTransform *TotalTransform;
  double Origin[4]; //the current origin in world coordinates
  double DisplayOrigin[3]; //the current origin in display coordinates
  double CurrentTranslation[3]; //translation this movement
  double StartWorldPosition[4]; //Start event position converted to world

  // Support picking
  double LastEventPosition[2];

  // Slice intersection point in XY
  double SliceIntersectionPoint[4];

  vtkMRMLApplicationLogic* MRMLApplicationLogic;

  class vtkInternal;
  vtkInternal * Internal;

private:
  vtkMRMLSliceIntersectionRepresentation2D(const vtkMRMLSliceIntersectionRepresentation2D&) = delete;
  void operator=(const vtkMRMLSliceIntersectionRepresentation2D&) = delete;
};

#endif
