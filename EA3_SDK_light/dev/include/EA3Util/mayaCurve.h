#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef EA_PLATFORM_MICROSOFT
#	include <io.h>
#pragma warning (disable : 4244)
#else
#	include <unistd.h>
#endif
#include <string.h>
#include <math.h>

#include <EA3.h>
#include <EA3names.h>

float engineAnimEvaluate( const EA3Block*, float );

#define DEG_OVER_PI (57.29588262f)
#define PI_OVER_DEG (0.017453261f)
#define RAD_TO_DEG(x) ((x)*(DEG_OVER_PI)) 
#define DEG_TO_RAD(x) ((x)*(PI_OVER_DEG))

// ***************************************************************************************
enum JoInfinityType {
	kInfinityConstant,
	kInfinityLinear,
	kInfinityCycle,
	kInfinityCycleRelative,
	kInfinityOscillate
};
typedef enum JoInfinityType JoInfinityType;

#define kDegRad 0.0174532925199432958
#define kFourThirds (4.0 / 3.0)
#define kTwoThirds (2.0 / 3.0)
#define kOneThird (1.0 / 3.0)
#define kMaxTan 5729577.9485111479

enum JoTangentType {
	kTangentFixed,
	kTangentLinear,
	kTangentFlat,
	kTangentStep,
	kTangentSlow,
	kTangentFast,
	kTangentSmooth,
	kTangentClamped
};
typedef enum JoTangentType JoTangentType;

static JoTangentType getTangentType( const XbIdentifier& type ) {
	if (type == XBfixed) {
		return kTangentFixed;
	} else if (type == XBlinear) {
		return kTangentLinear;
	} else if (type == XBflat) {
		return kTangentFlat;
	} else if (type == XBsmooth) {
		return kTangentSmooth;
	} else if (type == XBstep) {
		return kTangentStep;
	} else if (type == XBslow) {
		return kTangentSlow;
	} else if (type == XBfast) {
		return kTangentFast;
	} else if (type == XBclamped) {
		return kTangentClamped;
	}; /* if */
	return kTangentSmooth;
}; /* getTangentType */

static JoInfinityType getInfinityType( const XbIdentifier& type ) {
	if (type == XBCONSTANT) {
		return kInfinityConstant;
	} else if (type == XBADDITIVE_CYCLE) {
		return kInfinityCycleRelative;
	} else if (type == XBCYCLERELATIVE) {
		return kInfinityCycleRelative;
	} else if (type == XBOSCILLATE) {
		return kInfinityOscillate;
	} else if (type == XBLINEAR) {
		return kInfinityLinear;
	} else if (type == XBCYCLE) {
		return kInfinityCycle;
	}; /* if */

	return (kInfinityConstant);
}; /* getInfinityType */

