#include "terrain_editor.h"

#include "tools/editor/plugins/spatial_editor_plugin.h"
#include "scene/3d/camera.h"
#include "tools/editor/editor_settings.h"

#include "os/keyboard.h"
#include "geometry.h"
#include "servers/visual_server.h"

TerrainEditor::TerrainEditor(EditorNode* editor)
{
    m_editor_node = editor;

    m_current_mode = MODE_MODIFY_HEIGHT;
    m_current_brush = BRUSH_SQUARE;
    m_size = 3;
    m_mouse_down = false;
    m_last_pixel_edited = Point2(-1000, -1000);
    m_active_texture = 0;
    m_cursor_mesh = VS::get_singleton()->immediate_create();
    m_cursor = VS::get_singleton()->instance_create();
    m_current_color = Color();

    _make_ui();

    _brush_changed();
}

TerrainEditor::~TerrainEditor()
{
    VS::get_singleton()->free(m_cursor_mesh);
    VS::get_singleton()->free(m_cursor);
}

bool TerrainEditor::forward_spatial_input_event(Camera* c, const InputEvent& e)
{
    switch (e.type) {
    case InputEvent::KEY: {
        break;
    }
    case InputEvent::MOUSE_BUTTON: {

        if (e.mouse_button.pressed) {
            if (e.mouse_button.button_index == BUTTON_LEFT) {
                _handle_input_event(c, e);
            }
            m_mouse_down = true;
        }
        else {
            m_mouse_down = false;
        }

        break;
    }
    case InputEvent::MOUSE_MOTION: {

        if (m_mouse_down && (e.mouse_button.button_index == BUTTON_LEFT)) {
            _handle_input_event(c, e);
        }
        break;
    }
    }

    return false;
}

void TerrainEditor::edit(TerrainNode* terrain)
{
    m_terrain = terrain;

    if (terrain) {
        m_texture_chooser->clear();

        // todo: signal

        TreeItem* root_item = m_texture_chooser->create_item();
        root_item->set_text(0, "Terrain");

        TreeItem* textures_item = m_texture_chooser->create_item(root_item);
        textures_item->set_text(0, "Textures");

        tex0 = m_texture_chooser->create_item(textures_item);
        tex0->set_text(0, "0");
        tex0->set_icon_max_width(0, 32);
        tex0->set_icon(0, terrain->get_texture0());

        tex1 = m_texture_chooser->create_item(textures_item);
        tex1->set_text(0, "1");
        tex1->set_icon_max_width(0, 32);
        tex1->set_icon(0, terrain->get_texture1());

        tex2 = m_texture_chooser->create_item(textures_item);
        tex2->set_text(0, "2");
        tex2->set_icon_max_width(0, 32);
        tex2->set_icon(0, terrain->get_texture2());

        tex3 = m_texture_chooser->create_item(textures_item);
        tex3->set_text(0, "3");
        tex3->set_icon_max_width(0, 32);
        tex3->set_icon(0, terrain->get_texture3());

         tex4 = m_texture_chooser->create_item(textures_item);
        tex4->set_text(0, "4");
        tex4->set_icon_max_width(0, 32);
        tex4->set_icon(0, terrain->get_texture4());

        /* brush */

        TreeItem* brush_root = m_texture_chooser->create_item(root_item);
        brush_root->set_text(0, "Brush");

        TreeItem* brush0 = m_texture_chooser->create_item(brush_root);
        brush0->set_text(0, "Brush 0");

        /* debug */

        TreeItem* dbg_item = m_texture_chooser->create_item(root_item);
        dbg_item->set_text(0, "Debug");

        TreeItem* blend_tex = m_texture_chooser->create_item(dbg_item);
        blend_tex->set_text(0, "blendmap");
        blend_tex->set_icon_max_width(0, 32);
        //blend_tex->set_icon(0, terrain->get_data()->get_blends());
    }
    else {
        m_texture_chooser->clear();
    }
}

void TerrainEditor::show_menubar(bool visible)
{
    if (visible) {
        m_menubar->show();
    }
    else {
        m_menubar->hide();
    }
}

