/* Copyright (c) 1990 Regents of the University of California */

#ifndef lint
static char SCCSid[] = "$SunId$ LBL";
#endif

/*
 * Convert colors and spectral ranges.
 */

#include "color.h"


RGBPRIMS  stdprims = STDPRIMS;		/* standard primary chromaticities */

/*
 *	The following table contains the CIE tristimulus integrals
 *  for X, Y, and Z.  The table is cumulative, so that
 *  each color coordinate integrates to 1.
 */

#define  STARTWL	380		/* starting wavelength (nanometers) */
#define  INCWL		10		/* wavelength increment */
#define  NINC		40		/* # of values */

static BYTE  chroma[3][NINC] = {
	{							/* X */
		0,   0,   0,   2,   6,   13,  22,  30,  36,  41,
		42,  43,  43,  44,  46,  52,  60,  71,  87,  106,
		128, 153, 178, 200, 219, 233, 243, 249, 252, 254,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255
	}, {							/* Y */
		0,   0,   0,   0,   0,   1,   2,   4,   7,   11,
		17,  24,  34,  48,  64,  84,  105, 127, 148, 169,
		188, 205, 220, 232, 240, 246, 250, 253, 254, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255
	}, {							/* Z */
		0,   0,   2,   10,  32,  66,  118, 153, 191, 220,
		237, 246, 251, 253, 254, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255
	}
};

COLORMAT  xyz2rgbmat = {		/* XYZ to RGB */
	{(CIE_y_g - CIE_y_b - CIE_x_b*CIE_y_g + CIE_y_b*CIE_x_g)/CIE_C_rD,
	 (CIE_x_b - CIE_x_g - CIE_x_b*CIE_y_g + CIE_x_g*CIE_y_b)/CIE_C_rD,
	 (CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g)/CIE_C_rD},
	{(CIE_y_b - CIE_y_r - CIE_y_b*CIE_x_r + CIE_y_r*CIE_x_b)/CIE_C_gD,
	 (CIE_x_r - CIE_x_b - CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r)/CIE_C_gD,
	 (CIE_x_b*CIE_y_r - CIE_x_r*CIE_y_b)/CIE_C_gD},
	{(CIE_y_r - CIE_y_g - CIE_y_r*CIE_x_g + CIE_y_g*CIE_x_r)/CIE_C_bD,
	 (CIE_x_g - CIE_x_r - CIE_x_g*CIE_y_r + CIE_x_r*CIE_y_g)/CIE_C_bD,
	 (CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r)/CIE_C_bD}
};

COLORMAT  rgb2xyzmat = {		/* RGB to XYZ */
	{CIE_x_r*CIE_C_rD/CIE_D,CIE_x_g*CIE_C_gD/CIE_D,CIE_x_b*CIE_C_bD/CIE_D},
	{CIE_y_r*CIE_C_rD/CIE_D,CIE_y_g*CIE_C_gD/CIE_D,CIE_y_b*CIE_C_bD/CIE_D},
	{(1.-CIE_x_r-CIE_y_r)*CIE_C_rD/CIE_D,
	 (1.-CIE_x_g-CIE_y_g)*CIE_C_gD/CIE_D,
	 (1.-CIE_x_b-CIE_y_b)*CIE_C_bD/CIE_D}
};



spec_rgb(col, s, e)		/* compute RGB color from spectral range */
COLOR  col;
int  s, e;
{
	COLOR  ciecolor;

	spec_cie(ciecolor, s, e);
	cie_rgb(col, ciecolor);
}


spec_cie(col, s, e)		/* compute a color from a spectral range */
COLOR  col;		/* returned color */
int  s, e;		/* starting and ending wavelengths */
{
	register int  i, d, r;
	
	s -= STARTWL;
	if (s < 0)
		s = 0;

	e -= STARTWL;
	if (e <= s) {
		col[CIEX] = col[CIEY] = col[CIEZ] = 0.0;
		return;
	}
	if (e >= INCWL*(NINC - 1))
		e = INCWL*(NINC - 1) - 1;

	d = e / INCWL;			/* interpolate values */
	r = e % INCWL;
	for (i = 0; i < 3; i++)
		col[i] = chroma[i][d]*(INCWL - r) + chroma[i][d + 1]*r;

	d = s / INCWL;
	r = s % INCWL;
	for (i = 0; i < 3; i++)
		col[i] -= chroma[i][d]*(INCWL - r) - chroma[i][d + 1]*r;

	col[CIEX] = (col[CIEX] + 0.5) * (1./(256*INCWL));
	col[CIEY] = (col[CIEY] + 0.5) * (1./(256*INCWL));
	col[CIEZ] = (col[CIEZ] + 0.5) * (1./(256*INCWL));
}


