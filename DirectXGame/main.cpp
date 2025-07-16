
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
// RenderTextureResourceの作成
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor);
// DepthStencilTextureResourceの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

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
		Vector2 texcoord;
	};
	VertexData vertices[] = {
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 左上
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 右下
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}  // 右上
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
	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	//=============  RenderTexture関係  ===============
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateRenderTargetView(renderTextureResource, nullptr, rtvHandleCPU);

	//=============  深度バッファ  ========================

	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);

	//====================  SRV  ================================================

	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(renderTextureResource, &srvDesc, srvHandleCPU);
	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画開始
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrier);

		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.top = 0;
		scissorRect.bottom = WinApp::kWindowHeight;

		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		dxCommon->PreDraw();

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(pipelineState.Get());  // PSOを設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVを設定
		commandList->IASetIndexBuffer(ib.GetView());         // IBVを設定
		                                                     //  形状を設定。PSoに設定しているものとはまた別。同じものを設定すると考えておけばいい
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		// 描画！(DrawCall/ドローコール)。3頂点出一つのインスタンス。インスタンスについては今後
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
		// commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);
	}

	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();
	// エンジンの終了処理
	KamataEngine::Finalize();
	return 0;
}

//----------------------　関数　-----------------------------
void SetupPioelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	// InputLayout----------------------------------------------

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);
	resourceDesc.Height = UINT(height);
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// ClearValueの用意
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// RanderTextureResourceの生成
	ID3D12Resource* resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// ClearValueの用意
	D3D12_CLEAR_VALUE depthClearValue;

	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	// Resourceの生成
	ID3D12Resource* resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}
