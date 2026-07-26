#pragma once
using namespace std;

#include "Packet.h"
#include <cstdint>   // 提供 int64_t, uint64_t 等

class LevelType;
class GameType;

class RespawnPacket : public Packet, public enable_shared_from_this<RespawnPacket>
{
public:
	char dimension;
	char difficulty;
	int64_t mapSeed;
    int mapHeight;
    GameType *playerGameType;
	bool m_newSeaLevel;	// 4J added
	LevelType *m_pLevelType;
	int m_newEntityId;
	int64_t m_xzSize; // 4J Added
	int64_t m_hellScale; // 4J Added

	RespawnPacket();
	RespawnPacket(char dimension, int64_t mapSeed, int mapHeight, GameType *playerGameType, char difficulty, LevelType *pLevelType, bool newSeaLevel, int newEntityId, int64_t xzSize, int64_t hellScale);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<RespawnPacket>(); }
	virtual int getId() { return 9; }
};
