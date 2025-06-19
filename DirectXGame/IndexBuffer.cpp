#include "IndexBuffer.h"
#include"KamataEngine.h"

#include<d3d12.h>
#include<cassert>

using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) { 
	//strideの値によって、1つのインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4); 
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// インデックスリソース用のヒープの作成
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// インデックスリソースの設定
	D3D12_RESOURCE_DESC indexResourceDesc{};

	// バッファリソース
	indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexResourceDesc.Width = size;

	// バッファの場合はこれらは1にする決まり
	indexResourceDesc.Height = 1;
	indexResourceDesc.DepthOrArraySize = 1;
	indexResourceDesc.MipLevels = 1;
	indexResourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際にインデックスリソースを作る
	ID3D12Resource* indexResource = nullptr;
	HRESULT hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexResource));
	assert(SUCCEEDED(hr));
	indexBuffer_ = indexResource;

	// インデックスバッファビューを作成する----------------------------------

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ文のサイズ
	indexBufferView.SizeInBytes = size;

	// インデックスのフォーマット
	indexBufferView.Format = format;

	indexBufferView_ = indexBufferView;
}

ID3D12Resource* IndexBuffer::Get() { return indexBuffer_; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return &indexBufferView_; }

IndexBuffer::IndexBuffer() {}

IndexBuffer::~IndexBuffer() {
	if (indexBuffer_) {
		indexBuffer_->Release();
		indexBuffer_ = nullptr;
	}
}
