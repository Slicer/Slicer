#ifndef _TILG_ISO_3D_H_
#define _TILG_ISO_3D_H_

#define OBJ  1 
#define BG   0 

int Env_Code_3_img(int loc[3], unsigned char *img, int dim[3]);
// returns the neighbor code including the center at position loc
     
int Tilg_Test_3 (int c,int d,int type);
/* Calculation of Tilg-criterion, c is the Neighor-code of */
/* 3x3x3-Region, including the center                      */
// returns 0 if cannot be 'tilged' 
// if type == 1 -> sheet preserving tilg
// if type == 0 -> full tilg
// d = for parrel tilg -> 0,1,2,3,4,5   N,S,E,W,T,D 

void tilg_iso_3D(int dx,int dy,int dz,
                 unsigned char *data,
                 unsigned char *res,
                 int type);
// 3D isotropic tilg-procedure that does a 3D thinning
// dx,dy,dz  are the dimensions of the input (data) and output (res) image
// output image has to be allocated
// if type == 1 -> sheet preserving tilg
// if type == 0 -> full tilg

#endif
