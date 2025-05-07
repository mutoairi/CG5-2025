#pragma once
#include <d3dx12.h>
#include <string>

class Shader {
public:
	// シェーダーを読み込みコンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::string& shaderModel);
	// 生成したコンパイル済みデータを取得する
	ID3D10Blob* GetBlob();

	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();

private:
	ID3D10Blob* blob_ = nullptr;
};
