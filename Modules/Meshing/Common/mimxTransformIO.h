/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxTransformIO.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mimxTransformIO_h
#define __mimxTransformIO_h

#include "vtkCommon.h"


#include "itkObject.h"
#include "itkBSplineDeformableTransform.h"
#include "itkIdentityTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkAffineTransform.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkThinPlateR2LogRSplineKernelTransform.h"


namespace itk
{
/** \class mimxTransformIO
 * \brief Convience functions for transform I/O. These were required
 * for building on Windows Visual studio because the object sizes
 * from the templated code. 
 */  
class VTK_MIMXCOMMON_EXPORT mimxTransformIO : public itk::Object
{
public:
  typedef mimxTransformIO            Self;
  typedef itk::Object            Superclass;
  typedef itk::SmartPointer<Self>      Pointer;
  typedef itk::SmartPointer<const Self>    ConstPointer;
  itkTypeMacro(iamTransformIO, itk::Object);
  itkNewMacro(Self);  

  typedef double BSplineCoordinateRepType;
  typedef itk::BSplineDeformableTransform<
               BSplineCoordinateRepType, 3, 3 >     BSplineTransformType;
  
  typedef itk::VersorRigid3DTransform< double >     RigidTransformType;
  typedef itk::AffineTransform<double, 3>       AffineTransformType;
  typedef itk::IdentityTransform<double, 3>     IdentityTransformType;
  typedef itk::ThinPlateR2LogRSplineKernelTransform<BSplineCoordinateRepType,3 >  ThinPlateSplineTransformType;
  
  
  /*** Get/Set the Images for I/O Routines ***/
  itkSetObjectMacro (RigidTransform, RigidTransformType);
  itkSetObjectMacro (AffineTransform, AffineTransformType);
  itkSetObjectMacro (BSplineTransform, BSplineTransformType);
  itkSetObjectMacro (InverseBSplineTransform, ThinPlateSplineTransformType);
  
  itkGetObjectMacro (RigidTransform, RigidTransformType);
  itkGetObjectMacro (AffineTransform, AffineTransformType);
  itkGetObjectMacro (BSplineTransform, BSplineTransformType);
  itkGetObjectMacro (InverseBSplineTransform, ThinPlateSplineTransformType);
 
  
  
  /*** Additional API - Functions ***/
  void LoadTransform( );
  void SaveTransform( int type );
  
  
  void SetFileName(char *);
  void SetFileName(std::string);
      

 
     
  
protected:
  
  /** Constructor and Destructor */
  mimxTransformIO();
  ~mimxTransformIO() {};
    
  
  
  

private:
  mimxTransformIO( const Self& );        //purposely not implemented
  void operator=( const Self& );      //purposely not implemented

  RigidTransformType::Pointer               m_RigidTransform;
  AffineTransformType::Pointer              m_AffineTransform;
  BSplineTransformType::Pointer             m_BSplineTransform;
  ThinPlateSplineTransformType::Pointer     m_InverseBSplineTransform;

  std::string                 m_FileName;
  
};

}

#endif
