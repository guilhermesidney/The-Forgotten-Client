/*
  Tibia CLient
  Copyright (C) 2019 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "creature.h"
#include "engine.h"
#include "light.h"
#include "map.h"
#include "tile.h"
#include "game.h"

double Creature::speedA = 857.36;
double Creature::speedB = 261.29;
double Creature::speedC = -4795.01;

extern Engine g_engine;
extern Map g_map;
extern ThingManager g_thingManager;
extern LightSystem g_light;
extern Game g_game;
extern Uint32 g_frameTime;

Creature::Creature()
{
	m_thingType = NULL;
	m_mountType = NULL;
	for(Sint32 i = ThingFrameGroup_Default; i < ThingFrameGroup_Last; ++i)
	{
		m_outfitAnimator[i] = NULL;
		m_mountAnimator[i] = NULL;
	}

	m_tile = NULL;
	m_walkTile = NULL;
	m_drawnTile = NULL;

	m_nameLen = 0;
	m_updateTime = 0;
	m_walkStartTime = 0;
	m_walkEndTime = 0;
	m_walkTime = 0;
	m_timedSquareStartTime = 0;

	m_walkedPixels = 0;
	m_walkOffsetX = 0;
	m_walkOffsetY = 0;

	m_id = 0;
	m_light[0] = m_light[1] = 0;
	m_speed = 220;
	m_helpers = 0;

	m_outfit = 0x4C3A454E;
	m_lookAddons = 0;

	m_health = 100;
	m_skull = SKULL_NONE;
	m_shield = SHIELD_NONE;
	m_emblem = GUILDEMBLEM_NONE;
	m_icon = CREATUREICON_NONE;
	m_type = CREATURETYPE_NPC;

	m_timedSquareRed = 0;
	m_timedSquareGreen = 0;
	m_timedSquareBlue = 0;
	m_staticSquareRed = 0;
	m_staticSquareGreen = 0;
	m_staticSquareBlue = 0;
	m_red = 0;
	m_green = 188;
	m_blue = 0;

	m_direction = DIRECTION_SOUTH;
	m_turnDirection = DIRECTION_SOUTH;
	m_walkDirection = DIRECTION_SOUTH;

	m_outfitAnim = 0;
	m_mountAnim = 0;
	m_currentFrame = ThingFrameGroup_Idle;
	m_animationFrame = ThingFrameGroup_Idle;

	m_walking = false;
	m_preWalking = false;
	m_isLocalCreature = false;
	m_unpassable = true;
	m_showTimedSquare = false;
	m_showStaticSquare = false;
}

void Creature::preMove(const Position& fromPos, const Position& toPos)
{
	Tile* oldTile = g_map.getTile(fromPos);
	Tile* newTile = g_map.getTile(toPos);
	if(!oldTile || !newTile)
		return;

	move(fromPos, toPos, oldTile, newTile, true);
}

void Creature::move(const Position& fromPos, const Position& toPos, Tile* oldTile, Tile* newTile, bool preMove)
{
	if(!preMove)
	{
		if(!oldTile->removeThing(this, true))
			return;

		newTile->addThing(this, (g_clientVersion < 853), true);
	}
	if(fromPos.x > toPos.x)
	{
		m_direction = DIRECTION_WEST;
		m_turnDirection = DIRECTION_WEST;
		if(fromPos.y > toPos.y)
		{
			m_walkDirection = DIRECTION_NORTHWEST;
			m_walkOffsetX = 32;
			m_walkOffsetY = 32;
			addToDrawnTile(oldTile);
		}
		else if(fromPos.y < toPos.y)
		{
			m_walkDirection = DIRECTION_SOUTHWEST;

			Position newPos = Position(toPos.x+1, toPos.y, toPos.z);
			Tile* newestTile = g_map.getTile(newPos);
			if(newestTile)
			{
				m_walkOffsetX = 32;
				m_walkOffsetY = 0;
				addToDrawnTile(newestTile);
			}
			else
			{
				//If somehow failed let's keep the "glitchy" movement
				m_walkOffsetX = 0;
				m_walkOffsetY = 0;
				addToDrawnTile(newTile);
			}
		}
		else
		{
			m_walkDirection = DIRECTION_WEST;
			m_walkOffsetX = 32;
			m_walkOffsetY = 0;
			addToDrawnTile(oldTile);
		}
	}
	else if(fromPos.x < toPos.x)
	{
		m_direction = DIRECTION_EAST;
		m_turnDirection = DIRECTION_EAST;
		if(fromPos.y > toPos.y)
		{
			m_walkDirection = DIRECTION_NORTHEAST;

			Position newPos = Position(fromPos.x+1, fromPos.y, fromPos.z);
			Tile* newestTile = g_map.getTile(newPos);
			if(newestTile)
			{
				m_walkOffsetX = 0;
				m_walkOffsetY = 32;
				addToDrawnTile(newestTile);
			}
			else
			{
				//If somehow failed let's keep the "glitchy" movement
				m_walkOffsetX = -32;
				m_walkOffsetY = 32;
				addToDrawnTile(oldTile);
			}
		}
		else if(fromPos.y < toPos.y)
		{
			m_walkDirection = DIRECTION_SOUTHEAST;
			m_walkOffsetX = 0;
			m_walkOffsetY = 0;
			addToDrawnTile(newTile);
		}
		else
		{
			m_walkDirection = DIRECTION_EAST;
			m_walkOffsetX = 0;
			m_walkOffsetY = 0;
			addToDrawnTile(newTile);
		}
	}
	else if(fromPos.y > toPos.y)
	{
		m_direction = DIRECTION_NORTH;
		m_turnDirection = DIRECTION_NORTH;
		m_walkDirection = DIRECTION_NORTH;
		m_walkOffsetX = 0;
		m_walkOffsetY = 32;
		addToDrawnTile(oldTile);
	}
	else if(fromPos.y < toPos.y)
	{
		m_direction = DIRECTION_SOUTH;
		m_turnDirection = DIRECTION_SOUTH;
		m_walkDirection = DIRECTION_SOUTH;
		m_walkOffsetX = 0;
		m_walkOffsetY = 0;
		addToDrawnTile(newTile);
	}

	m_walkTile = newTile;
	if(m_preWalking && !preMove)
	{
		m_preWalking = false;
		return;
	}
	else if(m_isLocalCreature)
	{
		//pass to g_game to check if we need decrease autowalk directions
	}
	m_walking = true;
	m_preWalking = preMove;
	m_walkStartTime = g_frameTime;
	m_walkEndTime = g_frameTime+getStepDuration(false);
	m_walkTime = getStepDuration(true);
	m_walkedPixels = 0;
	if(m_thingType->hasFlag(ThingAttribute_NoMoveAnimation))
		m_currentFrame = ThingFrameGroup_Idle;
	else
		m_currentFrame = ThingFrameGroup_Moving;
}

void Creature::stopMove()
{
	m_currentFrame = ThingFrameGroup_Idle;
	m_preWalking = false;
	m_walking = false;
	if(m_isLocalCreature)
		g_game.checkLocalCreatureMovement();

	if(m_turnDirection != m_direction)
		m_direction = m_turnDirection;
}

void Creature::addToDrawnTile(Tile* tile)
{
	removeFromDrawnTile();

	m_drawnTile = tile;
	m_drawnTile->addWalkCreature(this);
}

void Creature::resetDrawnTile()
{
	stopMove();
	m_drawnTile = NULL;
}

void Creature::removeFromDrawnTile()
{
	if(m_drawnTile)
	{
		m_drawnTile->removeWalkCreature(this);
		m_drawnTile = NULL;
	}
}

void Creature::updateFrameGroup(Uint8 frameGroup)
{
	if(m_animationFrame != frameGroup)
	{
		m_animationFrame = frameGroup;
		if(m_outfitAnimator[frameGroup] && (m_outfitAnimation[frameGroup].m_lastPhaseTicks + CREATURE_ANIMATION_DELAY_RESET < g_frameTime || frameGroup != ThingFrameGroup_Moving))
			m_outfitAnimator[frameGroup]->resetAnimation(m_outfitAnimation[frameGroup]);

		if(m_mountAnimator[frameGroup] && (m_mountAnimation[frameGroup].m_lastPhaseTicks + CREATURE_ANIMATION_DELAY_RESET < g_frameTime || frameGroup != ThingFrameGroup_Moving))
			m_mountAnimator[frameGroup]->resetAnimation(m_mountAnimation[frameGroup]);
	}
}

void Creature::update()
{
	if(m_updateTime != g_frameTime)
	{
		m_updateTime = g_frameTime;
		if(m_walking)
		{
			float walkTicks = (m_walkTime*0.03125f);
			m_walkedPixels = UTIL_min<Sint32>(SDL_static_cast(Sint32, (g_frameTime - m_walkStartTime) / walkTicks), 32);
			if(m_walkedPixels >= 32)
			{
				if(!m_preWalking || (g_frameTime - m_walkStartTime) >= 1000)//If we don't get the walk packet in 1s just end prewalking
				{
					if(g_frameTime >= m_walkEndTime)
						stopMove();
					else
						m_currentFrame = ThingFrameGroup_Idle;//Since we don't move keep idle animation
				}
				else
					m_currentFrame = ThingFrameGroup_Idle;//Since we don't move keep idle animation
			}
		}
		updateFrameGroup(m_currentFrame);
		if(m_thingType)
		{
			if(m_outfitAnimator[m_currentFrame])
			{
				//Calculate with new animation
				m_outfitAnim = SDL_static_cast(Uint8, m_outfitAnimator[m_currentFrame]->getPhase(m_outfitAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : m_walkTime)));
			}
			else
			{
				//Calculate with old animation
				if(m_thingType->m_category == ThingCategory_Effect)
				{
					Uint8 animCount = m_thingType->m_frameGroup[m_currentFrame].m_animCount;
					if(animCount > 2)
						m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), animCount - 2) + 1;
					else
						m_outfitAnim = 0;
				}
				else if(m_thingType->m_category == ThingCategory_Item)
					m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), m_thingType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_thingType->hasFlag(ThingAttribute_AnimateAlways))
					m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_thingType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_walking)
				{
					Uint8 animCount = m_thingType->m_frameGroup[m_currentFrame].m_animCount;
					if(animCount > 1 && m_walkedPixels < 32)
						m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, m_walkedPixels / 8), animCount - 1) + 1;
					else
						m_outfitAnim = 0;
				}
				else
					m_outfitAnim = 0;
			}
		}
		if(m_mountType)
		{
			if(m_mountAnimator[m_currentFrame])
			{
				//Calculate with new animation
				m_mountAnim = SDL_static_cast(Uint8, m_mountAnimator[m_currentFrame]->getPhase(m_mountAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : m_walkTime)));
			}
			else
			{
				//Calculate with old animation
				if(m_mountType->hasFlag(ThingAttribute_AnimateAlways))
					m_mountAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_mountType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_walking)
				{
					Uint8 animCount = UTIL_max<Uint8>(0, (m_mountType->m_frameGroup[m_currentFrame].m_animCount - 1));
					if(animCount >= 1 && m_walkedPixels < 32)
						m_mountAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, m_walkedPixels / 8), animCount) + 1;
					else
						m_mountAnim = 0;
				}
				else
					m_mountAnim = 0;
			}
		}
	}
}

void Creature::render(Sint32 posX, Sint32 posY, bool)
{
	if(!m_thingType)
		return;

	Surface* renderer = g_engine.getRender();
	update();

	posX += getOffsetX();
	posY += getOffsetY();

	Uint8 zPattern = 0;
	if(m_thingType->m_category != ThingCategory_Creature)
	{
		if(m_thingType->m_category == ThingCategory_Effect)
		{
			posX -= 8;
			posY -= 8;
		}
	}
	else
	{
		if(m_mountType)
		{
			posX -= m_mountType->m_displacement[0];
			posY -= m_mountType->m_displacement[1];
			Sint32 posYc = posY;
			for(Uint8 y = 0; y < m_mountType->m_frameGroup[m_currentFrame].m_height; ++y)
			{
				Sint32 posXc = posX;
				for(Uint8 x = 0; x < m_mountType->m_frameGroup[m_currentFrame].m_width; ++x)
				{
					Uint32 sprite = m_mountType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, 0, m_mountAnim);
					if(sprite != 0)
						renderer->drawSprite(sprite, posXc, posYc);
					posXc -= 32;
				}
				posYc -= 32;
			}
			zPattern = UTIL_min<Uint8>(1, m_thingType->m_frameGroup[m_currentFrame].m_patternZ-1);
		}
		else
		{
			posX -= m_thingType->m_displacement[0];
			posY -= m_thingType->m_displacement[1];
		}
	}

	Uint16* light = m_thingType->m_light;
	if(m_light[0] > light[0])
		light = m_light;

	if(m_isLocalCreature && (g_light.getLightIntensity() < 64 || m_position.z > 7))
	{
		if(light[0] < 3)
			light[0] = 3;
		if(light[1] == 0)
			light[1] = 215;
	}

	if(light[0] > 0)
		g_light.addLightSource(posX, posY, light);

	//TODO: Optimize the line drawning in one go
	Sint32 squareX = posX;
	Sint32 squareY = posY;
	Sint32 squareW = 32;
	Sint32 squareH = 32;
	if(g_game.getAttackID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
		{
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 248, 164, 164, 255);
			renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, 248, 164, 164, 255);
		}
		else
		{
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 224, 64, 64, 255);
			renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, 224, 64, 64, 255);
		}
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	else if(g_game.getFollowID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
		{
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 180, 248, 180, 255);
			renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, 180, 248, 180, 255);
		}
		else
		{
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 64, 224, 64, 255);
			renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, 64, 224, 64, 255);
		}
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	else if(g_game.getSelectID() == m_id)
	{
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 248, 248, 248, 255);
		renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, 248, 248, 248, 255);
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	if(m_showStaticSquare)
	{
		renderer->drawRectangle(squareX, squareY, squareW, squareH, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue, 255);
		renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue, 255);
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	if(m_showTimedSquare)
	{
		if(g_frameTime-m_timedSquareStartTime >= 1000)
			removeTimedSquare();
		else
		{
			renderer->drawRectangle(squareX, squareY, squareW, squareH, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue, 255);
			renderer->drawRectangle(squareX+1, squareY+1, squareW-2, squareH-2, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue, 255);
		}
	}

	if(m_thingType->m_frameGroup[m_currentFrame].m_layers > 1)
	{
		for(Uint8 y = 0; y < m_thingType->m_frameGroup[m_currentFrame].m_height; ++y)
		{
			Sint32 posXc = posX;
			for(Uint8 x = 0; x < m_thingType->m_frameGroup[m_currentFrame].m_width; ++x)
			{
				Uint32 sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, zPattern, m_outfitAnim);
				Uint32 spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 0, zPattern, m_outfitAnim);
				if(sprite != 0)
				{
					if(spriteMask != 0)
						renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
					else
						renderer->drawSprite(sprite, posXc, posY);
				}
				if(m_thingType->m_frameGroup[m_currentFrame].m_patternY > 1)
				{
					if(m_lookAddons & 1)//First addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 1, zPattern, m_outfitAnim);
						spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 1, zPattern, m_outfitAnim);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
							else
								renderer->drawSprite(sprite, posXc, posY);
						}
					}
					if(m_lookAddons & 2)//Second addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 2, zPattern, m_outfitAnim);
						spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 2, zPattern, m_outfitAnim);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
							else
								renderer->drawSprite(sprite, posXc, posY);
						}
					}
				}
				posXc -= 32;
			}
			posY -= 32;
		}
	}
	else
	{
		for(Uint8 y = 0; y < m_thingType->m_frameGroup[m_currentFrame].m_height; ++y)
		{
			Sint32 posXc = posX;
			for(Uint8 x = 0; x < m_thingType->m_frameGroup[m_currentFrame].m_width; ++x)
			{
				Uint32 sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, zPattern, m_outfitAnim);
				if(sprite != 0)
					renderer->drawSprite(sprite, posXc, posY);
				if(m_thingType->m_frameGroup[m_currentFrame].m_patternY > 1)
				{
					if(m_lookAddons & 1)//First addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 1, zPattern, m_outfitAnim);
						if(sprite != 0)
							renderer->drawSprite(sprite, posXc, posY);
					}
					if(m_lookAddons & 2)//Second addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 2, zPattern, m_outfitAnim);
						if(sprite != 0)
							renderer->drawSprite(sprite, posXc, posY);
					}
				}
				posXc -= 32;
			}
			posY -= 32;
		}
	}
}

void Creature::renderInformations(Sint32 posX, Sint32 posY, Sint32 drawX, Sint32 drawY, float scale, bool visible)
{
	if(m_health < 1)
		return;

	Uint8 red = m_red, green = m_green, blue = m_blue;
	if(!visible)
	{
		red = 191;
		green = 191;
		blue = 191;
	}

	drawX += getOffsetX();
	drawY += getOffsetY();
	if(m_thingType->m_category != ThingCategory_Creature)
	{
		if(m_thingType->m_category == ThingCategory_Effect)
		{
			drawX -= 8;
			drawY -= 8;
		}
	}
	else
	{
		if(m_mountType)
		{
			drawX -= SDL_static_cast(Sint32, m_mountType->m_displacement[0]);
			drawY -= SDL_static_cast(Sint32, m_mountType->m_displacement[1]);
		}
		else
		{
			drawX -= SDL_static_cast(Sint32, m_thingType->m_displacement[0]);
			drawY -= SDL_static_cast(Sint32, m_thingType->m_displacement[1]);
		}
	}
	posX += SDL_static_cast(Sint32, drawX*scale);
	posY += SDL_static_cast(Sint32, drawY*scale)-16;
	g_engine.drawFont(CLIENT_FONT_OUTLINED, posX-m_nameLen, posY, m_name, red, green, blue, CLIENT_FONT_ALIGN_LEFT);

	Surface* renderer = g_engine.getRender();
	renderer->fillRectangle(posX-14, posY+12, 28, 4, 0, 0, 0, 255);
	renderer->fillRectangle(posX-13, posY+13, SDL_static_cast(Sint32, m_health*0.26f), 2, red, green, blue, 255);

	Sint32 POSX = posX+9;
	Sint32 POSY = posY+18;
	if(m_shield != SHIELD_NONE)
	{
		if(m_shield == SHIELD_BLUE_NOSHAREDEXP_BLINK || m_shield == SHIELD_YELLOW_NOSHAREDEXP_BLINK)
		{
			if(g_frameTime-m_shieldTime >= CREATURE_SHIELD_BLINK_TICKS)
			{
				m_shieldTime = g_frameTime;
				m_showShield = !m_showShield;
			}
			if(m_showShield)
				renderer->drawPicture(3, m_shieldX, m_shieldY, POSX, POSY, 11, 11);
		}
		else
			renderer->drawPicture(3, m_shieldX, m_shieldY, POSX, POSY, 11, 11);
		POSX += 13;
	}

	if(m_emblem != GUILDEMBLEM_NONE)
		renderer->drawPicture(3, m_emblemX, m_emblemY, POSX, POSY+13, 11, 11);

	if(m_skull != SKULL_NONE)
	{
		renderer->drawPicture(3, m_skullX, m_skullY, POSX, POSY, 11, 11);
		POSX += 13;
	}

	if(m_type >= CREATURETYPE_SUMMON_OWN)
	{
		renderer->drawPicture(3, m_typeX, m_typeY, POSX, POSY, 11, 11);
		POSX += 13;
	}

	if(m_icon != CREATUREICON_NONE)
	{
		renderer->drawPicture(3, m_iconX, m_iconY, POSX, POSY, 18, 18);
		POSX += 20;
	}
}

void Creature::addTimedSquare(Uint8 color)
{
	m_timedSquareStartTime = g_frameTime;
	colorFrom8bit(color, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue);
	m_showTimedSquare = true;
}

void Creature::removeTimedSquare()
{
	m_showTimedSquare = false;
}

void Creature::showStaticSquare(Uint8 color)
{
	colorFrom8bit(color, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue);
	m_showStaticSquare = true;
}

void Creature::hideStaticSquare()
{
	m_showStaticSquare = false;
}

void Creature::setName(const std::string name)
{
	PERFORM_MOVE(m_name, name);
	m_nameLen = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_name)/2;
}

void Creature::setHealth(Uint8 health)
{
	m_health = health;
	if(m_health < 4)
	{
		m_red = 96;
		m_green = 0;
		m_blue = 0;
	}
	else if(m_health < 10)
	{
		m_red = 192;
		m_green = 0;
		m_blue = 0;
	}
	else if(m_health < 30)
	{
		m_red = 192;
		m_green = 48;
		m_blue = 48;
	}
	else if(m_health < 60)
	{
		m_red = 192;
		m_green = 192;
		m_blue = 0;
	}
	else if(m_health < 95)
	{
		m_red = 96;
		m_green = 192;
		m_blue = 96;
	}
	else
	{
		m_red = 0;
		m_green = 192;
		m_blue = 0;
	}
}

void Creature::turnDirection(Direction direction)
{
	if(m_walking)
		m_turnDirection = direction;//Schedule turn direction after walk complete
	else
		m_direction = direction;//Change our direction right away if we not walking
}

void Creature::setOutfit(Uint16 lookType, Uint16 lookTypeEx, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount)
{
	if(lookType == 0)
	{
		if(lookTypeEx == 0)
			m_thingType = g_thingManager.getThingType(ThingCategory_Effect, 13);
		else
			m_thingType = g_thingManager.getThingType(ThingCategory_Item, lookTypeEx);
	}
	else
		m_thingType = g_thingManager.getThingType(ThingCategory_Creature, lookType);

	if(lookMount == 0)
		m_mountType = NULL;
	else
		m_mountType = g_thingManager.getThingType(ThingCategory_Creature, lookMount);

	for(Sint32 i = ThingFrameGroup_Default; i < ThingFrameGroup_Last; ++i)
	{
		m_outfitAnimator[i] = (m_thingType ? m_thingType->m_frameGroup[i].m_animator : NULL);
		if(m_outfitAnimator[i])
			m_outfitAnimator[i]->resetAnimation(m_outfitAnimation[i]);

		m_mountAnimator[i] = (m_mountType ? m_mountType->m_frameGroup[i].m_animator : NULL);
		if(m_mountAnimator[i])
			m_mountAnimator[i]->resetAnimation(m_mountAnimation[i]);
	}

	m_outfit = (lookFeet << 24) | (lookLegs << 16) | (lookBody << 8) | (lookHead);
	m_lookAddons = lookAddons;
}

void Creature::setSkull(Uint8 skull)
{
	m_skull = skull;
	switch(skull)
	{
		case SKULL_YELLOW:
		{
			m_skullX = 65;
			m_skullY = 225;
		}
		break;
		case SKULL_GREEN:
		{
			m_skullX = 54;
			m_skullY = 225;
		}
		break;
		case SKULL_WHITE:
		{
			m_skullX = 76;
			m_skullY = 225;
		}
		break;
		case SKULL_RED:
		{
			m_skullX = 87;
			m_skullY = 225;
		}
		break;
		case SKULL_BLACK:
		{
			m_skullX = 98;
			m_skullY = 297;
		}
		break;
		case SKULL_ORANGE:
		{
			m_skullX = 208;
			m_skullY = 218;
		}
		break;
		default:
		{
			m_skullX = 290;
			m_skullY = 12;
		}
		break;
	}
}

void Creature::setShield(Uint8 shield)
{
	m_shield = shield;
	switch(shield)
	{
		case SHIELD_WHITEYELLOW:
		{
			m_shieldX = 76;
			m_shieldY = 236;
		}
		break;
		case SHIELD_WHITEBLUE:
		{
			m_shieldX = 87;
			m_shieldY = 236;
		}
		break;
		case SHIELD_BLUE:
		{
			m_shieldX = 65;
			m_shieldY = 236;
		}
		break;
		case SHIELD_YELLOW:
		{
			m_shieldX = 54;
			m_shieldY = 236;
		}
		break;
		case SHIELD_BLUE_SHAREDEXP:
		{
			m_shieldX = 87;
			m_shieldY = 214;
		}
		break;
		case SHIELD_YELLOW_SHAREDEXP:
		{
			m_shieldX = 76;
			m_shieldY = 214;
		}
		break;
		case SHIELD_BLUE_NOSHAREDEXP_BLINK:
		{
			m_shieldX = 179;
			m_shieldY = 261;
			m_shieldTime = g_frameTime;
			m_showShield = true;
		}
		break;
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
		{
			m_shieldX = 168;
			m_shieldY = 261;
			m_shieldTime = g_frameTime;
			m_showShield = true;
		}
		break;
		case SHIELD_BLUE_NOSHAREDEXP:
		{
			m_shieldX = 179;
			m_shieldY = 261;
		}
		break;
		case SHIELD_YELLOW_NOSHAREDEXP:
		{
			m_shieldX = 168;
			m_shieldY = 261;
		}
		break;
		case SHIELD_GRAY:
		{
			m_shieldX = 43;
			m_shieldY = 236;
		}
		break;
		default:
		{
			m_shieldX = 290;
			m_shieldY = 12;
		}
		break;
	}
}

void Creature::setEmblem(Uint8 emblem)
{
	m_emblem = emblem;
	switch(emblem)
	{
		case GUILDEMBLEM_ALLY:
		{
			m_emblemX = 287;
			m_emblemY = 218;
		}
		break;
		case GUILDEMBLEM_ENEMY:
		{
			m_emblemX = 298;
			m_emblemY = 218;
		}
		break;
		case GUILDEMBLEM_NEUTRAL:
		{
			m_emblemX = 309;
			m_emblemY = 218;
		}
		break;
		case GUILDEMBLEM_MEMBER:
		{
			m_emblemX = 219;
			m_emblemY = 218;
		}
		break;
		case GUILDEMBLEM_OTHER:
		{
			m_emblemX = 276;
			m_emblemY = 218;
		}
		break;
		default:
		{
			m_emblemX = 290;
			m_emblemY = 12;
		}
		break;
	}
}

void Creature::setType(Uint8 type)
{
	m_type = type;
	switch(type)
	{
		case CREATURETYPE_SUMMON_OWN:
		{
			m_typeX = 220;
			m_typeY = 229;
		}
		break;
		case CREATURETYPE_SUMMON_OTHERS:
		{
			m_typeX = 220;
			m_typeY = 240;
		}
		break;
		default:
		{
			m_typeX = 290;
			m_typeY = 12;
		}
		break;
	}
}

void Creature::setIcon(Uint8 icon)
{
	m_icon = icon;
	switch(icon)
	{
		case CREATUREICON_SPEECH:
		{
			m_iconX = 240;
			m_iconY = 317;
		}
		break;
		case CREATUREICON_TRADE:
		{
			m_iconX = 240;
			m_iconY = 335;
		}
		break;
		case CREATUREICON_QUEST:
		{
			m_iconX = 240;
			m_iconY = 353;
		}
		break;
		case CREATUREICON_QUESTTRADER:
		{
			m_iconX = 190;
			m_iconY = 254;
		}
		break;
		default:
		{
			m_iconX = 290;
			m_iconY = 12;
		}
		break;
	}
}

Sint32 Creature::getOffsetX(bool checkPreWalk)
{
	if(!m_walking) return 0;
	switch(m_walkDirection)
	{
		case DIRECTION_NORTH:
			return 0;
		case DIRECTION_EAST:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTH:
			return 0;
		case DIRECTION_WEST:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHWEST:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHEAST:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHWEST:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHEAST:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
	}
	return 0;
}

Sint32 Creature::getOffsetY(bool checkPreWalk)
{
	if(!m_walking) return 0;
	switch(m_walkDirection)
	{
		case DIRECTION_NORTH:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_EAST:
			return 0;
		case DIRECTION_SOUTH:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_WEST:
			return 0;
		case DIRECTION_SOUTHWEST:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHEAST:
			return m_walkedPixels-((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHWEST:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHEAST:
			return -m_walkedPixels+((checkPreWalk && m_preWalking) ? 0 : 32);
	}
	return 0;
}

Sint32 Creature::getStepDuration(bool ignoreDiagonal)
{
	Sint32 speed = SDL_static_cast(Sint32, m_speed);
	if(speed < 1)
		return 0;

	Sint32 groundSpeed = 150;
	if(m_walkTile)
	{
		groundSpeed = SDL_static_cast(Sint32, m_walkTile->getGroundSpeed());
		if(groundSpeed == 0)
			groundSpeed = 150;
	}

	Sint32 interval = 1000;
	if(groundSpeed > 0)
		interval = 1000 * groundSpeed;

	if(g_game.hasGameFeature(GAME_FEATURE_NEWSPEED_LAW))
	{
		double formulatedSpeed = 1.0;
		if(speed > -Creature::speedB)
		{
			formulatedSpeed = SDL_floor((Creature::speedA * log(speed + Creature::speedB) + Creature::speedC) + 0.5);
			if(formulatedSpeed < 1.0)
				formulatedSpeed = 1.0;
		}
		interval = SDL_static_cast(Sint32, SDL_floor(interval / formulatedSpeed));
	}
	else
		interval /= speed;

	Sint32 serverBeat = SDL_static_cast(Sint32, g_game.getServerBeat());
	if(g_clientVersion >= 900)
		interval = (interval / (serverBeat + serverBeat - 1)) * serverBeat;

	if(!ignoreDiagonal && (m_walkDirection & DIRECTION_DIAGONAL_MASK) != 0)
		interval *= (g_game.hasGameFeature(GAME_FEATURE_LONGER_DIAGONAL) ? 3 : 2);

	return interval;
}