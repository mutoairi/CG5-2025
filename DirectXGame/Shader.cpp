#include "Shader.h"
#include <cassert>
#include <d3dcompiler.h>
#include"MiscUtility.h"

void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
	// filePath   :シェーダーのファイルパス
	// shaderModel:シェーダーのモデル

	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	std::string mbShaderModel = ConvertString(shaderModel);
	// シェーダーの読み込み

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイル
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", mbShaderModel.c_str(),                     // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &shaderBlob, &errorBlob);
	/*エラーが発生した場合、止める*/
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}

		assert(false);
	}
	// 生成したshaderBlobを取っておく
	blob_ = shaderBlob;
}

void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) {
	//DXCを初期化
	static IDxcUtils* dxcUtils = nullptr;
	static IDxcCompiler3* dxcCompiler = nullptr;
	static IDxcIncludeHandler* includeHandler = nullptr;

	HRESULT hr;

	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr));
	}
	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr));
	}

	if (includeHandler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
		assert(SUCCEEDED(hr));
	}
	//1.hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	//読みこんだファイルの内容をDxcBufferに設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//2.Compileする
	//Compileに必要なコンパイルオプションの準備
	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",
		L"main", // エントリーポイントン指定。基本main以外にしない
		L"-T", shaderModel.c_str(),//ShaderProfileの設定


	}
}

ID3D10Blob* Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() {
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
}
