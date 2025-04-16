
#include <KamataEngine.h>
#include <Windows.h>
#include<d3dcompiler.h>




using namespace KamataEngine;
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	KamataEngine::Initialize(L"LE3C_25_ムトウ_アイリ_CG5");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//DirectXCommonが管理しているウィンドウの幅と高さの値を取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{},height:{}\n", w, h). c_str());

	//DirectXCommonクラスが管理している、コマンドリストの管理
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();


	// RootSignature作成----------------------------------------
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

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

	//------------------------------------------------------------
	// 
	
	//コンパイル済みのShader,エラー時情報の格納場所の用意
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

	//頂点シェーダーの読み込みとコンパイル
	std::wstring vsFile = L"Resources/shaders/TestVS.hlsl";
	hr = D3DCompileFromFile(
	    vsFile.c_str(), // シェーダーファイル
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", "vs_5_0",                                // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &vsBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());

		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}

		assert(false);
	}

	//ピクセルシェーダーの読み込みとコンパイル
	std::wstring psFile = L"Resources/shaders/TestPS.hlsl";
	hr = D3DCompileFromFile(
	    psFile.c_str(), // シェーダーファイル
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", "ps_5_0",                                // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &psBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());

		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}

		assert(false);
	}


	//PSOの生成-----------------------------------------------
	// 

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	graphicPipelineStateDesc.pRootSignature = rootSignature; // RootSignature
	graphicPipelineStateDesc.InputLayout = inputLayoutDesc;  // InputLayout

	graphicPipelineStateDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()}; // VertexShader
	graphicPipelineStateDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};   // PixelShader
	graphicPipelineStateDesc.BlendState = blendDesc;                                                         // BlendDesc
	graphicPipelineStateDesc.RasterizerState = rasterizerDesc;                                               // RasterizerState

	// 書き込むRTVの情報
	graphicPipelineStateDesc.NumRenderTargets = 1;
	graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ（形状）のタイプ。三角形
	graphicPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色を打ち込むのかを設定
	graphicPipelineStateDesc.SampleDesc.Count = 1;
	graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));


	//VertexResourceの生成------------------------------------------

	// 頂点リソース用のヒープの作成
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};

	// バッファリソース
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;

	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));


	// 頂点バッファビューを作成する----------------------------------

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ文のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;

	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	//
	// 頂点リソースにデータを書き込む-------------------------------------
	
	Vector4* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};

	// 上
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};

	// 右下
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};

	//頂点リソースのマップを解除する
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}
		

		// 描画開始
		dxCommon->PreDraw();

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->SetPipelineState(graphicsPipelineState);     // PSOを設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
		                                                          // 形状を設定。PSoに設定しているものとはまた別。同じものを設定すると考えておけばいい
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画！(DrawCall/ドローコール)。3頂点出一つのインスタンス。インスタンスについては今後
		commandList->DrawInstanced(3, 1, 0, 0);
		

		// 描画終了
		dxCommon->PostDraw();
	}
	
	vertexResource->Release();
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	psBlob->Release();
	vsBlob->Release();
	// エンジンの終了処理
	KamataEngine::Finalize();
	return 0;
}
