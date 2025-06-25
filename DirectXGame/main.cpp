
#include "IndexBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;

// 関数のプロトタイプ宣言
void SetupPioelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	KamataEngine::Initialize(L"LE3C_25_ムトウ_アイリ_CG5");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonが管理しているウィンドウの幅と高さの値を取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{},height:{}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの管理
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignature作成----------------------------------------
	RootSignature rs;
	rs.Create();
	//-----------------------------------------------
	//

	// 頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	PipelineState pipelineState;
	SetupPioelineState(pipelineState, rs, vs, ps);

	struct VertexData {
		Vector4 position;
		
	};
	VertexData vertices[] = {
	    {-1.0f, -1.0f, 0.0f, 1.0f}, // 左下
	    {-1.0f, 1.0f,  0.0f, 1.0f}, // 左上
	    {1.0f,  -1.0f, 0.0f, 1.0f}, // 右下
	    {1.0f,  1.0f,  0.0f, 1.0f}, // 右上
	};

	// VertexResourceの生成------------------------------------------

	VertexBuffer vb;
	vb.Create(sizeof(vertices) * 3, sizeof(vertices[0]));

	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	for (int i = 0; i < _countof(vertices); ++i) {
		vertexData[i] = vertices[i];
	}
	//// 頂点リソースのマップを解除する
	vb.Get()->Unmap(0, nullptr);
	uint16_t indices[] = {
	    0, 1, 2, 2, 1, 3,
	};

	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));
	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}
	ib.Get()->Unmap(0, nullptr);
	//// 左下
	// vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};

	//// 上
	// vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};

	//// 右下
	// vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};

	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画開始
		dxCommon->PreDraw();

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(pipelineState.Get());  // PSOを設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVを設定
		commandList->IASetIndexBuffer(ib.GetView());         // IBVを設定
		                                             //  形状を設定。PSoに設定しているものとはまた別。同じものを設定すると考えておけばいい
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画！(DrawCall/ドローコール)。3頂点出一つのインスタンス。インスタンスについては今後
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
		// commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// エンジンの終了処理
	KamataEngine::Finalize();
	return 0;
}

//----------------------　関数　-----------------------------
void SetupPioelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	// InputLayout----------------------------------------------

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定------------------------------------------

	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerStateの設定-------------------------------------

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// PSOの生成-----------------------------------------------
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	graphicPipelineStateDesc.pRootSignature = rs.Get();     // RootSignature
	graphicPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout

	graphicPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	graphicPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	graphicPipelineStateDesc.BlendState = blendDesc;                                                       // BlendDesc
	graphicPipelineStateDesc.RasterizerState = rasterizerDesc;                                             // RasterizerState

	// 書き込むRTVの情報
	graphicPipelineStateDesc.NumRenderTargets = 1;
	graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ（形状）のタイプ。三角形
	graphicPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色を打ち込むのかを設定
	graphicPipelineStateDesc.SampleDesc.Count = 1;
	graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineState.Create(graphicPipelineStateDesc);
}