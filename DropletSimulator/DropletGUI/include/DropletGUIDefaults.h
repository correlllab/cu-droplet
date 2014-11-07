/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\DropletGUIDefaults.h
 *
 * \brief	Declares the global defaults.
 */

#define DEFAULT_ASSETDIR "./assets/"
#define DEFAULT_SHADERDIR "Shaders/"
#define DEFAULT_TEXTUREDIR "Textures/"
#define DEFAULT_MESHDIR "Models/"
#define DEFAULT_SETTINGS "defaults.txt"
#define DEFAULT_PROJECTDIR "Projector/"
#define DEFAULT_FLOORDIR "Floors/"
#define DEFAULT_SETUPDIR "Setup/"

#define DEFAULT_WINDOW_NAME "Droplets Renderer"
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define SCREENSHOT_WIDTH 4096
#define SCREENSHOT_HEIGHT 3072

#define DEFAULT_RESET_TIME 60.0 // 1 minute in seconds

// Droplet Simulator specific constants
#define DEFAULT_ROW_TILES 4
#define DEFAULT_COL_TILES 4

#define DEFAULT_TILE_LENGTH 25.0f
#define DEFAULT_DROPLET_RADIUS 2.2f
#define DEFAULT_WALL_HEIGHT 5.0f // in cm

#define DEFAULT_WALL_WIDTH	1.0f
#define DEFAULT_FPS 60.0f	// 60 frames per second

#define DEFAULT_DROPLET_MASS  24.2f // in g
#define DEFAULT_DROPLET_FRICTION .5f

// 0.0 is the bottom of the mesh, 1.0 is the top of the mesh
#define DEFAULT_DROPLET_CENTER_OF_MASS 0.15f 
#define DEFAULT_OBJECT_MASS  100.f // in g
#define DEFAULT_OBJECT_FRICTION .25f
#define DEFAULT_OBJECT_RADIUS 4.0f

#define DEFAULT_WALL_MASS 0.0f // in g
#define DEFAULT_WALL_FRICTION .5f

#define DEFAULT_FLOOR_MASS 0.0f // in g
#define DEFAULT_FLOOR_FRICTION .5f

#define DEBUG_MESH "cube"
#define DEBUG_DROPLET_MESH "cylinder"
#define DEBUG_SHADER "debug"

#define PROJECTOR_TEXTURE_NAME "projector_texture"

#define TOWER_MESH_NAME "tower_mesh"
#define TOWER_TEXTURE_NAME "tower_texture"
#define TOWER_SHADER_NAME "tower_shader"

#define FLOOR_MESH_NAME "floor_mesh"
#define FLOOR_TEXTURE_NAME "floor_texture"
#define FLOOR_SHADER_NAME "floor_shader"
#define FLOOR_PROJECTION_SHADER_NAME "floor_projection"

#define WALL_MESH_NAME "wall_mesh"
#define WALL_TEXTURE_NAME "wall_texture"
#define WALL_SHADER_NAME "wall_shader"

#define DROPLET_MESH_NAME "droplet_mesh"
#define DROPLET_TEXTURE_NAME "droplet_texture"
#define DROPLET_SHADER_NAME "droplet_shader"
#define DROPLET_PROJECTION_SHADER_NAME "droplet_projection"

#define OBJECT_MESH_NAME "object_mesh"
#define OBJECT_TEXTURE_NAME "object_texture"
#define OBJECT_SHADER_NAME "object_shader"
#define OBJECT_PROJECTION_SHADER_NAME "object_projection"

#define OBJECT_CUBE_MESH_NAME "object_cube_mesh"
#define OBJECT_CUBE_TEXTURE_NAME "object_cube_texture"
#define OBJECT_CUBE_SHADER_NAME "object_cube_shader"
#define OBJECT_CUBE_PROJECTION_SHADER_NAME "object_cube_projection"