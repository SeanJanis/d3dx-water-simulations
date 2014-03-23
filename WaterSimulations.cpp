//--------------------------------------------------------------------------------------
// File: CustomUI.cpp
//
// Sample to show usage of DXUT's GUI system
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTguiIME.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "resource.h"

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

#include "WaterSurface.h"
#include "LandEnvironment.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*                  g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*                g_pEffect = NULL;       // D3DX effect interface
CDXUTXFileMesh              g_Mesh;                 // Background mesh
D3DXMATRIXA16               g_mView;
CFirstPersonCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_WaterSimulationsUI;             // dialog for sample specific controls
IDirect3DDevice9*           g_pDirect3DDevice9 = NULL;
bool                        g_blWireframeMode = 0;

// -------------------------------------------------------------------------------------
// Demo Controls
// -------------------------------------------------------------------------------------
CWaterSurface* g_pWaterSurface;
CLandEnvironment* g_pLandEnvironment;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN                       1

#define IDC_STATIC_WIND_SPEED_X_DESC               2
#define IDC_STATIC_WIND_SPEED_X_VALUE              3
#define IDC_SLIDER_WIND_SPEED_X                    4

#define IDC_STATIC_WIND_SPEED_Z_DESC               5
#define IDC_STATIC_WIND_SPEED_Z_VALUE              6
#define IDC_SLIDER_WIND_SPEED_Z                    7

#define IDC_STATIC_PHILLIPS_CONSTANT_DESC          8
#define IDC_STATIC_PHILLIPS_CONSTANT_VALUE         9
#define IDC_SLIDER_PHILLIPS_CONSTANT               10

#define IDC_STATIC_GRAVITY_CONSTANT_DESC           11
#define IDC_STATIC_GRAVITY_CONSTANT_VALUE          12
#define IDC_SLIDER_GRAVITY_CONSTANT                13

#define IDC_CHECK_ENABLE_GERSTNER_WAVES            14

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
void CALLBACK OnFrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnLostDevice(void* pUserContext);
void CALLBACK OnDestroyDevice(void* pUserContext);

