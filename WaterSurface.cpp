#include "DXUT.h"
#include "WaterSurface.h"

#define PI                                      3.141593

CWaterSurface::CWaterSurface(IDirect3DDevice9* pDirect3D9Device,
                         int nNumRows, 
                         int nNumCols,
                         float fXSpacing,
                         float fZSpacing) : CAnimationObject(pDirect3D9Device)
{
   m_pDirect3D9Device = pDirect3D9Device;
   m_nNumRows = nNumRows;
   m_nNumCols = nNumCols;

   m_nNumGridVertices = m_nNumRows * m_nNumCols;
	m_nNumGridTriangles = m_nNumRows * m_nNumCols * 2;

   m_fXSpacing = fXSpacing;
   m_fZSpacing = fZSpacing;

   m_pVertexBuffer = NULL;
   m_pIndexBuffer = NULL;

   m_vecTexWaterOffset0 = D3DXVECTOR2(0.0f, 0.0f);
	m_vecTexWaterOffset1 = D3DXVECTOR2(0.0f, 0.0f);
   m_vecTexWaterOffset2 = D3DXVECTOR2(0.0f, 0.0f);
   m_vecTexWaterOffset3 = D3DXVECTOR2(0.0f, 0.0f);
   m_vecTexWaterOffset4 = D3DXVECTOR2(0.0f, 0.0f);
   m_vecTexWaterOffset5 = D3DXVECTOR2(0.0f, 0.0f);

   m_fXWindSpeed = 10.0f;
   m_fZWindSpeed = 10.0f;
   m_fPhillipsConstant = 0.00008;
   m_fGravityConstant = 2.0f;
   m_blEnableGerstnerWaves = false;

   m_pFX = NULL;   
   InitVertexDeclarations(pDirect3D9Device); 
}

CWaterSurface::~CWaterSurface(void)
{
   // -------------------------------------------------------------------------
   // Free Vertices
   // -------------------------------------------------------------------------
   m_pVertexBuffer->Release();
   m_pVertexBuffer = NULL;

   m_pIndexBuffer->Release();
   m_pIndexBuffer = NULL;

   // -------------------------------------------------------------------------
   // Free Textures
   // -------------------------------------------------------------------------
   if (m_pTexWater0 != NULL)
   {
      m_pTexWater0->Release();
      m_pTexWater0 = NULL;
   }

   if (m_pTexWater1 != NULL)
   {
      m_pTexWater1->Release();
      m_pTexWater1 = NULL;
   }

   if (m_pTexWater2 != NULL)
   {
      m_pTexWater2->Release();
      m_pTexWater2 = NULL;
   }

   if (m_pTexWater3 != NULL)
   {
      m_pTexWater3->Release();
      m_pTexWater3 = NULL;
   }

   if (m_pTexWater4 != NULL)
   {
      m_pTexWater4->Release();
      m_pTexWater4 = NULL;
   }

   if (m_pTexWater5 != NULL)
   {
      m_pTexWater5->Release();
      m_pTexWater5 = NULL;
   }

   DestroyVertexDeclarations();
}

bool CWaterSurface::Init()
{
   // -------------------------------------------------------------------------
   // Construct the Vertices.
   // -------------------------------------------------------------------------
   BuildGrid();

   // -------------------------------------------------------------------------
   // Initialize Objects
   // -------------------------------------------------------------------------
   if (!LoadShadingFX())
   {
      return false;
   }

   if (!LoadTextureFiles())
   {
      return false;
   }

   if (!BuildGerstnerWaves())
   {
      return false;
   }
   
   // -------------------------------------------------------------------------
   // Construct the Lighting & Shading Effects
   // -------------------------------------------------------------------------
   if (!CreateLighting())
   {
      return false;
   }

   // -------------------------------------------------------------------------
   // Build a Fourier Height Map which will help us statistically compute
   // height values in our vertex shader at each H(X, T) vertex position.
   // -------------------------------------------------------------------------
   if (!LoadInitialFourierHeightMap())
   {
      return false;
   }

   return true;
}

void CWaterSurface::SetXWindSpeed(float fValue)
{
   m_fXWindSpeed = fValue;
   LoadInitialFourierHeightMap();
}

float CWaterSurface::GetXWindSpeed()
{
   return m_fXWindSpeed;
}

void CWaterSurface::SetZWindSpeed(float fValue)
{
   m_fZWindSpeed = fValue;
   LoadInitialFourierHeightMap();
}

float CWaterSurface::GetZWindSpeed()
{
   return m_fZWindSpeed;
}

void CWaterSurface::SetPhillipsConstant(float fValue)
{
   m_fPhillipsConstant = fValue;
   LoadInitialFourierHeightMap();
}

float CWaterSurface::GetPhillipsConstant()
{
   return m_fPhillipsConstant;
}

