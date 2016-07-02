// RaptureAsset: An asset created with the Asset Tool, that is loaded by the game
#pragma once
#include <inttypes.h>

// Asset File version history
// v1 - Base type

#define RASS_HEADER		"RASS"
#define RASS_VERSION	1

#define ASSET_NAMELEN		128
#define AUTHOR_NAMELEN		64
#define GROUP_NAMELEN		64
#define COMP_NAMELEN		64
#define TILE_NAMELEN		64
#define ENT_NAMELEN			64
#define MAT_NAMELEN			64
#define ANIM_NAMELEN		64
#define COMPPART_NAMELEN	64

#define COMP_DATA_VERSION		1
#define COMP_MATERIAL_VERSION	1
#define COMP_IMAGE_VERSION		1
#define COMP_FONT_VERSION		1
#define COMP_LEVEL_VERSION		1
#define COMP_ANIM_VERSION		1
#define COMP_TILE_VERSION		1

/*
Component version history

Data:
v1 - Base type

Material:
v1 - Base type

Image:
v1 - Base type

Font:
v1 - Base type

Level:
v1 - Base type

Animation:
v1 - Base type

Tile:
v1 - Base type
*/

/* Forward Declarations */
struct AssetHeader;
struct AssetComponent;
struct ComponentData;
struct ComponentMaterial;
struct ComponentImage;
struct ComponentFont;
struct ComponentLevel;
struct ComponentComp;
struct ComponentTile;

/* Enumerations */
enum ComponentType {
	Asset_Undefined,
	Asset_Data,
	Asset_Material,
	Asset_Image,
	Asset_Font,
	Asset_Level,
	Asset_Composition,
	Asset_Tile,
};

enum CompressionType {
	Compression_None,
	Compression_Zlib
};

/* Data Structures */
struct AssetHeader {
	char				header[4];						// Always "RASS"
	uint8_t				version;						// The version of this asset file
	char				assetName[ASSET_NAMELEN];		// The name of this asset
	char				contentGroup[GROUP_NAMELEN];	// The content group (DLC) that this asset belongs to
	char				author[AUTHOR_NAMELEN];			// Who created this asset?
	char				originalAuth[AUTHOR_NAMELEN];	// Who was the original author of this asset?
	CompressionType		compressionType;				// What kind of compression is there? (if any)
	uint8_t				compressionLevel;				// How compressed should this file be?
	uint16_t			numberComponents;				// How many components this asset contains
};

struct RaptureAsset {									// A Rapture Asset contains two things: a header, and components
	AssetHeader			head;
	AssetComponent*		components;
};

struct AssetComponent {
	struct AssetCompHead {
		char				componentName[64];				// The name of this component
		ComponentType		componentType;					// What kind of component this is
		uint64_t			decompressedSize;				// The total size (in bytes) of this component when decompressed
		uint8_t				componentVersion;				// The version number of this component type
	};
	AssetCompHead			meta;						// Metadata for this component
	union ComponentDataType {
		void*				undefinedComponent;			// A component that contains general or unknown data
		ComponentData*		dataComponent;				// A component that contains data
		ComponentMaterial*	materialComponent;			// A component that contains a material (sprite rendered in pseudo-3d space)
		ComponentImage*		imageComponent;				// A component that contains an image (sprite rendered in 2d space)
		ComponentFont*		fontComponent;				// A component that contains a font
		ComponentLevel*		levelComponent;				// A component that contains a preset level file
		ComponentComp*		compComponent;				// A component that contains an animation for a material
		ComponentTile*		tileComponent;				// A component that contains data on a level tile
	} data;
};

/* Everything specific to Data Components */
struct ComponentData {
	struct DataHeader {
		char				mime[64];					// The MIME type of the data
	};
	DataHeader				head;
	char*					data;
};

/* Everything specific to Material Components */
enum MaterialMapTypes {
	Maptype_Diffuse,
	Maptype_Normal,
	Maptype_Depth
};

