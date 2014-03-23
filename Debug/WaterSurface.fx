uniform extern float4x4 g_WVP;
uniform extern float4x4 g_World; // Needed for specular calculation.
uniform extern float g_Time;
uniform extern bool g_EnableGerstnerWaves = false;

// -------------------------------------------------------------------------
// Needed to transform vertex normals into world space
// as lighting vectors are in a specific coordinate system.
// -------------------------------------------------------------------------
uniform extern float4x4 g_WorldInverseTranspose; 

// -------------------------------------------------------------------------
// Lighting Handles
// -------------------------------------------------------------------------
uniform extern float4 g_AmbientMtrl;
uniform extern float4 g_AmbientLight;
uniform extern float4 g_DiffuseMtrl;
uniform extern float4 g_DiffuseLight;
uniform extern float4 g_SpecularMtrl;
uniform extern float4 g_SpecularLight;
uniform extern float  g_SpecularPower;
uniform extern float3 g_LightVecW;
uniform extern float3 g_EyePosW;

// -------------------------------------------------------------------------
// Texture Handles
// -------------------------------------------------------------------------
uniform extern texture g_TexWater0;
uniform extern float2 g_TexWaterOffset0;

uniform extern texture g_TexWater1;
uniform extern float2 g_TexWaterOffset1;

uniform extern texture g_TexWater2;
uniform extern float2 g_TexWaterOffset2;

uniform extern texture g_TexWater3;
uniform extern float2 g_TexWaterOffset3;

uniform extern texture g_TexWater4;
uniform extern float2 g_TexWaterOffset4;

uniform extern texture g_TexWater5;
uniform extern float2 g_TexWaterOffset5;