void CWaterSurface::SetGravityConstant(float fValue)
{
   m_fGravityConstant = fValue;
   LoadInitialFourierHeightMap();
}

float CWaterSurface::GetGravityConstant()
{
   return m_fGravityConstant;
}

void CWaterSurface::SetEnableGerstnerWaves(bool blValue)
{
   m_blEnableGerstnerWaves = blValue;
}

float CWaterSurface::GetEnableGerstnerWaves()
{
   return m_blEnableGerstnerWaves;
}

bool CWaterSurface::BuildGrid()
{
   D3DXVECTOR3 vecGridCenter(0, 0, 0);

	float fWidth = (float)(m_nNumCols - 1) * m_fXSpacing;
	float fDepth = (float)(m_nNumRows - 1) * m_fZSpacing;

   // -------------------------------------------------------------------------
   // Initialize Height Map
   // -------------------------------------------------------------------------
   for (int x = 0; x < WATER_SURFACE_WIDTH; x++)
   {
      for (int z = 0; z < WATER_SURFACE_HEIGHT; z++)
      {
         m_VertexHeightMap[x][z] = 0.0f;
      }
   } 

	// -------------------------------------------------------------------------
   // Build the Vertices in a row-by-row, top-down fashion.
   // -------------------------------------------------------------------------
	m_Vertices.resize(m_nNumGridVertices);

   // -------------------------------------------------------------------------
	// Offsets to translate grid from quadrant 4 to center of 
	// coordinate system.
   // -------------------------------------------------------------------------
	float fXOffset = -fWidth * 0.5f; 
	float fZOffset = fDepth * 0.5f;

	int k = 0;
	for (float i = 0; i < m_nNumRows; ++i)
	{
		for (float j = 0; j < m_nNumCols; ++j)
		{
         // -------------------------------------------------------------------------
			// Negate the depth coordinate to put in quadrant four.  
			// Then offset to center about coordinate system.
         // -------------------------------------------------------------------------
			m_Vertices[k].x =  j * m_fXSpacing + fXOffset;
			m_Vertices[k].z = -i * m_fZSpacing + fZOffset;
			m_Vertices[k].y =  0.0f;

			k++; // Next Vertex
		}
	}

	// -------------------------------------------------------------------------
   // Build the Grid Triangle Indices
   // -------------------------------------------------------------------------
	m_Indices.resize(m_nNumGridTriangles * 3);
	 
	k = 0;
	for(DWORD i = 0; i < (DWORD)m_nNumRows - 1; ++i)
	{
		for (DWORD j = 0; j < (DWORD)m_nNumCols - 1; ++j)
		{
         m_Indices[k]     =   i   * m_nNumCols + j;
			m_Indices[k + 1] =   i   * m_nNumCols + j + 1;
			m_Indices[k + 2] = (i+1) * m_nNumCols + j;
					
			m_Indices[k + 3] = (i+1) * m_nNumCols + j;
			m_Indices[k + 4] =   i   * m_nNumCols + j + 1;
			m_Indices[k + 5] = (i+1) * m_nNumCols + j + 1;

			// Next Quad
			k += 6;
		}
	}
	 
   // -------------------------------------------------------------------------
   // Create the Grid Vertices on the Direct3D Device.
   // -------------------------------------------------------------------------
	if (S_OK != m_pDirect3D9Device->CreateVertexBuffer(
      m_nNumGridVertices * sizeof(CVertex), 
		D3DUSAGE_WRITEONLY, 
      0, 
      D3DPOOL_MANAGED, 
      &m_pVertexBuffer, 
      0))
   {
      return false;
   }

   // -------------------------------------------------------------------------
   // Create the Grid Indices on the Direct3D Device.
   // -------------------------------------------------------------------------
	if (S_OK != m_pDirect3D9Device->CreateIndexBuffer(
      m_nNumGridTriangles * 3 * sizeof(WORD), 
		D3DUSAGE_WRITEONLY, 
      D3DFMT_INDEX16, 
      D3DPOOL_MANAGED, 
      &m_pIndexBuffer, 
      0))
   {
      return false;
   }

   // -------------------------------------------------------------------------
   // Write the Vertex Buffer to Memory.
   // -------------------------------------------------------------------------
   CVertex* pVertex = 0;
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertex, 0);

   int nXIndex = 0;
   int nZIndex = 0;
   float fTexScale = 0.02f;

	for (DWORD i = 0; i < m_nNumGridVertices; i++)
   {
      nXIndex = i / m_nNumRows;
      nZIndex = i % m_nNumCols;
 
      pVertex[i] = CVertex(
         D3DXVECTOR3(m_Vertices[i].x, m_Vertices[i].y, m_Vertices[i].z), 
         D3DXVECTOR3(0, 1, 0), // Default Normal Vector (before animation).
         D3DXVECTOR2((float)nZIndex, (float)nXIndex) * fTexScale
         );
   }

	m_pVertexBuffer->Unlock();

   // -------------------------------------------------------------------------
   // Write the Index Buffer to Memory.
   // -------------------------------------------------------------------------
   WORD* pIndices = 0;
	m_pIndexBuffer->Lock(0, 0, (void**)&pIndices, 0);

	for (DWORD i = 0; i < m_nNumGridTriangles * 3; i++)
   {
		pIndices[i] = (WORD)m_Indices[i];
   }

	m_pIndexBuffer->Unlock();

   return true;
}