struct ComponentMaterial {
	struct MaterialHeader {
		uint8_t				mapsPresent;					// A bitmask containing the maps that are present (diffuse, normal, ...)
		uint32_t			width;							// Total width in pixels
		uint32_t			height;							// Total height in pixels
		uint32_t			depthWidth;						// Total width of depth map in pixels
		uint32_t			depthHeight;					// Total height of depth map in pixels
		uint32_t			normalWidth;					// Total width of normal map in pixels
		uint32_t			normalHeight;					// Total height of normal map in pixels
		uint32_t			xoffset;						// Number of pixels to offset by in the X axis when rendering
		uint32_t			yoffset;						// Number of pixels to offset by in the y axis when rendering
		uint8_t				numDirections;					// Number of directions this material has
		uint32_t			frameWidth;						// Width of each frame
		uint32_t			frameHeight;					// Height of each frame
		uint32_t			fps;
	};
	MaterialHeader		head;
	uint32_t*			diffusePixels;					// Pixels in the diffuse map
	uint32_t*			normalPixels;					// Pixels in the normal map
	uint16_t*			depthPixels;					// Pixels in the depth map
};

/* Everything specific to Image components */
struct ComponentImage {
	struct ImageHeader {
		uint32_t			width;							// Width of image in pixels
		uint32_t			height;							// Height of image in pixels
	};
	ImageHeader			head;
	uint32_t*			pixels;							// Pixels of this image
};

/* Everything specific to Font components */
struct ComponentFont {
	struct FontHeader {
		uint8_t				style;							// Style that gets applied
		uint32_t			pointSize;						// Point size
		uint32_t			fontFace;						// Font face
	};
	FontHeader			head;
	uint8_t*			fontData;						// Literally the TTF file
};

/* Everything specific to Level components */
struct ComponentLevel {
	struct LevelHeader {
		uint32_t			width;							// Width of the level
		uint32_t			height;							// Height of the level
		uint32_t			numTiles;						// Number of tiles in this level
		uint32_t			numEntities;					// Number of entities in this level
	};
	struct TileEntry {
		char			name[TILE_NAMELEN];				// The name of this tile
		uint32_t		x;								// X position of this tile
		uint32_t		y;								// Y position of this tile
		uint8_t			renderType;						// Render type of this tile
		uint8_t			layerOffset;					// Layer offset of this tile
	};
	struct EntityEntry {
		char			name[ENT_NAMELEN];				// The name of this entity
		float			x;								// X position of this entity
		float			y;								// Y position of this entity
		uint32_t		spawnflags;						// Spawnflags of this entity
	};
	LevelHeader			head;
	TileEntry*			tiles;							// All of the tiles contained on this map
	EntityEntry*		ents;							// All of the entities contained on this map
};

/* Everything specific to Animation components */
struct ComponentComp {
	struct CompHeader {
		uint32_t			numComponents;				// Number of components in this composition
		uint32_t			numKeyframes;				// Number of keyframes in this composition
	};
	struct CompComponent {
		char				partName[COMPPART_NAMELEN];	// The name of this part
		char				matName[MAT_NAMELEN];		// The name of the material to use
	};
	struct CompKeyframe {
		uint8_t				type;						// What type of keyframe this is
		uint32_t			frame;						// Which frame this keyframe belongs to
		uint32_t			parm;						// An additional parameter to pass along with the keyframe
	};
	CompHeader				head;						// The header of this composition
	CompComponent*			components;					// The components of this compsition
	CompKeyframe*			keyframes;					// Keyframes in this composition
};

/* Everything specific to Tile components */
struct ComponentTile {
	uint32_t			walkmask;						// Mask of subtiles which block the player
	uint32_t			jumpmask;						// Mask of subtiles which block jumping
	uint32_t			shotmask;						// Mask of subtiles which block shots
	uint32_t			lightmask;						// Mask of subtiles which block light
	uint32_t			vismask;						// Mask of subtiles which block player visibility
	uint32_t			warpmask;						// Mask of subtiles which warp the player to another location
	char				materialName[MAT_NAMELEN];		// The name of the material to use
	uint8_t				becomeTransparent;				// Does this tile become transparent when the player walks behind it?
	uint32_t			autoTransX;						// Become transparent when player is within this box
	uint32_t			autoTransY;						// ""
	uint32_t			autoTransW;						// ""
	uint32_t			autoTransH;						// ""
	char				transMaterialName[MAT_NAMELEN];	// The name of the material to use when the player is behind the object
	uint8_t				animated;						// Is this tile animated?
	uint32_t			frameNum;						// Which frame number to start from
	uint32_t			transFrameNum;					// Which frame number to start from
	float				depthscoreOffset;				// How much to offset the (non-normalized) depth score for this tile
};