// -------------------------------------------------------------------------
// Texture Declarations
// -------------------------------------------------------------------------
sampler WaterTex0 = sampler_state
{
	Texture = <g_TexWater0>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler WaterTex1 = sampler_state
{
	Texture = <g_TexWater1>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler WaterTex2 = sampler_state
{
	Texture = <g_TexWater2>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler WaterTex3 = sampler_state
{
	Texture = <g_TexWater3>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler WaterTex4 = sampler_state
{
	Texture = <g_TexWater4>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

sampler WaterTex5 = sampler_state
{
	Texture = <g_TexWater5>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
	AddressV  = WRAP;
};

// -------------------------------------------------------------------------
// Wave Calculation Variables
// -------------------------------------------------------------------------
uniform extern float PI = 3.141593;

struct OutputVS
{
	float4 posH    	: POSITION0;
	float3 normalW 	: TEXCOORD0;
	float3 posW	 		: TEXCOORD1;
	float2 tex0	 		: TEXCOORD2;
	float2 tex1	 		: TEXCOORD3;
	float2 tex2	 		: TEXCOORD4;
	float2 tex3	 		: TEXCOORD5;
	float2 tex4	 		: TEXCOORD6;
	float2 tex5	 		: TEXCOORD7;
};

// -------------------------------------------------------------------------
// Gerstner Wave Variables add wave variations to the Fast Fourier Wave
// version when they are enabled.
// -------------------------------------------------------------------------
struct GerstnerWave
{
	float3 vecWaveDirection;
	float fAmplitude;
	float fAngularFrequency;
	float fWaveLength;
	float fPhaseShift;
};

const static int MAX_NUM_WAVES = 10;
uniform extern GerstnerWave g_GerstnerWaves[MAX_NUM_WAVES];

// -------------------------------------------------------------------------
// Run the Gerstner Waves computation which sums together random sinusoidal
// waves to produce a realistic effect.
// -------------------------------------------------------------------------
float3 ComputeGerstnerWaves(float3 posL)
{
	float3 posL_Out = { posL.x, posL.y, posL.z };
	float2 vecX0 = { posL_Out.x, posL_Out.z };
	float fAngle = 0;
	float fMagnitude = 0;
	
	for (int i = 0; i < 1; i++)
	{
		// -------------------------------------------------------------------------
		// Intermediate Calculations
		// (k dot x0) - wt
		// -------------------------------------------------------------------------
		fAngle = (dot(g_GerstnerWaves[i].vecWaveDirection, vecX0) - (g_GerstnerWaves[i].fAngularFrequency * g_Time)) + g_GerstnerWaves[i].fPhaseShift;
		fMagnitude = (2 * PI) / g_GerstnerWaves[i].fWaveLength;

		posL_Out.y += g_GerstnerWaves[i].fAmplitude * cos(fAngle);  
	}
	
	return posL_Out;
}

// -------------------------------------------------------------------------
// Since the Waves are animated, we must compute custom vertex normals.
// Basically, find two tangent vectors at each vertex (via partial
// derivatives) and then take the cross product.
// -------------------------------------------------------------------------
float3 GetWaveVertexNormal(float3 posL)
{
	float fVertexDistance = sqrt(posL.x * posL.x + posL.z * posL.z);
	
	// -------------------------------------------------------------------------
	// The Derivative of a sum of functions is the sum of the derivatives.
	// -------------------------------------------------------------------------
	float dh_dx = 0.0f;
	float dh_dz = 0.0f;
	
	for (int i = 0; i < 1; i++)
	{
		dh_dx += (posL.x * posL.y) / fVertexDistance;
		dh_dz += (posL.z * posL.y) / fVertexDistance;
	}

	float3 vec_dh_dx_Tangent = { 1.0f, dh_dx, 0 };
	float3 vec_dh_dz_Tangent = { 0.0f, dh_dz, 1 };
	float3 vec_cross = cross(vec_dh_dx_Tangent, vec_dh_dz_Tangent);

	return vec_cross;
}

OutputVS Phong_VS(float3 posL : POSITION0,
                  float3 normalL : NORMAL0,
                  float2 tex0: TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;
	
	if (g_EnableGerstnerWaves == true)
	{
		posL = ComputeGerstnerWaves(posL);
	}
	
	// -------------------------------------------------------------------------
	// Transform the Normal to World Space as this is the Lighting Vector's
	// coordinate system.
	// -------------------------------------------------------------------------
	float3 vecNormal = normalL; //{0, 1, 0};//GetWaveVertexNormal(posL);
	float3 normalWorld = mul(float4(vecNormal, 0.0f), g_WorldInverseTranspose).xyz;
	outVS.normalW = normalize(normalWorld);
	
	// -------------------------------------------------------------------------
	// Transform vertex position to world space
	// -------------------------------------------------------------------------
	outVS.posW = mul(float4(posL, 1.0f), g_World).xyz;
	
	// -------------------------------------------------------------------------
	// Transform to homogeneous clip space.
	// -------------------------------------------------------------------------
	outVS.posH = mul(float4(posL, 1.0f), g_WVP);
	
	// -------------------------------------------------------------------------
	// Offset the Texture Coordinates to create a Scrolling Animation.
	// -------------------------------------------------------------------------
	outVS.tex0 = tex0 + g_TexWaterOffset0;
	outVS.tex1 = tex0 + g_TexWaterOffset1;
	outVS.tex2 = tex0 + g_TexWaterOffset2;
	outVS.tex3 = tex0 + g_TexWaterOffset3;
	outVS.tex4 = tex0 + g_TexWaterOffset4;
	outVS.tex5 = tex0 + g_TexWaterOffset5;

	return outVS;
}

float4 Phong_PS(float3 normalW : TEXCOORD0, 
                float3 posW : TEXCOORD1,
                float3 tex0 : TEXCOORD2,
                float3 tex1 : TEXCOORD3,
                float3 tex2 : TEXCOORD4,
                float3 tex3 : TEXCOORD5,
                float3 tex4 : TEXCOORD6,
                float3 tex5 : TEXCOORD7
                ) : COLOR
{
	normalW = normalize(normalW);
	normalW.x = 0;
	normalW.z = 0;
	
	// -------------------------------------------------------------------------
	// Compute Specular Lighting
	// -------------------------------------------------------------------------
	float3 vecEye = normalize(g_EyePosW - posW);
	float3 vecReflect = reflect(-g_LightVecW, normalW);
	float3 fSpecularIntensity = pow(max(dot(vecReflect, vecEye), 0.0f), g_SpecularPower);
	float3 fSpecular = fSpecularIntensity * (g_SpecularMtrl * g_SpecularLight).rgb;
	
	// -------------------------------------------------------------------------
	// Compute Diffuse Lighting
	// -------------------------------------------------------------------------
	float3 fDiffuseIntensity = max(dot(g_LightVecW, normalW), 0.0f);
	float3 fDiffuse = fDiffuseIntensity * (g_DiffuseMtrl * g_DiffuseLight).rgb;
	
	// -------------------------------------------------------------------------
	// Computing Ambient Lighting
	// -------------------------------------------------------------------------
	float3 fAmbient = g_AmbientMtrl * g_AmbientLight;
	
	// -------------------------------------------------------------------------
	// Sum Lighting attributes together.
	// -------------------------------------------------------------------------
	float3 color = fDiffuse*posW.y*2 + fAmbient + (posW.y*0.01);
	
	// -------------------------------------------------------------------------
	// Add Texture to pixels.
	// -------------------------------------------------------------------------
	float3 waterPixel0 = tex2D(WaterTex0, tex0).rgb;
	float3 waterPixel1 = tex2D(WaterTex1, tex1).rgb;
	float3 waterPixel2 = tex2D(WaterTex2, tex2).rgb;
	float3 waterPixel3 = tex2D(WaterTex3, tex3).rgb;
	float3 waterPixel4 = tex2D(WaterTex4, tex4).rgb;
	float3 waterPixel5 = tex2D(WaterTex5, tex5).rgb;
   
   float3 finalShadedColor = 
		waterPixel0 +
		waterPixel1 + 
		waterPixel2 + 
		waterPixel3 + 
		waterPixel4 +
		waterPixel5 +
		color;
  
   return float4(waterPixel0 + waterPixel1 + waterPixel2 + color, g_DiffuseMtrl.a);
}

technique FastFourierWavesTechnique
{
	pass P0
	{
		// ----------------------------------------------------------------------
		// Specify the vertex and pixel shader associated with this pass.
		// ----------------------------------------------------------------------
		vertexShader = compile vs_2_0 Phong_VS();
		pixelShader  = compile ps_2_0 Phong_PS();

		// ----------------------------------------------------------------------
		// Specify the render/device states associated with this pass.
		// ----------------------------------------------------------------------
		AlphaBlendEnable = true;
	   SrcBlend = SrcAlpha;
	   DestBlend = InvSrcAlpha;
	}
}