bool CWaterSurface::BuildGerstnerWaves()
{
   // -------------------------------------------------------------------------
   // Create some Random Gerstner Waves that will be added to the Fourier
   // Wave Heights. This will produce a nice emerging Wave Effect.
   // -------------------------------------------------------------------------
   GerstnerWave randomWave;

   randomWave.vecWaveDirection[0] = 0.9f;
   randomWave.vecWaveDirection[1] = 0.0f;
   randomWave.vecWaveDirection[2] = 0.2f;
   randomWave.fAmplitude = 0.905f;
   randomWave.fAngularFrequency = 10.28f;
   randomWave.fWaveLength = 100.141593f;
   randomWave.fPhaseShift = 5.0f;
   m_GerstnerWaves[0] = randomWave;   

   m_pFX->SetValue(m_hGerstnerWaves, (LPCVOID)m_GerstnerWaves, sizeof(GerstnerWave) * MAX_NUM_GERSTNER_WAVES);
   return true;
}

bool CWaterSurface::LoadShadingFX()
{
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
      (LPCSTR)"WaterSurface.fx", 
		0, 
      0, 
      D3DXSHADER_DEBUG, 
      0, 
      &m_pFX, 
      NULL))
   {
      return false;
   } 

   // -------------------------------------------------------------------------
   // Obtain Shading Handles
   // -------------------------------------------------------------------------
   m_hParam_FastFourierWavesTechnique = m_pFX->GetTechniqueByName("FastFourierWavesTechnique");   
	m_hParam_WVP = m_pFX->GetParameterByName(0, "g_WVP");
   m_hParam_WorldInverseTranspose = m_pFX->GetParameterByName(0, "g_WorldInverseTranspose");
   m_hParam_Time = m_pFX->GetParameterByName(0, "g_Time");
   m_hParam_EnableGerstnerWaves = m_pFX->GetParameterByName(0, "g_EnableGerstnerWaves");
   m_hGerstnerWaves = m_pFX->GetParameterByName(0, "g_GerstnerWaves");
   
   // -------------------------------------------------------------------------
   // Lighting Handles
   // -------------------------------------------------------------------------
   m_hParam_LightVecW = m_pFX->GetParameterByName(0, "g_LightVecW");
	m_hParam_DiffuseMtrl = m_pFX->GetParameterByName(0, "g_DiffuseMtrl");
	m_hParam_DiffuseLight = m_pFX->GetParameterByName(0, "g_DiffuseLight");
	m_hParam_AmbientMtrl = m_pFX->GetParameterByName(0, "g_AmbientMtrl");
	m_hParam_AmbientLight = m_pFX->GetParameterByName(0, "g_AmbientLight");
	m_hParam_SpecularMtrl = m_pFX->GetParameterByName(0, "g_SpecularMtrl");
	m_hParam_SpecularLight = m_pFX->GetParameterByName(0, "g_SpecularLight");
	m_hParam_SpecularPower = m_pFX->GetParameterByName(0, "g_SpecularPower");
	m_hParam_EyePos = m_pFX->GetParameterByName(0, "g_EyePosW");
	m_hParam_World = m_pFX->GetParameterByName(0, "g_World"); 

   // -------------------------------------------------------------------------
   // Scrolling Water Animation
   // -------------------------------------------------------------------------
   m_hParam_TexWater0 = m_pFX->GetParameterByName(0, "g_TexWater0");
   m_hParam_TexWaterOffset0 = m_pFX->GetParameterByName(0, "g_TexWaterOffset0");

   m_hParam_TexWater1 = m_pFX->GetParameterByName(0, "g_TexWater1");
   m_hParam_TexWaterOffset1 = m_pFX->GetParameterByName(0, "g_TexWaterOffset1");

   m_hParam_TexWater2 = m_pFX->GetParameterByName(0, "g_TexWater2");
   m_hParam_TexWaterOffset2 = m_pFX->GetParameterByName(0, "g_TexWaterOffset2");

   m_hParam_TexWater3 = m_pFX->GetParameterByName(0, "g_TexWater3");
   m_hParam_TexWaterOffset3 = m_pFX->GetParameterByName(0, "g_TexWaterOffset3");

   m_hParam_TexWater4 = m_pFX->GetParameterByName(0, "g_TexWater4");
   m_hParam_TexWaterOffset4 = m_pFX->GetParameterByName(0, "g_TexWaterOffset4");

   m_hParam_TexWater5 = m_pFX->GetParameterByName(0, "g_TexWater5");
   m_hParam_TexWaterOffset5 = m_pFX->GetParameterByName(0, "g_TexWaterOffset5");

   return true;
}

