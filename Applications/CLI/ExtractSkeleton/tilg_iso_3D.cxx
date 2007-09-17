/*****************************************************************************/
/* File:        tilg_iso_3D.cc                                              */
/* parallele Tilgung eines Voxel-Datensatzes                                 */
/* verbesserte Isotropie mittels 18 Subzyklen                                */
/*****************************************************************************/
/* Beginn:      20.12.91     Stand:      29.02.92                     */
/* Autor:       Patrick Drozz  IIIC/9   ETHZ                          */
// adapted to C++: Martin Styner 20.July.2000
// integrated into slicer: Stephen Aylward, 20, Aug, 2007
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

/********************************  Konstanten  *******************************/
#define LIM  1 /* Voxelwert >= LIM => Objekt (Input-Bild) */
#define OBJ  1 /* Objektwert  (Output-Bild)               */
#define BG   0 /* Hintergrund (Output-Bild)               */

/*********************************  Makros  **********************************/
#define Q(i,v)  ((pos[(i)] == OBJ) ? (v) : 0 )
#define P(n,x,y,z) n[(x) + nx*((y) + (z)*ny)]

/**************************** globale Variablen  *****************************/
static int nx,ny,nz,nzz;
static unsigned char *workbuf, *result;
static int f_tab[26];
static unsigned char p[5][5][5];

/*******************************  Hilfsprozeduren ****************************/
int bitcount (int i)
   /* gibt die Anzahl 1-en in i zurueck */
   {
   int c=0;
   int m;

   while (i != 0) {
      c++;
      m = i - 1;
      i &= m;
   }
   return (c);
}

void init_data()
   /* initialisiert p */
   {
   int x,y,z;

   for (z=0;z<5;z++) {
      for (y=0;y<5;y++) {
        for (x=0;x<5;x++) {
           p[x][y][z] = BG;
        }
      }
   }
}

void mark(int x,int y,int z)
   /* markiert alles was von x,y,z aus erreichbar ist */
   /* einfache rekursive Version                      */
   {
   int i,j,k;

   p[x][y][z] = BG;
   for (k=z-1;k<z+2;k++) {
      for (j=y-1;j<y+2;j++) {
        for (i=x-1;i<x+2;i++) {
           if (p[i][j][k] == OBJ) {
              mark(i,j,k);
           }
        }
      }
   }
}


int count_components(int nc)
   /* zaehlt die Komponenten im 26-Sinn des nc's */
   /* einfache rekursive Version                 */
   {
   int x,y,z,count;

   for (z=1;z<4;z++) {
      for (y=1;y<4;y++) {
        for (x=1;x<4;x++) {
           p[x][y][z] = (nc & 1) ? OBJ : BG;
           nc /= 2;
        }
      }
   }
   count = 0;
   for (z=1;z<4;z++) {
      for (y=1;y<4;y++) {
        for (x=1;x<4;x++) {
           if (p[x][y][z] != BG) {
              count++;
              mark(x,y,z);
           }
        }
      }
   }
   return (count) ;
}

int Env_Code_3_img(int loc[3], unsigned char *img, int dim[3])
   /* berechnet den Nachbarschaftscode der 3x3x3-Umgebung von P{i} */
   {
   int nc;
   unsigned char *pos;

   int _nzz = dim[0]*dim[1];
   int _nx = dim[0];

   pos = &img[loc[0] + dim[0] * (loc[1] + dim[1]*(loc[2]-1))];
   nc = Q(-1-_nx,1) + Q(-_nx,2) + Q(1-_nx,4) + Q(-1,8) + Q(0,16) + 
        Q(1,32) + Q(-1+_nx,64) + Q(_nx,128) + Q(1+_nx,256);
   pos += _nzz;
   nc +=  Q(-1-_nx,512) + Q(-_nx,1024) + Q(1-_nx,2048) + Q(-1,4096) + Q(0,8192) + 
          Q(1,16384) + Q(-1+_nx,32768) + Q(_nx,65536) + Q(1+_nx,131072);
   pos += _nzz;
   nc +=  Q(-1-_nx,262144) + Q(-_nx,524288) + Q(1-_nx,1048576) + Q(-1,2097152) +
          Q(0,4194304) + Q(1,8388608) + Q(-1+_nx,16777216) + Q(_nx,33554432) + 
          Q(1+_nx,67108864);
   return nc;
}

