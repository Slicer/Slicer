// Copied from dtiprocess
// available there: http://www.nitrc.org/projects/dtiprocess/
/*=========================================================================

  Program:   NeuroLib (DTI command line tools)
  Language:  C++
  Date:      $Date: 2010/03/09 18:01:02 $
  Version:   $Revision: 1.2 $
  Author:    Casey Goodlett (gcasey@sci.utah.edu)

  Copyright (c)  Casey Goodlett. All rights reserved.
  See NeuroLibCopyright.txt or http://www.ia.unc.edu/dev/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "deformationfieldio.h"
#include <string>
#include <itkImageFileReader.h>
#include "itkHFieldToDeformationFieldImageFilter.h"

DeformationImageType::Pointer readDeformationField(std::string warpfile, DeformationFieldType dft)
{
  typedef itk::ImageFileReader<DeformationImageType> DeformationImageReader;

  DeformationImageReader::Pointer defreader = DeformationImageReader::New();
  defreader->SetFileName(warpfile.c_str() );

  if( dft == HField )
    {

    typedef itk::HFieldToDeformationFieldImageFilter<DeformationImageType> DeformationConvertType;
    DeformationConvertType::Pointer defconv = DeformationConvertType::New();
    defconv->SetInput(defreader->GetOutput() );
//  defconv->SetSpacing(timg->GetSpacing());
    defconv->Update();
    return defconv->GetOutput();

    }
  else
    {

    defreader->Update();
    return defreader->GetOutput();

    }

}
