#include "terrain_data.h"

#define HEIGHT_SCALE 1000.0f

TerrainData::TerrainData()
{
    m_size = 0;
    m_blends_tex = VS::get_singleton()->texture_create();
    m_heights_tex = VS::get_singleton()->texture_create();
}

TerrainData::~TerrainData()
{
    VS::get_singleton()->free(m_blends_tex);
    VS::get_singleton()->free(m_heights_tex);
}

void TerrainData::set_size(const int new_size)
{
    if (new_size == m_size) {
        return;
    }

    m_size = new_size;

    _size_changed();
}

Image TerrainData::get_blends() const
{
    return m_blends;
}

Image TerrainData::get_heights() const
{
    return m_heights;
}

RID TerrainData::get_blends_texture() const
{
    return m_blends_tex;
}

RID TerrainData::get_heights_texture() const
{
    return m_heights_tex;
}

void TerrainData::reload_heights()
{
    VS::get_singleton()->texture_set_data(m_heights_tex, m_heights);
}

void TerrainData::reload_blends()
{
    VS::get_singleton()->texture_set_data(m_blends_tex, m_blends);
}


void TerrainData::paint_blend(const Image& brush, int x, int y, int texture, float alpha)
{
    if (brush.empty()) {
        return;
    }

    int brush_size = brush.get_width();

    Color modulate(0, 0, 0, 0);

    switch(texture) {
    case 1:
        modulate.r = alpha;
        break;
    case 2:
        modulate.g = alpha;
        break;
    case 3:
        modulate.b = alpha;
        break;
    case 4:
        modulate.a = alpha;
        break;
    case 0:
        modulate.r = -alpha;
        modulate.g = -alpha;
        modulate.b = -alpha;
        modulate.a = -alpha;
        break;
    }

    for (int i = x; i < x + brush_size; i++) {
        for (int j = y; j < y + brush_size; j ++) {
            float mask = brush.get_pixel(i - x, j - y).gray();
            Color curr = m_blends.get_pixel(i, j);

            m_blends.put_pixel(i, j, curr.linear_interpolate(modulate, mask));
        }
    }

    VS::get_singleton()->texture_set_data(m_blends_tex, m_blends);
}

void TerrainData::paint_height(const Image &brush, int x, int y, float alpha)
{
    if (brush.empty()) {
        return;
    }

    int brush_size = brush.get_width();

    for (int i = x; i < x + brush_size; i++) {
        for (int j = y; j < y + brush_size; j ++) {
            float mask = brush.get_pixel(i - x, j - y).gray();
            float height = get_height_at(i, j);
            float result = (height + mask * alpha);

            if (result > 65535 / HEIGHT_SCALE) {
                result = 65535 / HEIGHT_SCALE;
            }

            if (result < 0) {
                result = 0;
            }

            set_height_at(i, j, result);
        }
    }

    VS::get_singleton()->texture_set_data(m_heights_tex, m_heights);
}

float TerrainData::get_height_at(int x, int y)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x > m_size) x = m_size;
    if (y > m_size) y = m_size;

    int offset = y * (m_size + 1) + x;

    uint8_t a = m_heights.get_data().get(offset * 2);
    uint8_t b = m_heights.get_data().get(offset * 2 + 1);
    uint16_t c = (a << 8) + b;

    //65535
    //6553.5
    //655.35
    //65.535

    return (float)c / HEIGHT_SCALE;
}

void TerrainData::set_height_at(int x, int y, float h)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x > m_size + 1) x = m_size + 1;
    if (y > m_size + 1) y = m_size + 1;

    uint16_t h16 = h * HEIGHT_SCALE;

    int r = (uint8_t)(h16 >> 8);
    int a = (uint8_t)h16;

    m_heights.put_pixel(x, y, Color(r / 255.0f, r / 255.0f, r / 255.0f, a / 255.0f));
}

void TerrainData::_size_changed()
{
    m_heights.create(m_size + 1, m_size + 1, false, Image::FORMAT_GRAYSCALE_ALPHA);
    m_blends.create(m_size, m_size, false, Image::FORMAT_RGBA);

    VS::get_singleton()->texture_allocate(m_heights_tex, m_size + 1, m_size + 1, m_heights.get_format(), 0);
    VS::get_singleton()->texture_allocate(m_blends_tex, m_size, m_size, m_blends.get_format(), VS::TEXTURE_FLAG_FILTER);
    VS::get_singleton()->texture_set_data(m_blends_tex, m_blends);
    VS::get_singleton()->texture_set_data(m_heights_tex, m_heights);

    emit_signal(String("size_changed"));
}

int TerrainData::get_size() const
{
    return m_size;
}

void TerrainData::_set_data(Dictionary data)
{
    m_size = data["size"];
    m_heights.create(m_size + 1, m_size + 1, false, Image::FORMAT_GRAYSCALE_ALPHA, data["heights"]);
    m_blends.create(m_size, m_size, false, Image::FORMAT_RGBA, data["blends"]);

    VS::get_singleton()->texture_allocate(m_heights_tex, m_size + 1, m_size + 1, m_heights.get_format(), 0);
    VS::get_singleton()->texture_allocate(m_blends_tex, m_size, m_size, m_blends.get_format(), VS::TEXTURE_FLAG_FILTER);
    VS::get_singleton()->texture_set_data(m_blends_tex, m_blends);
    VS::get_singleton()->texture_set_data(m_heights_tex, m_heights);
}

Dictionary TerrainData::_get_data() const
{
    Dictionary d;

    d["size"] = m_size;
    d["heights"] = m_heights.get_data();
    d["blends"] = m_blends.get_data();

    return d;
}

void TerrainData::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("get_heights"), &TerrainData::get_heights);
    ObjectTypeDB::bind_method(_MD("get_blends"), &TerrainData::get_blends);

    ObjectTypeDB::bind_method(_MD("get_size"), &TerrainData::get_size);
    ObjectTypeDB::bind_method(_MD("set_size", "size"), &TerrainData::set_size);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "size"), _SCS("set_size"), _SCS("get_size"));

    ObjectTypeDB::bind_method(_MD("_set_data", "data"), &TerrainData::_set_data);
    ObjectTypeDB::bind_method(_MD("_get_data"), &TerrainData::_get_data);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), _SCS("_set_data"), _SCS("_get_data"));

    ADD_SIGNAL(MethodInfo("size_changed"));
}
