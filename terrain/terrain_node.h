#ifndef _TERRAIN_NODE_H
#define _TERRAIN_NODE_H

#include "rid.h"
#include "scene/3d/spatial.h"
#include "terrain_data.h"
#include "scene/resources/texture.h"
#include "os/os.h"

class Clock {
    uint32_t m_time;

public:
    Clock()
    {
        m_time = OS::get_singleton()->get_ticks_msec();
    }

    void check(String s)
    {
        m_time = OS::get_singleton()->get_ticks_msec() - m_time;
        print_line(s + ": " + itos(m_time) + " ms.");
    }

    ~Clock()
    {
    }
};

class TerrainNode : public Spatial {
    OBJ_TYPE(TerrainNode, Spatial)

    struct Chunk {
        RID mesh;
        RID instance;
        RID shape;
        bool surface_added;
        bool mesh_dirty;
        bool material_dirty;
        bool blend_dirty;
    };

public:
    TerrainNode();
    virtual ~TerrainNode();

    void set_data(const Ref<TerrainData>& heightmap);
    Ref<TerrainData> get_data() const;

    void set_texture0(const Ref<Texture>& texture);
    void set_texture1(const Ref<Texture>& texture);
    void set_texture2(const Ref<Texture>& texture);
    void set_texture3(const Ref<Texture>& texture);
    void set_texture4(const Ref<Texture>& texture);

    Ref<Texture> get_texture0() const;
    Ref<Texture> get_texture1() const;
    Ref<Texture> get_texture2() const;
    Ref<Texture> get_texture3() const;
    Ref<Texture> get_texture4() const;

    void set_chunk_scale(const float scale);
    float get_chunk_scale() const;

    void set_uv_scale(const float scale);
    float get_uv_scale() const;

    int get_pixel_x_at(const Vector3 pos, const float offset) const;
    int get_pixel_y_at(const Vector3 pos, const float offset) const;

    void mark_height_dirty(int x, int y);

    void update_dirty_chunks();

private:
    void _create_chunk(int offset);
    void _delete_chunk(int offset);
    void _update_chunk_mesh(int ch_offset);
    void _update_chunk_transform(int offset);
    void _update_chunk_blendmap(int offset);
    void _update_chunk_material(int offset);

    void _mark_blend_dirty(int x, int y);

    int get_chunk_offset_at(int x, int y);
    bool is_hmap_pixel_inside_chunk(int offset, int x, int y);

    void _update_material();
    void _chunks_mark_all_dirty();

    void _blendmap_changed();
    void _heightmap_changed();

    Ref<TerrainData> m_data;

    Ref<Texture> m_texture0;
    Ref<Texture> m_texture1;
    Ref<Texture> m_texture2;
    Ref<Texture> m_texture3;
    Ref<Texture> m_texture4;

    RID m_material;
    RID m_shader;

    float m_scale;
    float m_uv_scale;
    int m_chunk_size;
    int m_chunk_count;
    DVector<Chunk> m_chunks;

    bool m_chunks_dirty;
    bool m_chunks_created;

    /* physics */

    bool m_generate_collisions;
    RID m_body;

protected:
    void _notification(int what);
    static void _bind_methods();
    void _size_changed();
};

#endif
