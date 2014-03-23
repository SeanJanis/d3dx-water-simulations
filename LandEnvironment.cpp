#include "DXUT.h"
#include "LandEnvironment.h"

CLandEnvironment::CLandEnvironment(IDirect3DDevice9* pDirect3D9Device) :
CAnimationObject(pDirect3D9Device)
{
   m_fRadius = 10000.0f;
   m_pFX = NULL;
	m_pSphere = NULL;
	m_pEnvironmentMap = NULL;
}

CLandEnvironment::~CLandEnvironment()
{
	if (m_pFX != NULL)
   {
      m_pFX->Release();
      m_pFX = NULL;
   }

   if (m_pSphere != NULL)
   {
      m_pSphere->Release();
      m_pSphere = NULL;
   }

   if (m_pEnvironmentMap != NULL)
   {
      m_pEnvironmentMap->Release();
      m_pEnvironmentMap = NULL;
   }
}

bool CLandEnvironment::Init()
{
   if (!LoadShadingFX())
   {
      return false;
   }

   return true;
}

void CLandEnvironment::SetCamera(CFirstPersonCamera& camera)
{
   m_Camera = camera;
}

bool CLandEnvironment::LoadShadingFX()
{
	D3DXCreateSphere(m_pDirect3D9Device, m_fRadius, 30, 30, &m_pSphere, 0);
	D3DXCreateCubeTextureFromFile(m_pDirect3D9Device, L"LandEnvironment.dds", &m_pEnvironmentMap);

	// -------------------------------------------------------------------------
   // Sets the current working directory so the FX file will
   // always be found.
   // -------------------------------------------------------------------------
   char csModuleFilename[2048];
   ::GetModuleFileNameA(NULL, csModuleFilename, 2048); 
   string strModuleDirectory = csModuleFilename;

   strModuleDirectory = strModuleDirectory.substr(0, strModuleDirectory.find_last_of("\\"));
   ::SetCurrentDirectoryA(strModuleDirectory.c_str());

	if (S_OK != D3DXCreateEffectFromFileA(
      m_pDirect3D9Device, 
      (LPCSTR)"LandEnvironment.fx", 
		0, 
      0, 
      D3DXSHADER_DEBUG, 
      0, 
      &m_pFX, 
      NULL))
   {
      return false;
   } 

	m_hParam_EnvironmentTechnique = m_pFX->GetTechniqueByName("LandEnvironmentTechnique");
	m_hParam_WVP = m_pFX->GetParameterByName(0, "gWVP");
	m_hParam_EnvironmentMap = m_pFX->GetParameterByName(0, "gEnvMap");  

	m_pFX->SetTechnique(m_hParam_EnvironmentTechnique);
	m_pFX->SetTexture(m_hParam_EnvironmentMap, m_pEnvironmentMap);

   return true;
}

IDirect3DCubeTexture9* CLandEnvironment::GetEnvironmentMap()
{
	return m_pEnvironmentMap;
}

float CLandEnvironment::GetSkyRadius()
{
	return m_fRadius;
}

void CLandEnvironment::Update(float fCurrentTime, 
                              bool blMoveObject)
{

}

void CLandEnvironment::Draw(D3DXMATRIX& projectionMatrix, 
                            D3DXMATRIX& viewMatrix)
{
	D3DXMATRIX W;
	D3DXVECTOR3 p = *(m_Camera.GetEyePt());
	D3DXMatrixTranslation(&W, p.x, p.y, p.z);
	m_pFX->SetMatrix(m_hParam_WVP, &(viewMatrix * projectionMatrix));
	
	UINT numPasses = 0;
	m_pFX->Begin(&numPasses, 0);
	m_pFX->BeginPass(0);
	m_pSphere->DrawSubset(0);
	m_pFX->EndPass();
	m_pFX->End();
}
