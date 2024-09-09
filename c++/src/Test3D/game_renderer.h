#ifndef __WASTELADNS_GAMERENDERER_H__
#define __WASTELADNS_GAMERENDERER_H__

namespace Renderer {

    namespace Drawlist {
        using DL_color_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3Color4B
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_colorskinned_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3Color4BSkinned
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_textureMapped_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_textured_lit_normalmapped
            , Layout_Vec3TexturedMapped
            , Layout_CBuffer_3DScene
            , Layout_CBuffer_3DScene
            , Shaders::VSDrawType::Standard
        >;
        using DL_textured_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_textured_unlit
            , Layout_TexturedSkinnedVec3
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_texturedalphaclip_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_textured_unlitalphaclip
            , Layout_TexturedSkinnedVec3
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_unlit_instanced_ctx = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Instanced
        >;
        typedef DL_color_ctx::type DL_color_t;
        typedef DL_colorskinned_ctx::type DL_colorskinned_t;
        typedef DL_textureMapped_ctx::type DL_textureMapped_t;
        typedef DL_textured_ctx::type DL_textured_t;
        typedef DL_texturedalphaclip_ctx::type DL_texturedalphaclip_t;
        typedef DL_unlit_instanced_ctx::type DL_unlit_instanced_t;
        typedef DL_color_ctx::type::Handle DL_color_id;
        typedef DL_colorskinned_ctx::type::Handle DL_colorskinned_id;
        typedef DL_textureMapped_ctx::type::Handle DL_textureMapped_id;
        typedef DL_textured_ctx::type::Handle DL_textured_id;
        typedef DL_texturedalphaclip_ctx::type::Handle DL_texturedalphaclip_id;
        typedef DL_unlit_instanced_ctx::type::Handle DL_unlit_instanced_id;
        typedef DL_color_ctx::type::DL_VertexBuffer DL_color_vb;
        typedef DL_colorskinned_ctx::type::DL_VertexBuffer DL_colorskinned_vb;
        typedef DL_textureMapped_ctx::type::DL_VertexBuffer DL_textureMapped_vb;
        typedef DL_textured_ctx::type::DL_VertexBuffer DL_textured_vb;
        typedef DL_texturedalphaclip_ctx::type::DL_VertexBuffer DL_texturedalphaclip_vb;
        typedef DL_unlit_instanced_ctx::type::DL_VertexBuffer DL_unlit_instanced_vb;
        typedef DL_textureMapped_ctx::type::DL_Material DL_textureMapped_mat;
        typedef DL_textured_ctx::type::DL_Material DL_textured_mat;
        typedef DL_texturedalphaclip_ctx::type::DL_Material DL_texturedalphaclip_mat;

        template<typename... _T>
        struct Drawlist_handle : _T::Handle... {};
        template<typename... _T>
        struct Drawlist_set : _T... {
            using Handle = Drawlist_handle<_T...>;

            template<typename _Op, typename... _Args>
            static void for_each(_Args&&... args) {
                int dummy[] = { 0, (_Op::template execute<_T>(args...), 0)... };
                (void)dummy;
                //(_Op::execute<_T>(args...), ...); // c++17 only
            }
        };

        template<typename _drawlistSet, typename _drawlist>
        struct ConcatDrawlist;
        template<typename... _drawlists, typename _drawlist>
        struct ConcatDrawlist<_drawlist, Drawlist_set<_drawlists...>> { using type = Drawlist_set<_drawlist, _drawlists...>; };
        template <typename _filter, typename... _drawlists> struct DrawlistSubsetImpl;
        template <typename _filter, typename _drawlist, typename... _drawlists>
        struct DrawlistSubsetImpl<_filter, _drawlist, _drawlists...> {
            using next = typename DrawlistSubsetImpl<_filter, _drawlists...>::type;
            using type = Conditional_t<_filter::template eval<_drawlist>::eval::v,
                typename ConcatDrawlist<_drawlist, next>::type,
                next
            >;
        };
        template <typename _filter>
        struct DrawlistSubsetImpl<_filter> {
            using type = Drawlist_set<>;
        };
        template <typename _filter, typename _drawlistSet>
        struct DrawlistSubset;
        template <typename _filter, typename... _drawlists>
		struct DrawlistSubset<_filter, Drawlist_set<_drawlists...>> {
			using type = typename DrawlistSubsetImpl<_filter, _drawlists...>::type;
		};

        typedef Drawlist_set <
            DL_color_t,
            DL_colorskinned_t,
            DL_textured_t,
            DL_texturedalphaclip_t,
            DL_textureMapped_t,
            DL_unlit_instanced_t
        > Drawlist_game;
        struct Drawlist_node {
            Transform localTransform;
            Transform worldTransform;
            Drawlist_game::Handle handle;
        };

        struct HasInstanceData_filter {
            template<typename _drawlist>
            struct eval { enum { v = (Shaders::VSSkinType::Enum)_drawlist::DL_SkinnedType::type == Shaders::VSSkinType::Skinned
                                  || (Shaders::VSDrawType::Enum)_drawlist::DL_DrawType::type == Shaders::VSDrawType::Instanced
            }; };
        };
        typedef typename DrawlistSubset<HasInstanceData_filter, Drawlist_game>::type Drawlist_instanceData;

