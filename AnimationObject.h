#pragma once

#include <string>
#include <deque>
#include <d3d9.h>
#include <d3dx9.h>
//#include <dxerr9.h>
using namespace std;

class CAnimationObject
{
public:
   CAnimationObject(IDirect3DDevice9* pDirect3D9Device);
   virtual ~CAnimationObject(void);

   //
   // Main Rendering Methods
   //

   virtual void Update(float fCurrentTime, bool blMoveObject = false) = 0;
   virtual void Draw(D3DXMATRIX& projectionMatrix, D3DXMATRIX& viewMatrix) = 0;

   //
   // Attribute Accessors and Mutators
   //

   virtual void SetPosition(D3DXVECTOR3 vecPosition);
   virtual D3DXVECTOR3& GetPosition();

   virtual void SetPositionMatrix(D3DXMATRIX& PositionMatrix);
   virtual D3DXMATRIX& GetPositionMatrix();

   virtual void SetColor(int r, int g, int b);
   virtual COLORREF GetColor();

   virtual void SetWorldMatrix(D3DXMATRIX& WorldMatrix);
   virtual D3DXMATRIX& GetWorldMatrix();

   virtual void SetTranslationMatrix(D3DXMATRIX& TranslationMatrix);
   virtual D3DXMATRIX& GetTranslationMatrix();

   virtual void SetRotationMatrix(D3DXMATRIX& RotationMatrix);
   virtual D3DXMATRIX& GetRotationMatrix();

   virtual void SetVelocity(D3DXVECTOR3 vecVelocity);
   virtual D3DXVECTOR3& GetVelocity();

   virtual void SetAngularVelocity(D3DXVECTOR3 vecAngularVelocity);
   virtual D3DXVECTOR3& GetAngularVelocity();

   virtual void SetAngularAcceleration(D3DXVECTOR3 vecAngularAcceleration);
   virtual D3DXVECTOR3& GetAngularAcceleration();

   virtual void SetMomentOfInertia(float fMomentOfInertia);
   virtual float GetMomentOfInertia();

   virtual void SetFriction(float fFriction);
   virtual float GetFriction();

protected:
   IDirect3DDevice9* m_pDirect3D9Device;

   D3DXVECTOR3 m_Position;
   COLORREF m_Color;

   D3DXMATRIX m_WorldMatrix;
   D3DXMATRIX m_PositionMatrix;
   D3DXMATRIX m_TranslationMatrix;
   D3DXMATRIX m_RotationMatrix;

   D3DXVECTOR3 m_Velocity;
   D3DXVECTOR3 m_AngularVelocity;
   D3DXVECTOR3 m_AngularAcceleration;
   float m_fMomentOfInertia;
   float m_fFriction;
};
