/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToHexahedronMeshSource.txx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.5 $

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

#ifndef _itkImageToHexahedronMeshSource_txx
#define _itkImageToHexahedronMeshSource_txx


#include "itkImageToHexahedronMeshSource.h"
#include "itkNumericTraits.h"



namespace itk
{

template<class TInputImage, class TOutputMesh>
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::ImageToHexahedronMeshSource()
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);

  m_NumberOfCells = 0;
  m_NumberOfNodes = 0;

  //this->GetOutput()->GetPoints()->Reserve(m_NodeLimit);
  //this->GetOutput()->GetCells()->Reserve(m_CellLimit);

  //m_ObjectValue = NumericTraits< InputPixelType >::Zero;
}

template<class TInputImage, class TOutputMesh>
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::~ImageToHexahedronMeshSource()
{
  
}

  
template<class TInputImage, class TOutputMesh>
void
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::SetInput(const InputImageType* image)
{ 
  this->ProcessObject::SetNthInput(0, 
                                   const_cast< InputImageType * >( image ) );
}

/** Generate the data */
template<class TInputImage, class TOutputMesh>
void
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::GenerateData()
{
  this->InitializeLUT();
  this->CreateMesh();
}


template<class TInputImage, class TOutputMesh>
void
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::InitializeLUT()
{
  for (int i =0;i<256;i++)
    {  
    m_LUT[i][0] = 0;
        m_LUT[i][1] = 0;
    }
}

template<class TInputImage, class TOutputMesh>
void
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::CreateMesh()
{
  if (this->GetNumberOfInputs() < 1)
    {
    std::cout << "ImageToHexahedronMeshSource : Image input is not set" << std::endl;
    return;
    }

  MeshSourceTypePointer meshSource = MeshSourceType::New();
  
  std::cout << "\t New Filter" << std::endl;
  
  InputImageConstPointer m_InputImage =  static_cast<const InputImageType * >(this->ProcessObject::GetInput(0) );
  
  std::cout << "\t Get Image" << std::endl;

  InputImageIterator it( m_InputImage, m_InputImage->GetBufferedRegion() );
  
  SpacingType spacing = m_InputImage->GetSpacing();
  float spacingX = spacing[0]/2.0;
  float spacingY = spacing[1]/2.0;
  float spacingZ = spacing[2]/2.0;


  for ( it.GoToBegin(); ! it.IsAtEnd(); ++it)
    {   
        if (it.Value() > 0 )
          {
       /*** Add Voxel to Mesh ***/
           
       IndexType imageIndex = it.GetIndex();
           OriginType origin;
         //  std::cout << "\t Add point " << k++ << ", " << origin[0]-spacingX << ", " << std::endl;
       m_InputImage->TransformIndexToPhysicalPoint( imageIndex, origin);
   
      /**** New Code  ***/  
      meshSource->AddHexahedron(
                meshSource->AddPoint(origin[0]-spacingX, origin[1]-spacingY, origin[2]+spacingZ),
                meshSource->AddPoint(origin[0]-spacingX, origin[1]+spacingY, origin[2]+spacingZ),
                meshSource->AddPoint(origin[0]+spacingX, origin[1]+spacingY, origin[2]+spacingZ),
                meshSource->AddPoint(origin[0]+spacingX, origin[1]-spacingY, origin[2]+spacingZ),
                meshSource->AddPoint(origin[0]-spacingX, origin[1]-spacingY, origin[2]-spacingZ),
                meshSource->AddPoint(origin[0]-spacingX, origin[1]+spacingY, origin[2]-spacingZ),
                meshSource->AddPoint(origin[0]+spacingX, origin[1]+spacingY, origin[2]-spacingZ),
                meshSource->AddPoint(origin[0]+spacingX, origin[1]-spacingY, origin[2]-spacingZ));                                                            
          }

        }

  this->SetOutput( meshSource->GetOutput() );
  
  
  std::cout << "\t Set Outpout" << std::endl;



  // This indicates that the current BufferedRegion is equal to the 
  // requested region. This action prevents useless rexecutions of
  // the pipeline.
  this->GetOutput()->SetBufferedRegion( this->GetOutput()->GetRequestedRegion() );

  std::cout << "\t Set Buff Region" << std::endl;
  
}


/** PrintSelf */
template<class TInputImage, class TOutputMesh>
void
ImageToHexahedronMeshSource<TInputImage,TOutputMesh>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

//  os << indent
//     << "ObjectValue: " 
//     << static_cast<NumericTraits<unsigned char>::PrintType>(m_ObjectValue)
//     << std::endl;

  os << indent
     << "NumberOfNodes: "
     << m_NumberOfNodes
     << std::endl;

  os << indent
     << "NumberOfCells: "
     << m_NumberOfCells
     << std::endl;
}

} /** end namespace itk. */

#endif
