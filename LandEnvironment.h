// -------------------------------------------------------------------------
// Sean Janis
// spjanis@gmail.com
// Water Simulations
//
// CLandEnvironment
//       Creates a surrounding environment for our water simulation demo 
//       using Environment Mapping.
// -------------------------------------------------------------------------
#pragma once

#include <string>
#include <deque>
#include <d3d9.h>
#include <d3dx9.h>
//#include <dxerr9.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "AnimationObject.h"

using namespace std;

class CLandEnvironment : 
   public CAnimationObject
{
public:
	CLandEnvironment(IDirect3DDevice9* pDirect3D9Device);
	~CLandEnvironment();

   virtual bool Init();

   // -------------------------------------------------------------------------
   // Animation Object Interface Methods
   // -------------------------------------------------------------------------
   virtual void Update(float fCurrentTime, bool blMoveObject = false);
   virtual void Draw(
      D3DXMATRIX& projectionMatrix,
      D3DXMATRIX& viewMatrix);

   // -------------------------------------------------------------------------
   // General Accessors / Mutators
   // -------------------------------------------------------------------------
   IDirect3DCubeTexture9* GetEnvironmentMap();
	float GetSkyRadius();
   void SetCamera(CFirstPersonCamera& camera);

protected:
   //--------------------------------------------------------------------------
   // Initialization Methods
   //--------------------------------------------------------------------------
   virtual bool LoadShadingFX();

private:
   // -------------------------------------------------------------------------
   // DirectX Data
   // -------------------------------------------------------------------------
   ID3DXEffect* m_pFX;
	ID3DXMesh* m_pSphere;
	IDirect3DCubeTexture9* m_pEnvironmentMap;
   CFirstPersonCamera m_Camera;
   float m_fRadius;
	
   //--------------------------------------------------------------------------
   // Lighting and Environment Mapping
   //--------------------------------------------------------------------------
	D3DXHANDLE m_hParam_EnvironmentTechnique;
	D3DXHANDLE m_hParam_EnvironmentMap;
	D3DXHANDLE m_hParam_WVP;
};