static bool getTangent( EA3Array* eArray, int ind, float fr,
	bool isWeighted, float& xx0, float& yy0, float& xx1, float& yy1 ) {

	JoTangentType inTangentType, outTangentType;

	float py, ny, dx;
	bool hasSmooth;
	float length;
	float inTanX=0, inTanY=0, outTanX=0, outTanY=0;
	float inTanXs=0, inTanYs=0, outTanXs=0, outTanYs=0;

	/* make sure we have useful information */
	if ((eArray == NULL) || (eArray->getNumMembers() == 0) 
		|| (ind > eArray->getNumMembers())) {
		return false;
	}; /* if */

	EA3Field* pKey = NULL;
        if ((ind-1)>=0) {
          pKey = eArray->getByIndex( ind-1 );
        }; /* if */
	EA3Field* key = eArray->getByIndex( ind );
	EA3Field* nKey = NULL;
        if ((ind+1)<eArray->getNumMembers()) {
          nKey = eArray->getByIndex( ind+1 );
        }; /* if */

	if (!key) return false;
  if (key->getType()==XBsoft2DKey) {
    xx0 = key->data[0].f;
    yy0 = key->data[1].f;
    xx1 = key->data[4].f;
    yy1 = yy0 + key->data[5].f;
    return true;
  }

	inTangentType = getTangentType( key->data[2].str );
	outTangentType = getTangentType( key->data[5].str );

	if ((inTangentType == kTangentClamped) && (pKey != NULL)) {
		py = pKey->data[1].f - key->data[1].f;
		if (py < 0.0) py = -py;
		ny = (nKey == NULL ? py : nKey->data[1].f - key->data[1].f);
		if (ny < 0.0) ny = -ny;
		if ((ny <= 0.05) || (py <= 0.05)) {
			inTangentType = kTangentFlat;
		}; /* if */
	}; /* if */
	hasSmooth = false;
	switch (inTangentType) {
		case kTangentFixed:
			inTanX = key->data[4].f * cos( atan(tan(DEG_TO_RAD(key->data[3].f))*fr) ) * 3.0;
			inTanY = key->data[4].f * sin( atan(tan(DEG_TO_RAD(key->data[3].f))*fr) ) * 3.0;
			break;
		case kTangentLinear:
			if (pKey == NULL) {
				inTanX = 1.0;
				inTanY = 0.0;
			}	else {
				inTanX = key->data[0].f - pKey->data[0].f;
				inTanY = key->data[1].f - pKey->data[1].f;
			}; /* if */
			break;
		case kTangentFlat:
			if (pKey == NULL) {
				inTanX = (nKey == NULL ? 0.0 : nKey->data[0].f - key->data[0].f);
				inTanY = 0.0;
			}	else {
				inTanX = key->data[0].f - pKey->data[0].f;
				inTanY = 0.0;
			}; /* if */
			break;
		case kTangentStep:
			inTanX = 0.0;
			inTanY = 0.0;
			break;
		case kTangentSlow:
		case kTangentFast:
			inTangentType = kTangentSmooth;
			if (pKey == NULL) {
				inTanX = 1.0;
				inTanY = 0.0;
			}	else {
				inTanX = key->data[0].f - pKey->data[0].f;
				inTanY = key->data[1].f - pKey->data[1].f;
			}; /* if */
			break;
		case kTangentSmooth:
		case kTangentClamped:
			inTangentType = kTangentSmooth;
			hasSmooth = true;
			break;
	}; /* case */

	if ((outTangentType == kTangentClamped) && (nKey != NULL)) {
		ny = nKey->data[1].f - key->data[1].f;
		if (ny < 0.0) ny = -ny;
		py = (pKey == NULL ? ny : pKey->data[1].f - key->data[1].f);
		if (py < 0.0) py = -py;
		if ((ny <= 0.05) || (py <= 0.05)) {
			outTangentType = kTangentFlat;
		}; /* if */
	}; /* if */

	switch (outTangentType) {
		case kTangentFixed:
			outTanX = key->data[7].f * cos( atan(tan(DEG_TO_RAD(key->data[6].f))*fr) ) * 3.0;
			outTanY = key->data[7].f * sin( atan(tan(DEG_TO_RAD(key->data[6].f))*fr) ) * 3.0;
			break;
		case kTangentLinear:
			if (nKey == NULL) {
				outTanX = 1.0;
				outTanY = 0.0;
			}	else {
				outTanX = nKey->data[0].f - key->data[0].f;
				outTanY = nKey->data[1].f - key->data[1].f;
			}; /* if */
			break;
		case kTangentFlat:
			if (nKey == NULL) {
				outTanX = (pKey == NULL ? 0.0 : key->data[0].f - pKey->data[0].f);
				outTanY = 0.0;
			}	else {
				outTanX = nKey->data[0].f - key->data[0].f;
				outTanY = 0.0;
			}; /* if */
			break;
		case kTangentStep:
			outTanX = 0.0;
			outTanY = 0.0;
			break;
		case kTangentSlow:
		case kTangentFast:
			outTangentType = kTangentSmooth;
			if (nKey == NULL) {
				outTanX = 1.0;
				outTanY = 0.0;
			}	else {
				outTanX = nKey->data[0].f - key->data[0].f;
				outTanY = nKey->data[1].f - key->data[1].f;
			}; /* if */
			break;
		case kTangentSmooth:
		case kTangentClamped:
			outTangentType = kTangentSmooth;
			hasSmooth = true;
			break;
	}; /* case */

	if (hasSmooth) {
		if ((pKey == NULL) && (nKey != NULL)) {
			outTanXs = nKey->data[0].f - key->data[0].f;
			outTanYs = nKey->data[1].f - key->data[1].f;
			inTanXs = outTanXs;
			inTanYs = outTanYs;
		}	else if ((pKey != NULL) && (nKey == NULL)) {
			outTanXs = key->data[0].f - pKey->data[0].f;
			outTanYs = key->data[1].f - pKey->data[1].f;
			inTanXs = outTanXs;
			inTanYs = outTanYs;
		}	else if ((pKey != NULL) && (nKey != NULL)) {
			/* There is a CV before and after this one*/
			/* Find average of the adjacent in and out tangents. */

			dx = nKey->data[0].f - pKey->data[0].f;
			if (dx < 0.0001f) {
				outTanYs = (float)kMaxTan;
			} else {
				outTanYs = (nKey->data[1].f - pKey->data[1].f) / dx;
			}; /* if */

			outTanXs = nKey->data[0].f - key->data[0].f;
			inTanXs = key->data[0].f - pKey->data[0].f;
			inTanYs = outTanYs * inTanXs;
			outTanYs *= outTanXs;
		} else {
			inTanXs = 1.0f;
			inTanYs = 0.0f;
			outTanXs = 1.0f;
			outTanYs = 0.0f;
		}; /* if */

		if (inTangentType == kTangentSmooth) {
			inTanX = inTanXs;
			inTanY = inTanYs;
		}; /* if */
		if (outTangentType == kTangentSmooth) {
			outTanX = outTanXs;
			outTanY = outTanYs;
		}; /* if */
	}; /* if */

	if (isWeighted) {
		if (inTanX < 0.0) inTanX = 0.0;
		if (outTanX < 0.0) outTanX = 0.0;
	}	else {
		if (inTanX < 0.0) {
			inTanX = 0.0;
		}; /* if */
		length = sqrt ((inTanX * inTanX) + (inTanY * inTanY));
		if (length != 0.0) {
			inTanX /= length;
			inTanY /= length;
		}; /* if */
		if ((inTanX == 0.0) && (inTanY != 0.0)) {
			inTanX = 0.0001f;
			inTanY = (inTanY < 0.0 ? -1.0 : 1.0) * (inTanX * kMaxTan);
		}; /* if */
		if (outTanX < 0.0) {
			outTanX = 0.0;
		}; /* if */
		length = sqrt ((outTanX * outTanX) + (outTanY * outTanY));
		if (length != 0.0) {
			outTanX /= length;
			outTanY /= length;
		}; /* if */
		if ((outTanX == 0.0) && (outTanY != 0.0)) {
			outTanX = 0.0001f;
			outTanY = (outTanY < 0.0 ? -1.0 : 1.0) * (outTanX * kMaxTan);
		}; /* if */
	}; /* if */

	xx0 = inTanX;
	yy0 = inTanY;
	xx1 = outTanX;
	yy1 = outTanY;

	return true;
} /* getTangent */

