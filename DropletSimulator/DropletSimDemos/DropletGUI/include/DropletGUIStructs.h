/**
 * \file	\cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\DropletGUIStructs.h
 *
 * \brief	Declares global structs and types.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include <qglobal.h>
#include <QVector>
#include <QList>
#include <QString>
#include <QImage>
#include <DSimDataStructs.h>

/** @defgroup globals Globals
 *  \brief Global types, structures, and functions.
 *  	   
 *  Vector types inspired by Apple's GLKVector types: http://developer.apple.com/library/mac/#documentation/GLkit/Reference/GLKit_Collection/_index.html
 *  
 *  @{
 */

/**
 * \union	vec4
 *
 * \brief	Four element vector of floats.
 */

union vec4
{
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    struct { float s, t, p, q; };
    float v[4];
};

/**
 * \union	vec3i
 *
 * \brief	Three element vector of integers.
 */

union vec3i
{
    struct { unsigned char x, y, z; };
    struct { unsigned char r, g, b; };
    struct { unsigned char s, t, p; };
    unsigned char v[4];
};

/**
 * \union	vec3
 *
 * \brief	Three element vector of floats.
 */

union vec3
{
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float s, t, p; };
    float v[3];
};

/**
 * \union	vec2
 *
 * \brief	Two element vector of floats.
 */

union vec2
{
    struct { float x, y; };
    struct { float s, t; };
    float v[2];
};


/**
 * \fn	static inline vec3 vec3make(float x, float y, float z)
 *
 * \brief	Make a vec3 variable
 *
 * \param	x	The x coordinate.
 * \param	y	The y coordinate.
 * \param	z	The z coordinate.
 *
 * \return	.
 */

static inline vec3 vec3make(float x, float y, float z)
{
    vec3 v = { x, y, z };
    return v;
}

/**
 * \fn	static inline vec3i vec3imake(int x, int y, int z)
 *
 * \brief	Make a vec3i.
 *
 * \param	x	The x coordinate.
 * \param	y	The y coordinate.
 * \param	z	The z coordinate.
 *
 * \return	.
 */

static inline vec3i vec3imake(int x, int y, int z)
{
    vec3i v = { x, y, z };
    return v;
}

/**
 * \fn	static inline vec2 vec2make(float x, float y)
 *
 * \brief	Make a vec2.
 *
 * \param	x	The x coordinate.
 * \param	y	The y coordinate.
 *
 * \return	.
 */

static inline vec2 vec2make(float x, float y)
{
    vec2 v = { x, y};
    return v;
}

/**
 * \fn	static inline vec4 vec4make(float x, float y, float z, float w)
 *
 * \brief	Make a vec4.
 *
 * \param	x	The x coordinate.
 * \param	y	The y coordinate.
 * \param	z	The z coordinate.
 * \param	w	The width.
 *
 * \return	.
 */

static inline vec4 vec4make(float x, float y, float z, float w)
{
    vec4 v = { x, y, z, w};
    return v;
}

/**
 * @ingroup sim
 * \enum	droplet_t
 *
 * \brief	Enumerated type defining possible droplet programs.
 */

enum droplet_t {
	CustomOne,
	CustomTwo,
	CustomThree,
	CustomFour,
	CustomFive,
	CustomSix,
	CustomSeven,
	CustomEight,
	CustomNine,
	CustomTen,
	NUM_DROPLET_PROGRAMS
};

/* object structs */

/**
 * @ingroup sim
 * \struct	dropletStruct_t
 *
 * \brief	Defines properties about droplets that are emitted from SimInterface.
 */

struct dropletStruct_t {
	int dropletID;
	vec3 origin;
	vec3 rotation;
	vec4 quaternion;
	vec3i color;
	DropletCommData commData;
	bool changed;
};

/**
 * @ingroup sim
 * \enum	object_t
 *
 * \brief	Enumerated type defining possible physical objects.
 */

enum object_t {
	Sphere,
	Cube,
	Wall,
	Floor
};

/**
 * @ingroup sim
 * \struct	objectStruct_t
 *
 * \brief	Defines properties about physical objects that are emitted from SimInterface.
 */

struct objectStruct_t {
	int objectID;
	float objectRadius;
	object_t oType;
	vec3 scale;
	vec3 origin;
	vec3 rotation;
	vec4 quaternion;
	vec3i color;
	bool changed;
};


/**
 * @ingroup sim
 * \struct	simSetting_t
 *
 * \brief	Defines simulator settings that are passed in and out of SimInterface.
 */

struct simSetting_t {
	QString arenaName;
	int numRowTiles;
	int numColTiles;
	float tileLength;
	float dropletRadius;
	float wallHeight;
	float wallWidth;
	float fps;
	float dropletOffset;
	bool projecting;
	QString projTexture;
	QString floorFile;
	QVector<QStringList> startingDroplets;
	QVector<QStringList> startingObjects;
};

/**
 * \struct	collisionShapeStruct_t
 *
 * \brief	Defines the properties of a collision shape.
 */

struct collisionShapeStruct_t {
	object_t oType;
	float objectRadius;
	vec3 scale;
	int collisionID;
};

/**
 * @ingroup sim
 * \struct	simState_t
 *
 * \brief	Defines current state of the simulator that is emitted by SimInterface.
 */

struct simState_t {
	double simTime;
	double realTime;
	double resetTime;
	bool useResetTime;
	double timeRatio;
	QImage projTexture;
	bool projTextureChanged;
	float dropletOffset;
	QVector<dropletStruct_t> dropletData;
	QVector<objectStruct_t> dynamicObjectData;
	QVector<objectStruct_t> staticObjectData;
	QList<collisionShapeStruct_t> collisionShapes;
};

/**
 * @ingroup sim
 * \struct	simRate_t
 *
 * \brief	Defines the current state of rate limiting emitted by the SimInterface.
 */

struct simRate_t {
	float timeScale;
	bool limitRate;
};

/** @} */ // end of globals

Q_DECLARE_TYPEINFO(objectStruct_t,Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(dropletStruct_t,Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(collisionShapeStruct_t,Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(simSetting_t,Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(simState_t,Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(simRate_t,Q_PRIMITIVE_TYPE);

#endif