bool CWaterSurface::LoadTextureFiles()   
{
   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture0.dds", &m_pTexWater0) != S_OK)
   {
      return false;
   }

   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture1.dds", &m_pTexWater1) != S_OK)
   {
      return false;
   }

   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture2.dds", &m_pTexWater2) != S_OK)
   {
      return false;
   }

   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture3.dds", &m_pTexWater3) != S_OK)
   {
      return false;
   }

   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture4.dds", &m_pTexWater4) != S_OK)
   {
      return false;
   }

   if (D3DXCreateTextureFromFileA(m_pDirect3D9Device, "WaterTexture5.dds", &m_pTexWater5) != S_OK)
   {
      return false;
   }

   // -------------------------------------------------------------------------
   // Set Textures for Scrolling Animation.
   // -------------------------------------------------------------------------
   m_pFX->SetTexture(m_hParam_TexWater0, m_pTexWater0);
   m_pFX->SetTexture(m_hParam_TexWater1, m_pTexWater1);
   m_pFX->SetTexture(m_hParam_TexWater2, m_pTexWater2);
   m_pFX->SetTexture(m_hParam_TexWater3, m_pTexWater3);
   m_pFX->SetTexture(m_hParam_TexWater4, m_pTexWater4);
   m_pFX->SetTexture(m_hParam_TexWater5, m_pTexWater5);

   return true; 
}

bool CWaterSurface::CreateLighting()
{
   m_vecLightW = D3DXVECTOR3(0.0, 0.0f, 1.0f); 
	m_clrDiffuseMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_clrDiffuseLight = D3DXCOLOR(0.16f, 0.13f, 0.3f, 1.0f); 
	m_clrAmbientMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_clrAmbientLight = D3DXCOLOR(0.16f, 0.13f, 0.3f, 1.00f);
	m_clrSpecularMtrl = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	m_clrSpecularLight = D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.5f); 
	m_fSpecularPower = 2.0f;       

	m_pFX->SetValue(m_hParam_LightVecW, &m_vecLightW, sizeof(D3DXVECTOR3));
	m_pFX->SetValue(m_hParam_DiffuseMtrl, &m_clrDiffuseMtrl, sizeof(D3DXCOLOR));
	m_pFX->SetValue(m_hParam_DiffuseLight, &m_clrDiffuseLight, sizeof(D3DXCOLOR));
	m_pFX->SetValue(m_hParam_AmbientMtrl, &m_clrAmbientMtrl, sizeof(D3DXCOLOR));
	m_pFX->SetValue(m_hParam_AmbientLight, &m_clrAmbientLight, sizeof(D3DXCOLOR));
	m_pFX->SetValue(m_hParam_SpecularLight, &m_clrSpecularLight, sizeof(D3DXCOLOR));
	m_pFX->SetValue(m_hParam_SpecularMtrl, &m_clrSpecularMtrl, sizeof(D3DXCOLOR));
	m_pFX->SetFloat(m_hParam_SpecularPower, m_fSpecularPower);  

	D3DXMatrixIdentity(&m_World);  
   return true;
}

void CWaterSurface::SetCamera(CFirstPersonCamera& camera)
{
   m_Camera = camera;

   D3DXVECTOR3 vecEye(0.0f, 18.5f, -125.0f);
   D3DXVECTOR3 vecAt(0.0f, 0.2f, 0.0f);
   D3DXVECTOR3 vecUp(0.0f, 1.0f, 0.0f);
   m_Camera.SetViewParams(&vecEye, &vecAt);
}