colortrans(c2, mat, c1)		/* convert c1 by mat and put into c2 */
register COLORMAT  mat;
register COLOR  c1, c2;
{
	static float  cout[3];

	cout[0] = mat[0][0]*c1[0] + mat[0][1]*c1[1] + mat[0][2]*c1[2];
	cout[1] = mat[1][0]*c1[0] + mat[1][1]*c1[1] + mat[1][2]*c1[2];
	cout[2] = mat[2][0]*c1[0] + mat[2][1]*c1[1] + mat[2][2]*c1[2];
	if((c2[0] = cout[0]) < 0.) c2[0] = 0.;
	if((c2[1] = cout[1]) < 0.) c2[1] = 0.;
	if((c2[2] = cout[2]) < 0.) c2[2] = 0.;
}


multcolormat(m3, m2, m1)	/* multiply m1 by m2 and put into m3 */
COLORMAT  m1, m2, m3;		/* m3 can be either m1 or m2 w/o harm */
{
	COLORMAT  mt;
	register int  i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			mt[i][j] =	m1[i][0]*m2[0][j] +
					m1[i][1]*m2[1][j] +
					m1[i][2]*m2[2][j] ;
	cpcolormat(m3, mt);
}


compxyz2rgbmat(mat, pr)		/* compute conversion from CIE to RGB space */
COLORMAT  mat;
register RGBPRIMS  pr;
{
	double  C_rD, C_gD, C_bD;

	C_rD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[GRN][CIEY] - pr[BLU][CIEY]) -
			  pr[WHT][CIEY]*(pr[GRN][CIEX] - pr[BLU][CIEX]) +
		  pr[GRN][CIEX]*pr[BLU][CIEY] - pr[BLU][CIEX]*pr[GRN][CIEY] ) ;
	C_gD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[BLU][CIEY] - pr[RED][CIEY]) -
			  pr[WHT][CIEY]*(pr[BLU][CIEX] - pr[RED][CIEX]) -
		  pr[RED][CIEX]*pr[BLU][CIEY] + pr[BLU][CIEX]*pr[RED][CIEY] ) ;
	C_bD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[RED][CIEY] - pr[GRN][CIEY]) -
			  pr[WHT][CIEY]*(pr[RED][CIEX] - pr[GRN][CIEX]) +
		  pr[RED][CIEX]*pr[GRN][CIEY] - pr[GRN][CIEX]*pr[RED][CIEY] ) ;

	mat[0][0] = (pr[GRN][CIEY] - pr[BLU][CIEY] -
			pr[BLU][CIEX]*pr[GRN][CIEY] +
			pr[BLU][CIEY]*pr[GRN][CIEX])/C_rD ;
	mat[0][1] = (pr[BLU][CIEX] - pr[GRN][CIEX] -
			pr[BLU][CIEX]*pr[GRN][CIEY] +
			pr[GRN][CIEX]*pr[BLU][CIEY])/C_rD ;
	mat[0][2] = (pr[GRN][CIEX]*pr[BLU][CIEY] -
			pr[BLU][CIEX]*pr[GRN][CIEY])/C_rD ;
	mat[1][0] = (pr[BLU][CIEY] - pr[RED][CIEY] -
			pr[BLU][CIEY]*pr[RED][CIEX] +
			pr[RED][CIEY]*pr[BLU][CIEX])/C_gD ;
	mat[1][1] = (pr[RED][CIEX] - pr[BLU][CIEX] -
			pr[RED][CIEX]*pr[BLU][CIEY] +
			pr[BLU][CIEX]*pr[RED][CIEY])/C_gD ;
	mat[1][2] = (pr[BLU][CIEX]*pr[RED][CIEY] -
			pr[RED][CIEX]*pr[BLU][CIEY])/C_gD ;
	mat[2][0] = (pr[RED][CIEY] - pr[GRN][CIEY] -
			pr[RED][CIEY]*pr[GRN][CIEX] +
			pr[GRN][CIEY]*pr[RED][CIEX])/C_bD ;
	mat[2][1] = (pr[GRN][CIEX] - pr[RED][CIEX] -
			pr[GRN][CIEX]*pr[RED][CIEY] +
			pr[RED][CIEX]*pr[GRN][CIEY])/C_bD ;
	mat[2][2] = (pr[RED][CIEX]*pr[GRN][CIEY] -
			pr[GRN][CIEX]*pr[RED][CIEY])/C_bD ;
}


