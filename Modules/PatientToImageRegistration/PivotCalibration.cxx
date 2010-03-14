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

PivotCalibration::PivotCalibration() : validPivotCalibration(false)
{
    for (int i = 0; i < 3; i++)
    {
        pivotPosition[i] = 0.0;
        translation[i] = 0.0;
    }
    RMSE = 0.0;
 
}


PivotCalibration::~PivotCalibration()
{
    this->quaternionSampleCollection.clear();
    this->translationSampleCollection.clear();
}


unsigned int PivotCalibration::GetNumberOfSamples() const
{
    return this->quaternionSampleCollection.size();
}


void PivotCalibration::AddSample(double *_quat, double *_trans)
{
    QuaternionType quat;
    TranslationType trans;

    quat.Set(_quat[0],_quat[1],_quat[2],_quat[3]);
    this->quaternionSampleCollection.push_back(quat);
 
    trans.SetElement(0,_trans[0]);
    trans.SetElement(1,_trans[1]);
    trans.SetElement(2,_trans[2]);
    this->translationSampleCollection.push_back(trans);
}



void PivotCalibration::Clear()
{
    this->quaternionSampleCollection.clear();
    this->translationSampleCollection.clear();
}



void PivotCalibration::CalculateCalibration()
{
    unsigned int i;
    unsigned int j;
    unsigned int k;
    unsigned int r;
    unsigned int c;
    unsigned int num;

    num = this->GetNumberOfSamples();
    if (num == 0)
    {
        return;
    }


    // Set the number of sample, row and column number of matrix
    r = num * 3;
    c = 6;

    typedef vnl_matrix< double >            VnlMatrixType;

    typedef vnl_vector< double >            VnlVectorType;

    typedef vnl_svd< double >               VnlSVDType;


    // Define the Vnl matrix and intermediate variables
    VnlMatrixType matrix(r, c);
    VnlVectorType x(c);
    VnlVectorType b(r);
    VnlVectorType br(r);  
    itk::Versor<double> quat;
    itk::Versor<double>::MatrixType rotMatrix;

    // Fill the matrix of M
    for (k = 0; k < num; k++)
    {
        quat = this->quaternionSampleCollection.at(k);
        rotMatrix = quat.GetMatrix();

        for (j = 0; j < 3; j++)
        {
            for (i = 0; i < 3; i++)
            {
                matrix[3 * k + j][i] = rotMatrix[j][2-i];
            }
            for (i = 0; i < 3; i++)
            {
                matrix[3 * k + j][i + 3] = 0.0;
            }
            matrix[3 * k + j][j + 3] = -1.0;
        }

        for (j = 0; j < 3; j++)
        {
            b[3 * k + j] = -this->translationSampleCollection.at(k)[j];
        }
    }

    // Use SVD to solve M * x = b
    VnlSVDType svd(matrix);
    x = svd.solve(b);

    // Extract the offset components
    for (i = 0; i < 3; i++)
    {
        this->translation[2-i] = x[i];
    }

    // Extract the pivot position
    for (i = 0; i < 3; i++)
    {
        this->pivotPosition[i] = x[i + 3];
    }

    // Calculate the root mean square error
    br = matrix * x - b;  
    this->RMSE = sqrt(br.squared_magnitude() / num );

    // Set valid indicator
    this->validPivotCalibration = true;
}


void PivotCalibration::GetPivotPosition(double pos[3])
{
    pos[0]=this->pivotPosition[0];
    pos[1]=this->pivotPosition[1];
    pos[2]=this->pivotPosition[2];
}


void PivotCalibration::GetTranslation(double trans[3])
{
    trans[0]=this->translation[0];
    trans[1]=this->translation[1];
    trans[2]=this->translation[2];
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
