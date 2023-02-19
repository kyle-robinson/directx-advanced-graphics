#include "Skeleton.h"

Skeleton::Skeleton()
{
}

Skeleton::~Skeleton()
{
	CleanUp();
}

void Skeleton::Set(std::vector<int>& boneHierarchy, std::vector<XMFLOAT4X4>& boneOffsets, std::map<std::string, AnimationClip>& animations)
{
	_BoneHierarchy = boneHierarchy;
	_Animations = animations;
	for (size_t i = 0; i < boneOffsets.size(); i++)
	{
		_BoneData.push_back(new Bone(boneOffsets[i], _BoneHierarchy[i]));
	}

	for (size_t Bone = 0; Bone < boneHierarchy.size(); Bone++) {
		if (boneHierarchy[Bone] < 0 || boneHierarchy[Bone] >= _BoneData.size()) {
			continue;
		}
		_BoneData[boneHierarchy[Bone]]->AddChild(Bone);
	}

	RebuildBindPose();
}

/// <summary>
/// Froward kinamaits start
/// </summary>
/// <param name="clipName"></param>
/// <param name="timePos"></param>
/// <returns></returns>
std::vector<XMFLOAT4X4> Skeleton::GetFinalTransforms(const std::string& clipName, float timePos)
{
	std::vector<XMFLOAT4X4> finalTransforms;
	finalTransforms.resize(_BoneData.size());

	UINT numBones = _BoneData.size();

	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = _Animations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);
	

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = _BoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}
	
	
	//move animiation data into a useable transfrom space 
	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(_BoneData[i]->GetOffSet());
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}

	return finalTransforms;
}


std::vector<std::string> Skeleton::mAnimationClips()
{
	std::vector<std::string> AnimationName;

	for (auto Anmimations : _Animations) {
		AnimationName.push_back(Anmimations.first);
	}

	return AnimationName;
}

void Skeleton::CleanUp()
{

	for (size_t i = 0; i < _BoneData.size(); i++)
	{
		if (_BoneData[i]) {
			delete _BoneData[i];
			_BoneData[i] = nullptr;
		}
	}
	_BoneData.clear();


}

void Skeleton::RebuildBindPose()
{
	AnimationClip Clip;
	Clip.BoneAnimations.resize(BoneCount());

	for (size_t i = 0; i < BoneCount(); i++)
	{
		BoneAnimation BoneAnimation;
		BoneAnimation.Keyframes.resize(2);

		Keyframe Frame;
		if (i == 0) {
			Frame.Translation = XMFLOAT3(_BoneData[i]->GetWorldPos().x, _BoneData[i]->GetWorldPos().y, _BoneData[i]->GetWorldPos().z);
			Frame.Scale = XMFLOAT3(_BoneData[i]->GetWorldScale().x, _BoneData[i]->GetWorldScale().y, _BoneData[i]->GetWorldScale().z);
			Frame.RotationQuat = _BoneData[i]->GetWorldRot();
		}
		else {
			XMMATRIX RealMat = XMMatrixMultiply(XMLoadFloat4x4(_BoneData[i]->GetWorld()), XMMatrixInverse(nullptr, XMLoadFloat4x4(_BoneData[_BoneData[i]->Getparent()]->GetWorld())));

			XMVECTOR scalevec;
			XMVECTOR rot;
			XMVECTOR pos;

			XMMatrixDecompose(&scalevec, &rot, &pos, RealMat);

			XMFLOAT4X4 Real = XMFLOAT4X4();
			XMFLOAT4 PosReal = XMFLOAT4();
			XMFLOAT4 RotQuatReal = XMFLOAT4();
			XMFLOAT4 ScaleReal = XMFLOAT4();

			XMStoreFloat4(&ScaleReal, scalevec);
			XMStoreFloat4(&RotQuatReal, rot);
			XMStoreFloat4(&PosReal, pos);
			XMStoreFloat4x4(&Real, RealMat);
			Frame.Translation = XMFLOAT3(PosReal.x, PosReal.y, PosReal.z);
			Frame.Scale = XMFLOAT3(ScaleReal.x, ScaleReal.y, ScaleReal.z);
			Frame.RotationQuat = RotQuatReal;
			_BoneData[i]->SetReal(Real);
		}



		_BoneData[i]->SetRealPos(Frame.Translation);
		_BoneData[i]->SetRealRot(Frame.RotationQuat);
		_BoneData[i]->SetRealScale(Frame.Scale);

		Frame.TimePos = 0;
		BoneAnimation.Keyframes[0] = Frame;
		Frame.TimePos = 1;
		BoneAnimation.Keyframes[1] = Frame;


		Clip.BoneAnimations[i] = BoneAnimation;
	}
	_Animations["BindPose"] = Clip;
}