void CWaterSurface::Update(float fCurrentTime, bool blMoveObject)
{
   D3DXVECTOR3 vecEyePos = *m_Camera.GetEyePt();
   m_pFX->SetValue(m_hParam_EyePos, &vecEyePos, sizeof(D3DXVECTOR3));
   m_pFX->SetFloat(m_hParam_Time, fCurrentTime);    
   m_pFX->SetBool(m_hParam_EnableGerstnerWaves, (BOOL)m_blEnableGerstnerWaves);    

   // -------------------------------------------------------------------------
   // Update the Texture Offsets that will create a scrolling Animation in
   // the Pixel Shader.
   // -------------------------------------------------------------------------
   m_vecTexWaterOffset0 += D3DXVECTOR2(0.11f, 0.05f) * (0.0002 * fCurrentTime);  
	m_vecTexWaterOffset1 += D3DXVECTOR2(0.05f, -0.1f) * (0.0004 * fCurrentTime);
   m_vecTexWaterOffset2 += D3DXVECTOR2(0.25f, 0.15f) * (0.0001 * fCurrentTime);
   m_vecTexWaterOffset3 += D3DXVECTOR2(0.35f, 0.2f) * (0.0002 * fCurrentTime);
   m_vecTexWaterOffset4 += D3DXVECTOR2(0.05f, 0.8f) * (0.0005 * fCurrentTime);
   m_vecTexWaterOffset5 += D3DXVECTOR2(0.15f, 0.5f) * (0.0001 * fCurrentTime); 

   // -------------------------------------------------------------------------
   // Update Shader Offsets
   // -------------------------------------------------------------------------
	m_pFX->SetValue(m_hParam_TexWaterOffset0, &m_vecTexWaterOffset0, sizeof(D3DXVECTOR2));
	m_pFX->SetValue(m_hParam_TexWaterOffset1, &m_vecTexWaterOffset1, sizeof(D3DXVECTOR2));
	m_pFX->SetValue(m_hParam_TexWaterOffset2, &m_vecTexWaterOffset2, sizeof(D3DXVECTOR2));
	m_pFX->SetValue(m_hParam_TexWaterOffset3, &m_vecTexWaterOffset3, sizeof(D3DXVECTOR2));
	m_pFX->SetValue(m_hParam_TexWaterOffset4, &m_vecTexWaterOffset4, sizeof(D3DXVECTOR2));
	m_pFX->SetValue(m_hParam_TexWaterOffset5, &m_vecTexWaterOffset5, sizeof(D3DXVECTOR2));  

   // -------------------------------------------------------------------------
	// Textures repeat every 1.0 unit, so reset back down to zero
	// so the coordinates do not grow.
   // -------------------------------------------------------------------------
	if (m_vecTexWaterOffset0.x >= 1.0f || m_vecTexWaterOffset0.x <= -1.0f) 
   {
      m_vecTexWaterOffset0.x = 0.0f;
   }

   if (m_vecTexWaterOffset0.y >= 1.0f || m_vecTexWaterOffset0.y <= -1.0f)
   {
      m_vecTexWaterOffset0.y = 0.0f;
   }

	if (m_vecTexWaterOffset1.x >= 1.0f || m_vecTexWaterOffset1.x <= -1.0f)
   {
      m_vecTexWaterOffset1.x = 0.0f;
   }

	if (m_vecTexWaterOffset1.y >= 1.0f || m_vecTexWaterOffset1.y <= -1.0f)
   {
      m_vecTexWaterOffset1.y = 0.0f;
   }

   if (m_vecTexWaterOffset2.x >= 1.0f || m_vecTexWaterOffset2.x <= -1.0f)
   {
      m_vecTexWaterOffset2.x = 0.0f;
   }

	if (m_vecTexWaterOffset2.y >= 1.0f || m_vecTexWaterOffset2.y <= -1.0f)
   {
      m_vecTexWaterOffset2.y = 0.0f;
   }

   if (m_vecTexWaterOffset3.x >= 1.0f || m_vecTexWaterOffset3.x <= -1.0f)
   {
      m_vecTexWaterOffset3.x = 0.0f;
   }

	if (m_vecTexWaterOffset3.y >= 1.0f || m_vecTexWaterOffset3.y <= -1.0f)
   {
      m_vecTexWaterOffset3.y = 0.0f;
   }

   if (m_vecTexWaterOffset4.x >= 1.0f || m_vecTexWaterOffset4.x <= -1.0f)
   {
      m_vecTexWaterOffset4.x = 0.0f;
   }

	if (m_vecTexWaterOffset4.y >= 1.0f || m_vecTexWaterOffset4.y <= -1.0f)
   {
      m_vecTexWaterOffset4.y = 0.0f;
   }

   if (m_vecTexWaterOffset5.x >= 1.0f || m_vecTexWaterOffset5.x <= -1.0f)
   {
      m_vecTexWaterOffset5.x = 0.0f;
   }

	if (m_vecTexWaterOffset5.y >= 1.0f || m_vecTexWaterOffset5.y <= -1.0f)
   {
      m_vecTexWaterOffset5.y = 0.0f;
   }

   // -------------------------------------------------------------------------
   // Perform the Inverse Fast Fourier Transform to go from the Frequency
   // domain to the Spatial Domain. This will give us our Wave Heights.
   // -------------------------------------------------------------------------
   UpdateFourierHeightMap(fCurrentTime); 

   // -------------------------------------------------------------------------
   // Write the updated Vertex Buffer to Memory.
   // -------------------------------------------------------------------------
   float fYHeightValue = 0.0f;
   float fTexScale = 0.20f;
   int nXIndex = 0;
   int nZIndex = 0;

   CVertex* pVertex = 0;
	m_pVertexBuffer->Lock(0, 0, (void**)&pVertex, 0);

	for (DWORD i = 0; i < m_nNumGridVertices; i++)
   {
      nXIndex = i / WATER_SURFACE_HEIGHT;
      nZIndex = i % WATER_SURFACE_WIDTH;

      fYHeightValue = m_VertexHeightMap[nXIndex][nZIndex];

      pVertex[i] = CVertex(
         D3DXVECTOR3(m_Vertices[i].x, fYHeightValue, m_Vertices[i].z), 
         D3DXVECTOR3(0, 1, 0), // Default Normal Vector (before animation).
         D3DXVECTOR2((float)nZIndex, (float)nXIndex) * fTexScale
         );
   }

	m_pVertexBuffer->Unlock();
}