void InitApp();
void RenderText();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackD3D9DeviceAcceptable(IsDeviceAcceptable);
    DXUTSetCallbackD3D9DeviceCreated(OnCreateDevice);
    DXUTSetCallbackD3D9DeviceReset(OnResetDevice);
    DXUTSetCallbackD3D9FrameRender(OnFrameRender);
    DXUTSetCallbackD3D9DeviceLost(OnLostDevice);
    DXUTSetCallbackD3D9DeviceDestroyed(OnDestroyDevice);
    DXUTSetCallbackMsgProc(MsgProc);
    DXUTSetCallbackKeyboard(KeyboardProc);
    DXUTSetCallbackFrameMove(OnFrameMove);
    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

    DXUTSetCursorSettings( true, true );
    InitApp();
    DXUTInit(true, true); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );
    DXUTCreateWindow(L"Sean Janis : Water Simulations");
    DXUTCreateDevice(true, 800, 600);
    DXUTMainLoop();

    return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
   // -------------------------------------------------------------------------
   // Initialize dialogs
   // -------------------------------------------------------------------------
   g_SettingsDlg.Init(&g_DialogResourceManager);
   g_HUD.Init(&g_DialogResourceManager);
   g_WaterSimulationsUI.Init(&g_DialogResourceManager);

   g_HUD.SetCallback(OnGUIEvent); int iY = 10;
   g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22);
   g_WaterSimulationsUI.SetCallback(OnGUIEvent);
   g_WaterSimulationsUI.SetFont(1, L"Arial", 16, FW_NORMAL);

   g_WaterSimulationsUI.AddStatic(IDC_STATIC_WIND_SPEED_X_DESC, L"X-Wind Speed:", 8, 3, 95, 30);
   g_WaterSimulationsUI.AddSlider(IDC_SLIDER_WIND_SPEED_X, 110, 5, 200, 24, -100, 100, 0, false);
   g_WaterSimulationsUI.AddStatic(IDC_STATIC_WIND_SPEED_X_VALUE, L"0.00", 295, 3, 95, 30);

   g_WaterSimulationsUI.AddStatic(IDC_STATIC_WIND_SPEED_Z_DESC, L"Z-Wind Speed:", 8, 35, 95, 30);
   g_WaterSimulationsUI.AddSlider(IDC_SLIDER_WIND_SPEED_Z, 110, 38, 200, 24, -100, 100, 0, false);
   g_WaterSimulationsUI.AddStatic(IDC_STATIC_WIND_SPEED_Z_VALUE, L"0.00", 295, 35, 95, 30);

   g_WaterSimulationsUI.AddStatic(IDC_STATIC_PHILLIPS_CONSTANT_DESC, L"Phillips Constant:", 8, 68, 95, 30);
   g_WaterSimulationsUI.AddSlider(IDC_SLIDER_PHILLIPS_CONSTANT, 110, 71, 200, 24, 0, 100, 50, false);
   g_WaterSimulationsUI.AddStatic(IDC_STATIC_PHILLIPS_CONSTANT_VALUE, L"0.00", 295, 68, 95, 30);

   g_WaterSimulationsUI.AddStatic(IDC_STATIC_GRAVITY_CONSTANT_DESC, L"Gravity Constant:", 8, 101, 95, 30);
   g_WaterSimulationsUI.AddSlider(IDC_SLIDER_GRAVITY_CONSTANT, 110, 104, 200, 24, 1, 15, 8, false);
   g_WaterSimulationsUI.AddStatic(IDC_STATIC_GRAVITY_CONSTANT_VALUE, L"0.00", 295, 101, 95, 30);

   g_WaterSimulationsUI.AddCheckBox(IDC_CHECK_ENABLE_GERSTNER_WAVES, L"Enable Random Gerstner Waves", 10, 143, 350, 16, false, L'C', false);
}


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
   // -------------------------------------------------------------------------
   // Skip backbuffer formats that don't support alpha blending
   // -------------------------------------------------------------------------
   IDirect3D9* pD3D = DXUTGetD3D9Object();
   if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
                                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                    D3DRTYPE_TEXTURE, BackBufferFormat)))
   {
      return false;
   }

   // -------------------------------------------------------------------------
   // Must support pixel shader 2.0
   // -------------------------------------------------------------------------
   if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
   {
      return false;
   }

   return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
   assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver);

   HRESULT hr;
   IDirect3D9* pD3D = DXUTGetD3D9Object();
   D3DCAPS9 caps;

   V(pD3D->GetDeviceCaps(pDeviceSettings->d3d9.AdapterOrdinal,
                         pDeviceSettings->d3d9.DeviceType,
                         &caps));

   // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
   // then switch to SWVP.
   if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 || caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
   {
      pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
   }

   // Debugging vertex shaders requires either REF or software vertex processing 
   // and debugging pixel shaders requires REF.  
   #ifdef DEBUG_VS
   if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
   {
     pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
     pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
     pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
   }
   #endif
   #ifdef DEBUG_PS
   pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
   #endif

   // For the first device created if its a REF device, optionally display a warning dialog box
   static bool s_bFirstTime = true;
   if (s_bFirstTime)
   {
      s_bFirstTime = false;
      if (pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF)
      {
         DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver);
      }
   }

   return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
   g_pDirect3DDevice9 = pd3dDevice;

   HRESULT hr;

   CDXUTIMEEditBox::Initialize( DXUTGetHWND() );

   V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
   V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
   V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Arial", &g_pFont ) );

   DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
   #if defined( DEBUG ) || defined( _DEBUG )
     dwShaderFlags |= D3DXSHADER_DEBUG;
   #endif
   #ifdef DEBUG_VS
     dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
   #endif
   #ifdef DEBUG_PS
     dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
   #endif

   // ------------------------------------------------------------------------
   // Read the D3DX effect file
   // ------------------------------------------------------------------------
   WCHAR str[MAX_PATH];
   V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"CustomUI.fx"));
   V_RETURN(D3DXCreateEffectFromFile(pd3dDevice, str, NULL, NULL, dwShaderFlags,
                                   NULL, &g_pEffect, NULL ) );

   // ------------------------------------------------------------------------
   // Setup the camera's view parameters
   // ------------------------------------------------------------------------
   float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
   g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 1.0f, 1000.0f);

   D3DXVECTOR3 vecEye(0.0f, 18.5f, -125.0f);
   D3DXVECTOR3 vecAt(0.0f, 0.2f, 0.0f);
   D3DXVECTOR3 vecUp(0.0f, 1.0f, 0.0f);
   g_Camera.SetViewParams(&vecEye, &vecAt);
   D3DXMatrixLookAtLH(&g_mView, &vecEye, &vecAt, &vecUp);
   
   // -------------------------------------------------------------------------
   // Initialize Water Surface Grid
   // -------------------------------------------------------------------------
   g_pWaterSurface = new CWaterSurface(
      pd3dDevice, 
      WATER_SURFACE_WIDTH, 
      WATER_SURFACE_HEIGHT, 
      WATER_SURFACE_DX, 
      WATER_SURFACE_DZ);
   g_pWaterSurface->Init();

   // -------------------------------------------------------------------------
   // Initialize Surrounding Land Environment
   // -------------------------------------------------------------------------
   g_pLandEnvironment = new CLandEnvironment(pd3dDevice);
   g_pLandEnvironment->Init();

   // -------------------------------------------------------------------------
   // Set Default Slider Values
   // -------------------------------------------------------------------------
   WCHAR wszOutput[1024];

   float fXWindSpeed = g_pWaterSurface->GetXWindSpeed();
   g_WaterSimulationsUI.GetSlider(IDC_SLIDER_WIND_SPEED_X)->SetValue(fXWindSpeed);
   StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)fXWindSpeed);
   g_WaterSimulationsUI.GetStatic(IDC_STATIC_WIND_SPEED_X_VALUE)->SetText(wszOutput);

   float fZWindSpeed = g_pWaterSurface->GetZWindSpeed();
   g_WaterSimulationsUI.GetSlider(IDC_SLIDER_WIND_SPEED_Z)->SetValue(fZWindSpeed);
   StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)fZWindSpeed);
   g_WaterSimulationsUI.GetStatic(IDC_STATIC_WIND_SPEED_Z_VALUE)->SetText(wszOutput);

   float fPhillipsConstant = g_pWaterSurface->GetPhillipsConstant();
   float fPhillipsConstant_Slider = ((float)fPhillipsConstant * 100) / 0.00016;
   g_WaterSimulationsUI.GetSlider(IDC_SLIDER_PHILLIPS_CONSTANT)->SetValue(fPhillipsConstant_Slider);
   StringCchPrintf(wszOutput, 1024, L"%3.7f", (double)fPhillipsConstant);
   g_WaterSimulationsUI.GetStatic(IDC_STATIC_PHILLIPS_CONSTANT_VALUE)->SetText(wszOutput);

   float fGravityConstant = g_pWaterSurface->GetGravityConstant();
   g_WaterSimulationsUI.GetSlider(IDC_SLIDER_GRAVITY_CONSTANT)->SetValue(fGravityConstant);
   StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)fGravityConstant);
   g_WaterSimulationsUI.GetStatic(IDC_STATIC_GRAVITY_CONSTANT_VALUE)->SetText(wszOutput);

   bool blEnableGerstnerWaves = g_pWaterSurface->GetEnableGerstnerWaves();
   g_WaterSimulationsUI.GetCheckBox(IDC_CHECK_ENABLE_GERSTNER_WAVES)->SetChecked(blEnableGerstnerWaves);

   return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9* pd3dDevice,
                               const D3DSURFACE_DESC* pBackBufferSurfaceDesc, 
                               void* pUserContext)
{
   HRESULT hr;

   g_DialogResourceManager.OnD3D9ResetDevice();
   g_SettingsDlg.OnD3D9ResetDevice();

   if (g_pFont)
   {
      g_pFont->OnResetDevice();
   }

   if (g_pEffect)
   {
      g_pEffect->OnResetDevice();
   }

   // -------------------------------------------------------------------------
   // Create a sprite to help batch calls when drawing many lines of text
   // -------------------------------------------------------------------------
   D3DXCreateSprite(pd3dDevice, &g_pTextSprite);

   // -------------------------------------------------------------------------
   // Setup the camera's projection parameters
   // -------------------------------------------------------------------------
   g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
   g_HUD.SetSize(170, 170);
   g_WaterSimulationsUI.SetLocation(0, 0);
   g_WaterSimulationsUI.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

   return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
   g_Camera.FrameMove(fElapsedTime);

   // -------------------------------------------------------------------------
   // Update objects
   // -------------------------------------------------------------------------
   if (g_pWaterSurface != NULL)
   {
      g_pWaterSurface->SetCamera(g_Camera);
      g_pWaterSurface->Update(fTime);
   }

   if (g_pLandEnvironment != NULL)
   {
      g_pLandEnvironment->SetCamera(g_Camera);
      g_pLandEnvironment->Update(fTime);
   }
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
   if (g_SettingsDlg.IsActive())
   {
      g_SettingsDlg.OnRender(fElapsedTime);
      return;
   }

   HRESULT hr;
   D3DXMATRIXA16 mWorld;
   D3DXMATRIXA16 mView;
   D3DXMATRIXA16 mProj;
   D3DXMATRIXA16 mWorldViewProjection;

   //--------------------------------------------------------------------------
   // Clear the render target and the zbuffer 
   //--------------------------------------------------------------------------
   pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

   // Render the scene
   pd3dDevice->BeginScene();

   // Get the projection & view matrix from the camera class
   mWorld = *g_Camera.GetWorldMatrix();
   mProj = *g_Camera.GetProjMatrix();
   mView = *g_Camera.GetViewMatrix();

   mWorldViewProjection = mWorld * mView * mProj;

   // -------------------------------------------------------------------------
   // Draw objects
   // -------------------------------------------------------------------------
   if (g_pLandEnvironment != NULL)
   {
      g_pLandEnvironment->Draw(
         mProj,
         mView);
   }

   if (g_pWaterSurface != NULL)
   {
      g_pWaterSurface->Draw(
         mProj,
         mView);
   }

   // Update the effect's variables.  Instead of using strings, it would 
   // be more efficient to cache a handle to the parameter by calling 
   // ID3DXEffect::GetParameterByName
   g_pEffect->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection);
   g_pEffect->SetMatrix("g_mWorld", &mWorld);
   g_pEffect->SetFloat("g_fTime", (float)fTime);

   g_pEffect->SetTechnique("RenderScene");
   UINT cPasses;
   g_pEffect->Begin( &cPasses, 0 );
   ID3DXMesh* pMesh = g_Mesh.GetMesh();
   for( UINT p = 0; p < cPasses; ++p )
   {
      g_pEffect->BeginPass( p );
      for (UINT m = 0; m < g_Mesh.m_dwNumMaterials; m++)
      {
          g_pEffect->SetTexture( "g_txScene", g_Mesh.m_pTextures[m]);
          g_pEffect->CommitChanges();
          pMesh->DrawSubset( m );
      }

      g_pEffect->EndPass();
   }

   g_pEffect->End();
   //RenderText();

   g_HUD.OnRender(fElapsedTime);
   g_WaterSimulationsUI.OnRender(fElapsedTime);

   pd3dDevice->EndScene();
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos(5, 5);
    txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
    txtHelper.DrawTextLine(DXUTGetFrameStats( DXUTIsVsyncEnabled()));
    txtHelper.DrawTextLine(DXUTGetDeviceStats());
    txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
    txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
    txtHelper.DrawTextLine(L"Press ESC to quit");
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = CDXUTIMEEditBox::StaticMsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_WaterSimulationsUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
   if (!bKeyDown)
   {
      return;
   }

   switch (nChar)
   {
      //
      // Wireframe Mode
      //

      case 'R':  
      {
         g_blWireframeMode = !g_blWireframeMode;
         if (g_blWireframeMode) 
         {
            g_pDirect3DDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
         } 
         else
         {
            g_pDirect3DDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
         }  
      }
      break;
   }
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
   WCHAR wszOutput[1024];
   memset(wszOutput, '\0', sizeof(wszOutput));

   switch (nControlID)
   {
      case IDC_TOGGLEFULLSCREEN:
      {
         DXUTToggleFullScreen(); 
         break;
      }

      case IDC_SLIDER_WIND_SPEED_X:
      {
         int nSliderValue = ((CDXUTSlider*)pControl)->GetValue();
         StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)nSliderValue);
         g_WaterSimulationsUI.GetStatic(IDC_STATIC_WIND_SPEED_X_VALUE)->SetText(wszOutput);

         g_pWaterSurface->SetXWindSpeed((float)nSliderValue);
      }
      break;

      case IDC_SLIDER_WIND_SPEED_Z:
      {
         int nSliderValue = ((CDXUTSlider*)pControl)->GetValue();
         StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)nSliderValue);
         g_WaterSimulationsUI.GetStatic(IDC_STATIC_WIND_SPEED_Z_VALUE)->SetText(wszOutput);

         g_pWaterSurface->SetZWindSpeed((float)nSliderValue);
      }
      break;

      case IDC_SLIDER_PHILLIPS_CONSTANT:
      {
         int nSliderValue = ((CDXUTSlider*)pControl)->GetValue();
         
         // -------------------------------------------------------------------
         // Convert Phillips Slider value to appropriate scale.
         // -------------------------------------------------------------------
         float fPhillipsConstant = (float)(nSliderValue * 0.00016) / (float)100;

         StringCchPrintf(wszOutput, 1024, L"%3.7f", (double)fPhillipsConstant);
         g_WaterSimulationsUI.GetStatic(IDC_STATIC_PHILLIPS_CONSTANT_VALUE)->SetText(wszOutput);

         g_pWaterSurface->SetPhillipsConstant((float)fPhillipsConstant);
      }
      break;

      case IDC_SLIDER_GRAVITY_CONSTANT:
      {
         int nSliderValue = ((CDXUTSlider*)pControl)->GetValue();
         StringCchPrintf(wszOutput, 1024, L"%3.1f", (double)nSliderValue);
         g_WaterSimulationsUI.GetStatic(IDC_STATIC_GRAVITY_CONSTANT_VALUE)->SetText(wszOutput);

         g_pWaterSurface->SetGravityConstant((float)nSliderValue);
      }
      break;

      case IDC_CHECK_ENABLE_GERSTNER_WAVES:
      {
         bool blEnableGerstnerWaves = g_WaterSimulationsUI.GetCheckBox(IDC_CHECK_ENABLE_GERSTNER_WAVES)->GetChecked();
         g_pWaterSurface->SetEnableGerstnerWaves(blEnableGerstnerWaves);
      }
      break;
   }
}

//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice(void* pUserContext)
{
   g_DialogResourceManager.OnD3D9LostDevice();
   g_SettingsDlg.OnD3D9LostDevice();
   g_Mesh.InvalidateDeviceObjects();

   if (g_pFont)
   {
      g_pFont->OnLostDevice();
   }

   if (g_pEffect)
   {
      g_pEffect->OnLostDevice();
   }

   SAFE_RELEASE(g_pTextSprite);
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
   CDXUTIMEEditBox::Uninitialize();

   g_DialogResourceManager.OnD3D9DestroyDevice();
   g_SettingsDlg.OnD3D9DestroyDevice();
   g_Mesh.Destroy();
   g_pDirect3DDevice9->Release();

   SAFE_RELEASE(g_pEffect);
   SAFE_RELEASE(g_pFont);

   if (g_pWaterSurface != NULL)
   {
      delete g_pWaterSurface;
      g_pWaterSurface = NULL;
   }

   if (g_pLandEnvironment != NULL)
   {
      delete g_pLandEnvironment;
      g_pLandEnvironment = NULL;
   }
}



