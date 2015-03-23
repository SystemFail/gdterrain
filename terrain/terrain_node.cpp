#include "terrain_node.h"

#include "servers/visual_server.h"
#include "os/os.h"

TerrainNode::TerrainNode()
{
    const String frag_code = String(
        "uniform texture blendmap;"
        "uniform texture texture0;"
        "uniform texture texture1;"
        "uniform texture texture2;"
        "uniform texture texture3;"
        "uniform texture texture4;"
        "uniform float s;"
        "vec2 coord = s * UV;"
        "vec3 sample0 = tex(texture0, coord).rgb;"
        "vec3 sample1 = tex(texture1, coord).rgb;"
        "vec3 sample2 = tex(texture2, coord).rgb;"
        "vec3 sample3 = tex(texture3, coord).rgb;"
        "vec3 sample4 = tex(texture4, coord).rgb;"
        "vec4 blend = tex(blendmap, UV).rgba;"
        "vec3 c = sample0;"
        "c = mix(c, sample1, blend.r);"
        "c = mix(c, sample2, blend.g);"
        "c = mix(c, sample3, blend.b);"
        "c = mix(c, sample4, blend.a);"
        "DIFFUSE = c;"
        );
    m_scale = 1.0;
    m_chunk_size = 32;
    m_uv_scale = 10.0;
    m_chunk_count = 0;
    m_chunks_created = false;
    m_material = VS::get_singleton()->material_create();
    m_shader = VS::get_singleton()->shader_create();
    VS::get_singleton()->shader_set_code(m_shader, "", frag_code, "");
    VS::get_singleton()->material_set_shader(m_material, m_shader);
    VS::get_singleton()->material_set_param(m_material, "s", m_uv_scale);
}

TerrainNode::~TerrainNode()
{
    VS::get_singleton()->free(m_shader);
    VS::get_singleton()->free(m_material);
}

void TerrainNode::_notification(int what)
{
    switch (what) {
    case NOTIFICATION_ENTER_TREE: {

        if (!m_chunks_created) {
            for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
                _create_chunk(i);
            }

            m_chunks_created = true;

            _update_chunks();
        }

        break;
    }
    case NOTIFICATION_EXIT_TREE: {

        for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
            _delete_chunk(i);
        }

        m_chunks_created = false;

        break;
    }
    case NOTIFICATION_TRANSFORM_CHANGED: {

        if (m_chunks_created) {
            for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
                _update_chunk_transform(i);
            }
        }

        break;
    }
    }
}

void TerrainNode::set_heightmap(const Ref<TerrainHeightmap>& heightmap)
{
    if (m_heightmap.is_valid()) {
        m_heightmap->disconnect("size_changed", this, "_size_changed");
    }

    m_heightmap = heightmap;

    if (m_heightmap.is_valid()) {
        m_heightmap->connect("size_changed", this, "_size_changed");
    }

    _heightmap_changed();
}

Ref<TerrainHeightmap> TerrainNode::get_heightmap() const
{
    return m_heightmap;
}

void TerrainNode::set_texture0(const Ref<Texture> &texture)
{
    m_texture0 = texture;
    VS::get_singleton()->material_set_param(m_material, "texture0", m_texture0);
}

void TerrainNode::set_texture1(const Ref<Texture> &texture)
{
    m_texture1 = texture;
    VS::get_singleton()->material_set_param(m_material, "texture1", m_texture1);
}

void TerrainNode::set_texture2(const Ref<Texture> &texture)
{
    m_texture2 = texture;
    VS::get_singleton()->material_set_param(m_material, "texture2", m_texture2);
}

void TerrainNode::set_texture3(const Ref<Texture> &texture)
{
    m_texture3 = texture;
    VS::get_singleton()->material_set_param(m_material, "texture3", m_texture3);
}

void TerrainNode::set_texture4(const Ref<Texture> &texture)
{
    m_texture4 = texture;
    VS::get_singleton()->material_set_param(m_material, "texture4", m_texture4);
}

Ref<Texture> TerrainNode::get_texture0()
{
    return m_texture0;
}

Ref<Texture> TerrainNode::get_texture1()
{
    return m_texture1;
}

Ref<Texture> TerrainNode::get_texture2()
{
    return m_texture2;
}

Ref<Texture> TerrainNode::get_texture3()
{
    return m_texture3;
}

Ref<Texture> TerrainNode::get_texture4()
{
    return m_texture4;
}

void TerrainNode::set_scale(float scale)
{
    m_scale = scale;

    _chunks_mark_all_dirty();
    _update_chunks();
}

float TerrainNode::get_scale()
{
    return m_scale;
}

void TerrainNode::set_uv_scale(float scale)
{
    m_uv_scale = scale;
    VS::get_singleton()->material_set_param(m_material, "s", m_uv_scale);
}