static float sMachineTolerance;

static int dbl_gt( float *a, float *b ) {
	return (*a > *b ? 1 : 0);
}; /* dbl_gt */

static void dbl_mult( float *a, float *b, float *atimesb ) {
	float product = (*a) * (*b);
	*atimesb = product;
}; /* dbl_mult */

static void dbl_add( float *a, float *b, float *aplusb ) {
	float sum = (*a) + (*b);
	*aplusb = sum;
}; /* dbl_add */

static void init_tolerance() {
    float one, half, sum;
    one  = 1.0;
    half = 0.5;
    sMachineTolerance = 1.0;
    do {
		dbl_mult (&sMachineTolerance, &half, &sMachineTolerance);
		dbl_add (&sMachineTolerance, &one, &sum);
    } while (dbl_gt (&sum, &one));
	sMachineTolerance = 2.0 * sMachineTolerance;
}; /* init_tolerance */

static void constrainInsideBounds( float *x1, float *x2 ) {
	float b, c,  discr,  root;

	if ((*x1 + sMachineTolerance) < kFourThirds) {
		b = *x1 - 2.0;
		c = *x1 - 1.0;
		discr = sqrt (b * b - 4 * c * c);
		root = (-b + discr) * 0.5;
		if ((*x2 + sMachineTolerance) > root) {
			*x2 = root - sMachineTolerance;
		} else {
			root = (-b - discr) * 0.5;
			if (*x2 < (root + sMachineTolerance)) {
				*x2 = root + sMachineTolerance;
			}; /* if */
		}; /* if */
	}	else {
		*x1 = kFourThirds - sMachineTolerance;
		*x2 = kOneThird - sMachineTolerance;
	}; /* if */
}; /* constrainInsideBounds */

