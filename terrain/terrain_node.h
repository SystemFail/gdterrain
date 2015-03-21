#ifndef _TERRAIN_NODE_H
#define _TERRAIN_NODE_H

#include "rid.h"
#include "scene/3d/spatial.h"
#include "scene/resources/material.h"
#include "terrain_heightmap.h"


class TerrainNode : public Spatial {
    OBJ_TYPE(TerrainNode, Spatial)

    struct Chunk {
        RID mesh;
        RID instance;
        bool surface_added;
        bool mesh_dirty;
        bool material_dirty;
    };

public:
    TerrainNode();
    ~TerrainNode();

    void _notification(int what);
    static void _bind_methods();

    void set_heightmap(const Ref<TerrainHeightmap>& heightmap);
    Ref<TerrainHeightmap> get_heightmap() const;

    void set_material(const Ref<Material>& material);
    Ref<Material> get_material() const;

    void set_scale(float scale);
    float get_scale();

    unsigned int get_pixel_x_at(Vector3 pos);
    unsigned int get_pixel_y_at(Vector3 pos);
    float get_height_at(Vector3 pos);

    void modify_height_at(unsigned int x, unsigned int y, float height);
    unsigned int get_chunk_offset_at(unsigned int x, unsigned int y);
    void invalidate_chunks(unsigned int x, unsigned int y);
    bool is_inside_chunk(unsigned int offset, unsigned int x, unsigned int y);

    unsigned int fix_ray_x(Vector3 origin, unsigned int x, unsigned int y);
    unsigned int fix_ray_y(Vector3 origin, unsigned int x, unsigned int y);

    void blit(DVector<float>& pixels, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void blend(DVector<float>& pixels, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float alpha);

private:
    void _update();

    void _update_chunk(unsigned int ch_offset);
    void _create_chunk(unsigned int offset);
    void _delete_chunk(unsigned int offset);
    void _transform_chunk(unsigned int offset);
    void _clear_chunk(unsigned int offset);
    void _chunk_set_material();

    void _update_chunks();
    void _update_material();
    void _chunks_make_dirty();

    Ref<TerrainHeightmap> m_heightmap;
    Ref<Texture> m_blend_map;
    Ref<Material> m_material;

    float m_scale;
    unsigned int m_chunk_size;
    unsigned int m_chunks_w; // wertical size of chunk array
    unsigned int m_chunks_h; // horizontal size of chunk array
    DVector<Chunk> m_chunks;

    bool m_chunks_dirty;
    bool m_chunks_created;
};

#endif
