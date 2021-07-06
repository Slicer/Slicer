/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionInteractionRepresentationHelper.cxx

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkMRMLSliceIntersectionInteractionRepresentationHelper
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkMRMLSliceIntersectionInteractionRepresentationHelper_h
#define vtkMRMLSliceIntersectionInteractionRepresentationHelper_h

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
class vtkMRMLInteractionEventData;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceIntersectionInteractionRepresentationHelper : public vtkMRMLAbstractWidgetRepresentation
{
  public:
    /**
     * Instantiate this class.
     */
    static vtkMRMLSliceIntersectionInteractionRepresentationHelper* New();

    //@{
    /**
     * Standard methods for instances of this class.
     */
    vtkTypeMacro(vtkMRMLSliceIntersectionInteractionRepresentationHelper, vtkMRMLAbstractWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    //@}

    int IntersectWithFinitePlane(double n[3], double o[3], double pOrigin[3], double px[3], double py[3], double x0[3], double x1[3]);

    /// Compute intersection between a 2D line and the slice view boundaries
    void GetIntersectionWithSliceViewBoundaries(double* pointA, double* pointB, double* sliceViewBounds, double* intersectionPoint);

    /// Get boundaries of the slice view associated with a given vtkMRMLSliceNode
    void GetSliceViewBoundariesXY(vtkMRMLSliceNode* sliceNode, double* sliceViewBounds);

    int GetLineTipsFromIntersectingSliceNode(vtkMRMLSliceNode* intersectingSliceNode, vtkMatrix4x4* intersectingXYToXY,
        double intersectionLineTip1[3], double intersectionLineTip2[3]);

    void ComputeHandleToWorldTransformMatrix(double handlePosition[2], double handleOrientation[2], vtkMatrix4x4* handleToWorldTransformMatrix);
    void RotationMatrixFromVectors(double vector1[2], double vector2[2], vtkMatrix4x4* rotationMatrixHom);

  protected:
    vtkMRMLSliceIntersectionInteractionRepresentationHelper();
    ~vtkMRMLSliceIntersectionInteractionRepresentationHelper() override;

  private:
    vtkMRMLSliceIntersectionInteractionRepresentationHelper(const vtkMRMLSliceIntersectionInteractionRepresentationHelper&) = delete;
    void operator=(const vtkMRMLSliceIntersectionInteractionRepresentationHelper&) = delete;
};

#endif
