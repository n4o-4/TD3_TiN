#include "AnimationManager.h"

void AnimationManager::Update()
{
    PlayAnimation();
}

void AnimationManager::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
    Animation animation; // 今回作るアニメーション
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
    assert(scene->mNumAnimations != 0); // アニメーションがない
    aiAnimation* animationAssimp = scene->mAnimations[0]; //　最初のアニメーションだけ採用
    animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

    // assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報をとってくる
    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) 
    {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
        {
            //----------------------
            // translate
            aiVectorKey& posKeyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            KeyframeVector3 posKeyframe;
            posKeyframe.time = float(posKeyAssimp.mTime / animationAssimp->mTicksPerSecond); // 時間の単位を変換(秒)
            posKeyframe.value = { -posKeyAssimp.mValue.x,posKeyAssimp.mValue.y,posKeyAssimp.mValue.z }; // 右手->左手
            nodeAnimation.translate.keyframes.push_back(posKeyframe);
        }

        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
        {
            //----------------------
            // rotate
            aiQuatKey& rotateKeyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion rotateKeyframe;
            rotateKeyframe.time = float(rotateKeyAssimp.mTime / animationAssimp->mTicksPerSecond); // 時間の単位を変換(秒)
            rotateKeyframe.value = { rotateKeyAssimp.mValue.x, -rotateKeyAssimp.mValue.y ,-rotateKeyAssimp.mValue.z,rotateKeyAssimp.mValue.w }; // 右手->左手

            rotateKeyframe.value = qNormalize(rotateKeyframe.value);

            nodeAnimation.rotate.keyframes.push_back(rotateKeyframe);
        }

        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
        {
            //----------------------
            // scale
            aiVectorKey& scaleKeyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeVector3 scaleKeyframe;
            scaleKeyframe.time = float(scaleKeyAssimp.mTime / animationAssimp->mTicksPerSecond); // 時間の単位を変換(秒)
            scaleKeyframe.value = { scaleKeyAssimp.mValue.x,scaleKeyAssimp.mValue.y ,scaleKeyAssimp.mValue.z }; // 変わらず
            nodeAnimation.scale.keyframes.push_back(scaleKeyframe);
        }
    }
   
    animation.modelData = LoadModelFile(directoryPath, filename);

    animation.modelData.rootNode.transform.scale = { 0.5f,0.5f,0.5f };

	animation.skeleton = CreateSkeleton(animation.modelData.rootNode);

    // 解析完了
    animations[filename] = animation;
}

ModelData AnimationManager::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
    ModelData modelData;

    Assimp::Importer importer;

    std::string filePath = directoryPath + "/" + filename;

    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

    assert(scene->HasMeshes()); // メッシュがないのは対応しない

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応
        assert(mesh->HasTextureCoords(0)); // TexcoordがないMesh
        // ここからMeshの中身(Face)の解析を行っていく

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            aiFace& face = mesh->mFaces[faceIndex];

            assert(face.mNumIndices == 3); // 三角形のみサポート
            // ここからFaceの中身(Vertex)の解析を行っていく

            for (uint32_t element = 0; element < face.mNumIndices; ++element)
            {
                uint32_t vertexIndex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertexIndex];
                aiVector3D& normal = mesh->mNormals[vertexIndex];
                aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
                VertexData vertex;
                vertex.position = { position.x,position.y,position.z,1.0f };
                vertex.normal = { normal.x,normal.y,normal.z };
                vertex.texcoord = { texcoord.x,texcoord.y };
                // 
                vertex.position.x *= -1.0f;
                vertex.normal.x *= -1.0f;
                modelData.vertices.push_back(vertex);
            }

        }
    }

    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
        {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
            modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
        }
    }

    modelData.rootNode = Model::ReadNode(scene->mRootNode);



    return modelData;
}

void AnimationManager::StartAnimation(const std::string& filename, int loopCount)
{
    auto it = animations.find(filename);

    // 見つからなかった場合止める
    //assert(it == animations.end());

    //　アニメーションをコピー
    Animation animation = it->second;

    // アニメーションのステータスを設定
    AnimationState animationState;
    animationState.animation = animation;
    animationState.currentLoopCount = 0;

    // 引数が0より小さい場合ループ数を無限に設定
    if (loopCount <= 0)
    {
        animationState.isInfiniteLoop = true;
    }

    // それ以外の場合ループ数をそのまま代入
    else
    {
        animationState.maxLoopCount = loopCount;
        animationState.isInfiniteLoop = false;
    }

    // アニメーションの時間を0で初期化
    animationState.animationTime = 0.0f;

    activeAnimations[filename] = animationState;
}

