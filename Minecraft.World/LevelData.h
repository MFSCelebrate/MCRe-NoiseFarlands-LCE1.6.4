#pragma once
using namespace std;

#include "GameRules.h"
#include <cstdint>   // 提供 int64_t, uint64_t 等

class Player;
class CompoundTag;
class LevelSettings;
class LevelType;
class GameType;

class LevelData
{
	friend class DerivedLevelData;
private:
	int64_t seed;
	LevelType *m_pGenerator;// = LevelType.normal;
	wstring generatorOptions;
	int xSpawn;
	int ySpawn;
	int zSpawn;
	int64_t gameTime;
	int64_t dayTime;
	int64_t lastPlayed;
	int64_t sizeOnDisk;
//    CompoundTag *loadedPlayerTag;	// 4J removed
	int dimension;
	wstring levelName;
	int version;

	bool raining;
	int rainTime;

	bool thundering;
	int thunderTime;
	GameType *gameType;
	bool generateMapFeatures;
	bool hardcore;
	bool allowCommands;
	bool initialized;
	bool newSeaLevel;		// 4J added
	bool hasBeenInCreative; // 4J added
	bool spawnBonusChest;   // 4J added

	// 4J Added - 改为 long 以支持无限世界尺寸
	int64_t m_xzSize;	// 原来是 int
#ifdef _LARGE_WORLDS
	int m_xzSizeOld; // 4J MGH Added, for expanding worlds  // 未要求改，暂保留 int
	int m_hellScaleOld;
	bool	m_classicEdgeMoat;
	bool	m_smallEdgeMoat;
	bool	m_mediumEdgeMoat;
#endif
	int64_t m_hellScale; // 4J Added - 改为 long

	// 4J added
	int xStronghold;
	int yStronghold;
	int zStronghold;
	bool bStronghold;

	int xStrongholdEndPortal;
	int zStrongholdEndPortal;
	bool bStrongholdEndPortal;

	GameRules gameRules;

protected:
	LevelData();

public:
	LevelData(CompoundTag *tag);
	LevelData(LevelSettings *levelSettings, const wstring& levelName);
	LevelData(LevelData *copy);
	CompoundTag *createTag();
	CompoundTag *createTag(vector<shared_ptr<Player> > *players);

	enum
	{
		DIMENSION_NETHER=-1,
		DIMENSION_OVERWORLD=0,
		DIMENSION_END=1
	};

protected:
	virtual void setTagData(CompoundTag *tag);		// 4J - removed  CompoundTag *playerTag

public:
	virtual int64_t getSeed();
	virtual int getXSpawn();
	virtual int getYSpawn();
	virtual int getZSpawn();
	virtual int getXStronghold();
	virtual int getZStronghold();
	virtual int getXStrongholdEndPortal();
	virtual int getZStrongholdEndPortal();
	virtual int64_t getGameTime();
	virtual int64_t getDayTime();
	virtual  int64_t getSizeOnDisk();
	virtual CompoundTag *getLoadedPlayerTag();
	//int getDimension(); // 4J Removed TU 9 as it's never accurate
	virtual void setSeed(int64_t seed);
	virtual void setXSpawn(int xSpawn);
	virtual void setYSpawn(int ySpawn);
	virtual void setZSpawn(int zSpawn);
	virtual void setHasStronghold();
	virtual bool getHasStronghold();
	virtual void setXStronghold(int xStronghold);
	virtual void setZStronghold(int zStronghold);
	virtual void setHasStrongholdEndPortal();
	virtual bool getHasStrongholdEndPortal();
	virtual void setXStrongholdEndPortal(int xStrongholdEndPortal);
	virtual void setZStrongholdEndPortal(int zStrongholdEndPortal);

	virtual void setGameTime(int64_t time);
	virtual void setDayTime(int64_t time);
	virtual void setSizeOnDisk(int64_t sizeOnDisk);
	virtual void setLoadedPlayerTag(CompoundTag *loadedPlayerTag);
	//void setDimension(int dimension); // 4J Removed TU 9 as it's never used
	virtual void setSpawn(int xSpawn, int ySpawn, int zSpawn);
	virtual wstring getLevelName();
	virtual void setLevelName(const wstring& levelName);
	virtual int getVersion();
	virtual void setVersion(int version);
	virtual int64_t getLastPlayed();
	virtual bool isThundering();
	virtual void setThundering(bool thundering);
	virtual int getThunderTime();
	virtual void setThunderTime(int thunderTime);
	virtual bool isRaining();
	virtual void setRaining(bool raining);
	virtual int getRainTime();
	virtual void setRainTime(int rainTime);
	virtual GameType *getGameType();
	virtual bool isGenerateMapFeatures();
	virtual bool getSpawnBonusChest();
	virtual void setGameType(GameType *gameType);
	virtual bool useNewSeaLevel();
	virtual bool getHasBeenInCreative(); // 4J Added
	virtual void setHasBeenInCreative(bool value); // 4J Added
	virtual LevelType *getGenerator();
	virtual void setGenerator(LevelType *generator);
	virtual wstring getGeneratorOptions();
	virtual void setGeneratorOptions(const wstring &options);
	virtual bool isHardcore();
	virtual bool getAllowCommands();
	virtual void setAllowCommands(bool allowCommands);
	virtual bool isInitialized();
	virtual void setInitialized(bool initialized);
	virtual GameRules *getGameRules();

	// 4J Added - 返回类型改为 long 以匹配新类型
	virtual int64_t getXZSize();
#ifdef _LARGE_WORLDS
	virtual int getXZSizeOld(); // 未改，保持 int
	virtual void getMoatFlags(bool* bClassicEdgeMoat, bool* bSmallEdgeMoat, bool* bMediumEdgeMoat); //4J MGH - added
	virtual int getXZHellSizeOld(); // 未改
#endif
	virtual int64_t getHellScale(); // 改为 long
};
