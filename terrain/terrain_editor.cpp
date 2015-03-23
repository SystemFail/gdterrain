#include "terrain_editor.h"

#include "tools/editor/plugins/spatial_editor_plugin.h"
#include "scene/3d/camera.h"
#include "tools/editor/editor_settings.h"

#include "os/keyboard.h"
#include "geometry.h"

TerrainEditor::TerrainEditor(EditorNode* editor)
{
    m_editor_node = editor;

    m_current_mode = MODE_MODIFY_HEIGHT;
    m_current_brush = BRUSH_NONE;
    m_brush_w = 3;
    m_brush_h = 3;
    m_mouse_down = false;

    make_ui();

    change_brush(BRUSH_SQUARE);
}

TerrainEditor::~TerrainEditor()
{
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
                handle_input_event(c, e);
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
            handle_input_event(c, e);
        }
        break;
    }
    }

    return false;
}

void TerrainEditor::edit(TerrainNode* terrain)
{
    m_terrain = terrain;
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
        break;
    }
    case MENU_OPTION_SET: {
        m_current_mode = MODE_SET_HEIGHT;
        break;
    }
    case MENU_OPTION_SMOOTH: {
        m_current_mode = MODE_SMOOTH_TERRAIN;
        break;
    }
    case MENU_OPTION_PAINT: {
        m_current_mode = MODE_EDIT_BLENDMAP;
        break;
    }
    case MENU_OPTION_SQUARE: {
        change_brush(BRUSH_SQUARE);
        break;
    }
    case MENU_OPTION_CIRCLE: {
        change_brush(BRUSH_CIRCLE);
        break;
    }
    case MENU_OPTION_SMOOTH_CIRCLE: {
        change_brush(BRUSH_SMOOTH_CIRCLE);
        break;
    }
    case MENU_OPTION_NOISE: {
        change_brush(BRUSH_NOISE);
        break;
    }
    }
}

void TerrainEditor::_notifiacation(int what)
{
    switch (what) {
    case NOTIFICATION_ENTER_TREE: {
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
    ObjectTypeDB::bind_method("_brush_size_changed", &TerrainEditor::_brush_size_changed);
    ObjectTypeDB::bind_method("_menu_option", &TerrainEditor::_menu_option);
}

void TerrainEditor::make_ui()
{
    // sidebar
    //Control* ec = memnew(Control);
    //ec->set_custom_minimum_size(Size2(230, 0));
    //add_child(ec);

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
    m_brush_size->connect("value_changed", this, "_brush_size_changed");
    m_menubar->add_child(m_brush_size);

    m_color_picker = memnew(ColorPickerButton);
    m_color_picker->set_custom_minimum_size(Size2(30, 0));
    m_menubar->add_child(m_color_picker);
}

bool TerrainEditor::do_input_action(Camera* cam, int x, int y)
{
    return true;
}

void TerrainEditor::handle_input_event(Camera* c, const InputEvent& e)
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
        modify_terrain(intersection, e);
    }
}

void TerrainEditor::modify_terrain(Vector3 intersection, const InputEvent& e)
{
    int hx = m_terrain->get_pixel_x_at(intersection, 0.5f);
    int hy = m_terrain->get_pixel_y_at(intersection, 0.5f);
    int bx = m_terrain->get_pixel_x_at(intersection, 0.0f);
    int by = m_terrain->get_pixel_y_at(intersection, 0.0f);

    // center brush
    hx -= m_brush_w / 2.0f;
    hy -= m_brush_h / 2.0f;

    switch (m_current_mode) {

    case MODE_MODIFY_HEIGHT: {

        if (e.type == InputEvent::MOUSE_BUTTON) {
            m_terrain->blend(m_brush_pixels, hx, hy, hx + m_brush_w, hy + m_brush_h, (float)m_brush_strength->get_val() * 0.1);
        }
        break;
    }

    case MODE_SET_HEIGHT: {

        m_terrain->blit(m_brush_pixels, hx, hy, hx + m_brush_w, hy + m_brush_h);

        break;
    }

    case MODE_SMOOTH_TERRAIN: {
        break;
    }
    case MODE_EDIT_BLENDMAP: {
        Color c = m_color_picker->get_color();
        m_terrain->modify_blendmap_at(bx, by, c);
        break;
    }
    }
}

void TerrainEditor::create_square_brush(int w, int h)
{
    m_brush_w = w;
    m_brush_h = h;

    m_brush_pixels.resize(m_brush_w * m_brush_h);
    DVector<float>::Write w_brush = m_brush_pixels.write();

    for (int i = 0; i < m_brush_pixels.size(); i++) {
        w_brush[i] = 1.0f;
    }
}

void TerrainEditor::create_circle_brush(int w, int h)
{
}

void TerrainEditor::create_smooth_circle_brush(int w, int h)
{
}

void TerrainEditor::create_noise_brush(int w, int h)
{
}

void TerrainEditor::_brush_size_changed(int value)
{
    create_square_brush(value, value);
}

void TerrainEditor::change_brush(TerrainEditor::Brush new_brush)
{
    if (new_brush == m_current_brush) {
        return;
    }

    switch (new_brush) {

    case BRUSH_NONE: {
        break;
    }

    case BRUSH_SQUARE: {
        create_square_brush(m_brush_size->get_val(), m_brush_size->get_val());
        break;
    }

    case BRUSH_CIRCLE: {
        break;
    }

    case BRUSH_SMOOTH_CIRCLE: {
        break;
    }

    case BRUSH_NOISE: {
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

    /*
    if (visible) {
        m_terrain_editor->show();
    }
    else {
        m_terrain_editor->hide();
    }
    */
}