int Env_Code_3(int i)
   /* berechnet den Nachbarschaftscode der 3x3x3-Umgebung von P{i} */
   {
   int nc;
   unsigned char *pos;

   pos = &result[i-nzz];
   nc = Q(-1-nx,1) + Q(-nx,2) + Q(1-nx,4) + Q(-1,8) + Q(0,16) + 
        Q(1,32) + Q(-1+nx,64) + Q(nx,128) + Q(1+nx,256);
   pos += nzz;
   nc +=  Q(-1-nx,512) + Q(-nx,1024) + Q(1-nx,2048) + Q(-1,4096) + Q(0,8192) + 
          Q(1,16384) + Q(-1+nx,32768) + Q(nx,65536) + Q(1+nx,131072);
   pos += nzz;
   nc +=  Q(-1-nx,262144) + Q(-nx,524288) + Q(1-nx,1048576) + Q(-1,2097152) +
          Q(0,4194304) + Q(1,8388608) + Q(-1+nx,16777216) + Q(nx,33554432) + 
          Q(1+nx,67108864);
   return nc;
}
/*************************** ENDE  Hilfsprozeduren **************************/


/******************************  Hauptprozedur ******************************/
/* Nachbarschaftsdefinition: (Wertigkeiten)                    */
/*   00  01  02          09  10  11          18  19  20        */
/*   03  04  05          12  13  14          21  22  23        */
/*   06  07  08          15  16  17          24  25  26        */
/*      unten                                   oben           */

/* Maskendefinitionen:   */
/* Kanten:   */
#define MASK_E_01   4281360     /*  4,10,12,14,16,22 */
/* Flaechen: */
#define MASK_F_01      1042     /*  1, 4,10 */
#define MASK_F_02      4120     /*  3, 4,12 */
#define MASK_F_03     16432     /*  4, 5,14 */
#define MASK_F_04     65680     /*  4, 7,16 */
#define MASK_F_05      5632     /*  9,10,12 */
#define MASK_F_06     19456     /* 10,11,14 */
#define MASK_F_07    102400     /* 12,15,16 */
#define MASK_F_08    212992     /* 14,16,17 */
#define MASK_F_09   4719616     /* 10,19,22 */
#define MASK_F_10   6295552     /* 12,21,22 */
#define MASK_F_11  12599296     /* 14,22,23 */
#define MASK_F_12  37814272     /* 16,22,25 */
/* Wuerfel:  */
#define MASK_U_01      5659     /*  0, 1, 3, 4, 9,10,12 */
#define MASK_U_02     19510     /*  1, 2, 4, 5,10,11,14 */
#define MASK_U_03    102616     /*  3, 4, 6, 7,12,15,16 */
#define MASK_U_04    213424     /*  4, 5, 7, 8,14,16,17 */
#define MASK_U_05   7083520     /*  9,10,12,18,19,21,22 */
#define MASK_U_06  14175232     /* 10,11,14,19,20,22,23 */
#define MASK_U_07  56725504     /* 12,15,16,21,22,24,25 */
#define MASK_U_08 113459200     /* 14,16,17,22,23,25,26 */

#define MASK_EX  4260880    /*  4,10,16,22 */
#define MASK_EY  4214800    /*  4,12,14,22 */
#define MASK_EZ    87040    /* 10,12,14,16 */
#define MASK_CX 38339730    /*  1, 4, 7,10,16,19,22,25 */
#define MASK_CY 14700600    /*  3, 4, 5,12,14,21,22,23 */
#define MASK_CZ   253440    /*  9,10,11,12,14,15,16,17 */

