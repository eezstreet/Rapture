// Contains functions necessary to serialize RaptureAssets
#pragma once
#include "RaptureAsset.h"
#include <cereal/cereal.hpp>

/////////////////////////////////////////////////////////////////////////
//
// struct AssetHeader
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void serialize(Archive& archive, AssetHeader& m) {
	archive(cereal::binary_data(m.header, sizeof(char) * 4),
		m.version,
		cereal::binary_data(m.assetName, sizeof(char) * ASSET_NAMELEN),
		cereal::binary_data(m.contentGroup, sizeof(char) * GROUP_NAMELEN),
		cereal::binary_data(m.author, sizeof(char) * AUTHOR_NAMELEN),
		cereal::binary_data(m.originalAuth, sizeof(char) * AUTHOR_NAMELEN),
		m.compressionType, m.compressionLevel, m.numberComponents);
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentData
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save_(Archive& archive, ComponentData const& m, size_t const& dcs) {
	for (int i = 0; i < MIME_LEN; i++) {
		archive(m.head.mime[i]);
	}
	for (size_t i = 0; i < dcs; i++) {
		archive(m.data[i]);
	}
}

template<class Archive>
void load_(Archive& archive, ComponentData& m, size_t const& dcs) {
	m.data = (char*)malloc(dcs);
	archive(cereal::binary_data(m.head.mime, sizeof(char) * MIME_LEN),
		cereal::binary_data(m.data, dcs));
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentMaterial
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, ComponentMaterial const& m) {
	archive(m.head.mapsPresent, m.head.width, m.head.height,
		m.head.depthWidth, m.head.depthHeight, m.head.normalWidth, m.head.normalHeight,
		m.head.xoffset, m.head.yoffset, m.head.numDirections, m.head.frameWidth, m.head.frameHeight,
		m.head.fps);
	if (m.head.mapsPresent & (1 << Maptype_Diffuse)) {
		for (int i = 0; i < m.head.width * m.head.height; i++) {
			archive(m.diffusePixels[i]);
		}
	}
	if (m.head.mapsPresent & (1 << Maptype_Normal)) {
		for (int i = 0; i < m.head.normalWidth * m.head.normalHeight; i++) {
			archive(m.normalPixels[i]);
		}
	}
	if (m.head.mapsPresent & (1 << Maptype_Depth)) {
		for (int i = 0; i < m.head.depthWidth * m.head.depthHeight; i++) {
			archive(m.depthPixels[i]);
		}
	}
}

template<class Archive>
void load(Archive& archive, ComponentMaterial& m) {
	archive(m.head.mapsPresent, m.head.width, m.head.height,
		m.head.depthWidth, m.head.depthHeight, m.head.normalWidth, m.head.normalHeight,
		m.head.xoffset, m.head.yoffset, m.head.numDirections, m.head.frameWidth, m.head.frameHeight,
		m.head.fps);
	if (m.head.mapsPresent & (1 << Maptype_Diffuse)) {
		size_t diffuseSize = sizeof(uint32_t) * m.head.width * m.head.height;
		m.diffusePixels = (uint32_t*)malloc(diffuseSize);
		archive(cereal::binary_data(m.diffusePixels, diffuseSize));
	}
	if (m.head.mapsPresent & (1 << Maptype_Normal)) {
		size_t normalSize = sizeof(uint32_t) * m.head.normalWidth * m.head.normalHeight;
		m.normalPixels = (uint32_t*)malloc(normalSize);
		archive(cereal::binary_data(m.normalPixels, normalSize));
	}
	if (m.head.mapsPresent & (1 << Maptype_Depth)) {
		size_t depthSize = sizeof(uint16_t) * m.head.depthWidth * m.head.depthHeight;
		m.depthPixels = (uint16_t*)malloc(depthSize);
		archive(cereal::binary_data(m.depthPixels, depthSize));
	}
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentImage
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, ComponentImage const& m) {
	archive(m.head.width, m.head.height);
	for (int i = 0; i < m.head.width * m.head.height; i++) {
		archive(m.pixels[i]);
	}
}

template<class Archive>
void load(Archive& archive, ComponentImage& m) {
	archive(m.head.width, m.head.height);
	size_t imgSize = sizeof(uint32_t) * m.head.width * m.head.height;
	m.pixels = (uint32_t*)malloc(imgSize);
	archive(cereal::binary_data(m.pixels, imgSize));
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentFont
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save_(Archive& archive, ComponentFont const& m, size_t const& dcs) {
	archive(m.head.style, m.head.pointSize, m.head.fontFace);
	size_t loadSize = dcs - sizeof(ComponentFont::FontHeader);
	for (size_t i = 0; i < loadSize; i++) {
		archive(m.fontData[i]);
	}
}

template<class Archive>
void load_(Archive& archive, ComponentFont& m, size_t const& dcs) {
	archive(m.head.style, m.head.pointSize, m.head.fontFace);
	size_t loadSize = dcs - sizeof(ComponentFont::FontHeader);
	m.fontData = (uint8_t*)malloc(loadSize);
	archive(cereal::binary_data(m.fontData, loadSize));
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentLevel
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, ComponentLevel const& m) {
	archive(m.head.width, m.head.height, m.head.numTiles, m.head.numEntities);
	for (int i = 0; i < m.head.numTiles; i++) {
		for (int j = 0; j < TILE_NAMELEN; j++) {
			archive(m.tiles[i].name[j]);
		}
		archive(m.tiles[i].x, m.tiles[i].y, m.tiles[i].renderType, m.tiles[i].layerOffset);
	}
	for (int i = 0; i < m.head.numEntities; i++) {
		for (int j = 0; j < ENT_NAMELEN; j++) {
			archive(m.ents[i].name[j]);
		}
		archive(m.ents[i].x, m.ents[i].y, m.ents[i].spawnflags);
	}
}

template<class Archive>
void load(Archive& archive, ComponentLevel& m) {
	archive(m.head.width, m.head.height, m.head.numTiles, m.head.numEntities);
	m.tiles = (ComponentLevel::TileEntry*)malloc(m.head.numTiles * sizeof(ComponentLevel::TileEntry));
	m.ents = (ComponentLevel::EntityEntry*)malloc(m.head.numEntities * sizeof(ComponentLevel::EntityEntry));
	for (int i = 0; i < m.head.numTiles; i++) {
		archive(cereal::binary_data(m.tiles[i].name, sizeof(char) * TILE_NAMELEN),
			m.tiles[i].x, m.tiles[i].y, m.tiles[i].renderType, m.tiles[i].layerOffset);
	}
	for (int i = 0; i < m.head.numEntities; i++) {
		archive(cereal::binary_data(m.ents[i].name, sizeof(char) * ENT_NAMELEN),
			m.ents[i].x, m.ents[i].y, m.ents[i].spawnflags);
	}
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentComp
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, ComponentComp const& m) {
	archive(m.head.numComponents, m.head.numKeyframes);
	for (int i = 0; i < m.head.numComponents; i++) {
		for (int j = 0; j < COMPPART_NAMELEN; j++) {
			archive(m.components[i].partName[j]);
		}
		for (int j = 0; j < MAT_NAMELEN; j++) {
			archive(m.components[i].matName[j]);
		}
	}
	for (int i = 0; i < m.head.numKeyframes; i++) {
		archive(m.keyframes[i].type, m.keyframes[i].frame, m.keyframes[i].parm);
	}
}

template<class Archive>
void load(Archive& archive, ComponentComp& m) {
	archive(m.head.numComponents, m.head.numKeyframes);
	m.components = (ComponentComp::CompComponent*)malloc(sizeof(ComponentComp::CompComponent) * m.head.numComponents);
	m.keyframes = (ComponentComp::CompKeyframe*)malloc(sizeof(ComponentComp::CompKeyframe) * m.head.numKeyframes);
	for (int i = 0; i < m.head.numComponents; i++) {
		archive(cereal::binary_data(m.components[i].partName, sizeof(char) * COMPPART_NAMELEN),
			cereal::binary_data(m.components[i].matName, sizeof(char) * MAT_NAMELEN));
	}
	for (int i = 0; i < m.head.numKeyframes; i++) {
		archive(m.keyframes[i].type, m.keyframes[i].frame, m.keyframes[i].parm);
	}
}

/////////////////////////////////////////////////////////////////////////
//
// struct ComponentTile
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void serialize(Archive& archive, ComponentTile& m) {
	archive(m.walkmask, m.jumpmask, m.shotmask, m.lightmask, m.vismask, m.warpmask,
		cereal::binary_data(m.materialName, sizeof(char) * MAT_NAMELEN),
		m.becomeTransparent, m.autoTransX, m.autoTransY, m.autoTransW, m.autoTransH,
		cereal::binary_data(m.transMaterialName, sizeof(char) * MAT_NAMELEN),
		m.animated, m.frameNum, m.transFrameNum, m.depthscoreOffset);
}

/////////////////////////////////////////////////////////////////////////
//
// struct AssetComponent
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, AssetComponent const& m) {
	for (int i = 0; i < COMP_NAMELEN; i++) {
		archive(m.meta.componentName[i]);
	}
	archive(m.meta.componentType, m.meta.decompressedSize, m.meta.componentVersion);
	switch (m.meta.componentType) {
		case Asset_Undefined:
			archive(cereal::binary_data(m.data.undefinedComponent, m.meta.decompressedSize));
			break;
		case Asset_Data:
			save_(archive, *m.data.dataComponent, m.meta.decompressedSize);
			break;
		case Asset_Material:
			archive(*m.data.materialComponent);
			break;
		case Asset_Image:
			archive(*m.data.imageComponent);
			break;
		case Asset_Font:
			save_(archive, *m.data.fontComponent, m.meta.decompressedSize);
			break;
		case Asset_Level:
			archive(*m.data.levelComponent);
			break;
		case Asset_Composition:
			archive(*m.data.compComponent);
			break;
		case Asset_Tile:
			archive(*m.data.tileComponent);
			break;
	}
}

template<class Archive>
void load(Archive& archive, AssetComponent& m) {
	archive(cereal::binary_data(m.meta.componentName, sizeof(char) * COMP_NAMELEN),
		m.meta.componentType, m.meta.decompressedSize, m.meta.componentVersion);
	switch (m.meta.componentType) {
		case Asset_Undefined:
			m.data.undefinedComponent = malloc(m.meta.decompressedSize);
			archive(cereal::binary_data(m.data.undefinedComponent, m.meta.decompressedSize));
			break;
		case Asset_Data:
			m.data.dataComponent = (ComponentData*)malloc(sizeof(ComponentData));
			load_(archive, *m.data.dataComponent, m.meta.decompressedSize);
			break;
		case Asset_Material:
			m.data.materialComponent = (ComponentMaterial*)malloc(sizeof(ComponentMaterial));
			archive(*m.data.materialComponent);
			break;
		case Asset_Image:
			m.data.imageComponent = (ComponentImage*)malloc(sizeof(ComponentImage));
			archive(*m.data.imageComponent);
			break;
		case Asset_Font:
			m.data.fontComponent = (ComponentFont*)malloc(sizeof(ComponentFont));
			load_(archive, *m.data.fontComponent, m.meta.decompressedSize);
			break;
		case Asset_Level:
			m.data.levelComponent = (ComponentLevel*)malloc(sizeof(ComponentLevel));
			archive(*m.data.levelComponent);
			break;
		case Asset_Composition:
			m.data.compComponent = (ComponentComp*)malloc(sizeof(ComponentComp));
			archive(*m.data.compComponent);
			break;
		case Asset_Tile:
			m.data.tileComponent = (ComponentTile*)malloc(sizeof(ComponentTile));
			archive(*m.data.tileComponent);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////
//
// struct RaptureAsset
//
/////////////////////////////////////////////////////////////////////////
template<class Archive>
void save(Archive& archive, RaptureAsset const& m) {

}

template<class Archive>
void load(Archive& archive, RaptureAsset& m) {

}