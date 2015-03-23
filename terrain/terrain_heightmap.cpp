#include "terrain_heightmap.h"

TerrainHeightmap::TerrainHeightmap()
{
    m_size = 0;
    m_flags = VS::TEXTURE_FLAG_FILTER;
    m_texture = VS::get_singleton()->texture_create();
    m_blends.create(m_size, m_size, false, Image::FORMAT_RGBA);
}

TerrainHeightmap::~TerrainHeightmap()
{
    VS::get_singleton()->free(m_texture);
}

void TerrainHeightmap::set_size(int new_size)
{
    m_size = new_size;

    _size_changed();
}

float TerrainHeightmap::get_height(int x, int y)
{
    int offset = y * m_size + x;

    return m_heights[offset];
}

void TerrainHeightmap::set_height(int x, int y, float height)
{
    int offset = y * m_size + x;

    DVector<float>::Write w = m_heights.write();
    float* data = w.ptr();

    data[offset] = height;
}

void TerrainHeightmap::blit_height(DVector<float>& data, int x1, int y1, int x2, int y2)
{
    DVector<float>::Write w = m_heights.write();

    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= m_size) {
        x2 = m_size;
    }
    if (y2 >= m_size) {
        y2 = m_size;
    }

    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
            w[j * m_size + i] = data[(j - y1) * (x2 - x1) + (i - x1)];
        }
    }
}

void TerrainHeightmap::blend_height(DVector<float>& data, int x1, int y1, int x2, int y2, float alpha)
{
    DVector<float>::Write w = m_heights.write();

    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= m_size) {
        x2 = m_size;
    }
    if (y2 >= m_size) {
        y2 = m_size;
    }

    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
            float src = data[(j - y1) * (x2 - x1) + (i - x1)];
            float curr = w[j * m_size + i];
            w[j * m_size + i] = curr + src * alpha;
        }
    }
}

Color TerrainHeightmap::get_blend(int x, int y)
{
    if (m_blends.empty()) {
        return Color();
    }

    return m_blends.get_pixel(x, y);
}

void TerrainHeightmap::set_blend(int x, int y, Color c)
{
    if (m_blends.empty()) {
        return;
    }

    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x > m_blends.get_width())
        x = m_blends.get_width();
    if (y > m_blends.get_height())
        y = m_blends.get_height();

    m_blends.put_pixel(x, y, c);

    VS::get_singleton()->texture_set_data(m_texture, m_blends);
}

Image TerrainHeightmap::get_blends()
{
    return m_blends;
}

RID TerrainHeightmap::get_texture()
{
    return m_texture;
}

void TerrainHeightmap::_size_changed()
{
    m_heights.resize((m_size + 1) * (m_size + 1));
    m_blends.create(m_size, m_size, false, Image::FORMAT_RGBA);

    DVector<float>::Write w = m_heights.write();
    for (int i = 0; i < m_heights.size(); i++) {
        w[i] = 0.0f;
    }

    VS::get_singleton()->texture_allocate(m_texture, m_size, m_size, m_blends.get_format(), m_flags);
    VS::get_singleton()->texture_set_data(m_texture, m_blends);

    emit_signal(String("size_changed"));
}

int TerrainHeightmap::get_size() const
{
    return m_size;
}

void TerrainHeightmap::_set_data(Dictionary data)
{
    m_size = data["size"];
    m_heights = data["data"];
    m_blends = data["blends"];

    VS::get_singleton()->texture_allocate(m_texture, m_size, m_size, m_blends.get_format(), m_flags);
    VS::get_singleton()->texture_set_data(m_texture, m_blends);
}

Dictionary TerrainHeightmap::_get_data()
{
    Dictionary d;

    d["size"] = m_size;
    d["data"] = m_heights;
    d["blends"] = m_blends;

    return d;
}

void TerrainHeightmap::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("get_height", "x", "y"), &TerrainHeightmap::get_height);
    ObjectTypeDB::bind_method(_MD("set_height", "x", "y", "height"), &TerrainHeightmap::set_height);
    ObjectTypeDB::bind_method(_MD("get_size"), &TerrainHeightmap::get_size);
    ObjectTypeDB::bind_method(_MD("set_size", "size"), &TerrainHeightmap::set_size);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "size"), _SCS("set_size"), _SCS("get_size"));

    ObjectTypeDB::bind_method(_MD("_set_data", "data"), &TerrainHeightmap::_set_data);
    ObjectTypeDB::bind_method(_MD("_get_data"), &TerrainHeightmap::_get_data);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), _SCS("_set_data"), _SCS("_get_data"));

    ADD_SIGNAL(MethodInfo("size_changed"));
}