        struct update_worldmatrix_t {
            template <typename _T>
            static void execute(Drawlist_game& drawlist, const Drawlist_node& node) {
                typename _T::DL_VertexBuffer& leafData = drawlist._T::get_leaf((typename _T::Handle&)(node.handle));
                leafData.groupData.worldMatrix = node.worldTransform.matrix;
            }
        };
        void update_dl_node_matrix(Drawlist_game& drawlist, const Drawlist_node& node) {
            Drawlist_game::for_each<update_worldmatrix_t>(drawlist, node);
        }
        
        struct update_instance_data_t {
            template<typename _T>
            static void execute(Drawlist_game& drawlist, const Drawlist_game::Handle& handle, const Mat4* matrices) {
                typename _T::DL_VertexBuffer& leafData = drawlist._T::get_leaf((typename _T::Handle&)(handle));
                for (u32 i = 0; i < leafData.instancedData.size(); i++) {
                    leafData.instancedData[i] = matrices[i];
                }
            }
        };
        void update_dl_node_instance_data(Drawlist_game& drawlist, const Drawlist_game::Handle& handle, const Mat4* matrices) {
            Drawlist_instanceData::for_each<update_instance_data_t>(drawlist, handle, matrices);
        }
        struct draw_drawlist_t {
            template <typename _T>
            static void execute(const Drawlist_game& drawlist, Driver::RscCBuffer* cbuffers) {
                dl_drawPerShader((const _T&)drawlist, cbuffers);
            }
        };
        void draw_drawlists(const Drawlist_game& drawlist, Driver::RscCBuffer* cbuffers) {
            Drawlist_game::for_each<draw_drawlist_t>(drawlist, cbuffers);
        }
        struct draw_drawlist_nodes_t {
            template <typename _T>
            static void execute(const Drawlist_node& node, const Drawlist_game& drawlist, Driver::RscCBuffer* cbuffers) {
                dl_drawPerNode((const _T&)drawlist, (const typename _T::Handle&)(node.handle), cbuffers);
            }
        };
        void draw_drawlist_nodes(const Drawlist_node* nodes, const u32 node_count, const Drawlist_game& drawlist, Driver::RscCBuffer* cbuffers) {
            for (u32 i = 0; i < node_count; i++) {
                Drawlist_game::for_each<draw_drawlist_nodes_t>(nodes[i], drawlist, cbuffers);
            }
        }
    }

    namespace Animation {
        struct Skeleton {
            Mat4* joint_to_parent; // the local space of the joint in relation to their parents (posed, defaults to t-pose)
            Mat4* model_to_joint; // where each vertex is in relation to the joint
            s8* parentIndices; // the parent of each joint (-1 for root)
            u32 jointCount; // number of joints
        };
        struct Frame {
            Mat4* joint_to_parent; // must be initaliazed to Skeleton::jointCount
        };
        struct Clip {
            Frame* frames;
            u32 frameCount;
        };
        struct AnimatedNode {
            Skeleton skeleton; // constant mesh_to_joint matrices, as well as posed joint_to_parent matrices
            Clip* clips;
            u32 clipCount;
            Renderer::Drawlist::Drawlist_game::Handle mesh_DLhandle;
        };
    
        void updateAnimation(Mat4* world_to_joint, const AnimatedNode& node, const f32 time) {
            // unused time for now
            (void)time;
            
            memcpy(world_to_joint, node.skeleton.joint_to_parent, sizeof(Mat4) * node.skeleton.jointCount);
            for (u32 jointIndex = 1; jointIndex < node.skeleton.jointCount; jointIndex++) {
                s8 parentIndex = node.skeleton.parentIndices[jointIndex];
                world_to_joint[jointIndex] = Math::mult(node.skeleton.model_to_joint[parentIndex], node.skeleton.joint_to_parent[jointIndex]);
            }
        }
    }

    typedef Shader_Params<
          Renderer::Shaders::VSTechnique::fullscreen_blit, Renderer::Shaders::PSTechnique::fullscreen_blit_textured
        , Renderer::Layout_TexturedVec3, Layout_CNone, Layout_CNone
        , Renderer::Shaders::VSDrawType::Standard> ShaderParams_Blit;

    struct Store {
        tinystl::vector<Drawlist::Drawlist_node> drawlist_nodes;
        tinystl::vector<Animation::AnimatedNode> animated_nodes;
        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        Drawlist::Drawlist_game drawlist;
        ShaderParams_Blit::ShaderSet shader_blit;
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateFillCullNone, rasterizerStateLine;
        Renderer::Driver::RscDepthStencilState depthStateOn, depthStateReadOnly, depthStateOff;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscBlendState blendStateOff;
        Renderer::Driver::RscMainRenderTarget windowRT;
        Renderer::Driver::RscRenderTarget<1> gameRT;
    };

