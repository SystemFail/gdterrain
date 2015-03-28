#ifndef _TERRAIN_EDITOR_H
#define _TERRAIN_EDITOR_H

#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "terrain_node.h"
#include "tools/editor/pane_drag.h"

class SpatialEditorPlugin;

class TerrainEditor : public VBoxContainer {
    OBJ_TYPE(TerrainEditor, VBoxContainer)

    enum EditMode {
        MODE_MODIFY_HEIGHT,
        MODE_SMOOTH_TERRAIN,
        MODE_SET_HEIGHT,
        MODE_EDIT_BLENDMAP,
    };

    enum Menu {
        MENU_OPTION_MODIFY,
        MENU_OPTION_SMOOTH,
        MENU_OPTION_SET,
        // ---------------
        MENU_OPTION_PAINT,
        // ---------------
        MENU_OPTION_SQUARE,
        MENU_OPTION_CIRCLE,
        MENU_OPTION_SMOOTH_CIRCLE,
        MENU_OPTION_NOISE,
        MENU_OPTION_CUSTOM,
        // -----------
        MENU_OPTION_DBG_SAVE,
    };

    enum Brush {
        BRUSH_NONE,
        BRUSH_SQUARE,
        BRUSH_CIRCLE,
        BRUSH_SMOOTH_CIRCLE,
        BRUSH_NOISE,
        BRUSH_CUSTOM,
    };

public:
    TerrainEditor(){};
    TerrainEditor(EditorNode* editor);
    ~TerrainEditor();

    bool forward_spatial_input_event(Camera* c, const InputEvent& e);
    void edit(TerrainNode* terrain);

    void show_menubar(bool visible);

protected:
    void _notifiacation(int what);
    static void _bind_methods();
    void _menu_option(int option);

private:
    EditorNode* m_editor_node;
    TerrainNode* m_terrain; // curently edited node

    /* ui */

    HBoxContainer* m_menubar;
    MenuButton* m_menu;
    SpinBox* m_brush_strength;
    SpinBox* m_brush_size;
    SpinBox* m_brush_opacity;
    ColorPickerButton* m_color_picker;
    Tree* m_texture_chooser;
    VBoxContainer* m_sidebar;
    HSlider* m_alpha;

    /* sidebar */

    TreeItem* tex0;
    TreeItem* tex1;
    TreeItem* tex2;
    TreeItem* tex3;
    TreeItem* tex4;

    /* editing */

    EditMode m_current_mode;
    Image m_brush_image;
    Brush m_current_brush;
    bool m_mouse_down;
    Point2 m_last_pixel_edited;
    int m_active_texture;
    int m_size;
    RID m_cursor_mesh;
    RID m_cursor;
    Color m_current_color;

    void _make_ui();
    bool _do_input_action(Camera* cam, int x, int y);
    void _handle_input_event(Camera* c, const InputEvent& e);
    void _modify_terrain(Vector3 intersection, const InputEvent &e);

    void _create_square_brush();
    void _create_circle_brush();
    void _create_smooth_circle_brush();
    void _create_noise_brush();

    void _on_brush_size_changed(int value);
    void _on_active_texture_changed();
    void _brush_changed();
};

/* plugin */

class TerrainEditorPlugin : public EditorPlugin {
    OBJ_TYPE(TerrainEditorPlugin, EditorPlugin)

public:
    TerrainEditorPlugin(EditorNode* editor_node);
    ~TerrainEditorPlugin();

    virtual String get_name() const { return "TerrainNode"; }
    bool has_main_screen() const { return false; }

    virtual void edit(Object* object);
    virtual bool handles(Object* object) const;
    virtual void make_visible(bool visible);

    virtual bool forward_spatial_input_event(Camera* c, const InputEvent& e)
    {
        return m_terrain_editor->forward_spatial_input_event(c, e);
    }

private:
    TerrainEditor* m_terrain_editor;
    EditorNode* m_editor_node;
};

#endif
