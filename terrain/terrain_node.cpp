#include "terrain_node.h"

#include "servers/visual_server.h"

TerrainNode::TerrainNode()
{
    m_scale = 1.0;
    m_chunk_size = 32;
    m_chunks_w = 0;
    m_chunks_h = 0;
    m_chunks_created = false;
}

TerrainNode::~TerrainNode()
{
    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
        _delete_chunk(i);
    }
}

void TerrainNode::_notification(int what)
{
    switch (what) {
    case NOTIFICATION_ENTER_TREE: {

        if (!m_chunks_created) {
            for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
                _create_chunk(i);
            }

            m_chunks_created = true;

            _update_chunks();
        }

        break;
    }
    case NOTIFICATION_EXIT_TREE: {
        //
        break;
    }
    case NOTIFICATION_TRANSFORM_CHANGED: {

        for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
            _transform_chunk(i);
        }

        break;
    }
    }
}

void TerrainNode::set_heightmap(const Ref<TerrainHeightmap>& heightmap)
{
    m_heightmap = heightmap;

    if (m_heightmap.is_null()) {
        return;
    }

    unsigned int w = m_heightmap->get_width();
    unsigned int h = m_heightmap->get_height();

    m_chunks_h = h / m_chunk_size;
    m_chunks_w = w / m_chunk_size;

    m_chunks.resize(m_chunks_h * m_chunks_w);

    if (!is_inside_tree()) {
        return;
    }

    // remove existing chunks
    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
        _delete_chunk(i);
    }

    // create new chunks
    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
        _create_chunk(i);
    }

    m_chunks_created = true;

    _update_chunks();
}

Ref<TerrainHeightmap> TerrainNode::get_heightmap() const
{
    return m_heightmap;
}

void TerrainNode::set_material(const Ref<Material>& material)
{
    m_material = material;

    _update_material();
}

Ref<Material> TerrainNode::get_material() const
{
    return m_material;
}

void TerrainNode::set_scale(float scale)
{
    m_scale = scale;

    _chunks_make_dirty();
    _update_chunks();
}

float TerrainNode::get_scale()
{
    return m_scale;
}

float TerrainNode::get_height_at(Vector3 pos)
{
    if (m_heightmap.is_null()) {
        return 0.0f;
    }

    unsigned int x = get_pixel_x_at(pos);
    unsigned int y = get_pixel_y_at(pos);

    return m_heightmap->get_pixel(x, y);
}

unsigned int TerrainNode::get_pixel_x_at(Vector3 pos)
{
    if (m_heightmap.is_null()) {
        return 0;
    }

    unsigned int x = (pos.x + 0.5 * m_scale) / m_scale;
    unsigned int w = m_heightmap->get_width();

    if (x > w - 1) {
        x = w - 1;
    }

    return x;
}

unsigned int TerrainNode::get_pixel_y_at(Vector3 pos)
{
    if (m_heightmap.is_null()) {
        return 0;
    }

    unsigned int y = (pos.z + 0.5 * m_scale) / m_scale;
    unsigned int h = m_heightmap->get_height();

    if (y > h - 1) {
        y = h - 1;
    }

    return y;
}

void TerrainNode::modify_height_at(unsigned int x, unsigned int y, float height)
{
    if (m_heightmap.is_null()) {
        return;
    }

    m_heightmap->put_pixel(x, y, height);

    DVector<Chunk>::Write cw = m_chunks.write();

    invalidate_chunks(x, y);
    _update_chunks();
}

unsigned int TerrainNode::get_chunk_offset_at(unsigned int x, unsigned int y)
{
    return (y / m_chunk_size) * m_chunks_w + (x / m_chunk_size);
}

// mark chunks dirty that contain point
void TerrainNode::invalidate_chunks(unsigned int x, unsigned int y)
{
    DVector<Chunk>::Write w = m_chunks.write();

    for (unsigned int i = 0; i < m_chunks_w * m_chunks_h; i++) {
        if (is_inside_chunk(i, x, y)) {
            w[i].mesh_dirty = true;
        }
    }
}