void CWaterSurface::Draw(D3DXMATRIX& projectionMatrix,
                         D3DXMATRIX& viewMatrix)
{
	m_pDirect3D9Device->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(CVertex));
	m_pDirect3D9Device->SetIndices(m_pIndexBuffer);
   m_pDirect3D9Device->SetVertexDeclaration(CVertex::Decl);

   // -------------------------------------------------------------------------
   // Draw the animation objects while using the FX Shader file.
   // -------------------------------------------------------------------------
	m_pFX->SetTechnique(m_hParam_FastFourierWavesTechnique);

   // -------------------------------------------------------------------------
   // Set the Lighting Effects.
   // -------------------------------------------------------------------------
   D3DXMATRIX worldInverseTranspose;
	D3DXMatrixInverse(&worldInverseTranspose, 0, &m_World);
	D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
   m_pFX->SetMatrix(m_hParam_WorldInverseTranspose, &worldInverseTranspose);
	m_pFX->SetMatrix(m_hParam_World, &m_World); 

   // -------------------------------------------------------------------------
   // Transform and Shade each individual vertex.
   // -------------------------------------------------------------------------
	UINT numPasses = 0;
	m_pFX->Begin(&numPasses, 0);

	for (UINT i = 0; i < numPasses; ++i)
	{
	   m_pFX->BeginPass(i);  

		m_pFX->SetMatrix(m_hParam_WVP, &(viewMatrix * projectionMatrix));
		m_pFX->CommitChanges();     

      // -------------------------------------------------------------------------
      // Draw Grid
      // -------------------------------------------------------------------------
      m_pDirect3D9Device->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST, 
            0, 
            0, 
            m_nNumGridVertices, 
            0, 
            m_nNumGridTriangles
            );

		m_pFX->EndPass();   
	}

	m_pFX->End(); 
}

bool CWaterSurface::LoadInitialFourierHeightMap()
{
   KWaveVector vecKWaveVector;

   float fGaussian1 = 0.0f;
   float fGaussian2 = 0.0f;

   float fInverseRoot = (float)1 / (float)sqrt((float)2);
   float fPhillipsSpectrum = 0.0f;

   int nHalfGridWidth = WATER_SURFACE_WIDTH / 2;
   int nHalfGridHeight = WATER_SURFACE_HEIGHT / 2; 

   // -------------------------------------------------------------------------
   // Build a Fourier Height Map which will help us statistically compute
   // height values in our vertex shader at each H(X, T) vertex position.
   // -------------------------------------------------------------------------
   for (int x = 0; x < WATER_SURFACE_WIDTH; x++)
   {
      for (int z = 0; z < WATER_SURFACE_HEIGHT; z++)
      {
         // -------------------------------------------------------------------------
         // At each iteration stage, generate a 2D component vector that will be used
         // to calculate the Phillips Spectrum. Ensure that the vecKBounded's values
         // are within the (-N/2 <= n < N/2) or (-M/2 <= m < M/2) constraints as
         // specified by Tessendorf's paper.
         // -------------------------------------------------------------------------
         vecKWaveVector.fX = (2 * PI * x) / WATER_SURFACE_WIDTH;
         vecKWaveVector.fZ = (2 * PI * z) / WATER_SURFACE_HEIGHT; 

         // -------------------------------------------------------------------------
         // Once we compute the Fourier Height Map, we will later use the K-Wave
         // Vector and each's respective Angular Frequency to animate the wave.
         // -------------------------------------------------------------------------
         float fKVectorDistance = sqrt(vecKWaveVector.fX * vecKWaveVector.fX + vecKWaveVector.fZ * vecKWaveVector.fZ);
         m_AngularFreqs[x][z] = sqrt(fKVectorDistance * m_fGravityConstant);
         m_KWaveVectors[x][z] = vecKWaveVector;

         // -------------------------------------------------------------------------
         // Generate Gaussian Random Numbers for the Phillips Spectrum formula. These
         // Gaussian Random Numbers tend to follow the experimental data on ocean
         // waves.
         // -------------------------------------------------------------------------
         GetGaussian(fGaussian1, fGaussian2);

         // -------------------------------------------------------------------------
         // Calculate a wave spectrum based from Phillips Spectrum which is a useful
         // model for wind-driven waves. 
         // -------------------------------------------------------------------------
         fPhillipsSpectrum = GetPhillipsSpectrum(vecKWaveVector);

         if (fKVectorDistance == 0)
         {
            fPhillipsSpectrum = 0;
         }

         // -------------------------------------------------------------------------
         // Store the Results in the Fourier Height Map for later inverse transforms.
         // -------------------------------------------------------------------------
         m_InitialHeightMap[x][z].fReal = fInverseRoot * fGaussian1 * sqrt(fPhillipsSpectrum);
         m_InitialHeightMap[x][z].fImaginary = fInverseRoot * fGaussian2 * sqrt(fPhillipsSpectrum);
      }
   }

   return true;
}