float TerrainNode::get_uv_scale() const
{
    return m_uv_scale;
}

float TerrainNode::get_height_at(Vector3 pos)
{
    if (m_heightmap.is_null()) {
        return 0.0f;
    }

    int x = get_pixel_x_at(pos, 0.5);
    int y = get_pixel_y_at(pos, 0.5);

    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        x = 0;
    }

    return m_heightmap->get_height(x, y);
}

int TerrainNode::get_pixel_x_at(Vector3 pos, float offset)
{
    if (m_heightmap.is_null()) {
        return 0;
    }

    int x = (pos.x + offset * m_scale) / m_scale;
    int w = m_heightmap->get_size();

    if (x > w - 1) {
        x = w - 1;
    }

    return x;
}

int TerrainNode::get_pixel_y_at(Vector3 pos, float offset)
{
    if (m_heightmap.is_null()) {
        return 0;
    }

    int y = (pos.z + offset * m_scale) / m_scale;
    int h = m_heightmap->get_size();

    if (y > h - 1) {
        y = h - 1;
    }

    return y;
}

void TerrainNode::modify_height_at(int x, int y, float height)
{
    if (m_heightmap.is_null()) {
        return;
    }

    m_heightmap->set_height(x, y, height);

    _mark_mesh_dirty(x, y);
    _update_chunks();
}

void TerrainNode::modify_blendmap_at(int x, int y, Color c)
{
    if (m_heightmap.is_null()) {
        return;
    }

    m_heightmap->set_blend(x, y, c);
}

int TerrainNode::get_chunk_offset_at(int x, int y)
{
    return (y / m_chunk_size) * m_chunk_count + (x / m_chunk_size);
}

// mark chunks dirty that contain point
void TerrainNode::_mark_mesh_dirty(int x, int y)
{
    DVector<Chunk>::Write w = m_chunks.write();

    for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
        if (is_hmap_pixel_inside_chunk(i, x, y)) {
            w[i].mesh_dirty = true;
        }
    }
}

void TerrainNode::_mark_blend_dirty(int x, int y)
{
    DVector<Chunk>::Write w = m_chunks.write();

    for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
        if (is_hmap_pixel_inside_chunk(i, x, y)) {
            w[i].blend_dirty = true;
        }
    }
}

bool TerrainNode::is_hmap_pixel_inside_chunk(int offset, int x, int y)
{
    int cy = offset / m_chunk_count;
    int cx = offset - (cy * m_chunk_count);
    int map_x2, map_y2, map_x1, map_y1;

    map_x1 = cx * m_chunk_size;
    map_y1 = cy * m_chunk_size;
    map_x2 = map_x1 + m_chunk_size + 1; // chunks share verices on edges
    map_y2 = map_y1 + m_chunk_size + 1;

    if (x >= map_x1 && x <= map_x2) {
        if (y >= map_y1 && y <= map_y2) {
            return true;
        }
    }

    return false;
}

void TerrainNode::blit(DVector<float>& pixels, int x1, int y1, int x2, int y2)
{
    m_heightmap->blit_height(pixels, x1, y1, x2, y2);

    _mark_mesh_dirty(x1, y1);
    _mark_mesh_dirty(x2, y1);
    _mark_mesh_dirty(x1, y2);
    _mark_mesh_dirty(x2, y2);

    _update_chunks();
}

void TerrainNode::blend(DVector<float> &pixels, int x1, int y1, int x2, int y2, float alpha)
{
    m_heightmap->blend_height(pixels, x1, y1, x2, y2, alpha);

    _mark_mesh_dirty(x1, y1);
    _mark_mesh_dirty(x2, y1);
    _mark_mesh_dirty(x1, y2);
    _mark_mesh_dirty(x2, y2);

    _update_chunks();
}

Point2i TerrainNode::local_to_blendmap(Vector3 pos)
{
    /*
    if (m_blendmap.is_null()) {
        return Point2i(0,0);
    }

    int x = (pos.x * m_scale) / m_scale;
    int y = (pos.x * m_scale) / m_scale;

    return Point2i(x, y);
    */
}

Point2i TerrainNode::local_to_heightmap(Vector3 pos)
{
    if (m_heightmap.is_null()) {
        return Point2i(0, 0);
    }

    int x = (pos.x + 0.5 * m_scale) / m_scale;
    int y = (pos.z + 0.5 * m_scale) / m_scale;

    return Point2i(x, y);
}

