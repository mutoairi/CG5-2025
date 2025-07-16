#include "RootSignature.h"
#include <KamataEngine.h>
using namespace KamataEngine;

void RootSignature::Create() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	// RootSignature作成----------------------------------------
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// でスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE srvDescRange[1]{};
	srvDescRange[0].BaseShaderRegister = 0;
	srvDescRange[0].NumDescriptors = 1;
	srvDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[1]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = srvDescRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(srvDescRange);

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_STATIC_SAMPLER_DESC staricSamplers[1] = {};
	staricSamplers[0].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	staricSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staricSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staricSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staricSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staricSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staricSamplers[0].ShaderRegister = 0;
	staricSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staricSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staricSamplers);

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

	signatureBlob->Release();
	rootSignature_ = rootSignature;
}

ID3D12RootSignature* RootSignature::Get() { return rootSignature_; }

RootSignature::RootSignature() {}

RootSignature::~RootSignature() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}