#define MASK_C06   19203364  /*  2  5  8 10 16 18 21 24 */
#define MASK_C08   76612681  /*  0  3  6 10 16 20 23 26 */
#define MASK_C10    1855936  /*  6  7  8 12 14 18 19 20 */
#define MASK_C12  117460999  /*  0  1  2 12 14 24 25 26 */
#define MASK_C14   22054996  /*  2  4  6 11 15 20 22 24 */
#define MASK_C16   71697169  /*  0  4  8  9 17 18 22 26 */

#define MASK_E_06   2163744  /*  5 10 16 21 */
#define MASK_E_08   8455176  /*  3 10 16 23 */
#define MASK_E_10    544896  /*  7 12 14 19 */
#define MASK_E_12  33574914  /*  1 12 14 25 */
#define MASK_E_14   4229136  /*  4 15 11 22 */
#define MASK_E_16   4325904  /*  4  9 17 22 */

#define MASK_F_13   2360320  /* 10 18 21 */
#define MASK_F_14  18939904  /* 16 21 24 */
#define MASK_F_15      1060  /* 10 02 05 */
#define MASK_F_16     65824  /* 05 08 16 */
#define MASK_F_17      1033  /* 00 03 10 */
#define MASK_F_18     65608  /* 03 06 16 */
#define MASK_F_19   9438208  /* 10 20 23 */
#define MASK_F_20  75563008  /* 16 23 26 */
#define MASK_F_21    790528  /* 12 18 19 */
#define MASK_F_22   1589248  /* 14 19 20 */
#define MASK_F_23      4288  /* 06 07 12 */
#define MASK_F_24     16768  /* 07 08 14 */
#define MASK_F_25  50335744  /* 12 24 25 */
#define MASK_F_26 100679680  /* 14 25 26 */
#define MASK_F_27      4099  /* 00 01 12 */
#define MASK_F_28     16390  /* 01 02 14 */
#define MASK_F_29     32848  /* 04 06 15 */
#define MASK_F_30      2068  /* 02 04 11 */
#define MASK_F_31  21004288  /* 15 22 24 */
#define MASK_F_32   5244928  /* 11 20 22 */
#define MASK_F_33       529  /* 00 04 09 */
#define MASK_F_34    131344  /* 04 08 17 */
#define MASK_F_35   4456960  /* 09 18 22 */
#define MASK_F_36  71434240  /* 17 22 26 */

