#include "terrain_blendmap.h"

TerrainBlendmap::TerrainBlendmap()
{
    m_width = 0;
    m_height = 0;
    m_texture = VS::get_singleton()->texture_create();
    m_image.create(m_width, m_height, false, Image::FORMAT_RGBA);
    VS::get_singleton()->texture_set_data(m_texture, m_image);
}

TerrainBlendmap::~TerrainBlendmap()
{
    VS::get_singleton()->free(m_texture);
}

void TerrainBlendmap::create_from_image(Image &image, uint32_t flags)
{
    m_flags = flags;
    m_width = image.get_width();
    m_height = image.get_height();

    VS::get_singleton()->texture_allocate(m_texture, m_width, m_height, image.get_format(),  flags);
    VS::get_singleton()->texture_set_data(m_texture, image);
}

void TerrainBlendmap::set_image(const Image& data)
{
    m_image = data;
    VS::get_singleton()->texture_set_data(m_texture, m_image);
}

Image TerrainBlendmap::get_image() const
{
    return m_image;
}

RID TerrainBlendmap::get_rid() const
{
    return m_texture;
}

void TerrainBlendmap::set_size(Size2i new_size)
{
    m_width = new_size.width;
    m_height = new_size.height;

    _size_changed();
}

Size2i TerrainBlendmap::get_size() const
{
    return Size2i(m_width, m_height);
}

void TerrainBlendmap::_size_changed()
{
    m_image.resize(m_width, m_height);

    VS::get_singleton()->texture_set_data(m_texture, m_image);
}

void TerrainBlendmap::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("_set_data"), &TerrainBlendmap::_set_data);
    ObjectTypeDB::bind_method(_MD("_get_data"), &TerrainBlendmap::_get_data);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), _SCS("_set_data"), _SCS("_get_data"));
}

void TerrainBlendmap::_set_data(Dictionary data)
{
    Image img = data["image"];
    uint32_t flags = data["flags"];

    create_from_image(img, flags);

    m_width = img.get_width();
    m_height = img.get_height();
}

Dictionary TerrainBlendmap::_get_data() const
{
    Dictionary d;

    d["image"] = get_image();
    d["flags"] = m_flags;

    return d;
}