void CWaterSurface::UpdateFourierHeightMap(float fCurrentTime)
{
   // -------------------------------------------------------------------------
   // Given a set of angular frequencies, perform the inverse Fast Fourier 
   // animation by iterating over the h0 Height Map.
   // -------------------------------------------------------------------------
   for (int x = 0; x < WATER_SURFACE_WIDTH; x++)
   {
      for (int z = 0; z < WATER_SURFACE_HEIGHT; z++)
      {
         float fAngularFreq = m_AngularFreqs[x][z] * fCurrentTime;
         float fAngularSine = sin(fAngularFreq);
         float fAngularCosine = cos(fAngularFreq);   
         
         // -------------------------------------------------------------------------
         // Convert from Fourier Space to the Spatial Domain by combining the effects
         // of the each sinus waveform to get a surface height.
         //
         // Trying to compute: h0(k)exp{iw(k)t} + h0(-k)exp{-iw(k)t}
         // exp{iwkt} can be represented as: cos(wkt) + sin(-wkt)
         // -------------------------------------------------------------------------
         m_FourierHeightMap[x][z].fReal = 
            m_InitialHeightMap[x][z].fReal * fAngularCosine +
            m_InitialHeightMap[x][z].fImaginary * fAngularSine +
            m_InitialHeightMap[WATER_SURFACE_WIDTH - x - 1][WATER_SURFACE_HEIGHT - z - 1].fReal * fAngularCosine -
            m_InitialHeightMap[WATER_SURFACE_WIDTH - x - 1][WATER_SURFACE_HEIGHT - z - 1].fImaginary * fAngularSine;

         m_FourierHeightMap[x][z].fImaginary = 
            m_InitialHeightMap[x][z].fImaginary * fAngularCosine +
            m_InitialHeightMap[x][z].fReal * fAngularSine - 
            m_InitialHeightMap[WATER_SURFACE_WIDTH - x - 1][WATER_SURFACE_HEIGHT - z - 1].fImaginary * fAngularCosine -
            m_InitialHeightMap[WATER_SURFACE_WIDTH - x - 1][WATER_SURFACE_HEIGHT - z - 1].fReal * fAngularSine;
      }
   }   

   // -------------------------------------------------------------------------
   // Perform an inverse Fourier Transform to get height map values.
   // -------------------------------------------------------------------------
   FFT2D();

   // -------------------------------------------------------------------------
   // Store the Height Map values in a simple float matrix.
   // -------------------------------------------------------------------------
   int nHeightMapIndex = 0;
   for (int x = 0; x < WATER_SURFACE_WIDTH; x++)
   {
      for (int z = 0; z < WATER_SURFACE_HEIGHT; z++)
      {
         m_VertexHeightMap[x][z] = m_FourierHeightMap[x][z].fReal /= 5.0f;
      }  
   }    
}