static void checkMonotonic( float *x1, float *x2 ) {
	float d;

	*x2 = 1.0 - *x2;
	if (*x1 < 0.0) *x1 = 0.0;
	if (*x2 < 0.0) *x2 = 0.0;
	if ((*x1 > 1.0) || (*x2 > 1.0)) {
		d = *x1 * (*x1 - 2.0 + *x2) + *x2 * (*x2 - 2.0) + 1.0;
		if ((d + sMachineTolerance) > 0.0) {
			constrainInsideBounds (x1, x2);
		}; /* if */
	}; /* if */
	*x2 = 1.0 - *x2;
}; /* checkMonotonic */

static void bezierToPower( float a1, float b1, float c1, float d1,
	float *a2, float *b2, float *c2, float *d2 ) {
	float a = b1 - a1;
	float b = c1 - b1;
	float c = d1 - c1;
	float d = b - a;
	*a2 = c - b - d;
	*b2 = d + d + d;
	*c2 = a + a + a;
	*d2 = a1;
}; /* bezierToPower */

static float ag_horner1( float P[], int deg, float s ) {
	float h = P[deg];
	while (--deg >= 0) h = (s * h) + P[deg];
	return (h);
}; /* ag_horner1 */

typedef struct ag_polynomial {
	float *p;
	int deg;
} AG_POLYNOMIAL;

static float ag_zeroin2( float a, float b, float fa, float fb, float tol, AG_POLYNOMIAL *pars ) {
	int test;
	float c, d, e, fc, del, m, machtol, p, q, r, s;

	machtol = sMachineTolerance;

label1:
	c = a;  fc = fa;  d = b-a;  e = d;
label2:
	if (fabs(fc) < fabs(fb)) {
		a = b;   b = c;   c = a;   fa = fb;   fb = fc;   fc = fa;
	}; /* if */

	del = 2.0 * machtol * fabs(b) + 0.5*tol;
	m = 0.5 * (c - b);
	test = ((fabs(m) > del) && (fb != 0.0));
	if (test) {
		if ((fabs(e) < del) || (fabs(fa) <= fabs(fb))) {
			d = m;  e= d;
		}	else {
			s = fb / fa;
			if (a == c) {
				p = 2.0*m*s;    q = 1.0 - s;
			}	else {
				q = fa/fc;
				r = fb/fc;
				p = s*(2.0*m*q*(q-r)-(b-a)*(r-1.0));
				q = (q-1.0)*(r-1.0)*(s-1.0);
			}; /* if */
			if (p > 0.0) q = -q;  else p = -p;
			s = e;   e = d;
			if ((2.0*p < 3.0*m*q-fabs(del*q))&&(p < fabs(0.5*s*q))) {
				d = p/q;
			}	else {
				d = m;	e = d;
			}; /* if */
		}; /* if */
		a = b;	fa = fb;
		if ( fabs(d) > del )   b += d;
		else if (m > 0.0) b += del;  else b -= del;
		fb = ag_horner1 (pars->p, pars->deg, b);
		if (fb*(fc/fabs(fc)) > 0.0 ) {
			goto label1;
		}	else {
			goto label2;
		}; /* if */
	}; /* if */
	return (b);
}; /* ag_zeroin2 */