int Tilg_Test_3 (int c,int d,int type)
   /* direkte Berechnung der Tilgbarkeit, c enthaelt den Nachbarcode der */
   /* 3x3x3-Umgebung, wobei das Zentrum mitgerechnet ist!                */
   /* Rueckgabewert: BG = "tilgbar", OBJ = "nicht tilgbar"               */
   /* d gibt die Richtung an: 0,1,2,3,4,5   N,S,E,W,T,D                  */
   /* d wird fuer den Zusatztest bei paralleler Tilgung verwendet        */
   {
   int de=0;
   register int df=0;
   register int du=0;
   int dex=0,dfx=0,bcx=0;
   int dey=0,dfy=0,bcy=0;
   int dez=0,dfz=0,bcz=0;
   int free_mask;

   /* delta-Kanten berechnen */
   de = 6 - bitcount(c & MASK_E_01);
   /* delta-Flaechen berechnen */
   if ((c & MASK_F_01) == 0) {df++;dfx++;}
   if ((c & MASK_F_02) == 0) {df++;dfy++;}
   if ((c & MASK_F_03) == 0) {df++;dfy++;}
   if ((c & MASK_F_04) == 0) {df++;dfx++;}
   if ((c & MASK_F_05) == 0) {df++;dfz++;}
   if ((c & MASK_F_06) == 0) {df++;dfz++;}
   if ((c & MASK_F_07) == 0) {df++;dfz++;}
   if ((c & MASK_F_08) == 0) {df++;dfz++;}
   if ((c & MASK_F_09) == 0) {df++;dfx++;}
   if ((c & MASK_F_10) == 0) {df++;dfy++;}
   if ((c & MASK_F_11) == 0) {df++;dfy++;}
   if ((c & MASK_F_12) == 0) {df++;dfx++;}
   /* delta-Wuerfel berechnen */
   if ((c & MASK_U_01) == 0) du++;
   if ((c & MASK_U_02) == 0) du++;
   if ((c & MASK_U_03) == 0) du++;
   if ((c & MASK_U_04) == 0) du++;
   if ((c & MASK_U_05) == 0) du++;
   if ((c & MASK_U_06) == 0) du++;
   if ((c & MASK_U_07) == 0) du++;
   if ((c & MASK_U_08) == 0) du++;
   /* Tilgbarkeitsbedingung */
   if ( (de - df + du) == 1 ) {
      free_mask = f_tab[d];
      dex = 4 - bitcount(c & MASK_EX);
      dey = 4 - bitcount(c & MASK_EY);
      dez = 4 - bitcount(c & MASK_EZ);
      bcx = bitcount(c & MASK_CX);
      bcy = bitcount(c & MASK_CY);
      bcz = bitcount(c & MASK_CZ);
      if (type) {
      /* fuer Mittelflaechen */
      if ( (d == 0) || (d == 1) ) {
         if ( ((dex - dfx) == 1) && ((dez - dfz) == 1) && ( ((bcx > 1) && 
(bcz > 1))))
            return (BG);
          }
       else if ( (d == 2)  || (d == 3) ) {
         if ( ((dey - dfy) == 1) && ((dez - dfz) == 1) && ( ((bcy > 1) && 
(bcz > 1))))
            return (BG);
       }
       else {
         if ( ((dex - dfx) == 1) && ((dey - dfy) == 1) && ( ((bcx > 1) && 
(bcy > 1))))
            return (BG);
        }
      }
      else {
      /* fuer Mittelachsen */
      switch (d) {
         case 0 :
         case 1 :
            if (  ((dex - dfx) == 1) && ((dez - dfz) == 1)  &&
               (( (bcx > 1) && (bcz > 1))  || (c & free_mask))  )
               return (BG);
            break;
         case 2 :
         case 3 :
            if (  ((dey - dfy) == 1) && ((dez - dfz) == 1)  &&
               (( (bcy > 1) && (bcz > 1)) || (c & free_mask))  )
               return (BG);
            break;
         case 4 :
         case 5 :
            if (  ((dex - dfx) == 1) && ((dey - dfy) == 1)  &&
               (( (bcx > 1) && (bcy > 1) ) || (c & free_mask))  )
               return (BG);
            break;
         case 6 :
         case 7 :
            dex = 4 - bitcount(c & MASK_E_06);
            bcx = bitcount(c & MASK_C06);
            dfx = 0;
            if ((c & MASK_F_13) == 0) dfx++;
            if ((c & MASK_F_14) == 0) dfx++;
            if ((c & MASK_F_15) == 0) dfx++;
            if ((c & MASK_F_16) == 0) dfx++;
            if ( ((dey - dfy) == 1) && ((dex - dfx) == 1) &&
               (( (bcy > 1) && (bcx > 1) ) || (c & free_mask))  )
               return (BG);
            break;
         case 8 :
         case 9 :
            dex = 4 - bitcount(c & MASK_E_08);
            bcx = bitcount(c & MASK_C08);
            dfx = 0;
            if ((c & MASK_F_17) == 0) dfx++;
            if ((c & MASK_F_18) == 0) dfx++;
            if ((c & MASK_F_19) == 0) dfx++;
            if ((c & MASK_F_20) == 0) dfx++;
            if ( ((dey - dfy) == 1) && ((dex - dfx) == 1) &&
               (( (bcy > 1) && (bcx > 1) ) || (c & free_mask))  )
               return (BG);
            break;
         case 10 :
         case 11 :
            dey = 4 - bitcount(c & MASK_E_10);
            bcy = bitcount(c & MASK_C10);
            dfy = 0;
            if ((c & MASK_F_21) == 0) dfy++;
            if ((c & MASK_F_22) == 0) dfy++;
            if ((c & MASK_F_23) == 0) dfy++;
            if ((c & MASK_F_24) == 0) dfy++;
            if (  ((dex - dfx) == 1) && ((dey - dfy) == 1)  &&
               (( (bcx > 1) && (bcy > 1) ) || (c & free_mask))  )
               return (BG);
            break;
         case 12 :
         case 13 :
            dey = 4 - bitcount(c & MASK_E_12);
            bcy = bitcount(c & MASK_C12);
            dfy = 0;
            if ((c & MASK_F_25) == 0) dfy++;
            if ((c & MASK_F_26) == 0) dfy++;
            if ((c & MASK_F_27) == 0) dfy++;
            if ((c & MASK_F_28) == 0) dfy++;
            if (  ((dex - dfx) == 1) && ((dey - dfy) == 1)  &&
               (( (bcx > 1) && (bcy > 1) ) || (c & free_mask))  )
               return (BG);
            break;
         case 14 :
         case 15 :
            dex = 4 - bitcount(c & MASK_E_14);
            bcx = bitcount(c & MASK_C14);
            dfx = 0;
            if ((c & MASK_F_29) == 0) dfx++;
            if ((c & MASK_F_30) == 0) dfx++;
            if ((c & MASK_F_31) == 0) dfx++;
            if ((c & MASK_F_32) == 0) dfx++;
            if (  ((dex - dfx) == 1) && ((dez - dfz) == 1)  &&
               (( (bcx > 1) && (bcz > 1))  || (c & free_mask))  )
               return (BG);
            break;
         case 16 :
         case 17 :
            dex = 4 - bitcount(c & MASK_E_16);
            bcx = bitcount(c & MASK_C16);
            dfx = 0;
            if ((c & MASK_F_33) == 0) dfx++;
            if ((c & MASK_F_34) == 0) dfx++;
            if ((c & MASK_F_35) == 0) dfx++;
            if ((c & MASK_F_36) == 0) dfx++;
            if (  ((dex - dfx) == 1) && ((dez - dfz) == 1)  &&
               (( (bcx > 1) && (bcz > 1))  || (c & free_mask))  )
               return (BG);
            break;
         case 18 :
            if (count_components(c & ~(8192)) == 1)
               return(BG);
            break;
      }
      }
   }
   return (OBJ);
}


