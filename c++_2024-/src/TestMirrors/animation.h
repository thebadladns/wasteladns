#ifndef __WASTELADNS_ANIMATION_H__
#define __WASTELADNS_ANIMATION_H__

namespace animation {
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
    float4x4 skinning[32]; // geometry to posed geometry matrices for each joint // todo: improve??
    u32 animIndex;
    f32 time;
    f32 scale;
};
typedef u32 Handle;
struct NodeMeta { enum Enum { HandleBits = 32, HandleMask = 0xffffffff, MaxNodes = HandleMask - 1 }; }; // handle=0 reserved for 0 initialization
struct Node {
    Skeleton skeleton; // constant mesh_to_joint matrices, as well as posed joint_to_parent matrices
    Clip* clips;
    State state; // separate???
    u32 clipCount;
};
struct Scene {
    allocator::Pool<Node> nodes;
};

Node& get_node(Scene& scene, const Handle handle) { return allocator::get_pool_slot(scene.nodes, handle - 1); }
Handle handle_from_node(Scene& scene, Node& node) { return allocator::get_pool_index(scene.nodes, node) + 1; }

void updateAnimation(Scene& scene, const f32 dt) {

    for (u32 n = 0, count = 0; n < scene.nodes.cap && count < scene.nodes.count; n++) {
        if (scene.nodes.data[n].alive == 0) { continue; }
        count++;

        animation::Node& animatedData = scene.nodes.data[n].state.live;
        State& state = animatedData.state;
        const Clip& clip = animatedData.clips[state.animIndex];
        const Skeleton& skeleton = animatedData.skeleton;

        state.time = state.time + dt * state.scale;
        if (state.time >= clip.timeEnd) {
            state.time -= clip.timeEnd;
        }
        f32 t = state.time / clip.timeEnd;
        u32 f0 = (u32)(t * (clip.frameCount - 1));
        u32 f1 = f0 + 1;

        for (u32 jointIndex = 0; jointIndex < skeleton.jointCount; jointIndex++) {

            const animation::Joint_TRS& joint_to_parent0 = clip.joints[jointIndex].joint_to_parent_trs[f0];
            const animation::Joint_TRS& joint_to_parent1 = clip.joints[jointIndex].joint_to_parent_trs[f1];
            // note: this will likely always default to lerp, since the angle delta will likely be small
            float4 q = math::quaternionSlerp(t, joint_to_parent0.rotation, joint_to_parent1.rotation);
            float3 translation = math::lerp(t, joint_to_parent0.translation, joint_to_parent1.translation);
            float3 scale = math::lerp(t, joint_to_parent0.scale, joint_to_parent1.scale);
            skeleton.parentFromPosedJoint[jointIndex] = math::trsToMatrix(translation, q, scale);
        }

        skeleton.geometryFromPosedJoint[0] =
            math::mult(skeleton.geometryFromRoot, skeleton.parentFromPosedJoint[0]);
        for (u32 jointIndex = 1; jointIndex < skeleton.jointCount; jointIndex++) {
            s8 parentIndex = skeleton.parentIndices[jointIndex];
            skeleton.geometryFromPosedJoint[jointIndex] =
                math::mult(
                    skeleton.geometryFromPosedJoint[parentIndex],
                    skeleton.parentFromPosedJoint[jointIndex]);
        }
        for (u32 jointIndex = 0; jointIndex < skeleton.jointCount; jointIndex++) {
            animatedData.state.skinning[jointIndex] =
                math::mult(
                    skeleton.geometryFromPosedJoint[jointIndex],
                    skeleton.jointFromGeometry[jointIndex]);
        }
    }
}
}

#endif // __WASTELADNS_ANIMATION_H__