static float ag_zeroin( float a, float b, float tol, AG_POLYNOMIAL *pars ) {
	float fa, fb;

	fa = ag_horner1 (pars->p, pars->deg, a);
	if (fabs(fa) < sMachineTolerance) return(a);
	fb = ag_horner1 (pars->p, pars->deg, b);
	if (fabs(fb) < sMachineTolerance) return(b);

	return (ag_zeroin2 (a, b, fa, fb, tol, pars));
}; /* ag_zeroin */

static int polyZeroes( float Poly[], int deg, float a, int a_closed, float b, int b_closed, float Roots[]) {
	int i, left_ok, right_ok, nr, ndr, skip;
	float e, f, s, pe, ps, tol, *p, p_x[22], *d, d_x[22], *dr, dr_x[22];
	AG_POLYNOMIAL ply;

	e = pe = 0.0;  
	f = 0.0;

	for (i = 0 ; i < deg + 1; ++i)
		f += fabs(Poly[i]);
	tol = (fabs(a) + fabs(b))*(deg+1)*sMachineTolerance;
	if (f <= tol)  return(-1);

	p = p_x;  d = d_x;  dr = dr_x;
	for (i = 0 ; i < deg + 1; ++i)
		p[i] = 1.0/f * Poly[i];

	while (fabs(p[deg]) < tol) deg--;

	nr = 0;
	if (deg == 0) return (nr);

	if (deg == 1) {
		Roots[0] = -p[0] / p[1];
		left_ok  = (a_closed) ? (a<Roots[0]+tol) : (a<Roots[0]-tol);
		right_ok = (b_closed) ? (b>Roots[0]-tol) : (b>Roots[0]+tol);
		nr = (left_ok && right_ok) ? 1 : 0;
		if (nr) {
			if (a_closed && Roots[0]<a) Roots[0] = a;
			else if (b_closed && Roots[0]>b) Roots[0] = b;
		}
		return (nr);
	}	else {
		ply.p = p;  ply.deg = deg;

		for (i=1; i<=deg; i++) d[i-1] = i*p[i];

		ndr = polyZeroes ( d, deg-1, a, 0, b, 0, dr );
		if (ndr == -1) return (0);

		for (i=skip=0; i<=ndr; i++) {
			if (nr>deg) return (nr);
			if (i==0) {
				s=a; ps = ag_horner1( p, deg, s);
				if ( fabs(ps)<=tol && a_closed) Roots[nr++]=a;
			}
			else { s=e; ps=pe; }
			if (i==ndr) { e = b; skip = 0;}
			else e=dr[i];
			pe = ag_horner1( p, deg, e );
			if (skip) skip = 0;
			else {
				if ( fabs(pe) < tol ) {
					if (i!=ndr || b_closed) {
						Roots[nr++] = e;
						skip = 1;
					}
				} else if ((ps<0 && pe>0)||(ps>0 && pe<0)) {
					Roots[nr++] = ag_zeroin(s, e, 0.0, &ply );
					if ((nr>1) && Roots[nr-2]>=Roots[nr-1]-tol) { 
						Roots[nr-2] = (Roots[nr-2]+Roots[nr-1]) * 0.5;
						nr--;
					}
				}
			}
		}
	}
	return (nr);
} 

