#pragma once
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>

#include "Structs.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp\postprocess.h>

#include "Model.h"

class AnimationManager
{
private:
	template <typename tValue>
	struct Keyframe
	{
		float time; // キーフレームの時刻
		tValue value; // キーフレームの値
	};
	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;
	
	template <typename tValue>
	struct AnimationCurve
	{
		std::vector<Keyframe<tValue>> keyframes;
	};

	struct NodeAnimation
	{
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};

	struct Animation
	{
		float duration; // アニメーション全体の尺(秒)

		// NodeAnimationの集合体
		std::unordered_map<std::string, NodeAnimation> nodeAnimations;
		ModelData modelData;

		Skeleton skeleton;
	};

	struct AnimationState
	{
		Animation animation;
		int maxLoopCount;
		int currentLoopCount;
		float animationTime;
		bool isInfiniteLoop;
	};

///------------------------------
/// メンバ関数
///------------------------------
public:

	void Update();

   // Animation読み込み
	void LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
	

	void StartAnimation(const std::string& filename,int loopCount);

	void PlayAnimation();

	Matrix4x4 GetLocalMatrix() { return localMatrix; }

	Skeleton CreateSkeleton(const Node& rootNode);

	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);


	void SkeletonUpdate(Skeleton& skeleton);

	Animation GetAnimation(const std::string& filename) { return animations[filename]; }

	Animation GetActiveAnimation(const std::string& filename) { return activeAnimations[filename].animation; }

private:

	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);

	Quaternion CalculateQuaternion(const std::vector<KeyframeQuaternion>& keyframes, float time);

///------------------------------
/// メンバ変数
///------------------------------
public:

	std::unordered_map<std::string, Animation> animations;

	std::unordered_map<std::string, AnimationState> activeAnimations;

private:
	
	Matrix4x4 localMatrix;

	const float deltaTime = 1.0f / 60.0f;

	float animationTime_ = 0.0f;

	int loopCount_ = 0;
};