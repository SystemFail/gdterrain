#ifndef _TERRAIN_HEIGHTMAP_H
#define _TERRAIN_HEIGHTMAP_H

#include "resource.h"
#include "dictionary.h"

class TerrainHeightmap : public Resource {
    OBJ_TYPE(TerrainHeightmap, Resource)
    RES_BASE_EXTENSION("hmap");

public:
    TerrainHeightmap();
    ~TerrainHeightmap();

    static void _bind_methods();

    void create(const Size2& p_size);

    void set_width(unsigned int new_width);
    void set_height(unsigned int new_height);

    unsigned int get_width();
    unsigned int get_height();

    Size2 get_size() const;

    float get_pixel(unsigned int x, unsigned int y);
    void put_pixel(unsigned int x, unsigned int y, float height);
    void blit(DVector<float>& data, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void blend(DVector<float>& data, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float alpha);

private:
    unsigned int m_width;
    unsigned int m_height;
    DVector<float> m_pixels;

    void _resize();
protected:
    void _set_data(const Dictionary& data);
    Dictionary _get_data() const;
};

#endif // _TERRAIN_HEIGHTMAP_H
