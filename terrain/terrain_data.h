#ifndef _TERRAIN_HEIGHTMAP_H
#define _TERRAIN_HEIGHTMAP_H

#include "resource.h"
#include "dictionary.h"
#include "servers/visual_server.h"

class TerrainData : public Resource {
    OBJ_TYPE(TerrainData, Resource)
    RES_BASE_EXTENSION("hmap");

public:
    TerrainData();
    ~TerrainData();

    void set_size(const int new_size);
    int get_size() const;

    Image get_blends() const;
    Image get_heights() const;

    RID get_blends_texture() const;
    RID get_heights_texture() const;

    void reload_heights();
    void reload_blends();

    void paint_blend(const Image& brush, int x, int y, int texture, float alpha);
    void paint_height(const Image& brush, int x, int y, float alpha);

    float get_height_at(int x, int y);
    void set_height_at(int x, int y, float h);
private:
    int m_size;
    Image m_blends;
    Image m_heights;
    RID m_blends_tex;
    RID m_heights_tex;

    void _size_changed();

protected:
    void _set_data(Dictionary data);
    Dictionary _get_data() const;
    static void _bind_methods();

};

#endif // _TERRAIN_HEIGHTMAP_H
