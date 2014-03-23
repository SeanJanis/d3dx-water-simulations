// -------------------------------------------------------------------------
// Sean Janis
// spjanis@gmail.com
// Water Simulations
//
// CWaterSurface
//       Defines a grid container class for simulating a Water Surface using
//       Inverse Fast Fourier Transforms, Gerstner Waves and Texturing.
// -------------------------------------------------------------------------
#pragma once

#include <string>
#include <deque>
#include <d3d9.h>
#include <d3dx9.h>
//#include <dxerr9.h>
#include "DXUT.h"
#include "DXUTcamera.h"

#include "Vertex.h"
#include "AnimationObject.h"
#include "ComplexNumber.h"
#include "KWaveVector.h"
#include "GerstnerWave.h"

using namespace std;

#define WATER_SURFACE_WIDTH           64
#define WATER_SURFACE_HEIGHT          64
#define WATER_SURFACE_DX              10.05
#define WATER_SURFACE_DZ              10.05 

class CWaterSurface : public CAnimationObject
{
public:
   CWaterSurface(
      IDirect3DDevice9* pDirect3D9Device, 
      int nNumRows, 
      int nNumCols,
      float fXSpacing,
      float fZSpacing);

   virtual ~CWaterSurface(void);
   virtual bool Init();

   // -------------------------------------------------------------------------
   // Animation Object Interface Methods
   // -------------------------------------------------------------------------
   virtual void Update(float fCurrentTime, bool blMoveObject = false);
   virtual void Draw(
      D3DXMATRIX& projectionMatrix,
      D3DXMATRIX& viewMatrix);

   void SetCamera(CFirstPersonCamera& camera);
   
   // -------------------------------------------------------------------------
   // Basic Mutators / Accessors
   // -------------------------------------------------------------------------
   void SetXWindSpeed(float fValue);
   float GetXWindSpeed();

   void SetZWindSpeed(float fValue);
   float GetZWindSpeed();

   void SetPhillipsConstant(float fValue);
   float GetPhillipsConstant();

   void SetGravityConstant(float fValue);
   float GetGravityConstant();

   void SetEnableGerstnerWaves(bool blValue);
   float GetEnableGerstnerWaves();

protected:
   //--------------------------------------------------------------------------
   // Initialization Methods
   //--------------------------------------------------------------------------
   virtual bool BuildGrid();
   virtual bool BuildGerstnerWaves();
   virtual bool LoadShadingFX();
   virtual bool LoadTextureFiles();
   virtual bool CreateLighting();

   //--------------------------------------------------------------------------
   // Basically, create a wave field having the same spectrum as the ocean and
   // then transform it to the spatial domain by an inverse Fast Fourier
   // Transform. The Transformation will occur within the vertex shader.
   //--------------------------------------------------------------------------
   virtual bool LoadInitialFourierHeightMap();
   virtual void UpdateFourierHeightMap(float fCurrentTime);

   // -------------------------------------------------------------------------
   // Fast Fourier Helper Methods
   // -------------------------------------------------------------------------
   int FFT2D();
   int FFT(int dir,int m,float *x,float *y);
   int Powerof2(int n,int *m,int *twopm);
   void GetGaussian(float& fGaussian1, float& fGaussian2);
   float GetPhillipsSpectrum(KWaveVector vecKBounded);

protected:
   // -------------------------------------------------------------------------
   // DirectX Data
   // -------------------------------------------------------------------------
   IDirect3DVertexBuffer9* m_pVertexBuffer;
	IDirect3DIndexBuffer9* m_pIndexBuffer;
   CFirstPersonCamera m_Camera;

   // -------------------------------------------------------------------------
   // Grid Data
   // -------------------------------------------------------------------------
   int m_nNumRows;
   int m_nNumCols;
   int m_nNumGridVertices;
   int m_nNumGridTriangles;
   
   float m_fXSpacing;
   float m_fZSpacing;

   deque<D3DXVECTOR3> m_Vertices;
	deque<DWORD> m_Indices;

