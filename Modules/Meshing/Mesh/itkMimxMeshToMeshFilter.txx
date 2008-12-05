/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxMeshToMeshFilter.txx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
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

#ifndef _itkMeshToMeshFilter_txx
#define _itkMeshToMeshFilter_txx
#include "itkMeshToMeshFilter.h"


namespace itk
{

/**
 *
 */
template <class TInputMesh, class TOutputMesh>
MeshToMeshFilter<TInputMesh,TOutputMesh>
::MeshToMeshFilter()
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);

  OutputMeshPointer output
    = dynamic_cast<OutputMeshType*>(this->MakeOutput(0).GetPointer()); 

  this->ProcessObject::SetNumberOfRequiredOutputs(1);
  this->ProcessObject::SetNthOutput(0, output.GetPointer());

}

/**
 *
 */
template <class TInputMesh, class TOutputMesh>
MeshToMeshFilter<TInputMesh,TOutputMesh>
::~MeshToMeshFilter()
{
}
  

/**
 *   Make Ouput
 */
template <class TInputMesh, class TOutputMesh>
DataObject::Pointer
MeshToMeshFilter<TInputMesh,TOutputMesh>
::MakeOutput(unsigned int)
{
  OutputMeshPointer  outputMesh = OutputMeshType::New();
  return dynamic_cast< DataObject *>( outputMesh.GetPointer() );
}




/**
 *
 */
template <class TInputMesh, class TOutputMesh>
void 
MeshToMeshFilter<TInputMesh,TOutputMesh>
::SetInput(unsigned int idx,const InputMeshType *input)
{
  // process object is not const-correct, the const_cast
  // is required here.
  this->ProcessObject::SetNthInput(idx, 
                                   const_cast< InputMeshType * >(input) );
}


  
/**
 *
 */
template <class TInputMesh, class TOutputMesh>
const typename MeshToMeshFilter<TInputMesh,TOutputMesh>::InputMeshType *
MeshToMeshFilter<TInputMesh,TOutputMesh>
::GetInput(unsigned int idx) 
{
  return dynamic_cast<const InputMeshType*>
    (this->ProcessObject::GetInput(idx));
}

 
/**
 *
 */
template <class TInputMesh, class TOutputMesh>
typename MeshToMeshFilter<TInputMesh,TOutputMesh>::OutputMeshType *
MeshToMeshFilter<TInputMesh,TOutputMesh>
::GetOutput(void) 
{
  return dynamic_cast<OutputMeshType*>
    (this->ProcessObject::GetOutput(0));
}


/**
 *
 */
template <class TInputMesh, class TOutputMesh>
void 
MeshToMeshFilter<TInputMesh,TOutputMesh>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}



/**
 * copy information from first input to all outputs
 * This is a void implementation to prevent the 
 * ProcessObject version to be called
 */
template <class TInputMesh, class TOutputMesh>
void 
MeshToMeshFilter<TInputMesh,TOutputMesh>
::GenerateOutputInformation()
{
}


} // end namespace itk

#endif
