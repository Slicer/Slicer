
/*=========================================================================

  Program:   FaceHedgeHog
  Module:    $RCSfile: vtkFaceHedgeHog.h,v $

  Copyright KnowledgeVis,LLC
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE. 

  Description:  Draw oriented vectors from the center of the faces of 
                            a polygonal object. 
                            
=========================================================================*/

#ifndef VTKFACEHEDGEHOG_H_
#define VTKFACEHEDGEHOG_H_

#include "vtkPolyDataAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

#define VTK_USE_VECTOR 0
#define VTK_USE_NORMAL 1


class VTK_MIMXFILTER_EXPORT vtkFaceHedgeHog  : public vtkPolyDataAlgorithm
{
public:
          static vtkFaceHedgeHog* New();
          vtkTypeRevisionMacro(vtkFaceHedgeHog,vtkPolyDataAlgorithm);
          void PrintSelf(ostream& os, vtkIndent indent);

           // Description:
          // Set scale factor to control size of oriented lines.
          vtkSetMacro(ScaleFactor,double);
          vtkGetMacro(ScaleFactor,double);

          // Description:
          // Specify whether to use vector or normal to perform vector operations.
          vtkSetMacro(VectorMode,int);
          vtkGetMacro(VectorMode,int);
          void SetVectorModeToUseVector() {this->SetVectorMode(VTK_USE_VECTOR);};
          void SetVectorModeToUseNormal() {this->SetVectorMode(VTK_USE_NORMAL);};
          const char *GetVectorModeAsString();

        
protected:
        vtkFaceHedgeHog();
    ~vtkFaceHedgeHog() {};

   virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
    virtual int FillInputPortInformation(int port, vtkInformation *info);
    double ScaleFactor;
    int VectorMode; // Orient/scale via normal or via vector data

  private:
        vtkFaceHedgeHog(const vtkFaceHedgeHog&);  // Not implemented.
    void operator=(const vtkFaceHedgeHog&);  // Not implemented.
    
    // used internally for calculating face normals
    int CalculateFaceNormal(vtkCell* cell, double* normal);


};

// Description:
// Return the vector mode as a character string.
inline const char *vtkFaceHedgeHog::GetVectorModeAsString(void)
{
  if ( this->VectorMode == VTK_USE_VECTOR) 
    {
    return "UseVector";
    }
  else if ( this->VectorMode == VTK_USE_NORMAL) 
    {
    return "UseNormal";
    }
  else 
    {
    return "Unknown";
    }
}

#endif /*vtkFaceHedgeHog_H_*/
