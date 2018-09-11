#include "common.h"

#ifndef	SSAO_QUALITY

float	calc_ssao( float3 P, float3 N, float2 tc, float2 tcJ)
{
	return 1.0f;
}

#else	//	SSAO_QUALITY

#if SSAO_QUALITY == 3
#define RINGS 3
#define DIRS 8
static const float rads[4] =
{ //I know it will be more focused in the cener, but that's OK
    0.20000f,
	0.57735f,
	0.81650f,
	1.00000f
};
static const float angles[9] =
{
	0.0000f,
	0.7854f,
	1.5708f,
	2.3562f,
	3.1416f,
	3.9267f,
	4.7124f,
	5.4978f,
	6.2832f
};
#elif SSAO_QUALITY == 2
#define RINGS  3
#define DIRS 4
static const float rads[4] =
{ //I know it will be more focused in the cener, but that's OK
    0.20000f,
	0.57735f,
	0.81650f,
	1.00000f
};
static const float angles[5] =
{
	0.0000f,
	1.5708f,
	3.1416f,
	4.7124f,
	6.2832f
};
#elif SSAO_QUALITY == 1
#define RINGS 2
#define DIRS 4
static const float rads[3] =
{ //I know it will be more focused in the cener, but that's OK
    0.2000f,
    0.7071f,
	1.0000f,
};
static const float angles[5] =
{
	0.0000f,
	1.5708f,
	3.1416f,
	4.7124f,
	6.2832f
};
#endif

float4 ssao_params;
float4 pos_decompression_params;

uniform sampler2D 	jitter0;

float3 uv_to_eye(float2 uv, float eye_z)
{
    uv = (uv * float2(2.0, 2.0) - float2(1.0, 1.0));
    return float3(uv * pos_decompression_params.xy * eye_z, eye_z);
}
//	Screen space ambient occlusion
//	P	screen space position of the original point
//	N	screen space normal of the original point
//	tc	G-buffer coordinates of the original point
float	calc_ssao( float3 P, float3 N, float2 tc, float2 tcJ)
{
	const float ssao_noise_tile_factor = ssao_params.x;
	const float ssao_kernel_size = ssao_params.y;

	float point_depth = P.z;
	if (point_depth<0.01) point_depth = 100000.0f;	//	filter for the sky
//	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*100/point_depth;
//	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*100/max(point_depth,1.3);
//	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*70/point_depth;
//	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*150/max(point_depth,1.3);
//	Looks better but triggers some strange hardware(?) bug.
	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*ssao_kernel_size/max(point_depth,1.3);
//	float2 	scale 	= float2	(.5f / 1024.h, .5f / 768.h)*min( ssao_kernel_size/point_depth , ssao_kernel_size/1.3 );

	// sample 
	float 	occ	= 0.1f;	
	float num_dir	= 0.1f;
//	float 	occ	= 1.0f;
//	float num_dir	= 1.0f;

////////////////////////////////
//	jittering

//	float2	Mirror = tex2D( jitter0, tcJ );
//	float2	Mirror = float2(1,1);
	float3 tc1	= mul(m_invV, float4(P, 1.0f));
	tc1 *= ssao_noise_tile_factor;
//	tc1 *= 2;
//	tc1 *= 4;
	tc1.xz += tc1.y;
	float2	SmallTap = tex2D( jitter0, tc1.xz );
//	float2	Mirror = tex2D( jitter0, tc1.xz );
//	Mirror	= normalize(Mirror);

for (int rad=0; rad < RINGS; rad++)
{
	for (int dir=0; dir < DIRS; dir++)
	{
		SmallTap.x += 0.31337f;
		SmallTap.y += 0.73313f;
		SmallTap = frac(SmallTap);
		float	r		= lerp(rads[rad]*1.3, rads[rad+1]*1.3, SmallTap.x);
		float   a		= lerp(angles[dir], angles[dir+1], SmallTap.y);
		float s, c;
		sincos( a, s, c );
		float2	tap = float2( r * c, r * s );
				tap		*= scale;
				tap		+= tc;
#ifndef SSAO_OPT_DATA
		float3	tap_pos	= tex2D	(s_position,tap);
#else // SSAO_OPT_DATA
		float	z	= tex2Dlod(s_half_depth,float4(tap, 0, 0));
		float3	tap_pos	= uv_to_eye(tap, z);
#endif // SSAO_OPT_DATA
		float3 	dir 	= tap_pos-P.xyz;
		float	dist	= length(dir);
			dir 	= normalize(dir);
		float 	infl 	= saturate(dot( dir, N.xyz));
		float 	occ_factor = saturate(dist);

		{
//			occ += lerp( 1, occ_factor, infl);
//			num_dir += 1;		

			occ += (infl+0.01)*lerp( 1, occ_factor, infl)/(occ_factor+0.1);
			num_dir += (infl+0.01)/(occ_factor+0.1);

//			occ += (infl+0.1)*lerp( 1, occ_factor, infl)/(occ_factor+0.1);
//			num_dir += (infl+0.1)/(occ_factor+0.1);

//			occ += (infl+0.1)*lerp( 1, occ_factor, infl);
//			num_dir += (infl+0.1);
		}
	}
}
	occ /= num_dir;

//	occ = lerp(1, occ, saturate(point_depth/1.5f));

	occ = saturate(occ);
//	occ = Contrast(occ,2);
//	occ = occ*1.5 - 0.5;
//	occ = occ*occ;
//	occ = occ*0.5+0.5;
#if SSAO_QUALITY==1
	occ = (occ+0.3)/(1+0.3);
#else	//	SSAO_QUALITY==1
	occ = (occ+0.2)/(1+0.2);
#endif	//	SSAO_QUALITY==1
//	occ = 1;

	float WeaponAttenuation = smoothstep( 0.8, 0.9, length( P.xyz ));
	occ = lerp( 1, occ, WeaponAttenuation );

	return occ;
}

#endif	//	SSAO_QUALITY