void TerrainNode::_update_chunk_mesh(int ch_offset)
{
    Array arr;
    DVector<Vector3> points;
    DVector<Vector3> normals;
    DVector<Vector2> uvs;
    DVector<int> indices;

    int map_size = m_heightmap->get_size();

    // get chunk coords
    int chunk_y = ch_offset / m_chunk_count;
    int chunk_x = ch_offset - (chunk_y * m_chunk_count);

    int map_x2, map_y2, map_x1, map_y1;

    // chunk xy to height map xy
    map_x1 = chunk_x * m_chunk_size;
    map_y1 = chunk_y * m_chunk_size;
    map_x2 = map_x1 + m_chunk_size + 1; // chunks share vertices on edges
    map_y2 = map_y1 + m_chunk_size + 1;

    /* build vertex array */

    int vert_count = (map_y2 - map_y1) * (map_x2 - map_x1);

    points.resize(vert_count);
    uvs.resize(vert_count);

    DVector<Vector3>::Write pointsw = points.write();
    DVector<Vector2>::Write uvsw = uvs.write();

    int counter = 0;

    for (int x = map_x1; x < map_x2; x++) {
        for (int y = map_y1; y < map_y2; y++) {

            float height = m_heightmap->get_height(x, y);

            Vector3 point = Vector3(x * m_scale, height * m_scale, y * m_scale);
            Vector2 uv = Vector2(x / (map_size - 1.0f), y / (map_size - 1.0f));

            pointsw[counter] = point;
            uvsw[counter] = uv;

            counter++;
        }
    }

    pointsw = DVector<Vector3>::Write();
    uvsw = DVector<Vector2>::Write();

    /* build index buffer */

    int quads_w = map_x2 - map_x1 - 1;
    int quads_h = map_y2 - map_y1 - 1;

    int tri_count = quads_w * quads_h * 2;

    indices.resize(tri_count * 3);

    DVector<int>::Write indicesw = indices.write();

    int index = 0;

    // loop for each quad
    for (int x = 0; x < quads_w; x++) {
        for (int y = 0; y < quads_h; y++) {
            int offset = y * (quads_w + 1) + x;

            indicesw[index++] = offset;
            indicesw[index++] = offset + quads_w + 2;
            indicesw[index++] = offset + 1;

            indicesw[index++] = offset;
            indicesw[index++] = offset + quads_w + 1;
            indicesw[index++] = offset + quads_w + 2;
        }
    }

    indicesw = DVector<int>::Write();

    /* generate normals */

    normals.resize(vert_count);
    DVector<Vector3>::Write normalsw = normals.write();

    for (int i = 0; i < indices.size(); i += 3) {
        Vector3 v0 = points[indices[i + 0]];
        Vector3 v1 = points[indices[i + 1]];
        Vector3 v2 = points[indices[i + 2]];

        Vector3 normal = Plane(v0, v1, v2).normal;

        normalsw[indices[i + 0]] += normal;
        normalsw[indices[i + 1]] += normal;
        normalsw[indices[i + 2]] += normal;
    }

    for (int i = 0; i < normals.size(); i++) {
        normalsw[i] = normalsw[i].normalized();
    }

    normalsw = DVector<Vector3>::Write();

    /* remove surface if exists */

    if (m_chunks[ch_offset].surface_added) {
        VS::get_singleton()->mesh_remove_surface(m_chunks[ch_offset].mesh, 0);
    }

    /* give arrays to visual server */

    arr.resize(VS::ARRAY_MAX);
    arr[VS::ARRAY_VERTEX] = points;
    arr[VS::ARRAY_NORMAL] = normals;
    arr[VS::ARRAY_TEX_UV] = uvs;
    arr[VS::ARRAY_INDEX] = indices;

    VS::get_singleton()->mesh_add_surface(
        m_chunks[ch_offset].mesh,
        VS::PRIMITIVE_TRIANGLES,
        arr);

    VS::get_singleton()->mesh_surface_set_material(m_chunks[ch_offset].mesh, 0, m_material);

    /* remove dirty flag */
    DVector<Chunk>::Write cw = m_chunks.write();

    cw[ch_offset].mesh_dirty = false;
    cw[ch_offset].surface_added = true;

    cw = DVector<Chunk>::Write();
}

void TerrainNode::_create_chunk(int offset)
{
    DVector<Chunk>::Write w = m_chunks.write();

    w[offset].mesh = VS::get_singleton()->mesh_create();
    w[offset].instance = VS::get_singleton()->instance_create();
    w[offset].surface_added = false;
    w[offset].mesh_dirty = true;
    w[offset].material_dirty = true;
    w[offset].blend_dirty = true;

    w = DVector<Chunk>::Write();

    VS::get_singleton()->instance_set_scenario(m_chunks[offset].instance, get_world()->get_scenario());
    VS::get_singleton()->instance_set_base(m_chunks[offset].instance, m_chunks[offset].mesh);

    _update_chunk_transform(offset);
}

void TerrainNode::_delete_chunk(int offset)
{
    VS::get_singleton()->free(m_chunks[offset].mesh);
    VS::get_singleton()->free(m_chunks[offset].instance);

    DVector<Chunk>::Write w = m_chunks.write();

    w[offset].surface_added = false;
}

