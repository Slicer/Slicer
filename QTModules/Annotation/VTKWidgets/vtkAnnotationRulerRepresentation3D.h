/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManagerHelper,v $
 Date:      $Date: Aug 4, 2010 10:44:52 AM $
 Version:   $Revision: 1.0 $

 =========================================================================auto=*/

#ifndef VTKANNOTATIONRULERREPRESENTATION3D_H_
#define VTKANNOTATIONRULERREPRESENTATION3D_H_

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// VTK includes
#include <vtkDistanceRepresentation3D.h>

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkAnnotationRulerRepresentation3D :
    public vtkDistanceRepresentation3D
{
public:

  static vtkAnnotationRulerRepresentation3D *New();
  vtkTypeRevisionMacro(vtkAnnotationRulerRepresentation3D, vtkDistanceRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetDistance(double distance);

  // Description:
  // Scale the glyphs used as tick marks
  void SetGlyphScale(double scale);
  vtkGetMacro(GlyphScale, double);

  // Description:
  // Get the line actor property
  virtual vtkProperty *GetLineProperty();

  // Description:
  // Set/Get position of the label title. 0 is at the start of the
  // line whereas 1 is at the end.
  //vtkSetMacro(LabelPosition, double);
  void SetLabelPosition(double labelPosition);
  vtkGetMacro(LabelPosition, double);

  // Description:
  // Set/Get the maximum number of ticks in ruler mode
  vtkSetMacro(MaxTicks, int);
  vtkGetMacro(MaxTicks, int);

  // Description:
  // Change the polydata displayed as tick markers
  void UpdateGlyphPolyData(vtkPolyData *polyData);

  // Description:
  // Get the glyph actor
  vtkGetObjectMacro(GlyphActor, vtkActor);
  
  // Description:
  // Get the label actor
  vtkGetObjectMacro(LabelActor, vtkFollower);

protected:

  vtkAnnotationRulerRepresentation3D();
  virtual ~vtkAnnotationRulerRepresentation3D();

  void BuildRepresentation();

  // Internal use: set the label actor's position from current world point 1
  // and 2 positions and label position factor
  void UpdateLabelActorPosition();


private:

  vtkAnnotationRulerRepresentation3D(const vtkAnnotationRulerRepresentation3D&); /// Not implemented
  void operator=(const vtkAnnotationRulerRepresentation3D&); /// Not Implemented

  double m_Distance;

  // Maximum number of ticks on the 3d ruler
  int MaxTicks;

  bool GlyphScaleSpecified;
  
  // Label title position
  double LabelPosition;
 
  // Glyph3D scale
  double GlyphScale;

};

#endif /* VTKANNOTATIONRULERREPRESENTATION3D_H_ */