void TerrainEditor::_menu_option(int option)
{
    switch (option) {
    case MENU_OPTION_MODIFY: {
        m_current_mode = MODE_MODIFY_HEIGHT;
        m_alpha->set_min(-255);
        break;
    }
    case MENU_OPTION_SET: {
        m_current_mode = MODE_SET_HEIGHT;
        m_alpha->set_min(0);
        break;
    }
    case MENU_OPTION_SMOOTH: {
        m_current_mode = MODE_SMOOTH_TERRAIN;
        m_alpha->set_min(0);
        break;
    }
    case MENU_OPTION_PAINT: {
        m_current_mode = MODE_EDIT_BLENDMAP;
        m_alpha->set_min(0);
        break;
    }
    case MENU_OPTION_SQUARE: {
        m_current_brush = BRUSH_SQUARE;
        _brush_changed();
        break;
    }
    case MENU_OPTION_CIRCLE: {
        m_current_brush = BRUSH_CIRCLE;
        _brush_changed();
        break;
    }
    case MENU_OPTION_SMOOTH_CIRCLE: {
        m_current_brush = BRUSH_CIRCLE;
        _brush_changed();
        break;
    }
    case MENU_OPTION_NOISE: {
        m_current_brush = BRUSH_NOISE;
        _brush_changed();
        break;
    }
    case MENU_OPTION_CUSTOM: {
        m_current_brush = BRUSH_CUSTOM;
        _brush_changed();
        break;
    }
    case MENU_OPTION_DBG_SAVE: {
        m_terrain->get_data()->get_blends().save_png("./blendmap.png");
        m_terrain->get_data()->get_heights().save_png("./heightmap.png");
        break;
    }
    }
}

void TerrainEditor::_notifiacation(int what)
{
    switch (what) {
    case NOTIFICATION_ENTER_TREE: {
        VS::get_singleton()->instance_set_scenario(m_cursor, get_viewport()->get_world()->get_rid());
        VS::get_singleton()->instance_set_base(m_cursor, m_cursor_mesh);

        VS::get_singleton()->immediate_begin(m_cursor_mesh, VS::PRIMITIVE_LINES);
        VS::get_singleton()->immediate_vertex(m_cursor_mesh, Vector3(-1, 0.01f, 1));
        VS::get_singleton()->immediate_vertex(m_cursor_mesh, Vector3(1, 0.01f, 1));
        VS::get_singleton()->immediate_vertex(m_cursor_mesh, Vector3(1, 0.01f, -1));
        VS::get_singleton()->immediate_vertex(m_cursor_mesh, Vector3(-1, 0.01f, -1));
        VS::get_singleton()->immediate_end(m_cursor_mesh);
        break;
    }
    case NOTIFICATION_EXIT_TREE: {
        break;
    }
    case NOTIFICATION_PROCESS: {
        break;
    }
    }
}

void TerrainEditor::_bind_methods()
{
    ObjectTypeDB::bind_method("_on_brush_size_changed", &TerrainEditor::_on_brush_size_changed);
    ObjectTypeDB::bind_method("_on_active_texture_changed", &TerrainEditor::_on_active_texture_changed);
    ObjectTypeDB::bind_method("_menu_option", &TerrainEditor::_menu_option);
}

