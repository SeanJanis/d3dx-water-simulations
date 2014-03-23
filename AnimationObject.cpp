#include "DXUT.h"
#include "AnimationObject.h"

CAnimationObject::CAnimationObject(IDirect3DDevice9* pDirect3D9Device) :
m_pDirect3D9Device(pDirect3D9Device)
{
   m_Position = D3DXVECTOR3(0, 0, 0);
   m_Color = 0;

   D3DXMatrixIdentity(&m_WorldMatrix);
   D3DXMatrixIdentity(&m_PositionMatrix);
   D3DXMatrixIdentity(&m_TranslationMatrix);
   D3DXMatrixIdentity(&m_RotationMatrix);

   m_Velocity = D3DXVECTOR3(0, 0, 0);
   m_AngularVelocity = D3DXVECTOR3(0, 0, 0);
   m_AngularAcceleration = D3DXVECTOR3(0, 0, 0);
   m_fFriction = 0;
   m_fMomentOfInertia = 0;
}

CAnimationObject::~CAnimationObject(void)
{
   m_pDirect3D9Device->Release();
   m_pDirect3D9Device = NULL;
}

void CAnimationObject::SetPosition(D3DXVECTOR3 vecPosition)
{
   m_Position = vecPosition;

   D3DXMatrixTranslation(
      &m_PositionMatrix, 
      m_Position.x, 
      m_Position.y, 
      m_Position.z);
}

D3DXVECTOR3& CAnimationObject::GetPosition()
{
   return m_Position;
}

void CAnimationObject::SetColor(int r, int g, int b)
{
   m_Color = RGB(r, g, b);
}

COLORREF CAnimationObject::GetColor()
{
   return m_Color;
}

void CAnimationObject::SetPositionMatrix(D3DXMATRIX& PositionMatrix)
{
   m_PositionMatrix = PositionMatrix;
}

D3DXMATRIX& CAnimationObject::GetPositionMatrix()
{
   return m_PositionMatrix;
}

void CAnimationObject::SetWorldMatrix(D3DXMATRIX& WorldMatrix)
{
   m_WorldMatrix = WorldMatrix;
}

D3DXMATRIX& CAnimationObject::GetWorldMatrix()
{
   return m_WorldMatrix;
}

void CAnimationObject::SetTranslationMatrix(D3DXMATRIX& TranslationMatrix)
{
   m_TranslationMatrix = TranslationMatrix;
}

D3DXMATRIX& CAnimationObject::GetTranslationMatrix()
{
   return m_TranslationMatrix;
}

void CAnimationObject::SetRotationMatrix(D3DXMATRIX& RotationMatrix)
{
   m_RotationMatrix = RotationMatrix;
}

D3DXMATRIX& CAnimationObject::GetRotationMatrix()
{
   return m_RotationMatrix;
}

void CAnimationObject::SetVelocity(D3DXVECTOR3 vecVelocity)
{
   m_Velocity = vecVelocity;
}

D3DXVECTOR3& CAnimationObject::GetVelocity()
{
   return m_Velocity;
}

void CAnimationObject::SetAngularVelocity(D3DXVECTOR3 vecAngularVelocity)
{
   m_AngularVelocity = vecAngularVelocity;
}

D3DXVECTOR3& CAnimationObject::GetAngularVelocity()
{
   return m_AngularVelocity;
}

void CAnimationObject::SetAngularAcceleration(D3DXVECTOR3 vecAngularAcceleration)
{
   m_AngularAcceleration = vecAngularAcceleration;
}

D3DXVECTOR3& CAnimationObject::GetAngularAcceleration()
{
   return m_AngularAcceleration;
}

void CAnimationObject::SetMomentOfInertia(float fMomentOfInertia)
{
   m_fMomentOfInertia = fMomentOfInertia;
}

float CAnimationObject::GetMomentOfInertia()
{
   return m_fMomentOfInertia;
}

void CAnimationObject::SetFriction(float fFriction)
{
   m_fFriction = fFriction;
}

float CAnimationObject::GetFriction()
{
   return m_fFriction;
}