    void start_store(Store& store, const Platform::State& platform, Allocator::Arena scratchArena) {

        Renderer::Driver::MainRenderTargetParams windowRTparams = {};
        windowRTparams.depth = true;
        windowRTparams.width = platform.screen.window_width;
        windowRTparams.height = platform.screen.window_height;
        Renderer::Driver::create_main_RT(store.windowRT, windowRTparams);

        Renderer::Driver::RenderTargetParams gameRTparams;
        gameRTparams.depth = true;
        gameRTparams.width = platform.screen.width;
        gameRTparams.height = platform.screen.height;
        gameRTparams.textureFormat = Renderer::Driver::TextureFormat::V4_8;
        gameRTparams.textureInternalFormat = Renderer::Driver::InternalTextureFormat::V4_8;
        gameRTparams.textureFormatType = Renderer::Driver::Type::Float;
        Renderer::Driver::create_RT(store.gameRT, gameRTparams);

        // rasterizer states
        Renderer::Driver::create_blend_state(store.blendStateOn, { true });
        Renderer::Driver::create_blend_state(store.blendStateOff, { false });
        Renderer::Driver::create_RS(store.rasterizerStateFill, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullBack });
        Renderer::Driver::create_RS(store.rasterizerStateFillCullNone, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullNone });
        Renderer::Driver::create_RS(store.rasterizerStateLine, { Renderer::Driver::RasterizerFillMode::Line, Renderer::Driver::RasterizerCullMode::CullNone });
        Renderer::Driver::create_DS(store.depthStateOn, { true, Renderer::Driver::DepthFunc::Less, Renderer::Driver::DepthWriteMask::All });
        Renderer::Driver::create_DS(store.depthStateReadOnly, { true, Renderer::Driver::DepthFunc::Less, Renderer::Driver::DepthWriteMask::Zero });
        Renderer::Driver::create_DS(store.depthStateOff, { false });

        ShaderParams_Blit::create_shader_from_techniques(store.shader_blit, nullptr);

        {
            using namespace Renderer::Drawlist;

            auto& dl = store.drawlist;
            dl = {};
            Renderer::create_cbuffers_3DScene(store.cbuffers);
            {
                dl.DL_color_t::rasterizerState = &store.rasterizerStateFill;
                dl.DL_color_t::blendState = &store.blendStateOff;
                dl.DL_color_t::depthStencilState = &store.depthStateOn;
                SET_MARKER_NAME(dl.DL_color_t::markerName, "OPAQUE COLOR")
                DL_color_ctx::load_dl_technique((DL_color_t&)dl, store.cbuffers);
            }
            {
                dl.DL_colorskinned_t::rasterizerState = &store.rasterizerStateFill;
                dl.DL_colorskinned_t::blendState = &store.blendStateOff;
                dl.DL_colorskinned_t::depthStencilState = &store.depthStateOn;
                SET_MARKER_NAME(dl.DL_colorskinned_t::markerName, "OPAQUE COLOR SKINNED")
                DL_colorskinned_ctx::load_dl_technique((DL_colorskinned_t&)dl, store.cbuffers);
            }
            {
                dl.DL_textureMapped_t::rasterizerState = &store.rasterizerStateFill;
                dl.DL_textureMapped_t::blendState = &store.blendStateOff;
                dl.DL_textureMapped_t::depthStencilState = &store.depthStateOn;
                SET_MARKER_NAME(dl.DL_textureMapped_t::markerName, "OPAQUE TEXTURE MAPPED")
                DL_textureMapped_ctx::load_dl_technique((DL_textureMapped_t&)dl, store.cbuffers);
            }
            {
                dl.DL_textured_t::rasterizerState = &store.rasterizerStateFill;
                dl.DL_textured_t::blendState = &store.blendStateOff;
                dl.DL_textured_t::depthStencilState = &store.depthStateOn;
                SET_MARKER_NAME(dl.DL_textured_t::markerName, "OPAQUE TEXTURED SKINNED")
                DL_textured_ctx::load_dl_technique((DL_textured_t&)dl, store.cbuffers);
            }
            {

                dl.DL_texturedalphaclip_t::rasterizerState = &store.rasterizerStateFillCullNone;
                dl.DL_texturedalphaclip_t::blendState = &store.blendStateOn;
                dl.DL_texturedalphaclip_t::depthStencilState = &store.depthStateOn;
                SET_MARKER_NAME(dl.DL_texturedalphaclip_t::markerName, "OPAQUE TEXTURED SKINNED ALPHACLIP")
                DL_texturedalphaclip_ctx::load_dl_technique((DL_texturedalphaclip_t&)dl, store.cbuffers);
            }
            {
                dl.DL_unlit_instanced_t::rasterizerState = &store.rasterizerStateFill;
                dl.DL_unlit_instanced_t::blendState = &store.blendStateOn;
                dl.DL_unlit_instanced_t::depthStencilState = &store.depthStateReadOnly;
                SET_MARKER_NAME(dl.DL_unlit_instanced_t::markerName, "TRANSPARENT INSTANCED")
                DL_unlit_instanced_ctx::load_dl_technique((DL_unlit_instanced_t&)dl, store.cbuffers);
            }
        }
    }

    namespace FBX {
        struct Vertex_src {
            Vec3 coords;
        };
        typedef tinystl::vector<Vertex_src, Allocator::ArenaSTL<Vertex_src>> Vertices_src_stream;
        template<typename _vertexLayout>
        struct Streams {
            void init(Allocator::Arena* arena, const u32 maxVertices) {
                vertices.set_alloc(arena);
                indices.set_alloc(arena);
                vertices.reserve(maxVertices);
                indices.reserve(maxVertices * 3 * 2);
            }
            tinystl::vector<_vertexLayout, Allocator::ArenaSTL<_vertexLayout>> vertices;
            tinystl::vector<u32, Allocator::ArenaSTL<u32>> indices;
        };
        template<typename _vertexLayout, Shaders::PSMaterialUsage::Enum _materialUsage>
        struct DrawlistSrc;
        template<typename _vertexLayout>
        struct DrawlistSrc<_vertexLayout, Shaders::PSMaterialUsage::None> {
            void init(Allocator::Arena* arena, const u32 maxVertices) {
                streams.init(arena, maxVertices);
            }
            Streams<_vertexLayout> streams;
        };
        template<typename _vertexLayout>
        struct DrawlistSrc<_vertexLayout, Shaders::PSMaterialUsage::Uses> {
            void init(Allocator::Arena* arena, const u32 maxVertices) {
                streams.init(arena, maxVertices);
                user = 0;
            }
            Streams<_vertexLayout> streams;
            void* user; // for textures
        };
        template <typename _vertexLayout>
        void create_vertex_buffer(Driver::RscIndexedBuffer<_vertexLayout>& rscBuffer, _vertexLayout* vertices, const u32 vertices_count, u32* indices, const u32 indices_count) {
            Renderer::Driver::IndexedBufferParams bufferParams;
            bufferParams.vertexData = vertices;
            bufferParams.vertexSize = (u32)vertices_count * sizeof(_vertexLayout);
            bufferParams.indexData = indices;
            bufferParams.indexSize = indices_count * sizeof(u32);
            bufferParams.indexCount = indices_count;
            bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
            bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
            bufferParams.indexType = Renderer::Driver::BufferItemType::U32;
            bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
            Renderer::Driver::create_indexed_vertex_buffer(rscBuffer, bufferParams);
        }
        template<typename _vertexLayout, Shaders::VSSkinType::Enum _skinType, typename _drawlist>
        struct InitVertexBuffer;
        template<typename _vertexLayout, typename _drawlist>
        struct InitVertexBuffer<_vertexLayout, Shaders::VSSkinType::Unskinned, _drawlist> {
            static void op(typename _drawlist::DL_VertexBuffer& dlBuffer, const typename _drawlist::DL_VertexBuffer::GroupData& groupData, const Driver::RscIndexedBuffer<_vertexLayout>& rscBuffer, const Animation::AnimatedNode& animatedNode) {
                dlBuffer.buffer = rscBuffer;
                dlBuffer.groupData = groupData;
            }
        };
        template<typename _vertexLayout, typename _drawlist>
        struct InitVertexBuffer<_vertexLayout, Shaders::VSSkinType::Skinned, _drawlist> {
            static void op(typename _drawlist::DL_VertexBuffer& dlBuffer, const typename _drawlist::DL_VertexBuffer::GroupData& groupData, const Driver::RscIndexedBuffer<_vertexLayout>& rscBuffer, const Animation::AnimatedNode& animatedNode) {
                dlBuffer.buffer = rscBuffer;
                dlBuffer.groupData = groupData;
                dlBuffer.instancedData.resize(animatedNode.skeleton.jointCount);
            }
        };
        template<typename _vertexLayout, Shaders::VSSkinType::Enum _skinType, typename _drawlist>
        static void create_dl_perVertexBuffer(typename _drawlist::DL_VertexBuffer& dlBuffer, Drawlist::Drawlist_node& nodeToAdd, Store& renderStore, Streams<_vertexLayout>& streams, const Animation::AnimatedNode& animatedNode) {
            auto& dst_vertices = streams.vertices;
            auto& dst_indices = streams.indices;
            typename _drawlist::DL_VertexBuffer::GroupData groupData = {};
            groupData.groupColor = Col(0.0f, 0.0f, 0.0f, 0.f).RGBAv4();
            groupData.worldMatrix = Math::mult(nodeToAdd.worldTransform.matrix, nodeToAdd.localTransform.matrix);
            Driver::RscIndexedBuffer<_vertexLayout> rscBuffer = {};
            create_vertex_buffer(rscBuffer, &dst_vertices[0], (u32)dst_vertices.size(), &dst_indices[0], (u32)dst_indices.size());
            InitVertexBuffer<_vertexLayout, _skinType, _drawlist>::op(dlBuffer, groupData, rscBuffer, animatedNode);
        }
        template<typename _vertexLayout, Shaders::VSSkinType::Enum _skinType, Shaders::PSMaterialUsage::Enum _materialUsage, typename _drawlist, typename _handle>
        struct Generate_DL;
        template<typename _vertexLayout, Shaders::VSSkinType::Enum _skinType, typename _drawlist, typename _handle>
        struct Generate_DL<_vertexLayout, _skinType, Shaders::PSMaterialUsage::None, _drawlist, _handle> {
            static void op(Drawlist::Drawlist_node& nodeToAdd, Store& renderStore, DrawlistSrc<_vertexLayout, Shaders::PSMaterialUsage::None>& src, const Animation::AnimatedNode& animatedNode) {
                if (src.streams.vertices.size() == 0) { return; }
                typename _drawlist::DL_VertexBuffer dlBuffer = {};
                create_dl_perVertexBuffer<_vertexLayout, _skinType, _drawlist>(dlBuffer, nodeToAdd, renderStore, src.streams, animatedNode);
                nodeToAdd.handle._handle::id = (s32)renderStore.drawlist._drawlist::dl_perVertexBuffer.size();
                renderStore.drawlist._drawlist::dl_perVertexBuffer.push_back(dlBuffer);
            }
        };
        template<typename _vertexLayout, Shaders::VSSkinType::Enum _skinType, typename _drawlist, typename _handle>
        struct Generate_DL<_vertexLayout, _skinType, Shaders::PSMaterialUsage::Uses, _drawlist, _handle> {
            static void op(Drawlist::Drawlist_node& nodeToAdd, Store& renderStore, DrawlistSrc<_vertexLayout, Shaders::PSMaterialUsage::Uses>& src, const Animation::AnimatedNode& animatedNode) {
                if (src.streams.vertices.size() == 0) { return; }
                typename _drawlist::DL_Material material = {};
                Driver::create_texture_from_file(material.textures[0], { ((ufbx_texture*)src.user)->filename.data });
                {
                    typename _drawlist::DL_VertexBuffer dlBuffer = {};
                    create_dl_perVertexBuffer<_vertexLayout, _skinType, _drawlist>(dlBuffer, nodeToAdd, renderStore, src.streams, animatedNode);
                    nodeToAdd.handle._handle::buffer = (s16)material.dl_perVertexBuffer.size();
                    material.dl_perVertexBuffer.push_back(dlBuffer);
                }
                nodeToAdd.handle._handle::material = (s16)renderStore.drawlist._drawlist::dl_perMaterial.size();
                renderStore.drawlist._drawlist::dl_perMaterial.push_back(material);
            }
        };
        template<typename _vertexLayout, typename _drawlist>
        struct DrawlistSrcSelector {
            using SkinnedType = typename Shaders::SkinnedType<_vertexLayout>;
            using MaterialUsageType = typename Shaders::MaterialUsage<_vertexLayout>;
            using Handle = typename _drawlist::Handle;
            using Src = DrawlistSrc<_vertexLayout, (Shaders::PSMaterialUsage::Enum) MaterialUsageType::type>;
            using Generate_DL = Generate_DL<_vertexLayout, (Shaders::VSSkinType::Enum) SkinnedType::type, (Shaders::PSMaterialUsage::Enum) MaterialUsageType::type, _drawlist, Handle>;

            Src src;
        };
        template<typename... _T>
        struct DrawlistSrcSet : _T... {
            template<typename _Op, typename... _Args>
            static void for_each(_Args&&... args) {
                int dummy[] = { 0, (_Op::template execute<_T>(args...), 0)... };
                (void)dummy;
            }
        };
        typedef DrawlistSrcSelector<Layout_Vec3Color4B, Drawlist::DL_color_t> DL_color_t;
        typedef DrawlistSrcSelector<Layout_Vec3Color4BSkinned, Drawlist::DL_colorskinned_t> DL_colorskinned_t;
        typedef DrawlistSrcSelector<Layout_TexturedSkinnedVec3, Drawlist::DL_textured_t> DL_textured_t;
        typedef DrawlistSrcSelector<Layout_TexturedSkinnedVec3, Drawlist::DL_texturedalphaclip_t> DL_texturedalphaclip_t;
        typedef DrawlistSrcSet<DL_color_t, DL_colorskinned_t, DL_textured_t, DL_texturedalphaclip_t> FBXSupportedDrawlists;

        struct init_drawlists_t {
            template <typename _T>
            static void execute(FBXSupportedDrawlists& drawlist, Allocator::Arena* arena, const u32 maxVertices) {
                drawlist._T::src.init(arena, maxVertices);
            }
        };
        struct process_streams_t {
            template <typename _T>
            static void execute(FBXSupportedDrawlists& drawlist, Drawlist::Drawlist_node& nodeToAdd, Store& renderStore, const Animation::AnimatedNode& animatedNode) {
                _T::Generate_DL::op(nodeToAdd, renderStore, drawlist._T::src, animatedNode);
            }
        };

        template<typename _vertexLayout>
        void extract_skinning_attribs(_vertexLayout& vertex, const ufbx_mesh& mesh, const u32 vertex_id) {
            ufbx_skin_deformer& skin = *(mesh.skin_deformers.data[0]);
            ufbx_skin_vertex& skinned = skin.vertices.data[vertex_id];
            f32 total_weight = 0.f;
            const u32 num_weights = Math::min(skinned.num_weights, 4u);
            u8 bone_index[4]{};
            f32 bone_weights[4]{};
            u32 num_bones = 0;
            for (u32 wi = 0; wi < num_weights; wi++) {
                ufbx_skin_weight weight = skin.weights.data[skinned.weight_begin + wi];
                if (weight.cluster_index >= 255) { continue; }
                total_weight += (f32)weight.weight;
                bone_index[num_bones] = (u8)weight.cluster_index;
                bone_weights[num_bones] = (f32)weight.weight;
                num_bones++;
            }
            if (total_weight > 0.0f) {
                u32 quantized_sum = 0;
                for (size_t i = 0; i < 4; i++) {
                    u8 quantized_weight = (u8)(255.f * (bone_weights[i] / total_weight));
                    quantized_sum += quantized_weight;
                    vertex.joint_indices[i] = bone_index[i];
                    vertex.joint_weights[i] = quantized_weight;
                }
                vertex.joint_weights[0] += 255 - quantized_sum; // if the sum is not 255, adjust first weight
            }
        }
        struct DstDataType { enum Enum { Vertex, Material, Dummy }; };
        template<typename _vertexLayout, DstDataType::Enum _dataType>
        void extract_vertex_attrib(_vertexLayout& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material& mesh_mat, const u32 index_id, const u32 vertex_id);
        template<>
        void extract_vertex_attrib<Layout_TexturedVec3, DstDataType::Vertex>(Layout_TexturedVec3& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material&, const u32 index_id, const u32 vertex_id) {
            ufbx_vec2& uv = mesh.vertex_uv[index_id];
            vertex.uv.x = uv.x;
            vertex.uv.y = uv.y;
        }
        template<>
        void extract_vertex_attrib<Layout_TexturedSkinnedVec3, DstDataType::Vertex>(Layout_TexturedSkinnedVec3& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material&, const u32 index_id, const u32 vertex_id) {
            ufbx_vec2& uv = mesh.vertex_uv[index_id];
            ufbx_vec3& normal = mesh.vertex_normal[index_id];
            vertex.uv.x = uv.x;
            vertex.uv.y = uv.y;
            vertex.normal = Vec3(normal.x, normal.y, normal.z);
            extract_skinning_attribs(vertex, mesh, vertex_id);
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4B, DstDataType::Vertex>(Layout_Vec3Color4B& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material&, const u32 index_id, const u32 vertex_id) {
            ufbx_vec4& c = mesh.vertex_color[index_id];
            vertex.color = Col32(c.x, c.y, c.z, c.w).ABGR();
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4B, DstDataType::Material>(Layout_Vec3Color4B& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material& mesh_mat, const u32 index_id, const u32 vertex_id) {
            ufbx_vec4& c = mesh_mat.material->pbr.base_color.value_vec4;
            vertex.color = Col32(c.x, c.y, c.z, c.w).ABGR();
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4B, DstDataType::Dummy>(Layout_Vec3Color4B& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material& mesh_mat, const u32 index_id, const u32 vertex_id) {
            vertex.color = Col32(1.f, 0.f, 1.f, 0.f).ABGR();
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4BSkinned, DstDataType::Vertex>(Layout_Vec3Color4BSkinned& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material&, const u32 index_id, const u32 vertex_id) {
            ufbx_vec4& c = mesh.vertex_color[index_id];
            vertex.color = Col32(c.x, c.y, c.z, c.w).ABGR();
            extract_skinning_attribs(vertex, mesh, vertex_id);
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4BSkinned, DstDataType::Material>(Layout_Vec3Color4BSkinned& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material& mesh_mat, const u32 index_id, const u32 vertex_id) {
            ufbx_vec4& c = mesh_mat.material->pbr.base_color.value_vec4;
            vertex.color = Col32(c.x, c.y, c.z, c.w).ABGR();
            extract_skinning_attribs(vertex, mesh, vertex_id);
        }
        template<>
        void extract_vertex_attrib<Layout_Vec3Color4BSkinned, DstDataType::Dummy>(Layout_Vec3Color4BSkinned& vertex, const ufbx_mesh& mesh, const ufbx_mesh_material& mesh_mat, const u32 index_id, const u32 vertex_id) {
            vertex.color = Col32(1.f, 0.f, 1.f, 0.f).ABGR();
            extract_skinning_attribs(vertex, mesh, vertex_id);
        }

        template<typename _vertexLayout, DstDataType::Enum _dataType>
        void process_material(const Vertex_src* vertices_src, const u32 vertices_src_count, Streams<_vertexLayout>& stream_dst, ufbx_mesh& mesh, ufbx_mesh_material& mesh_mat, Allocator::Arena scratchArena) {
            u32* index_in_dst_array = (u32*)Allocator::alloc_arena(scratchArena, sizeof(u32) * vertices_src_count, alignof(u32));
            memset(index_in_dst_array, 0xffffffff, sizeof(u32) * vertices_src_count);

            for (size_t f = 0; f < mesh_mat.num_faces; f++) {
                const u32 maxTriIndices = 32;
                u32 triIndices[maxTriIndices];

                ufbx_face& face = mesh.faces.data[mesh_mat.face_indices.data[f]];
                size_t num_tris = ufbx_triangulate_face(triIndices, maxTriIndices, &mesh, face);
                for (size_t v = 0; v < num_tris * 3; v++) {
                    const uint32_t triangulatedFaceIndex = triIndices[v];
                    const u32 src_index = mesh.vertex_indices[triangulatedFaceIndex];

                    const Vertex_src& vertex_src = vertices_src[src_index];
                    if (index_in_dst_array[src_index] != 0xffffffff) {
                        // Vertex has been updated in the new buffer, copy the index over
                        stream_dst.indices.push_back(index_in_dst_array[src_index]);
                    } else {
                        // Vertex needs to be added to the new buffer, and index updated
                        u32 dst_index = (u32)stream_dst.vertices.size();
                        _vertexLayout vertex = {};
                        extract_vertex_attrib<_vertexLayout, _dataType>(vertex, mesh, mesh_mat, triangulatedFaceIndex, src_index);
                        vertex.pos.x = vertex_src.coords.x;
                        vertex.pos.y = vertex_src.coords.y;
                        vertex.pos.z = vertex_src.coords.z;
                        stream_dst.vertices.push_back(vertex);
                        // mark where this vertex is in the destination array
                        index_in_dst_array[src_index] = dst_index;
                        stream_dst.indices.push_back(dst_index);
                    }
                }
            }
        }
        void extract_anim_data(Animation::Skeleton& skeleton, Animation::Clip*& clips, u32& clipCount, const ufbx_mesh& mesh, const ufbx_scene& scene) {

            auto from_ufbx_mat = [](Mat4& dst, const ufbx_matrix& src) {
                dst.col0.x = src.cols[0].x; dst.col0.y = src.cols[0].y; dst.col0.z = src.cols[0].z; dst.col0.w = 0.f;
                dst.col1.x = src.cols[1].x; dst.col1.y = src.cols[1].y; dst.col1.z = src.cols[1].z; dst.col1.w = 0.f;
                dst.col2.x = src.cols[2].x; dst.col2.y = src.cols[2].y; dst.col2.z = src.cols[2].z; dst.col2.w = 0.f;
                dst.col3.x = src.cols[3].x; dst.col3.y = src.cols[3].y; dst.col3.z = src.cols[3].z; dst.col3.w = 1.f;
            };

            ufbx_matrix model_to_joint[128];
            u32 node_indices[COUNT_OF(model_to_joint)];
            u8 jointCount = 0;

            // Extract all matrices model_to_joint
            ufbx_skin_deformer& skin = *(mesh.skin_deformers.data[0]);
            for (size_t ci = 0; ci < skin.clusters.count; ci++) {
                ufbx_skin_cluster* cluster = skin.clusters.data[ci];
                if (jointCount == COUNT_OF(model_to_joint)) { continue; }
                model_to_joint[jointCount] = cluster->geometry_to_bone;
                node_indices[jointCount] = (s8)cluster->bone_node->typed_id;
                jointCount++;
            }

            skeleton.model_to_joint = (Mat4*)malloc(sizeof(Mat4) * jointCount);
            skeleton.joint_to_parent = (Mat4*)malloc(sizeof(Mat4) * jointCount);
            skeleton.parentIndices = (s8*)malloc(sizeof(s8) * jointCount);
            skeleton.jointCount = jointCount;
            // todo: are joints already sorted from root to children? otherwise we'll have to sort them
            for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
                from_ufbx_mat(skeleton.model_to_joint[joint_index], model_to_joint[joint_index]);
                ufbx_node& node = *(scene.nodes.data[node_indices[joint_index]]);
                if (node.parent) {
                    skeleton.parentIndices[joint_index] = node.parent->typed_id;
                    from_ufbx_mat(skeleton.joint_to_parent[joint_index], node.node_to_parent);
                }
                else {
                    skeleton.parentIndices[joint_index] = -1;
                    from_ufbx_mat(skeleton.joint_to_parent[joint_index], node.node_to_world);
                }
            }
            
            // anim clip
            clipCount = (u32)scene.anim_stacks.count;
            clips = (Animation::Clip*)malloc(sizeof(Animation::Clip) * scene.anim_stacks.count);
            for (size_t i = 0; i < clipCount; i++) {
                const ufbx_anim_stack& stack = *(scene.anim_stacks.data[i]);
                const f64 targetFramerate = 30.f;
                const u32 maxFrames = 4096;
                const f64 duration = (f32)stack.time_end - (f32)stack.time_begin;
                const u32 numFrames = Math::clamp((u32)(duration * targetFramerate), 2u, maxFrames);
                const f64 framerate = (numFrames-1) / duration;

                Animation::Clip& clip = clips[i];
                clip.frameCount = numFrames;
                clip.frames = (Animation::Frame*)malloc(sizeof(Animation::Frame) * numFrames);
                for (u32 f = 0; f < numFrames; f++) {
                    Animation::Frame& frame = clip.frames[f];
                    frame.joint_to_parent = (Mat4*)malloc(sizeof(Mat4) * jointCount);
                    for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
                        f64 time = stack.time_begin + (double)i / framerate;
                        ufbx_node* node = scene.nodes.data[node_indices[joint_index]];
                        ufbx_transform transform = ufbx_evaluate_transform(&stack.anim, node, time);
                        from_ufbx_mat(frame.joint_to_parent[i], ufbx_transform_to_matrix(&transform));
                    }
                }
            }
        }
        void load_with_material(Drawlist::Drawlist_node& nodeToAdd, Store& renderStore, const char* path, Allocator::Arena scratchArena) {
            ufbx_load_opts opts = {};
            opts.target_axes = { UFBX_COORDINATE_AXIS_POSITIVE_X, UFBX_COORDINATE_AXIS_POSITIVE_Z, UFBX_COORDINATE_AXIS_POSITIVE_Y };
            //opts.evaluate_skinning = true;
            opts.allow_null_material = true;
            ufbx_error error;
            ufbx_scene* scene = ufbx_load_file(path, &opts, &error);
            if (scene) {
                // We'll process all vertices first, and then split then into the supported material buffers
                u32 maxVertices = 0;
                for (size_t i = 0; i < scene->meshes.count; i++) { maxVertices += (u32)scene->meshes.data[i]->num_vertices; }

                Vertices_src_stream vertices;
                vertices.set_alloc(&scratchArena);
                vertices.reserve(maxVertices);
                FBXSupportedDrawlists supportedDrawlists;
                FBXSupportedDrawlists::for_each<init_drawlists_t>(supportedDrawlists, &scratchArena, maxVertices);

                // hack: only consider skinning from first mesh
                Animation::AnimatedNode animatedNode = {};
                if (scene->meshes.count && scene->meshes[0]->skin_deformers.count) {
                    extract_anim_data(animatedNode.skeleton, animatedNode.clips, animatedNode.clipCount, *(scene->meshes[0]), *scene);
                }

                u32 vertexOffset = 0;
                for (size_t i = 0; i < scene->meshes.count; i++) {
                    ufbx_mesh& mesh = *scene->meshes.data[i];

                    // Extract vertices from this mesh and flatten any transform trees
                    // This assumes there's only one instance of this mesh, we don't support more at the moment
                    {
                        // todo: consider asset transform
                        assert(mesh.instances.count == 1);
                        const ufbx_matrix& m = mesh.instances.data[0]->geometry_to_world;
                        for (size_t v = 0; v < mesh.num_vertices; v++) {
                            Vertex_src vertex;
                            const ufbx_vec3& v_fbx_ls = mesh.vertices[v];
                            vertex.coords.x = v_fbx_ls.x * m.m00 + v_fbx_ls.y * m.m01 + v_fbx_ls.z * m.m02 + m.m03;
                            vertex.coords.y = v_fbx_ls.x * m.m10 + v_fbx_ls.y * m.m11 + v_fbx_ls.z * m.m12 + m.m13;
                            vertex.coords.z = v_fbx_ls.x * m.m20 + v_fbx_ls.y * m.m21 + v_fbx_ls.z * m.m22 + m.m23;
                            vertices.push_back(vertex);
                        }
                    }

                    // Extract all indices, separated by supported drawlist
                    // They will all point the same vertex buffer array, which is not ideal, but I don't want to deal with
                    // re-building a vertex buffer for each index buffer

                    auto& streamColor = supportedDrawlists.DL_color_t::src;
                    auto& streamColorSkinned = supportedDrawlists.DL_colorskinned_t::src;
                    auto& streamTexturedSkinned = supportedDrawlists.DL_textured_t::src;
                    auto& streamTexturedSkinnedAlphaClip = supportedDrawlists.DL_texturedalphaclip_t::src;

                    const u32 mesh_vertices_count = (u32)mesh.num_vertices;
                    const Vertex_src* mesh_vertices = &vertices[vertexOffset];
                    for (size_t m = 0; m < mesh.materials.count; m++) {
                        ufbx_mesh_material& mesh_mat = mesh.materials.data[m];
                        if (animatedNode.skeleton.jointCount) { // hack: handle textured shaders only for skinned meshes
                            if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                                if (mesh_mat.material->pbr.base_color.texture_enabled) {
                                    // Assume that opacity tied to a texture means that we should use the alpha clip shader
                                    if (mesh_mat.material->pbr.opacity.texture_enabled) {
                                        assert(!streamTexturedSkinnedAlphaClip.user); // we only support one textured-material of each pass
                                        process_material<Layout_TexturedSkinnedVec3, DstDataType::Vertex>(mesh_vertices, mesh_vertices_count, streamTexturedSkinnedAlphaClip.streams, mesh, mesh_mat, scratchArena);
                                        streamTexturedSkinnedAlphaClip.user = mesh_mat.material->pbr.base_color.texture;
                                    } else {
                                        assert(!streamTexturedSkinned.user); // we only support one textured-material of each pass
                                        process_material<Layout_TexturedSkinnedVec3, DstDataType::Vertex>(mesh_vertices, mesh_vertices_count, streamTexturedSkinned.streams, mesh, mesh_mat, scratchArena);
                                        streamTexturedSkinned.user = mesh_mat.material->pbr.base_color.texture;
                                    }
                                } else {
                                    process_material<Layout_Vec3Color4BSkinned, DstDataType::Material>(mesh_vertices, mesh_vertices_count, streamColorSkinned.streams, mesh, mesh_mat, scratchArena);
                                }
                            } else {
                                if (mesh.vertex_color.exists) {
                                    process_material<Layout_Vec3Color4BSkinned, DstDataType::Vertex>(mesh_vertices, mesh_vertices_count, streamColorSkinned.streams, mesh, mesh_mat, scratchArena);
                                } else {
                                    process_material<Layout_Vec3Color4BSkinned, DstDataType::Dummy>(mesh_vertices, mesh_vertices_count, streamColorSkinned.streams, mesh, mesh_mat, scratchArena);
                                }
                            }
                        } else {
                            if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                                process_material<Layout_Vec3Color4B, DstDataType::Material>(mesh_vertices, mesh_vertices_count, streamColor.streams, mesh, mesh_mat, scratchArena);
                            } else if (mesh.vertex_color.exists) {
                                process_material<Layout_Vec3Color4B, DstDataType::Vertex>(mesh_vertices, mesh_vertices_count, streamColor.streams, mesh, mesh_mat, scratchArena);
                            } else {
                                process_material<Layout_Vec3Color4B, DstDataType::Dummy>(mesh_vertices, mesh_vertices_count, streamColor.streams, mesh, mesh_mat, scratchArena);
                            }
                        }
                    }
                    vertexOffset += (u32)mesh.num_vertices;
                }

                {
                    FBXSupportedDrawlists::for_each<process_streams_t>(supportedDrawlists, nodeToAdd, renderStore, animatedNode);
                    if (animatedNode.skeleton.jointCount) {
                        animatedNode.mesh_DLhandle = nodeToAdd.handle;
                        renderStore.animated_nodes.push_back(animatedNode);
                    }
                    renderStore.drawlist_nodes.push_back(nodeToAdd);
                }
            }
        }
    }
}

#endif // __WASTELADNS_GAMERENDERER_H__