void TerrainEditor::_make_ui()
{
    /* sidebar */

    m_sidebar = memnew(VBoxContainer);
    m_sidebar->set_v_size_flags(SIZE_EXPAND_FILL);
    m_sidebar->set_custom_minimum_size(Size2(80, 0));
    add_child(m_sidebar);

    m_alpha = memnew(HSlider);
    m_alpha->set_max(255);
    m_alpha->set_min(-255);
    m_alpha->set_val(255);
    m_sidebar->add_child(m_alpha);

    m_texture_chooser = memnew(Tree);
    m_texture_chooser->set_v_size_flags(SIZE_EXPAND_FILL);
    m_texture_chooser->set_h_size_flags(SIZE_EXPAND_FILL);
    m_texture_chooser->connect("cell_selected", this, "_on_active_texture_changed");
    m_sidebar->add_child(m_texture_chooser);

    /* menubar */

    m_menubar = memnew(HBoxContainer);
    SpatialEditor::get_singleton()->add_control_to_menu_panel(m_menubar);
    m_menu = memnew(MenuButton);
    m_menubar->add_child(m_menu);
    m_menubar->hide();

    m_menu->set_text("Terrain");
    m_menu->get_popup()->add_item("Modify height", MENU_OPTION_MODIFY);
    m_menu->get_popup()->add_item("Set height", MENU_OPTION_SET);
    m_menu->get_popup()->add_item("Smooth", MENU_OPTION_SMOOTH);
    m_menu->get_popup()->add_separator();
    m_menu->get_popup()->add_item("Paint", MENU_OPTION_PAINT);
    m_menu->get_popup()->add_separator();
    m_menu->get_popup()->add_item("Square", MENU_OPTION_SQUARE);
    m_menu->get_popup()->add_item("Circle", MENU_OPTION_CIRCLE);
    m_menu->get_popup()->add_item("Smooth circle", MENU_OPTION_SMOOTH_CIRCLE);
    m_menu->get_popup()->add_item("Noise", MENU_OPTION_NOISE);
    m_menu->get_popup()->add_separator();
    m_menu->get_popup()->add_item("Debug save", MENU_OPTION_DBG_SAVE);

    m_menu->get_popup()->connect("item_pressed", this, "_menu_option");

    m_brush_strength = memnew(SpinBox);
    m_brush_strength->set_min(-100);
    m_brush_strength->set_max(100);
    m_brush_strength->set_step(1);

    m_menubar->add_child(m_brush_strength);

    m_brush_size = memnew(SpinBox);
    m_brush_size->set_max(10);
    m_brush_size->set_min(1);
    m_brush_size->set_step(1);
    m_brush_size->set_val(3);
    m_brush_size->connect("value_changed", this, "_on_brush_size_changed");
    m_menubar->add_child(m_brush_size);

    m_color_picker = memnew(ColorPickerButton);
    m_color_picker->set_custom_minimum_size(Size2(30, 0));
    m_menubar->add_child(m_color_picker);

    m_brush_opacity = memnew(SpinBox);
    m_brush_opacity->set_max(255);
    m_brush_opacity->set_min(0);
    m_brush_opacity->set_val(255);
    m_menubar->add_child(m_brush_opacity);
}

bool TerrainEditor::_do_input_action(Camera* cam, int x, int y)
{
    return true;
}

void TerrainEditor::_handle_input_event(Camera* c, const InputEvent& e)
{
    Point2 point = Point2(e.mouse_button.x, e.mouse_button.y);
    Vector3 from = c->project_ray_origin(point);
    Vector3 normal = c->project_ray_normal(point);
    // get model space coords
    Transform local_xform = m_terrain->get_global_transform().affine_inverse();
    from = local_xform.xform(from);
    normal = local_xform.basis.xform(normal).normalized();

    // check intersection

    Plane plane = Plane(Vector3(0, 1, 0), 0);
    Vector3 intersection;

    if (plane.intersects_ray(from, normal, &intersection)) {
        _modify_terrain(intersection, e);
    }
}

void TerrainEditor::_modify_terrain(Vector3 intersection, const InputEvent& e)
{
    int hx = m_terrain->get_pixel_x_at(intersection, 0.5f);
    int hy = m_terrain->get_pixel_y_at(intersection, 0.5f);
    int bx = m_terrain->get_pixel_x_at(intersection, 0.0f);
    int by = m_terrain->get_pixel_y_at(intersection, 0.0f);

    // center brush
    hx -= m_size / 2.0f;
    hy -= m_size / 2.0f;
    bx -= m_size / 2.0f;
    by -= m_size / 2.0f;

    float alpha = (float)m_alpha->get_val() / 255.0f;

    switch (m_current_mode) {

    case MODE_MODIFY_HEIGHT: {

        if (e.type == InputEvent::MOUSE_BUTTON) {

            m_terrain->get_data()->paint_height(m_brush_image, hx, hy, alpha);
            m_terrain->mark_height_dirty(hx, hy);
            m_terrain->mark_height_dirty(hx, hy + m_size);
            m_terrain->mark_height_dirty(hx + m_size, hy);
            m_terrain->mark_height_dirty(hx + m_size, hy + m_size);
            m_terrain->update_dirty_chunks();
        }

        break;
    }

    case MODE_SET_HEIGHT: {

        //m_terrain->blit(m_brush_pixels, hx, hy, hx + m_size, hy + m_size);

        break;
    }

    case MODE_SMOOTH_TERRAIN: {
        break;
    }
    case MODE_EDIT_BLENDMAP: {

        m_terrain->get_data()->paint_blend(m_brush_image, bx, by, m_active_texture, alpha);
        break;
    }
    }
}

