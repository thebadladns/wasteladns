#ifndef __WASTELADNS_ANIMATION_H__
#define __WASTELADNS_ANIMATION_H__

namespace Animation {
struct Skeleton {
    float4x4 geometryFromRoot;
    float4x4* parentFromPosedJoint;
    float4x4* geometryFromPosedJoint;
    float4x4* jointFromGeometry;
    s8* parentIndices; // the parent of each joint (-1 for root)
    u32 jointCount; // number of joints
};
struct Joint_TRS {
    float3 translation;
    float4 rotation;
    float3 scale;
};
struct JointKeyframes {
    Joint_TRS* joint_to_parent_trs; // must be initaliazed to Clip::frameCount
};
struct Clip {
    JointKeyframes* joints;
    u32 frameCount;
    f32 timeEnd;
};
struct State {
    u32 animIndex;
    f32 time;
    f32 scale;
};
struct AnimatedNode {
    Skeleton skeleton; // constant mesh_to_joint matrices, as well as posed joint_to_parent matrices
    Clip* clips;
    State state; // separate???
    u32 clipCount;
    u32 drawNodeHandle; // todo: not here!
};
struct AnimatedState {
    u32 animIndex;
    f32 time;
    f32 scale;
};

void updateAnimation(float4x4* skinning, State& state, const Clip& clip, const Skeleton& skeleton, const f32 dt) {
    {
        state.time = state.time + dt * state.scale;
        if (state.time >= clip.timeEnd) {
            state.time -= clip.timeEnd;
        }
        f32 t = state.time / clip.timeEnd;
        u32 f0 = (u32)(t * (clip.frameCount - 1));
        u32 f1 = f0 + 1;

        for (u32 jointIndex = 0; jointIndex < skeleton.jointCount; jointIndex++) {

            const Animation::Joint_TRS& joint_to_parent0 = clip.joints[jointIndex].joint_to_parent_trs[f0];
            const Animation::Joint_TRS& joint_to_parent1 = clip.joints[jointIndex].joint_to_parent_trs[f1];
            // note: this will likely always default to lerp, since the angle delta will likely be small
            float4 q = Math::quaternionSlerp(t, joint_to_parent0.rotation, joint_to_parent1.rotation);
            float3 translation = Math::lerp(t, joint_to_parent0.translation, joint_to_parent1.translation);
            float3 scale = Math::lerp(t, joint_to_parent0.scale, joint_to_parent1.scale);
            skeleton.parentFromPosedJoint[jointIndex] = Math::trsToMatrix(translation, q, scale);
        }
    }

    skeleton.geometryFromPosedJoint[0] = Math::mult(skeleton.geometryFromRoot, skeleton.parentFromPosedJoint[0]);
    for (u32 jointIndex = 1; jointIndex < skeleton.jointCount; jointIndex++) {
        s8 parentIndex = skeleton.parentIndices[jointIndex];
        skeleton.geometryFromPosedJoint[jointIndex] = Math::mult(skeleton.geometryFromPosedJoint[parentIndex], skeleton.parentFromPosedJoint[jointIndex]);
    }
    for (u32 jointIndex = 0; jointIndex < skeleton.jointCount; jointIndex++) {
        skinning[jointIndex] = Math::mult(skeleton.geometryFromPosedJoint[jointIndex], skeleton.jointFromGeometry[jointIndex]);
    }
}
}

#endif // __WASTELADNS_ANIMATION_H__
