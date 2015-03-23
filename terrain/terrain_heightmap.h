#ifndef _TERRAIN_HEIGHTMAP_H
#define _TERRAIN_HEIGHTMAP_H

#include "resource.h"
#include "dictionary.h"
#include "servers/visual_server.h"

class TerrainHeightmap : public Resource {
    OBJ_TYPE(TerrainHeightmap, Resource)
    RES_BASE_EXTENSION("hmap");

public:
    TerrainHeightmap();
    ~TerrainHeightmap();

    void set_size(int new_size);
    int get_size() const;

    float get_height(int x, int y);
    void set_height(int x, int y, float height);
    void blit_height(DVector<float>& data, int x1, int y1, int x2, int y2);
    void blend_height(DVector<float>& data, int x1, int y1, int x2, int y2, float alpha);

    Color get_blend(int x, int y);
    void set_blend(int x, int y, Color c);

    Image get_blends();
    RID get_texture();

private:
    int m_size;
    DVector<float> m_heights;
    Image m_blends;
    RID m_texture;
    uint32_t m_flags;

    void _size_changed();

protected:
    void _set_data(Dictionary data);
    Dictionary _get_data();
    static void _bind_methods();

};

#endif // _TERRAIN_HEIGHTMAP_H
