#include "common.h"

//	Igor: used for volumetric light
uniform sampler2D 	s_vollight;

struct         _input      {
        float4	hpos	: POSITION;
#ifdef        USE_VTF
	float4	tc0	: TEXCOORD0;        // tc.xy, tc.w = tonemap scale
#else
	float2	tc0	: TEXCOORD0;        // tc.xy
#endif
};

struct        _out                {
        float4	low	: COLOR0;
        float4	high	: COLOR1;
};

_out         main                ( _input I )
{
        // final tone-mapping
#ifdef        USE_VTF
        float          	tm_scale        = I.tc0.w;	// interpolated from VS
#else
        float         	tm_scale        = tex2D(s_tonemap,I.tc0).x;
#endif
	_out	o;
	float4	color;

	color = tex2D(s_vollight, I.tc0);

	tonemap(o.low, o.high, color, tm_scale );

//	o.low = float4(1,0,0,0);

//	o.high = float4(0,0,0,0);
//	o.low = float4(0,0,0,0);

	return o;
}