void TerrainEditor::_create_square_brush()
{
    m_brush_image.create(m_size, m_size, false, Image::FORMAT_GRAYSCALE);

    for (int i = 0; i < m_size * m_size; i++) {
        int y = i / m_size;
        int x = i - (y * m_size);

        m_brush_image.put_pixel(x, y, Color(1.0, 1.0, 1.0, 1.0));
    }
}

void TerrainEditor::_create_circle_brush()
{
    m_brush_image.create(m_size, m_size, false, Image::FORMAT_GRAYSCALE);

    for (int i = 0; i < m_size * m_size; i++) {
        int y = i / m_size;
        int x = i - (y * m_size);

        m_brush_image.put_pixel(x, y, Color(1.0, 1.0, 1.0, 1.0));
    }
}

void TerrainEditor::_create_smooth_circle_brush()
{
    m_brush_image.create(m_size, m_size, false, Image::FORMAT_GRAYSCALE);

    for (int i = 0; i < m_size * m_size; i++) {
        int y = i / m_size;
        int x = i - (y * m_size);

        m_brush_image.put_pixel(x, y, Color(1.0, 1.0, 1.0, 1.0));
    }
}

void TerrainEditor::_create_noise_brush()
{
    m_brush_image.create(m_size, m_size, false, Image::FORMAT_GRAYSCALE);

    for (int i = 0; i < m_size * m_size; i++) {
        int y = i / m_size;
        int x = i - (y * m_size);

        int noize = Math::random(0, 1);

        m_brush_image.put_pixel(x, y, Color(noize, noize, noize, noize));
    }
}

void TerrainEditor::_on_brush_size_changed(int value)
{
    m_size = value;

    _brush_changed();
}

void TerrainEditor::_on_active_texture_changed()
{
    TreeItem* sel = m_texture_chooser->get_selected();

    if (sel == tex0) {
        m_active_texture = 0;
    }
    else if (sel == tex1) {
        m_active_texture = 1;
    }
    else if (sel == tex2) {
        m_active_texture = 2;
    }
    else if (sel == tex3) {
        m_active_texture = 3;
    }
    else if (sel == tex4) {
        m_active_texture = 4;
    }
}

void TerrainEditor::_brush_changed()
{
    switch (m_current_brush) {

    case BRUSH_NONE: {
        break;
    }

    case BRUSH_SQUARE: {
        _create_square_brush();
        break;
    }

    case BRUSH_CIRCLE: {
        _create_circle_brush();
        break;
    }

    case BRUSH_SMOOTH_CIRCLE: {
        _create_smooth_circle_brush();
        break;
    }

    case BRUSH_NOISE: {
        _create_noise_brush();
        break;
    }
    }
}

/* Terain Editor Plugin implementation */

TerrainEditorPlugin::TerrainEditorPlugin(EditorNode* editor_node)
{
    m_editor_node = editor_node;

    m_terrain_editor = memnew(TerrainEditor(editor_node));
    SpatialEditor::get_singleton()->get_palette_split()->add_child(m_terrain_editor);
    SpatialEditor::get_singleton()->get_palette_split()->move_child(m_terrain_editor, 0);
    m_terrain_editor->hide();
}

TerrainEditorPlugin::~TerrainEditorPlugin()
{
}

void TerrainEditorPlugin::edit(Object* object)
{
    if (NULL != object) {
        m_terrain_editor->edit(object->cast_to<TerrainNode>());
    }
}

bool TerrainEditorPlugin::handles(Object* object) const
{
    return object->is_type("TerrainNode");
}

void TerrainEditorPlugin::make_visible(bool visible)
{
    m_terrain_editor->set_process(visible);
    m_terrain_editor->show_menubar(visible);

    if (visible) {
        m_terrain_editor->show();
    }
    else {
        m_terrain_editor->hide();
    }
}
