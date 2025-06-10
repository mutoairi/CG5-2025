#include "PipelineState.h"
#include"KamataEngine.h"
using namespace KamataEngine;

void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc) {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance(); 
	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	pipelinState_ = graphicsPipelineState;
}

ID3D12PipelineState* PipelineState::Get() { return pipelinState_; }

PipelineState::PipelineState() {}

PipelineState::~PipelineState() {
	if (pipelinState_) {
		pipelinState_->Release();
		pipelinState_ = nullptr;
	}
}