void Skeleton::SetChild(int Bone)
{


	if (_BoneData[Bone]->GetChild()[0] == -1) {
		return;
	}
	else {
		for (size_t i = 0; i < _BoneData[Bone]->GetChild().size(); i++)
		{
			XMMATRIX world = XMMatrixMultiply(XMLoadFloat4x4(_BoneData[_BoneData[Bone]->GetChild()[i]]->GetReal()), XMLoadFloat4x4(_BoneData[Bone]->GetWorld()));
			XMFLOAT4X4 worldFloat;
			XMStoreFloat4x4(&worldFloat, world);
			_BoneData[_BoneData[Bone]->GetChild()[i]]->SetWorld(worldFloat);
			SetChild(_BoneData[Bone]->GetChild()[i]);
		}
	}


}

XMFLOAT3 Skeleton::Rotate(XMFLOAT4 rotquat, int Currjoint)
{
	
	return XMFLOAT3();
}

XMFLOAT3 Skeleton::CalculateError(XMFLOAT3 endEffector, XMFLOAT3 Target)
{
	XMFLOAT3 Dist = XMFLOAT3(Target.x - endEffector.x, Target.z - endEffector.z, Target.z - endEffector.z);
	XMVECTOR DistVec = XMLoadFloat3(&Dist);
	DistVec=XMVector3Length(DistVec);
	Dist.x = XMVectorGetX(DistVec);
	Dist.y = XMVectorGetX(DistVec);
	Dist.z = XMVectorGetX(DistVec);
	return Dist;
}

void Skeleton::InverKin(int Endfectro, XMFLOAT3 Tatget)
{
	
	float MAXerrand = 0;
	float MAXitr = 2;

	XMFLOAT3 target = Tatget;
	XMFLOAT3 endeffector = XMFLOAT3(_BoneData[Endfectro]->GetRealPos().x, _BoneData[Endfectro]->GetRealPos().y, _BoneData[Endfectro]->GetRealPos().z);
	float error = CalculateError(endeffector, target).x;
	int iteraction = 0;

	//animation
	AnimationClip Clip;
	Clip.BoneAnimations.resize(_BoneData.size());
	while (error>MAXerrand && iteraction<MAXitr)
	{
		//animation frame
		for (size_t i = 0; i < _BoneData.size(); i++)
		{
			
			XMFLOAT3 e = endeffector;
			XMFLOAT3 j = XMFLOAT3(_BoneData[i]->GetRealPos().x, _BoneData[i]->GetRealPos().y, _BoneData[i]->GetRealPos().z);

			
			XMVECTOR endpos = XMLoadFloat3(&e);
			XMVECTOR jointPos = XMLoadFloat3(&j);
			XMVECTOR targetpos= XMLoadFloat3(&target);
			if (XMVector3Equal(endpos,jointPos)) {
				Clip.BoneAnimations[i].Keyframes.push_back(Keyframe());
				Clip.BoneAnimations[i].Keyframes.back().Translation = j;
				Clip.BoneAnimations[i].Keyframes.back().RotationQuat = _BoneData[Endfectro]->GetWorldRot();
				Clip.BoneAnimations[i].Keyframes.back().Scale = XMFLOAT3(1, 1, 1);
				Clip.BoneAnimations[i].Keyframes.back().TimePos = iteraction;
				continue;
			}
			XMVECTOR tocurrentVec = endpos - jointPos;
			XMVECTOR toTargetVec = targetpos - jointPos;

			tocurrentVec=XMVector3Normalize(tocurrentVec);
			toTargetVec = XMVector3Normalize(toTargetVec);

			float cosPhi = XMVectorGetX(XMVector3Dot(tocurrentVec, toTargetVec));

			float phiStep = acos(cosPhi);

			XMFLOAT3 axis;
			XMStoreFloat3(&axis, XMVector3Cross(tocurrentVec, toTargetVec));

			//rotation of this joint
			XMVECTOR qut= XMQuaternionRotationAxis(XMLoadFloat3(&axis), phiStep);
			XMFLOAT4 quat;
			XMStoreFloat4(&quat, qut);
			//SetBoneRotQuat(i, quat);

			Clip.BoneAnimations[i].Keyframes.push_back(Keyframe());
			Clip.BoneAnimations[i].Keyframes.back().Translation = j;
			XMStoreFloat4(&Clip.BoneAnimations[i].Keyframes.back().RotationQuat, qut);
			Clip.BoneAnimations[i].Keyframes.back().Scale = XMFLOAT3(1,1,1);
			Clip.BoneAnimations[i].Keyframes.back().TimePos = iteraction;
			
			
		}

		error= CalculateError(endeffector, target).x;
		iteraction++;
	}
	
	_Animations["Clip2"] = Clip;
	
}