static float engineBezierEvaluate( const EA3Block* eBlock, float time, float x[4], float y[4] ) {
	bool isLinear = false;
	float t, s, poly[4], roots[5];
	float fCoeff[4], fPolyY[4];
	int numRoots;

	if (eBlock == NULL) {
		return (0.0f);
	}; /* if */

	static bool sInited = false;
	float rangeX, dx1, dx2, nX1, nX2, oldX1, oldX2;

	if (!sInited) {
		init_tolerance ();
		sInited = true;
	}; /* if */

	rangeX = x[3] - x[0];
	if (rangeX == 0.0) {
		return(0.0f);
	}; /* if */
	dx1 = x[1] - x[0];
	dx2 = x[2] - x[0];

	nX1 = dx1 / rangeX;
	nX2 = dx2 / rangeX;

	if ((nX1 == kOneThird) && (nX2 == kTwoThirds)) {
		isLinear = true;
	} else {
		isLinear = false;
	}; /* if */

	oldX1 = nX1;
	oldX2 = nX2;

	if (nX1 < 0.0) nX1 = 0.0;
	if (nX2 > 1.0) nX2 = 1.0;
	if ((nX1 > 1.0) || (nX2 < -1.0)) {
		checkMonotonic (&nX1, &nX2);
	}; /* if */

	if (nX1 != oldX1) {
		x[1] = x[0] + nX1 * rangeX;
		if (oldX1 != 0.0) {
			y[1] = y[0] + (y[1] - y[0]) * nX1 / oldX1;
		}; /* if */
	}; /* if */
	if (nX2 != oldX2) {
		x[2] = x[0] + nX2 * rangeX;
		if (oldX2 != 1.0) {
			y[2] = y[3] - (y[3] - y[2]) * (1.0 - nX2) / (1.0 - oldX2);
		}; /* if */
	}; /* if */

	bezierToPower (	0.0, nX1, nX2, 1.0,
		&(fCoeff[3]), &(fCoeff[2]), &(fCoeff[1]), &(fCoeff[0]) );
	bezierToPower (	y[0], y[1], y[2], y[3],
		&(fPolyY[3]), &(fPolyY[2]), &(fPolyY[1]), &(fPolyY[0]) );

	if (x[0] == time) {
		s = 0.0;
	}	else if (x[3] == time) {
		s = 1.0;
	}	else {
		s = (time - x[0]) / (x[3] - x[0]);
	}; /* if */

	if (isLinear) {
		t = s;
	}	else {
		poly[3] = fCoeff[3];
		poly[2] = fCoeff[2];
		poly[1] = fCoeff[1];
		poly[0] = fCoeff[0] - s;

		numRoots = polyZeroes (poly, 3, 0.0, 1, 1.0, 1, roots);
		if (numRoots == 1) {
			t = roots[0];
		} else {
			t = 0.0;
		}; /* if */
	}; /* if */
	return (t * (t * (t * fPolyY[3] + fPolyY[2]) + fPolyY[1]) + fPolyY[0]);
}; /* engineBezierEvaluate */

static float engineHermiteEvaluate( const EA3Block* eBlock, float time, float x[4], float y[4] ) {
	float t;
	float fCoeff[4];
	float dx, dy, tan_x, m1, m2, length, d1, d2;

	if (eBlock == NULL) {
		return (0.0f);
	}; /* if */

	dx = x[3] - x[0];
	dy = y[3] - y[0];

	tan_x = x[1] - x[0];
	m1 = m2 = (float)kMaxTan;
	if (tan_x != 0.0) {
		m1 = (y[1] - y[0]) / tan_x;
	}; /* if */

	tan_x = x[3] - x[2];
	if (tan_x != 0.0) {
		m2 = (y[3] - y[2]) / tan_x;
	}; /* if */

	length = 1.0 / (dx * dx);
	d1 = dx * m1;
	d2 = dx * m2;
	fCoeff[0] = (d1 + d2 - dy - dy) * length / dx;
	fCoeff[1] = (dy + dy + dy - d1 - d1 - d2) * length;
	fCoeff[2] = m1;
	fCoeff[3] = y[0];

	t = time - x[0];
	return (t * (t * (t * fCoeff[0] + fCoeff[1]) + fCoeff[2]) + fCoeff[3]);
}; /* engineHermiteEvaluate */

