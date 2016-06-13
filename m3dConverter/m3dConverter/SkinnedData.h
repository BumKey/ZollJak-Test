#ifndef SKINNED_DATA_H
#define SKINNED_DATA_H

#include <D3D11.h>
#include <xnamath.h>
#include <map>
#include <vector>

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	std::vector<Keyframe> Keyframes; 	
};

///<summary>
/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
/// An AnimationClip requires a BoneAnimation for every bone to form
/// the animation clip.    
///</summary>
struct AnimationClip
{
    std::vector<BoneAnimation> BoneAnimations; 	
};

class SkinnedData
{
public:

	void Set(
		std::vector<int>& boneHierarchy, 
		std::vector<XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations);

    // Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;
	std::vector<XMFLOAT4X4> mBoneOffsets;
	std::map<std::string, AnimationClip> mAnimations;
};

#endif // SKINNED_DATA_H