   // -------------------------------------------------------------------------
   // Water Parameters
   // -------------------------------------------------------------------------
   float m_fXWindSpeed;
   float m_fZWindSpeed;
   float m_fPhillipsConstant;
   float m_fGravityConstant;

protected:
   // -------------------------------------------------------------------------
   // Fourier Height Map Data (Computed at each iteration).
   // -------------------------------------------------------------------------   
   ComplexNumber m_InitialHeightMap[WATER_SURFACE_WIDTH][WATER_SURFACE_HEIGHT];
   ComplexNumber m_FourierHeightMap[WATER_SURFACE_WIDTH][WATER_SURFACE_HEIGHT];
   float m_VertexHeightMap[WATER_SURFACE_WIDTH][WATER_SURFACE_HEIGHT];

   KWaveVector m_KWaveVectors[WATER_SURFACE_WIDTH][WATER_SURFACE_HEIGHT];
   float m_AngularFreqs[WATER_SURFACE_WIDTH][WATER_SURFACE_HEIGHT];

protected:
   // -------------------------------------------------------------------------
   // Optional Gerstner Waves
   // -------------------------------------------------------------------------   
   GerstnerWave m_GerstnerWaves[MAX_NUM_GERSTNER_WAVES];
   D3DXHANDLE m_hGerstnerWaves;
   D3DXHANDLE m_hParam_EnableGerstnerWaves;
   bool m_blEnableGerstnerWaves;

protected:
   //--------------------------------------------------------------------------
   // Vertex & Pixel Shading Effects
   //--------------------------------------------------------------------------
   ID3DXEffect* m_pFX;
	D3DXHANDLE m_hParam_WVP;
   D3DXHANDLE m_hParam_WorldInverseTranspose;
   D3DXHANDLE m_hParam_FastFourierWavesTechnique;
   D3DXHANDLE m_hParam_Time;
   
   //--------------------------------------------------------------------------
   // Lighting
   //--------------------------------------------------------------------------
   D3DXMATRIX m_World;
   D3DXHANDLE m_hParam_LightVecW;
	D3DXHANDLE m_hParam_DiffuseMtrl;
	D3DXHANDLE m_hParam_DiffuseLight;
	D3DXHANDLE m_hParam_AmbientMtrl;
	D3DXHANDLE m_hParam_AmbientLight;
	D3DXHANDLE m_hParam_SpecularMtrl;
	D3DXHANDLE m_hParam_SpecularLight;
	D3DXHANDLE m_hParam_SpecularPower;
	D3DXHANDLE m_hParam_EyePos; 
	D3DXHANDLE m_hParam_World;

	D3DXVECTOR3 m_vecLightW;
	D3DXCOLOR m_clrAmbientMtrl;
	D3DXCOLOR m_clrAmbientLight;
	D3DXCOLOR m_clrDiffuseMtrl;
	D3DXCOLOR m_clrDiffuseLight;
	D3DXCOLOR m_clrSpecularMtrl;
	D3DXCOLOR m_clrSpecularLight;
	float m_fSpecularPower;

protected:
   //--------------------------------------------------------------------------
   // Animated Water Textures
   //--------------------------------------------------------------------------
   IDirect3DTexture9* m_pTexWater0;
   D3DXHANDLE m_hParam_TexWater0;
   D3DXVECTOR2 m_vecTexWaterOffset0;
   D3DXHANDLE m_hParam_TexWaterOffset0;
   
   IDirect3DTexture9* m_pTexWater1;
   D3DXHANDLE m_hParam_TexWater1;
   D3DXVECTOR2 m_vecTexWaterOffset1;
   D3DXHANDLE m_hParam_TexWaterOffset1;

   IDirect3DTexture9* m_pTexWater2;
   D3DXHANDLE m_hParam_TexWater2;
   D3DXVECTOR2 m_vecTexWaterOffset2;
   D3DXHANDLE m_hParam_TexWaterOffset2;

   IDirect3DTexture9* m_pTexWater3;
   D3DXHANDLE m_hParam_TexWater3;
   D3DXVECTOR2 m_vecTexWaterOffset3;
   D3DXHANDLE m_hParam_TexWaterOffset3;

   IDirect3DTexture9* m_pTexWater4;
   D3DXHANDLE m_hParam_TexWater4;
   D3DXVECTOR2 m_vecTexWaterOffset4;
   D3DXHANDLE m_hParam_TexWaterOffset4;

   IDirect3DTexture9* m_pTexWater5;
   D3DXHANDLE m_hParam_TexWater5;
   D3DXVECTOR2 m_vecTexWaterOffset5;
   D3DXHANDLE m_hParam_TexWaterOffset5;
};
