/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/
#include "EMLocalInterface.h"
#include <stdlib.h>

EMLocal_Hierarchical_Class_Parameters::EMLocal_Hierarchical_Class_Parameters() {
  NumClasses        = -1;
  NumTotalTypeCLASS = -1;
  NumChildClasses   = NULL;
  ProbDataIncY      = NULL; 
  ProbDataIncZ      = NULL; 
  ProbDataWeight    = NULL; 
  ProbDataMinusWeight = NULL; 
  ProbDataType      = -1;
  LogMu             = NULL; 
  InvLogCov         = NULL; 
  InvSqrtDetLogCov  = NULL; 
  TissueProbability = NULL; 
  VirtualNumInputImages = NULL; 
  MrfParams = NULL; 
}

void EMLocal_Hierarchical_Class_Parameters::Copy(EMLocal_Hierarchical_Class_Parameters init) {
  NumClasses        = init.NumClasses;
  NumTotalTypeCLASS = init.NumTotalTypeCLASS;
  NumChildClasses   = init.NumChildClasses;
  ProbDataIncY      = init.ProbDataIncY; 
  ProbDataIncZ      = init.ProbDataIncZ ; 
  ProbDataWeight    = init.ProbDataWeight; 
  ProbDataMinusWeight  = init.ProbDataMinusWeight; 
  ProbDataType         = init.ProbDataType;
  LogMu                = init.LogMu ; 
  InvLogCov            = init.InvLogCov; 
  InvSqrtDetLogCov     = init.InvSqrtDetLogCov; 
  TissueProbability    = init.TissueProbability; 
  VirtualNumInputImages = init.VirtualNumInputImages; 
  MrfParams             = init.MrfParams; 
}

