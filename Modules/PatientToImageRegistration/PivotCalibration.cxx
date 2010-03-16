/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: PivotCalibration.cxx,v $
  Date:      $Date: 2008/02/15 23:15:19 $
  Version:   $Revision: 1.1.2.3 $
=========================================================================auto=*/

#include "PivotCalibration.h"
#include "itkVersorRigid3DTransform.h"

#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"


using namespace std;

PivotCalibration::PivotCalibration() : ValidPivotCalibration(false)
{
    for (int i = 0; i < 3; i++)
    {
        this->PivotPosition[i] = 0.0;
        this->Translation[i] = 0.0;
    }
    this->RMSE = 0.0;
 
}


PivotCalibration::~PivotCalibration()
{
  this->Clear();
}


unsigned int PivotCalibration::GetNumberOfSamples() const
{
    return this->Transforms.size();
}


void PivotCalibration::AddSample(vtkMatrix4x4 *matrix)
{
    this->Transforms.push_back(matrix);
}



void PivotCalibration::Clear()
{
  std::vector<vtkMatrix4x4*>::const_iterator it, transformsEnd = this->Transforms.end();
  for(it = this->Transforms.begin(); it != transformsEnd; it++)
    {
    (*it)->Delete();
    }
  this->Transforms.clear();        
}



void PivotCalibration::ComputeCalibration()
{
  if(this->Transforms.empty())
    {
    this->ValidPivotCalibration = false;
    return;
    }

  std::cerr << "Starting computation" << std::endl;
  unsigned int rows = 3*this->Transforms.size();
  unsigned int columns = 4;


  vnl_matrix< double > A( rows, columns ), minusI( 3, 3, 0 ), R(3,1); //minusI is initialized with ones
  vnl_vector< double > b(rows), x(columns), t(3), r(3);
  minusI( 0, 0 ) = -1;
  minusI( 1, 1 ) = -1;
  minusI( 2, 2 ) = -1;

  std::cerr << "number of transforms: " << this->Transforms.size() << std::endl;

  //do the computation and set the internal variables
  std::vector<vtkMatrix4x4*>::const_iterator it, transformsEnd = this->Transforms.end();
  unsigned int currentRow;
  for( currentRow = 0, it = this->Transforms.begin();
       it != transformsEnd;
       it++, currentRow += 3 )
    {
    //Build the vector of independent coefficients
    for (int i = 0; i < 3; i++)
      {
      t(i) = (*it)->GetElement(i, 3);
      }
    t *= -1;
    b.update( t, currentRow );
    //Build the matrix with the equation coefficients
    for (int i = 0; i < 3; i++)
      {
      //for (int j = 0; j < 3; j++)
      R(i, 0) = (*it)->GetElement(i, 2);
      }
    A.update(R, currentRow, 0);
    A.update( minusI, currentRow, 1 );
    }

  //solve the equations
  vnl_svd<double> svdA(A);


  //svdA.zero_out_absolute( this->m_SingularValueThreshold );
  svdA.zero_out_absolute( 1e-1 );

  //there is a solution only if rank(A)=4 (columns are linearly
  //independent)
  if( svdA.rank() < 4 )
    {
    this->ValidPivotCalibration = false;
    }
  else
    {
    x = svdA.solve( b );

    //set the RMSE
    this->RMSE = ( A * x - b ).rms();

    //set the transformation
//    this->CalibrationTransform->Identity();
    this->Translation[0] = 0.0;
    this->Translation[1] = 0.0;
    this->Translation[2] = x[0];
//    this->CalibrationTransform->SetElement(2,3,Translation[2]);

    //set the pivot point
    this->PivotPosition[0] = x[1];
    this->PivotPosition[1] = x[2];
    this->PivotPosition[2] = x[3];

    //Print out solution to screen
//    std::cerr << "Pivot point location: " << this->PivotPosition << std::endl;
    std::cerr << "Offset from tip to sensor: " << x[0] << std::endl;
    std::cerr << "Root mean squared error: " << this->RMSE << std::endl;

    this->ValidPivotCalibration = true;
    }
}



void PivotCalibration::GetPivotPosition(double pos[3])
{
    pos[0]=this->PivotPosition[0];
    pos[1]=this->PivotPosition[1];
    pos[2]=this->PivotPosition[2];
}


void PivotCalibration::GetTranslation(double trans[3])
{
    trans[0]=this->Translation[0];
    trans[1]=this->Translation[1];
    trans[2]=this->Translation[2];
}


double PivotCalibration::GetRMSE()
{
    return this->RMSE;
}


/*=========================================================================
The algorithm used in CalculateCalibration is a slightly modified version of the
pivot calibration algorithm in IGSTK.

Copyright (c) 2002-2005 IGSTK Software Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the IGSTK Software Consortium, nor the names of any
   consortium members, nor of any contributors, may be used to endorse or
   promote products derived from this software without specific prior written
   permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/  