int CWaterSurface::FFT2D()
{
   int i,j;
   int m,twopm; 
   float *real,*imag; 

   /* Transform the rows */
   real = (float *)malloc(WATER_SURFACE_WIDTH * sizeof(float));
   imag = (float *)malloc(WATER_SURFACE_HEIGHT * sizeof(float));
   if (real == NULL || imag == NULL)
      return(FALSE);
   if (!Powerof2(WATER_SURFACE_WIDTH,&m,&twopm) || twopm != WATER_SURFACE_WIDTH)
      return(FALSE);
   for (j=0;j<WATER_SURFACE_HEIGHT;j++) {
      for (i=0;i<WATER_SURFACE_WIDTH;i++) {
         real[i] = m_FourierHeightMap[i][j].fReal;
         imag[i] = m_FourierHeightMap[i][j].fImaginary;
      }
      FFT(-1,m,real,imag);
      for (i=0;i<WATER_SURFACE_WIDTH;i++) {
         m_FourierHeightMap[i][j].fReal = real[i];
         m_FourierHeightMap[i][j].fImaginary = imag[i];
      }
   }
   free(real);
   free(imag);

   /* Transform the columns */
   real = (float *)malloc(WATER_SURFACE_HEIGHT * sizeof(float));
   imag = (float *)malloc(WATER_SURFACE_HEIGHT * sizeof(float));
   if (real == NULL || imag == NULL)
      return(FALSE);
   if (!Powerof2(WATER_SURFACE_HEIGHT,&m,&twopm) || twopm != WATER_SURFACE_HEIGHT)
      return(FALSE);
   for (i=0;i<WATER_SURFACE_WIDTH;i++) {
      for (j=0;j<WATER_SURFACE_HEIGHT;j++) {
         real[j] = m_FourierHeightMap[i][j].fReal;
         imag[j] = m_FourierHeightMap[i][j].fImaginary;
      }
      FFT(-1,m,real,imag);
      for (j=0;j<WATER_SURFACE_HEIGHT;j++) {
         m_FourierHeightMap[i][j].fReal = real[j];
         m_FourierHeightMap[i][j].fImaginary = imag[j];
      }
   }
   free(real);
   free(imag);

   return(TRUE);
}

int CWaterSurface::FFT(int dir,int m,float *x,float *y)
{
   long nn,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

   /* Calculate the number of points 
   nn = 1;
   for (i=0;i<m;i++)
      nn *= 2;
	*/
	nn = 1 << m;

   /* Do the bit reversal */
   i2 = nn >> 1;
   j = 0;
   for (i=0;i<nn-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<nn;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<nn;i++) {
         x[i] /= (float)nn;
         y[i] /= (float)nn;
      }
   }

   return(TRUE);
}

int CWaterSurface::Powerof2(int n,int *m,int *twopm)
{
	if (n <= 1) {
		*m = 0;
		*twopm = 1;
		return(FALSE);
	}

   *m = 1;
   *twopm = 2;
   do {
      (*m)++;
      (*twopm) *= 2;
   } while (2*(*twopm) <= n);

   if (*twopm != n) 
		return(FALSE);
	else
		return(TRUE);
}

void CWaterSurface::GetGaussian(float& fGaussian1, float& fGaussian2)
{
   // -------------------------------------------------------------------------
   // Generate pseudo-random numbers with mean 0 and standard deviation 1.
   // -------------------------------------------------------------------------
   float x1, x2, w;
   do 
   {
      x1 = 2.0 * ((float)rand()/(float)RAND_MAX) - 1.0;
      x2 = 2.0 * ((float)rand()/(float)RAND_MAX) - 1.0;
      w = x1 * x1 + x2 * x2;
   } while (w >= 1.0);

   w = (float)sqrt((-2.0 * log(w)) / w);
   fGaussian1 = x1 * w;
   fGaussian2 = x2 * w;
}

float CWaterSurface::GetPhillipsSpectrum(KWaveVector vecKBounded)
{   
   // -------------------------------------------------------------------------
   // Wind Direction
   // -------------------------------------------------------------------------
   KWaveVector vecWindSpeed;
   vecWindSpeed.fX = m_fXWindSpeed;
   vecWindSpeed.fZ = m_fZWindSpeed;

   // -------------------------------------------------------------------------
   // Represents the largest possible waves arising from a continuous wind of
   // speed V (or vecWindSpeed).
   // -------------------------------------------------------------------------
   float fWindspeedGravity = (
      (vecWindSpeed.fX * vecWindSpeed.fX) + 
      (vecWindSpeed.fZ * vecWindSpeed.fZ)) / m_fGravityConstant;

   float fKSquared = (vecKBounded.fX * vecKBounded.fX) + (vecKBounded.fZ * vecKBounded.fZ);
   float fKSquaredWindspeed = fKSquared * fWindspeedGravity * fWindspeedGravity;
   
   // -------------------------------------------------------------------------
   // Eliminates waves that move perpendicular to the wind direction.
   // -------------------------------------------------------------------------
   float fPerpendWaveEliminator = 
      (vecKBounded.fX * vecWindSpeed.fX) +
      (vecKBounded.fZ * vecWindSpeed.fZ);

   // -------------------------------------------------------------------------
   // Compute the Phillips Spectrum variable which is used for modeling
   // realistic waves.
   // -------------------------------------------------------------------------
   float fPhillipsSpectrum = 
      m_fPhillipsConstant 
      * ((exp(-1 / fKSquaredWindspeed)) / (fKSquared * fKSquared))  
      * (fPerpendWaveEliminator * fPerpendWaveEliminator);

   return fPhillipsSpectrum;
}