bool TerrainNode::is_inside_chunk(unsigned int offset, unsigned int x, unsigned int y)
{
    unsigned int cy = offset / m_chunks_w;
    unsigned int cx = offset - (cy * m_chunks_h);
    unsigned int map_x2, map_y2, map_x1, map_y1;

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

unsigned int TerrainNode::fix_ray_x(Vector3 origin, unsigned int x, unsigned int y)
{
    return 0;
}

unsigned int TerrainNode::fix_ray_y(Vector3 origin, unsigned int x, unsigned int y)
{
    return 0;
}

void TerrainNode::blit(DVector<float>& pixels, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    m_heightmap->blit(pixels, x1, y1, x2, y2);

    invalidate_chunks(x1, y1);
    invalidate_chunks(x2, y1);
    invalidate_chunks(x1, y2);
    invalidate_chunks(x2, y2);

    _update_chunks();
}

void TerrainNode::blend(DVector<float> &pixels, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float alpha)
{
    m_heightmap->blend(pixels, x1, y1, x2, y2, alpha);

    invalidate_chunks(x1, y1);
    invalidate_chunks(x2, y1);
    invalidate_chunks(x1, y2);
    invalidate_chunks(x2, y2);

    _update_chunks();
}

void TerrainNode::_update_chunk(unsigned int ch_offset)
{
    Array arr;
    DVector<Vector3> points;
    DVector<Vector3> normals;
    DVector<Vector2> uvs;
    DVector<int> indices;

    unsigned int map_w = m_heightmap->get_width();
    unsigned int map_h = m_heightmap->get_height();

    // get chunk coords
    unsigned int chunk_y = ch_offset / m_chunks_w; // 0, 1
    unsigned int chunk_x = ch_offset - (chunk_y * m_chunks_h); // 0, 1

    unsigned int map_x2, map_y2, map_x1, map_y1;

    // chunk xy to height map xy
    map_x1 = chunk_x * m_chunk_size;
    map_y1 = chunk_y * m_chunk_size;
    map_x2 = map_x1 + m_chunk_size + 1; // chunks share vertices on edges
    map_y2 = map_y1 + m_chunk_size + 1;

    /* build vertex array */

    unsigned int vert_count = (map_y2 - map_y1) * (map_x2 - map_x1); // 3 * 3, 3 * 3

    points.resize(vert_count);
    uvs.resize(vert_count);

    DVector<Vector3>::Write pointsw = points.write();
    DVector<Vector2>::Write uvsw = uvs.write();

    unsigned int counter = 0;

    for (unsigned int x = map_x1; x < map_x2; x++) {
        for (unsigned int y = map_y1; y < map_y2; y++) {

            float height = m_heightmap->get_pixel(x, y);

            Vector3 point = Vector3(x * m_scale, height * m_scale, y * m_scale);
            Vector2 uv = Vector2(x / (map_h - 1.0f), y / (map_w - 1.0f));

            pointsw[counter] = point;
            uvsw[counter] = uv;

            counter++;
        }
    }

    pointsw = DVector<Vector3>::Write();
    uvsw = DVector<Vector2>::Write();

    /* build index buffer */

    unsigned int quads_w = map_x2 - map_x1 - 1;
    unsigned int quads_h = map_y2 - map_y1 - 1;

    unsigned int tri_count = quads_w * quads_h * 2;

    indices.resize(tri_count * 3);

    DVector<int>::Write indicesw = indices.write();

    unsigned int index = 0;

    // loop for each quad
    for (unsigned int x = 0; x < quads_w; x++) {
        for (unsigned int y = 0; y < quads_h; y++) {
            unsigned int offset = y * (quads_w + 1) + x;

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

    /* remove dirty flag */
    DVector<Chunk>::Write cw = m_chunks.write();

    cw[ch_offset].mesh_dirty = false;
    cw[ch_offset].surface_added = true;

    cw = DVector<Chunk>::Write();
}

void TerrainNode::_create_chunk(unsigned int offset)
{
    DVector<Chunk>::Write w = m_chunks.write();

    w[offset].mesh = VS::get_singleton()->mesh_create();
    w[offset].instance = VS::get_singleton()->instance_create();
    w[offset].surface_added = false;
    w[offset].mesh_dirty = true;
    w[offset].material_dirty = true;

    w = DVector<Chunk>::Write();

    VS::get_singleton()->instance_set_scenario(m_chunks[offset].instance, get_world()->get_scenario());
    VS::get_singleton()->instance_set_base(m_chunks[offset].instance, m_chunks[offset].mesh);

    _transform_chunk(offset);
}

void TerrainNode::_delete_chunk(unsigned int offset)
{
    VS::get_singleton()->free(m_chunks[offset].mesh);
    VS::get_singleton()->free(m_chunks[offset].instance);

    DVector<Chunk>::Write w = m_chunks.write();

    w[offset].surface_added = false;

    w = DVector<Chunk>::Write();
}

void TerrainNode::_transform_chunk(unsigned int offset)
{
    Transform t = get_global_transform();
    VS::get_singleton()->instance_set_transform(m_chunks[offset].instance, t);
}

void TerrainNode::_clear_chunk(unsigned int offset)
{
    // if height map removed remove surface too
    if (m_heightmap.is_null()) {
        if (m_chunks[offset].surface_added) {
            VS::get_singleton()->mesh_remove_surface(m_chunks[offset].mesh, 0);
        }
    }

    DVector<Chunk>::Write w = m_chunks.write();

    w[offset].surface_added = false;
    w[offset].mesh_dirty = true;

    w = DVector<Chunk>::Write();
}

/// update dirty chunks
void TerrainNode::_update_chunks()
{
    if (!is_inside_tree()) {
        return;
    }

    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {

        if (m_chunks[i].mesh_dirty) {
            _update_chunk(i);
        }
    }
}

void TerrainNode::_update_material()
{
    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {

        VS::get_singleton()->instance_geometry_set_material_override(
            m_chunks[i].instance, m_material.is_valid() ? m_material->get_rid() : RID());
    }
}

void TerrainNode::_chunks_make_dirty()
{
    DVector<Chunk>::Write cw = m_chunks.write();

    for (unsigned int i = 0; i < m_chunks_h * m_chunks_w; i++) {
        cw[i].mesh_dirty = true;
    }
}

void TerrainNode::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("set_heightmap", "heightmap:TerrainHeightmap"),
        &TerrainNode::set_heightmap);

    ObjectTypeDB::bind_method(_MD("get_heightmap:TerrainHeightmap"),
        &TerrainNode::get_heightmap);

    ADD_PROPERTY(
        PropertyInfo(Variant::OBJECT, "heightmap/heightmap", PROPERTY_HINT_RESOURCE_TYPE, "TerrainHeightmap"),
        _SCS("set_heightmap"), _SCS("get_heightmap"));

    ////////////////////////////////////

    ObjectTypeDB::bind_method(_MD("set_material", "heightmap:Material"),
        &TerrainNode::set_material);

    ObjectTypeDB::bind_method(_MD("get_material:Material"),
        &TerrainNode::get_material);

    ADD_PROPERTY(
        PropertyInfo(Variant::OBJECT, "heightmap/material", PROPERTY_HINT_RESOURCE_TYPE, "Material"),
        _SCS("set_material"), _SCS("get_material"));

    ////////////////////////////////////////

    ObjectTypeDB::bind_method(
        _MD("set_scale", "scale"),
        &TerrainNode::set_scale);

    ObjectTypeDB::bind_method(
        _MD("get_scale"),
        &TerrainNode::get_scale);

    ADD_PROPERTY(
        PropertyInfo(Variant::REAL, "scale"),
        _SCS("set_scale"), _SCS("get_scale"));

    ObjectTypeDB::bind_method(_MD("get_height_at", "position"),
        &TerrainNode::get_height_at);

    ObjectTypeDB::bind_method(_MD("get_pixel_x_at", "position"),
        &TerrainNode::get_pixel_x_at);

    ObjectTypeDB::bind_method(_MD("get_pixel_y_at", "position"),
        &TerrainNode::get_pixel_y_at);
}
