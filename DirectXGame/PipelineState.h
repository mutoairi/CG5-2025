#pragma once
#include<d3dx12.h>
class PipelineState {
public:
	void Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);
	ID3D12PipelineState* Get();
	PipelineState();
	~PipelineState();

private:
	ID3D12PipelineState* pipelinState_ = nullptr;

};
