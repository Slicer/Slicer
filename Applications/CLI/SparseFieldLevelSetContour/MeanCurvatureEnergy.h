#ifndef MEANCURVATUREENERGY
#define MEANCURVATUREENERGY

#include "MeshEnergy.h"

class MeanCurvatureEnergy : public MeshEnergy
{
public:
MeanCurvatureEnergy( MeshData* data ) {meshdata = data;}
~MeanCurvatureEnergy( ) {}

double eval_energy( const vector<int>& C );
valarray<double> getforce( const vector<int>& C);
valarray<double> getforce( const vector<int>& C, const vector<double>& phi);
valarray<double> getforce( const vector<int>& C, 
const vector<int>& L_p1, const vector<int>& L_n1,
const vector<double>& phi);

};



#endif