static float evaluateInfinities( const EA3Block* eBlock, float time, float fr, bool evalPre ) {
	float value = 0.0;
	float	valueRange;
	float	factoredTime, firstTime, lastTime, timeRange;
	float	remaind, tanX, tanY;
	double numCycles, notUsed;

	EA3Array* eArray = NULL;

	if ((eBlock == NULL) || ((eArray = eBlock->getByType( XBkey )->toArray())->getNumMembers()== 0)) {
		return value;
	}; /* if */

	firstTime = eArray->getByIndex(0)->toField()->data[0].f;
	lastTime = eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[0].f;
	timeRange = lastTime - firstTime;
	if (timeRange == 0.0) {
		return eArray->getByIndex(0)->toField()->data[1].f;
	}; /* if */

	if (time > lastTime) {
		remaind= fabs(modf((time - lastTime) / timeRange, &numCycles));
	}	else {
		remaind= fabs(modf((time - firstTime) / timeRange, &numCycles));
	}; /* if */
	factoredTime = timeRange * remaind;
	numCycles = fabs(numCycles) + 1;

	if (evalPre) {
		if (getInfinityType(eBlock->getByType( XBpreInfinity )->toField()->data[0].str) == kInfinityOscillate) {
			if ((remaind= modf (numCycles / 2.0, &notUsed)) != 0.0) {
				factoredTime = firstTime + factoredTime;
			}	else {
				factoredTime = lastTime - factoredTime;
			}; /* if */
		}	else if ((getInfinityType(eBlock->getByType( XBpreInfinity )->toField()->data[0].str) == kInfinityCycle)
			|| (getInfinityType(eBlock->getByType( XBpreInfinity )->toField()->data[0].str) == kInfinityCycleRelative)) {
			factoredTime = lastTime - factoredTime;
		}	else if (getInfinityType(eBlock->getByType( XBpreInfinity )->toField()->data[0].str) == kInfinityLinear) {
			factoredTime = firstTime - time;

			float crap, crap2;
      bool isMayaW = eArray->getByIndex(0)->getType()==XBmayaW?true:false;
			getTangent( eArray, 0, fr, isMayaW, tanX, tanY, crap, crap2 ); 
			value = eArray->getByIndex(0)->toField()->data[1].f;
			if (tanX != 0.0f) {
				value -= ((factoredTime * tanY) / tanX);
			}; /* if */
			return (value);
		}; /* if */

	}	else {
		if (getInfinityType(eBlock->getByType( XBpostInfinity )->toField()->data[0].str) == kInfinityOscillate) {
			if ((remaind= modf (numCycles / 2.0, &notUsed)) != 0.0) {
				factoredTime = lastTime - factoredTime;
			}	else {
				factoredTime = firstTime + factoredTime;
			}; /* if */
		}	else if ((getInfinityType(eBlock->getByType( XBpostInfinity )->toField()->data[0].str) == kInfinityCycle)
		  || (getInfinityType(eBlock->getByType( XBpostInfinity )->toField()->data[0].str) == kInfinityCycleRelative)) {
			factoredTime = firstTime + factoredTime;
		}	else if (getInfinityType(eBlock->getByType( XBpostInfinity )->toField()->data[0].str) == kInfinityLinear) {
			factoredTime = time - lastTime;

			float crap, crap2;
      bool isMayaW = eArray->getByIndex(eArray->getNumMembers()-1)->getType()==XBmayaW?true:false;
			getTangent( eArray, eArray->getNumMembers()-1, fr, isMayaW, crap, crap2, tanX, tanY );
			value = eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[1].f;
			if (tanX != 0.0) {
				value += ((factoredTime * tanY) / tanX);
			}; /* if */
			return value;
		}; /* if */
	}; /* if */

	value = engineAnimEvaluate( eBlock, factoredTime );

	if (evalPre && (getInfinityType(eBlock->getByType( XBpreInfinity )->toField()->data[0].str) == kInfinityCycleRelative)) {
		valueRange = eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[1].f - eArray->getByIndex(0)->toField()->data[1].f;
		value -= (numCycles * valueRange);
	} else if (!evalPre && (getInfinityType(eBlock->getByType( XBpostInfinity )->toField()->data[0].str) == kInfinityCycleRelative)) {
		valueRange = eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[1].f - eArray->getByIndex(0)->toField()->data[1].f;
		value += (numCycles * valueRange);
	}; /* if */

	return value;
}; /* evaluateInfinities */

