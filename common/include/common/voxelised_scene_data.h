#pragma once

#include "common/scene_data.h"
#include "common/voxel_collection.h"

class voxelised_scene_data final {
public:
    voxelised_scene_data(const copyable_scene_data& scene_data,
                         size_t octree_depth,
                         const geo::box& aabb);

    const copyable_scene_data& get_scene_data() const;
    const voxel_collection<3>& get_voxels() const;

private:
    copyable_scene_data scene_data;
    voxel_collection<3> voxels;
};

//----------------------------------------------------------------------------//

geo::intersection intersects(const voxelised_scene_data& voxelised,
                             const geo::ray& ray);

bool inside(const voxelised_scene_data& voxelised, const glm::vec3& pt);