void TerrainNode::_update_chunk_transform(int offset)
{
    Transform t = get_global_transform();
    VS::get_singleton()->instance_set_transform(m_chunks[offset].instance, t);
}

// update dirty chunks
void TerrainNode::_update_chunks()
{
    uint32_t benchmark = OS::get_singleton()->get_ticks_msec();

    if (!is_inside_tree()) {
        return;
    }

    for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {

        if (m_chunks[i].mesh_dirty) {
            _update_chunk_mesh(i);
        }
    }

    benchmark = OS::get_singleton()->get_ticks_msec() - benchmark;

    print_line("TerrainNode::_update_chunks() benchmark:" + itos(benchmark));
}

void TerrainNode::_update_material()
{
}

void TerrainNode::_chunks_mark_all_dirty()
{
    DVector<Chunk>::Write cw = m_chunks.write();

    for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
        cw[i].mesh_dirty = true;
    }
}

void TerrainNode::_blendmap_changed()
{

}

void TerrainNode::_heightmap_changed()
{
    if (m_heightmap.is_null()) {
        if (m_chunks_created) {
            for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
                _delete_chunk(i);
            }
        }

        return;
    }

    int wmap_size = m_heightmap->get_size();

    m_chunk_count = wmap_size / m_chunk_size;

    m_chunks.resize(m_chunk_count * m_chunk_count);

    if (!is_inside_tree()) {
        return;
    }

    if (m_chunks_created) {
        // remove existing chunks
        for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
            _delete_chunk(i);
        }
    }

    // create new chunks
    for (int i = 0; i < m_chunk_count * m_chunk_count; i++) {
        _create_chunk(i);
    }

    m_chunks_created = true;

    VS::get_singleton()->material_set_param(m_material, "blendmap", m_heightmap->get_texture());


    _update_chunks();
}

void TerrainNode::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("set_heightmap", "TerrainHeightmap"), &TerrainNode::set_heightmap);
    ObjectTypeDB::bind_method(_MD("get_heightmap"), &TerrainNode::get_heightmap);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "heightmap", PROPERTY_HINT_RESOURCE_TYPE, "TerrainHeightmap"), _SCS("set_heightmap"), _SCS("get_heightmap"));

    ObjectTypeDB::bind_method(_MD("set_scale", "scale"), &TerrainNode::set_scale);
    ObjectTypeDB::bind_method(_MD("get_scale"), &TerrainNode::get_scale);
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale"), _SCS("set_scale"), _SCS("get_scale"));

    ObjectTypeDB::bind_method(_MD("set_uv_scale", "scale"), &TerrainNode::set_uv_scale);
    ObjectTypeDB::bind_method(_MD("get_uv_scale"), &TerrainNode::get_uv_scale);
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "uv_scale"), _SCS("set_uv_scale"), _SCS("get_uv_scale"));

    ObjectTypeDB::bind_method(_MD("get_height_at", "position"), &TerrainNode::get_height_at);
    ObjectTypeDB::bind_method(_MD("get_pixel_x_at", "position"), &TerrainNode::get_pixel_x_at);
    ObjectTypeDB::bind_method(_MD("get_pixel_y_at", "position"), &TerrainNode::get_pixel_y_at);

    ObjectTypeDB::bind_method(_MD("set_texture0", "texture"), &TerrainNode::set_texture0);
    ObjectTypeDB::bind_method(_MD("get_texture0"), &TerrainNode::get_texture0);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture0", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_texture0"), _SCS("get_texture0"));

    ObjectTypeDB::bind_method(_MD("set_texture1", "texture"), &TerrainNode::set_texture1);
    ObjectTypeDB::bind_method(_MD("get_texture1"), &TerrainNode::get_texture1);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture1", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_texture1"), _SCS("get_texture1"));

    ObjectTypeDB::bind_method(_MD("set_texture2", "texture"), &TerrainNode::set_texture2);
    ObjectTypeDB::bind_method(_MD("get_texture2"), &TerrainNode::get_texture2);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture2", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_texture2"), _SCS("get_texture2"));

    ObjectTypeDB::bind_method(_MD("set_texture3", "texture"), &TerrainNode::set_texture3);
    ObjectTypeDB::bind_method(_MD("get_texture3"), &TerrainNode::get_texture3);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture3", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_texture3"), _SCS("get_texture3"));

    ObjectTypeDB::bind_method(_MD("set_texture4", "texture"), &TerrainNode::set_texture4);
    ObjectTypeDB::bind_method(_MD("get_texture4"), &TerrainNode::get_texture4);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture4", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_texture4"), _SCS("get_texture4"));

    ObjectTypeDB::bind_method(_MD("_size_changed"), &TerrainNode::_size_changed);
}

void TerrainNode::_size_changed()
{
    _heightmap_changed();
}
