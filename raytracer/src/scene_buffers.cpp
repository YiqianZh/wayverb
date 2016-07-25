#include "raytracer/scene_buffers.h"

#include "common/conversions.h"

namespace {
template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool readonly) {
    return cl::Buffer(context, std::begin(t), std::end(t), readonly);
};
}  // namespace

namespace raytracer {

scene_buffers::scene_buffers(const cl::Context& context,
                             const cl::Device& device,
                             const CopyableSceneData& scene_data)
        : scene_buffers(context,
                        device,
                        scene_data,
                        VoxelCollection(scene_data, 4, 0.1)) {}

scene_buffers::scene_buffers(const cl::Context& context,
                             const cl::Device& device,
                             const CopyableSceneData& scene_data,
                             const VoxelCollection& vox)
        : queue(context, device)
        , voxel_index_buffer(load_to_buffer(context, vox.get_flattened(), true))
        , global_aabb(AABB{to_cl_float3(vox.get_aabb().get_c0()),
                           to_cl_float3(vox.get_aabb().get_c1())})
        , side(vox.get_side())
        , triangles_buffer(
                  load_to_buffer(context, scene_data.get_triangles(), true))
        , vertices_buffer(
                  load_to_buffer(context, scene_data.get_vertices(), true))
        , surfaces_buffer(
                  load_to_buffer(context, scene_data.get_surfaces(), true)) {}

const cl::Buffer& scene_buffers::get_voxel_index_buffer() const {
    return voxel_index_buffer;
}

AABB scene_buffers::get_global_aabb() const { return global_aabb; }

cl_ulong scene_buffers::get_side() const { return side; }

const cl::Buffer& scene_buffers::get_triangles_buffer() const {
    return triangles_buffer;
}

const cl::Buffer& scene_buffers::get_vertices_buffer() const {
    return vertices_buffer;
}

const cl::Buffer& scene_buffers::get_surfaces_buffer() const {
    return surfaces_buffer;
}

cl::CommandQueue& scene_buffers::get_queue() { return queue; }
const cl::CommandQueue& scene_buffers::get_queue() const { return queue; }

}  // namespace raytracer