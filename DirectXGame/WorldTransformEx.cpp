#include "WorldTransformEx.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void WorldTransformEx::UpdateMatrix() { 
	matWorld_ = MakeAffineMatrix(); 
	TransferMatrix();
}

KamataEngine::Matrix4x4 WorldTransformEx::MakeAffineMatrix() { 
	// スケーリング行列の作成
	Matrix4x4 matScale = MakeScaleMatrix(scale_);
	// 回転行列の作成
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	
	Matrix4x4 matRot = matRotZ *matRotX* matRotY;

	// 平行移動行列の作成
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);
	// スケーリング、回転、平行移動の合成
	Matrix4x4 matWorld= matScale * matRot * matTrans;
	return matWorld;
}
