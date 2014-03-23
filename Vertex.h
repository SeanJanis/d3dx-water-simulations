#pragma once

#include <string>
#include <deque>
#include <d3d9.h>
#include <d3dx9.h>
//#include <dxerr9.h>

using namespace std;

void InitVertexDeclarations(IDirect3DDevice9* pDirect3D9Device);
void DestroyVertexDeclarations();

class CVertex
{
public:
	CVertex(D3DXVECTOR3 pos, D3DXVECTOR3 normal, D3DXVECTOR2 texture);
   // virtual ~CVertex(); // Don't include destructor as DirectX won't draw vertices.

	D3DXVECTOR3 m_Pos;
   D3DXVECTOR3 m_Normal;
   D3DXVECTOR2 m_Texture;

	static IDirect3DVertexDeclaration9* Decl;
};

