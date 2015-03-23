#include "register_types.h"
#ifndef _3D_DISABLED
#include "object_type_db.h"
#include "terrain_node.h"
#include "terrain_material.h"
#include "terrain_editor.h"
#include "terrain_heightmap.h"
#include "terrain_blendmap.h"
#endif // _3D_DISABLED

void register_terrain_types()
{
#ifndef _3D_DISABLED
    ObjectTypeDB::register_type<TerrainNode>();
    ObjectTypeDB::register_type<TerrainMaterial>();
    ObjectTypeDB::register_type<TerrainHeightmap>();
    ObjectTypeDB::register_type<TerrainBlendmap>();
#ifdef TOOLS_ENABLED
    EditorPlugins::add_by_type<TerrainEditorPlugin>();
#endif // tools
#endif // 3d
}

void unregister_terrain_types()
{
}
