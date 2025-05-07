#include "Shader.h"
#include<d3dcompiler.h>
#include<cassert>

void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {
	// filePath   :シェーダーのファイルパス
	// shaderModel:シェーダーのモデル

	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	// シェーダーの読み込み

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイル
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダーモデル指定
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

ID3D10Blob* Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() {
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
}