comprgb2xyzmat(mat, pr)		/* compute conversion from RGB to CIE space */
COLORMAT  mat;
register RGBPRIMS  pr;
{
	double  C_rD, C_gD, C_bD, D;

	C_rD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[GRN][CIEY] - pr[BLU][CIEY]) -
			  pr[WHT][CIEY]*(pr[GRN][CIEX] - pr[BLU][CIEX]) +
		  pr[GRN][CIEX]*pr[BLU][CIEY] - pr[BLU][CIEX]*pr[GRN][CIEY] ) ;
	C_gD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[BLU][CIEY] - pr[RED][CIEY]) -
			  pr[WHT][CIEY]*(pr[BLU][CIEX] - pr[RED][CIEX]) -
		  pr[RED][CIEX]*pr[BLU][CIEY] + pr[BLU][CIEX]*pr[RED][CIEY] ) ;
	C_bD = (1./pr[WHT][CIEY]) *
			( pr[WHT][CIEX]*(pr[RED][CIEY] - pr[GRN][CIEY]) -
			  pr[WHT][CIEY]*(pr[RED][CIEX] - pr[GRN][CIEX]) +
		  pr[RED][CIEX]*pr[GRN][CIEY] - pr[GRN][CIEX]*pr[RED][CIEY] ) ;
	D = pr[RED][CIEX]*(pr[GRN][CIEY] - pr[BLU][CIEY]) +
			pr[GRN][CIEX]*(pr[BLU][CIEY] - pr[RED][CIEY]) +
			pr[BLU][CIEX]*(pr[RED][CIEY] - pr[GRN][CIEY]) ;
	mat[0][0] = pr[RED][CIEX]*C_rD/D;
	mat[0][1] = pr[GRN][CIEX]*C_gD/D;
	mat[0][2] = pr[BLU][CIEX]*C_bD/D;
	mat[1][0] = pr[RED][CIEY]*C_rD/D;
	mat[1][1] = pr[GRN][CIEY]*C_gD/D;
	mat[1][2] = pr[BLU][CIEY]*C_bD/D;
	mat[2][0] = (1.-pr[RED][CIEX]-pr[RED][CIEY])*C_rD/D;
	mat[2][1] = (1.-pr[GRN][CIEX]-pr[GRN][CIEY])*C_gD/D;
	mat[2][2] = (1.-pr[BLU][CIEX]-pr[BLU][CIEY])*C_bD/D;
}


comprgb2rgbmat(mat, pr1, pr2)	/* compute conversion from RGB1 to RGB2 */
COLORMAT  mat;
RGBPRIMS  pr1, pr2;
{
	COLORMAT  pr1toxyz, xyztopr2;

	if (pr1 == stdprims)	/* can use rgb2xyzmat */
		cpcolormat(pr1toxyz, rgb2xyzmat);
	else			/* otherwise compute it */
		comprgb2xyzmat(pr1toxyz, pr1);
	if (pr2 == stdprims)	/* can use xyz2rgbmat */
		cpcolormat(xyztopr2, xyz2rgbmat);
	else			/* otherwise compute it */
		compxyz2rgbmat(xyztopr2, pr2);
				/* combine transforms */
	multcolormat(mat, pr1toxyz, xyztopr2);
}
