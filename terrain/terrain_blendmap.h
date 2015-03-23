#ifndef TERRAINBLENDMAP_H
#define TERRAINBLENDMAP_H

#include "resource.h"
#include "servers/visual_server.h"

class TerrainBlendmap : public Resource {
    OBJ_TYPE(TerrainBlendmap, Resource)
    RES_BASE_EXTENSION("bmap")

public:
    TerrainBlendmap();
    ~TerrainBlendmap();

    void create_from_image(Image& image, uint32_t flags);

    void set_image(const Image& data);
    Image get_image() const;

    virtual RID get_rid() const;

    void set_size(Size2i new_size);
    Size2i get_size() const;

private:
    RID m_texture;
    Image m_image;
    uint32_t m_flags;
    unsigned int m_width;
    unsigned int m_height;

    void _size_changed();

protected:
    static void _bind_methods();
    void _set_data(Dictionary data);
    Dictionary _get_data() const;
};

#endif // TERRAINBLENDMAP_H