static bool find( const EA3Block* eBlock, float time, int* ind) {
	int len, mid, low, high;

	if ((eBlock == NULL) || (ind == NULL)) {
		return false;
	}; /* if */

	EA3Array* eArray = eBlock->getByType( XBkey )->toArray();

	*ind = 0;
	len = eArray->getNumMembers();
	if (len > 0) {
		low = 0;
		high = len - 1;

		do {
			mid = (low + high) >> 1;

			if (time < eArray->getByIndex(mid)->toField()->data[0].f) {
				high = mid - 1;
			} else if (time > eArray->getByIndex(mid)->toField()->data[0].f) {
				low  = mid + 1;
			}	else {
				*ind = mid;
				return true;
			}; /* if */
		} while (low <= high);

		*ind = low;
	}; /* if */

	return false;
}; /* find */

float engineAnimEvaluate( const EA3Block *eBlock, float time ) {
	//bool withinInterval = false;
	int ind;
	float value = 0.0;
	float fr = 30.0f;
	float x[4];
	float y[4];

	EA3Array* eArray = NULL;
	
	if ((eBlock == NULL) || ((eArray = eBlock->getByType( XBkey )->toArray())->getNumMembers()== 0)) {
		return (value);
	}; /* if */

	fr = eBlock->getContainer()->getContainer()->toBlock()->getByType( XBFPS )->toField()->data[0].f;

	if (time < eArray->getByIndex(0)->toField()->data[0].f) {
		if (getInfinityType(eBlock->getByType(XBpreInfinity)->toField()->data[0].str) == kInfinityConstant) {
			return (eArray->getByIndex(0)->toField()->data[1].f);
		}; /* if */
		return evaluateInfinities( eBlock, time, fr, true );
	}; /* if  */

	if (time > eArray->getByIndex( eArray->getNumMembers()-1 )->toField()->data[0].f) {
		if (getInfinityType(eBlock->getByType(XBpostInfinity)->toField()->data[0].str) == kInfinityConstant) {
			return (eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[1].f);
		}; /* if */
		return evaluateInfinities( eBlock, time, fr, false );
	}; /* if  */

	if (find (eBlock, time, &ind) || (ind == 0)) {
		return eArray->getByIndex( ind )->toField()->data[1].f;
	} else if (ind == eArray->getNumMembers()) {
		return eArray->getByIndex( eArray->getNumMembers() - 1 )->toField()->data[1].f;
	}; /* if */

	float inTanX, inTanY, outTanX, outTanY, crap1, crap2;
  bool isMayaW = eArray->getByIndex(0)->getType()==XBmayaW?true:false;
	getTangent( eArray, (ind-1), fr, isMayaW, crap1, crap2, outTanX, outTanY );
	getTangent( eArray, (ind), fr, isMayaW, inTanX, inTanY, crap1, crap2 );

	if ((outTanX == 0.0) &&	(outTanY == 0.0)) {
		value = eArray->getByIndex( ind-1 )->toField()->data[1].f;
	}	else {

		x[0] = eArray->getByIndex( ind-1 )->toField()->data[0].f;
		y[0] = eArray->getByIndex( ind-1 )->toField()->data[1].f;
		x[1] = x[0] + (outTanX * kOneThird);
		y[1] = y[0] + (outTanY * kOneThird);

		x[3] = eArray->getByIndex( ind )->toField()->data[0].f;
		y[3] = eArray->getByIndex( ind )->toField()->data[1].f;
		x[2] = x[3] - (inTanX * kOneThird);
		y[2] = y[3] - (inTanY * kOneThird);

printf( "\t1=[%2.3f %2.3f] 2=[%2.3f %2.3f] 3=[%2.3f %2.3f] 4=[%2.3f %2.3f]\n",
	x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3] );

		if (eArray->getByIndex(0)->getType() == XBmayaW) {
			value = engineBezierEvaluate( eBlock, time, x, y );
		}	else {
			value = engineHermiteEvaluate( eBlock, time, x, y );
		}; /* if */
	}; /* if */

	return value;
}; /* engineAnimEvaluate */
