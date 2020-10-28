//=============================================================================
//
// ポリゴン表示処理 [polygon.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "polygon.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX		(1)							// テクスチャの数

#define	VALUE_MOVE		(5.0f)						// 移動量
#define	VALUE_ROTATE	(D3DX_PI * 0.02f)			// 回転量

#define	SIZE_X			(100.0f)					// 地面のサイズ(X方向)
#define	SIZE_Z			(100.0f)					// 地面のサイズ(Z方向)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexPolygon(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// 頂点情報
static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static POLYGON						g_Polygon;				// ポリゴンデータ
static int							g_TexNo;				// テクスチャ番号

static char* g_TextureName[] = {
	"data/TEXTURE/field000.jpg",
};

// 頂点配列
static VERTEX_3D g_VertexArray[4] = {
	{D3DXVECTOR3(-SIZE_X, 0.0f,  SIZE_Z), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f)},
	{D3DXVECTOR3( SIZE_X, 0.0f,  SIZE_Z), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f)},
	{D3DXVECTOR3(-SIZE_X, 0.0f, -SIZE_Z), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f)},
	{D3DXVECTOR3( SIZE_X, 0.0f, -SIZE_Z), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f)},
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPolygon(void)
{
	// 頂点バッファの作成
	MakeVertexPolygon();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 位置・回転・スケールの初期設定
	g_Polygon.pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_Polygon.rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_Polygon.scl = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	g_TexNo = 0;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPolygon(void)
{
	// 頂点バッファの解放
	if (g_VertexBuffer) {
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	// テクスチャの解放
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePolygon(void)
{
	CAMERA *cam = GetCamera();

	// 回転させちゃう
	if (GetKeyboardPress(DIK_LEFT))
	{
		g_Polygon.rot.y -= VALUE_ROTATE;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{
		g_Polygon.rot.y += VALUE_ROTATE;
	}
	if (GetKeyboardPress(DIK_UP))
	{
		g_Polygon.rot.x -= VALUE_ROTATE;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{
		g_Polygon.rot.x += VALUE_ROTATE;
	}

	if (GetKeyboardPress(DIK_SPACE))
	{
		g_Polygon.rot.y = g_Polygon.rot.x = 0.0f;
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	char* str = GetDebugStr();
	sprintf(&str[strlen(str)], " rotX:%.2f rotY:%.2f", g_Polygon.rot.x, g_Polygon.rot.y);

#endif
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPolygon(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);


	D3DXMATRIX mtxScl, mtxRot, mtxTranslate;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&g_Polygon.mtxWorld);

	// スケールを反映
	D3DXMatrixScaling(&mtxScl, g_Polygon.scl.x, g_Polygon.scl.y, g_Polygon.scl.z);
	D3DXMatrixMultiply(&g_Polygon.mtxWorld, &g_Polygon.mtxWorld, &mtxScl);

	// 回転を反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, g_Polygon.rot.y, g_Polygon.rot.x, g_Polygon.rot.z);
	D3DXMatrixMultiply(&g_Polygon.mtxWorld, &g_Polygon.mtxWorld, &mtxRot);

	// 移動を反映
	D3DXMatrixTranslation(&mtxTranslate, g_Polygon.pos.x, g_Polygon.pos.y, g_Polygon.pos.z);
	D3DXMatrixMultiply(&g_Polygon.mtxWorld, &g_Polygon.mtxWorld, &mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&g_Polygon.mtxWorld);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}

HRESULT MakeVertexPolygon(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// グローバルの頂点配列から頂点バッファをつくる
	D3D11_SUBRESOURCE_DATA sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = g_VertexArray;

	GetDevice()->CreateBuffer(&bd, &sd, &g_VertexBuffer);

	return S_OK;
}