void tilg_iso_3D(int dx,int dy,int dz,
         unsigned char *data,
         unsigned char *res,
         int type)
  // dx,dy,dz  are the dimensions of the input (data) and output (res) image
  // output image has to be allocated
  // if type == 1 -> sheet preserving tilg
  // if type == 0 -> full tilg
{

   int cnt=0,cnt1=0;
   int nc,x,y,z;
   int end,i,dir,dir_mask;
   //int free_mask;
   int *list;
   int dir_tab[26];
   //int b[3][3][3];

   nx = dx; ny = dy; nz = dz;
   init_data();
   /* Speicher allozieren */
   result = res;

   workbuf = data;
   nzz = nx*ny;
   /* Speicher allozieren */
   if ( (list = (int *) malloc(nzz*nz*sizeof(int)/4)) == NULL) {
      printf("out of memory\n");
      return;
   }
   /* Arbeitskopie des Bildes erstellen und binaerisieren */
   end = nx*ny*nz;
   for (i=0;i<end;i++) {
      result[i] = ( (workbuf[i] >= LIM) ? OBJ : BG  );
   }
   /* Rand von 1-Voxel-Breite auf 0 setzen */

   for(y=0;y<ny;y++) {
      for (x=0;x<nx;x++) {
         P(result,x,y,0) = (P(result,x,y,nz-1) = BG);
      }
   }
   for(y=0;y<ny;y++) {
      for (z=0;z<nz;z++) {
         P(result,0,y,z) = (P(result,nx-1,y,z) = BG);
      }
   }
   for(z=0;z<nz;z++) {
      for (x=0;x<nx;x++) {
         P(result,x,0,z) = (P(result,x,ny-1,z) = BG);
      }
   }
   /* Richtungstabelle initialisieren */
   dir_tab[0] = 1024;       /* 10 */
   dir_tab[1] = 65536;      /* 16 */
   dir_tab[2] = 16384;      /* 14 */
   dir_tab[3] = 4096;       /* 12 */
   dir_tab[4] = 4194304;    /* 22 */
   dir_tab[5] = 16;         /*  4 */

   f_tab[0] =  dir_tab[1];
   f_tab[1] =  dir_tab[0];
   f_tab[2] =  dir_tab[3];
   f_tab[3] =  dir_tab[2];
   f_tab[4] =  dir_tab[5];
   f_tab[5] =  dir_tab[4];

   dir_tab[6]  = 4198400;    /* 12 22 */
   dir_tab[7]  = 16400;      /*  4 14 */
   dir_tab[8]  = 4210688;    /* 14 22 */
   dir_tab[9]  = 4112;       /*  4 12 */
   dir_tab[10] =  65552;    /*  4 16 */
   dir_tab[11] =  4195328;  /* 10 22 */
   dir_tab[12] =  1040;     /*  4 10 */
   dir_tab[13] =  4259840;  /* 16 22 */
   dir_tab[14] =  69632;    /* 12 16 */
   dir_tab[15] =  17408;    /* 10 14 */
   dir_tab[16] =  5120;     /* 10 12 */
   dir_tab[17] =  81920;    /* 14 16 */

   f_tab[6]  =       32;   /*  5 */
   f_tab[7]  =  2097152;   /* 21 */
   f_tab[8]  =        8;   /*  3 */
   f_tab[9]  =  8388608;   /* 23 */
   f_tab[10] =   524288;   /* 19 */
   f_tab[11] =      128;   /*  7 */
   f_tab[12] = 33554432;   /* 25 */
   f_tab[13] =        2;   /*  1 */
   f_tab[14] =     2048;   /* 11 */
   f_tab[15] =    32768;   /* 15 */
   f_tab[16] =   131072;   /* 17 */
   f_tab[17] =      512;   /*  9 */

   /* eigentliches Bildparsing */
   end = end - nzz - nx - 1;
   cnt = 1;
   while (cnt) {
      cnt = 0;
      for (dir=0;dir<18;dir++) {
         cnt1 = 0;
         dir_mask = dir_tab[dir];
         for(i=nzz+nx+1;i<end;i++) {
            if ( (result[i] == OBJ) ) {
               nc = Env_Code_3(i);
               if ( ( (~nc) & dir_mask) == dir_mask ){
                  if (bitcount(nc) > 2) {
                     if ( Tilg_Test_3(nc,dir,type) == BG ) {
                        list[cnt1++] = i;
                     }
                 }
              }
            }
         }
         /* Voxel der Liste loeschen */
         for (i=0;i<cnt1;i++) {
            result[list[i]] = BG;
         }
         cnt += cnt1;
      }
   }
   /* sequentiell maximal Verduennen */
   cnt = 1;
   while (cnt) {
      cnt = 0;
      for(i=nzz+nx+1;i<end;i++) {
         if (result[i] == OBJ) {
            nc = Env_Code_3(i);
            if (bitcount(nc) > 2) {
               if (Tilg_Test_3(nc,18,type) == BG) {
                  cnt++;
                  result[i] = BG;
              }
           }
        }
      }
   }
   free(list);
}
