#include "common.h"
#include "lmodel.h"
#include "shadow.h"

#ifdef USE_SUNFILTER
float4 	main		( float2 tc : TEXCOORD0, float4 tcJ : TEXCOORD1 ) : COLOR
{
  float4 	_P	= tex2D 	(s_position, 	tc)	;
		_P.w 	= 1.f					;
	float4 	PS	= mul		(m_shadow, 	_P)	;
	float 	s 	= shadowtest_sun(PS,tcJ)*sunmask(_P)	;
	return 	s	;
}
#else
float4 	main		( float2 tc : TEXCOORD0, float4 tcJ : TEXCOORD1 ) : COLOR
{
  float4 _P		= tex2D 	(s_position, 	tc); 
  float4  _N		= tex2D 	(s_normal,   	tc); 

	// ----- light-model
	float4	light 	= plight_infinity (_P.w,_P,_N,Ldynamic_dir);

	// ----- shadow
  	float4 	P4 	= float4	(_P.x,_P.y,_P.z,1.f);
	float4 	PS	= mul		(m_shadow, 	P4);
	float 	s 	= sunmask	(P4);
	#ifdef 	USE_SJITTER
	  s 	*= shadowtest_sun 	(PS,tcJ);
	#else
	  s 	*= shadow		(PS);
	#endif

	return 		blend		(Ldynamic_color * light * s,tc);
}
#endif
