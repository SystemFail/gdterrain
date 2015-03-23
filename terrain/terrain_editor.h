#ifndef _TARRAIN_EDITOR_H
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
    };

    enum Brush {
        BRUSH_NONE,
        BRUSH_SQUARE,
        BRUSH_CIRCLE,
        BRUSH_SMOOTH_CIRCLE,
        BRUSH_NOISE,
    };

public:
    TerrainEditor(){};
    TerrainEditor(EditorNode* editor);
    ~TerrainEditor();

    bool forward_spatial_input_event(Camera* c, const InputEvent& e);
    void edit(TerrainNode* terrain);

    void show_menubar(bool visible);

    void _menu_option(int option);

protected:
    void _notifiacation(int what);
    static void _bind_methods();

private:
    EditorNode* m_editor_node;
    TerrainNode* m_terrain; // curently edited node

    /* ui */

    HBoxContainer* m_menubar;
    MenuButton* m_menu;
    SpinBox* m_brush_strength;
    SpinBox* m_brush_size;
    ColorPickerButton* m_color_picker;

    /* editing */

    EditMode m_current_mode;
    DVector<float> m_brush_pixels;
    float m_brush_w;
    float m_brush_h;
    Brush m_current_brush;
    bool m_mouse_down;

    void make_ui();
    bool do_input_action(Camera* cam, int x, int y);
    void handle_input_event(Camera* c, const InputEvent& e);
    void modify_terrain(Vector3 intersection, const InputEvent &e);

    void create_square_brush(int w, int h);
    void create_circle_brush(int w, int h);
    void create_smooth_circle_brush(int w, int h);
    void create_noise_brush(int w, int h);

    void _brush_size_changed(int value);
    void change_brush(Brush new_brush);
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
