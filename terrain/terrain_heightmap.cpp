#include "terrain_heightmap.h"

TerrainHeightmap::TerrainHeightmap()
{
    m_width = 513;
    m_height = 513;

    _resize();
}

TerrainHeightmap::~TerrainHeightmap()
{
}

void TerrainHeightmap::set_width(unsigned int new_width)
{
    m_width = new_width;

    _resize();
}

void TerrainHeightmap::set_height(unsigned int new_height)
{
    m_height = new_height;

    _resize();
}

unsigned int TerrainHeightmap::get_width()
{
    return m_width;
}

unsigned int TerrainHeightmap::get_height()
{
    return m_height;
}

float TerrainHeightmap::get_pixel(unsigned int x, unsigned int y)
{
    unsigned int offset = y * m_width + x;

    return m_pixels[offset];
}

void TerrainHeightmap::put_pixel(unsigned int x, unsigned int y, float height)
{
    unsigned int offset = y * m_width + x;

    DVector<float>::Write w = m_pixels.write();
    float* data = w.ptr();

    data[offset] = height;
}

void TerrainHeightmap::blit(DVector<float>& data, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    DVector<float>::Write w = m_pixels.write();

    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 > m_width)
        x2 = m_width;
    if (y2 > m_height)
        y2 = m_height;

    for (unsigned int i = x1; i < x2; i++) {
        for (unsigned int j = y1; j < y2; j++) {
            w[j * m_width + i] = data[(j - y1) * (x2 - x1) + (i - x1)];
        }
    }
}

void TerrainHeightmap::blend(DVector<float>& data, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float alpha)
{
    DVector<float>::Write w = m_pixels.write();

    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 > m_width)
        x2 = m_width;
    if (y2 > m_height)
        y2 = m_height;

    for (unsigned int i = x1; i < x2; i++) {
        for (unsigned int j = y1; j < y2; j++) {
            float src = data[(j - y1) * (x2 - x1) + (i - x1)];
            float curr = w[j * m_width + i];
            w[j * m_width + i] = curr + src * alpha;
        }
    }
}

void TerrainHeightmap::_resize()
{
    m_pixels.resize(m_width * m_height);

    DVector<float>::Write w = m_pixels.write();
    for (unsigned int i = 0; i < m_pixels.size(); i++) {
        w[i] = 0.0f;
    }
}

Size2 TerrainHeightmap::get_size() const {

    return Size2(m_width, m_height);
}

void TerrainHeightmap::_set_data(const Dictionary &data)
{
    ERR_FAIL_COND(!data.has("size"));
    ERR_FAIL_COND(!data.has("data"));

    create(data["size"]);
    m_pixels = data["data"];
}

Dictionary TerrainHeightmap::_get_data() const
{
    Dictionary d;

    d["size"] = get_size();
    d["data"] = m_pixels;

    return d;
}

void TerrainHeightmap::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("get_pixel", "x", "y"),
        &TerrainHeightmap::get_pixel);

    ObjectTypeDB::bind_method(_MD("put_pixel", "x", "y", "height"),
        &TerrainHeightmap::put_pixel);

    ObjectTypeDB::bind_method(_MD("get_width"),
        &TerrainHeightmap::get_width);

    ObjectTypeDB::bind_method(_MD("get_height"),
        &TerrainHeightmap::get_height);

    ObjectTypeDB::bind_method(_MD("set_width", "width"),
        &TerrainHeightmap::set_width);

    ObjectTypeDB::bind_method(_MD("set_height", "height"),
        &TerrainHeightmap::set_height);

    ADD_PROPERTY(
        PropertyInfo(Variant::REAL, "width"),
        _SCS("set_width"), _SCS("get_width"));

    ADD_PROPERTY(
        PropertyInfo(Variant::REAL, "height"),
        _SCS("set_height"), _SCS("get_height"));

    ObjectTypeDB::bind_method(_MD("_set_data"),
        &TerrainHeightmap::_set_data);
    ObjectTypeDB::bind_method(_MD("_get_data"),
        &TerrainHeightmap::_get_data);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), _SCS("_set_data"), _SCS("_get_data"));
}

void TerrainHeightmap::create(const Size2 &p_size)
{
    ERR_FAIL_COND(p_size.width < 1);
    ERR_FAIL_COND(p_size.height < 1);

    m_width = p_size.width;
    m_height = p_size.height;

    _resize();
}
