#include "common.h"

struct v2p
{
 	float2 	tc0		: TEXCOORD0;// base
	float4	c		: COLOR0;	// diffuse

#ifdef USE_SOFT_PARTICLES
	float4 tctexgen	: TEXCOORD1;
#endif
	float	fog		: FOG;		// fog
};

uniform sampler		s_distort;

float4 main (v2p I) : COLOR
{
	float4	distort		= tex2D(s_distort, I.tc0);
	
	if (distort.w <= 0.00001f)
		discard;
	
	float	factor		= distort.a * dot(I.c.rgb, 0.333f);
	float4	result		= float4(distort.rgb, factor);
			result.a   *= I.fog * I.fog; // skyloader: fog fix

	return result;
}