void AnimationManager::PlayAnimation()
{
    for (auto it = activeAnimations.begin(); it != activeAnimations.end();)
    {
        AnimationState animationState = it->second;

        animationState.animationTime += deltaTime; // 後に可変フレームに対応させる

        if (animationState.isInfiniteLoop)
        {
            animationState.animationTime = fmod(animationState.animationTime, animationState.animation.duration);
        }
        else
        {
            if (animationState.animationTime = fmod(animationState.animationTime, animationState.animation.duration))
            {
                ++animationState.currentLoopCount;

                if (animationState.maxLoopCount == animationState.currentLoopCount)
                {
                    return;
                }
            }
        }

        Vector3 scale ;
        Vector3 translate;
        Quaternion rotate;

        if (auto it = animationState.animation.nodeAnimations.find(animationState.animation.modelData.rootNode.name); it != animationState.animation.nodeAnimations.end())
        {
            const NodeAnimation& nodeAnimation = it->second;
            translate = CalculateValue(nodeAnimation.translate.keyframes, animationState.animationTime);
            rotate = CalculateQuaternion(nodeAnimation.rotate.keyframes, animationState.animationTime);
            scale = CalculateValue(nodeAnimation.scale.keyframes, animationState.animationTime);

            localMatrix = MakeAffineMatrixforQuater(scale, rotate, translate);
        }
      
        /*NodeAnimation& rootNodeAnimation = animationState.animation.nodeAnimations[animationState.animation.modelData.rootNode.name];
        Vector3 translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationState.animationTime);
        Quaternion rotate = CalculateQuaternion(rootNodeAnimation.rotate.keyframes, animationState.animationTime);
        Vector3 scale= CalculateValue(rootNodeAnimation.scale.keyframes, animationState.animationTime);*/

        for (Joint& joint : animationState.animation.skeleton.joints)
        {
            // 対象のJointのAnimationがあれば、値の適用を行う
			if (auto it = animationState.animation.nodeAnimations.find(joint.name); it != animationState.animation.nodeAnimations.end())
			{
				const NodeAnimation& nodeAnimation = it->second;
				joint.transform.translate = CalculateValue(nodeAnimation.translate.keyframes, animationState.animationTime);
                joint.transform.rotate = CalculateQuaternion(nodeAnimation.rotate.keyframes, animationState.animationTime);
                joint.transform.scale = CalculateValue(nodeAnimation.scale.keyframes, animationState.animationTime);
			}
        }

        SkeletonUpdate(animationState.animation.skeleton);

        it->second = animationState;

        ++it;
    }
}

Skeleton AnimationManager::CreateSkeleton(const Node& rootNode)
{
    Skeleton skeleton;
    skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

    // 名前とIndexのマッピングを行いアクセスしやすくする
    for (const Joint& joint : skeleton.joints)
    {
        skeleton.jointMap.emplace(joint.name, joint.index);
    }

    return skeleton;
}

int32_t AnimationManager::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints)
{
    Joint joint;
    joint.name = node.name;
    joint.localMatrix = node.localMatrix;
    joint.skeletonSpaceMatrix = MakeIdentity4x4();
    joint.transform = node.transform;
    joint.index = int32_t(joints.size()); // 現在登録されている数をInddexに
    joint.parent = parent;
    joints.push_back(joint); // SkeletonのJoint列に追加
    for (const Node& child : node.children)
    {
        // 子Jointを作成し、そのIndexを登録
        int32_t childIndex = CreateJoint(child, joint.index, joints);
        joints[joint.index].children.push_back(childIndex);
    }
    // 自身のIndexを返す
    return joint.index;
}

void AnimationManager::SkeletonUpdate(Skeleton& skeleton)
{
    // すべてのJointを更新。親が若いので通常ループで処理可能
	for (Joint& joint : skeleton.joints)
	{
		joint.localMatrix = MakeAffineMatrixforQuater(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
        if (joint.parent) // 親が居れば親の行列を掛ける
        {
            joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
        }
		else // 親がいないのでlocalMatrixをそのまま採用
		{
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

Vector3 AnimationManager::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time)
{
    //assert(!keyframes.empty()); // キーがないのでとめる
    //if (keyframes.size() == 1 || time <= keyframes[0].time)
    //{
    //    return keyframes[0].value;
    //}

    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;

        //indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補間する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
           
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    return (*keyframes.rbegin()).value;
}

Quaternion AnimationManager::CalculateQuaternion(const std::vector<KeyframeQuaternion>& keyframes, float time)
{
    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;

        //indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補間する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);

            Quaternion result = qLerp(keyframes[index].value, keyframes[nextIndex].value, t);

            return result;
        }
    }

    return (*keyframes.rbegin()).value;
}
