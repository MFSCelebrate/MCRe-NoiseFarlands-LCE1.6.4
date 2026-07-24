#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "DyePowderItem.h"
#include "WoolTileItem.h"

const unsigned int WoolTileItem::COLOR_DESCS[] = 
{ 
	IDS_TILE_CLOTH_BLACK,
	IDS_TILE_CLOTH_RED,
	IDS_TILE_CLOTH_GREEN,
	IDS_TILE_CLOTH_BROWN,
	IDS_TILE_CLOTH_BLUE,
	IDS_TILE_CLOTH_PURPLE,
	IDS_TILE_CLOTH_CYAN,
	IDS_TILE_CLOTH_SILVER,
	IDS_TILE_CLOTH_GRAY,
	IDS_TILE_CLOTH_PINK,
	IDS_TILE_CLOTH_LIME,
	IDS_TILE_CLOTH_YELLOW,
	IDS_TILE_CLOTH_LIGHT_BLUE,
	IDS_TILE_CLOTH_MAGENTA,
	IDS_TILE_CLOTH_ORANGE,
	IDS_TILE_CLOTH_WHITE
};

const unsigned int WoolTileItem::CARPET_COLOR_DESCS[] = 
{ 
	IDS_TILE_CARPET_BLACK,
	IDS_TILE_CARPET_RED,
	IDS_TILE_CARPET_GREEN,
	IDS_TILE_CARPET_BROWN,
	IDS_TILE_CARPET_BLUE,
	IDS_TILE_CARPET_PURPLE,
	IDS_TILE_CARPET_CYAN,
	IDS_TILE_CARPET_SILVER,
	IDS_TILE_CARPET_GRAY,
	IDS_TILE_CARPET_PINK,
	IDS_TILE_CARPET_LIME,
	IDS_TILE_CARPET_YELLOW,
	IDS_TILE_CARPET_LIGHT_BLUE,
	IDS_TILE_CARPET_MAGENTA,
	IDS_TILE_CARPET_ORANGE,
	IDS_TILE_CARPET_WHITE
};

const unsigned int WoolTileItem::CLAY_COLOR_DESCS[] = 
{ 
	IDS_TILE_STAINED_CLAY_BLACK,
	IDS_TILE_STAINED_CLAY_RED,
	IDS_TILE_STAINED_CLAY_GREEN,
	IDS_TILE_STAINED_CLAY_BROWN,
	IDS_TILE_STAINED_CLAY_BLUE,
	IDS_TILE_STAINED_CLAY_PURPLE,
	IDS_TILE_STAINED_CLAY_CYAN,
	IDS_TILE_STAINED_CLAY_SILVER,
	IDS_TILE_STAINED_CLAY_GRAY,
	IDS_TILE_STAINED_CLAY_PINK,
	IDS_TILE_STAINED_CLAY_LIME,
	IDS_TILE_STAINED_CLAY_YELLOW,
	IDS_TILE_STAINED_CLAY_LIGHT_BLUE,
	IDS_TILE_STAINED_CLAY_MAGENTA,
	IDS_TILE_STAINED_CLAY_ORANGE,
	IDS_TILE_STAINED_CLAY_WHITE
};

WoolTileItem::WoolTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *WoolTileItem::getIcon(int itemAuxValue) 
{
	{
		return Tile::wool->getTexture(2, ColoredTile::getTileDataForItemAuxValue(itemAuxValue));
	}
}

int WoolTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

unsigned int WoolTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	int tileId = getTileId();
	switch(getTileId())
	{
	case Tile::clayHardened_colored_Id:
		return CLAY_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::woolCarpet_Id:
		return CARPET_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::wool_Id:
	default:
		return COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	};
}
