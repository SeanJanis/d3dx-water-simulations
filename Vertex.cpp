#include "DXUT.h"
#include "Vertex.h"

IDirect3DVertexDeclaration9* CVertex::Decl = 0;

CVertex::CVertex(D3DXVECTOR3 pos, D3DXVECTOR3 normal, D3DXVECTOR2 texture)
{
   m_Pos = pos;
   m_Normal = normal;
   m_Texture = texture;
}

void InitVertexDeclarations(IDirect3DDevice9* pDirect3D9Device)
{
	D3DVERTEXELEMENT9 VertexPosElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
      {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};	

	pDirect3D9Device->CreateVertexDeclaration(VertexPosElements, &CVertex::Decl);
}

void DestroyVertexDeclarations()
{
	CVertex::Decl->Release();
}