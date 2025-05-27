#pragma once
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <dxcapi.h>
#include <string>
#pragma comment(lib, "dxcompiler.lib")
class Shader {
public:
	// シェーダーを読み込みコンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::wstring& shaderModel);
	void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);
	// 生成したコンパイル済みデータを取得する
	ID3D10Blob* GetBlob();
	IDxcBlob* GetDxcBlob();

	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();

private:
	ID3D10Blob* blob_ = nullptr;
	IDxcBlob* dxcBlob_ = nullptr;
};
