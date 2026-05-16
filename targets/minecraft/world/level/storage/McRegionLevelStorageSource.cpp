#include "McRegionLevelStorageSource.h"

#include <assert.h>

#include <algorithm>
#include <memory>

#include "LevelData.h"
#include "LevelSummary.h"
#include "McRegionLevelStorage.h"
#include "java/File.h"
#include "java/InputOutputStream/FileInputStream.h"
#include "java/InputOutputStream/FileOutputStream.h"
#include "java/JavaMath.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/util/Log.h"
#include "minecraft/util/ProgressListener.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/chunk/storage/RegionFileCache.h"
#include "minecraft/world/level/storage/DirectoryLevelStorageSource.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"
#include "nbt/StringTag.h"

McRegionLevelStorageSource::McRegionLevelStorageSource(File dir)
    : DirectoryLevelStorageSource(dir) {}

std::string McRegionLevelStorageSource::getName() {
    return "Scaevolus' McRegion";
}

std::vector<LevelSummary*>* McRegionLevelStorageSource::getLevelList() {
    std::vector<LevelSummary*>* levels = new std::vector<LevelSummary*>();

    Log::info("[getLevelList] Scanning baseDir=%s\n", baseDir.getPath().c_str());

    if (!baseDir.exists()) {
        baseDir.mkdirs();
        return levels;
    }

    std::vector<File*>* entries = baseDir.listFiles();
    if (entries == nullptr) return levels;

    for (auto it = entries->begin(); it != entries->end(); ++it) {
        File* entry = *it;
        if (entry == nullptr) continue;

        // On Linux, each world is a directory inside the Saves/ folder
        if (!entry->isDirectory()) {
            delete entry;
            continue;
        }

        File levelDat(*entry, "level.dat");
        if (!levelDat.exists()) {
            delete entry;
            continue;
        }

        std::string levelId = entry->getName();
        std::string levelName = levelId;
        int64_t lastPlayed = entry->lastModified();
        int64_t sizeOnDisk = levelDat.length();
        GameType* gameMode = GameType::SURVIVAL;
        bool hardcore = false;
        bool hasCheats = false;

        // Try to read level.dat to get the pretty name and game mode
        try {
            FileInputStream fis(levelDat);
            std::unique_ptr<CompoundTag> root(NbtIo::readCompressed(&fis));
            if (root) {
                CompoundTag* data = root->getCompound("Data");
                if (data) {
                    if (data->contains("LevelName")) levelName = data->getString("LevelName");
                    if (data->contains("LastPlayed")) lastPlayed = data->getLong("LastPlayed");
                    if (data->contains("SizeOnDisk")) sizeOnDisk = data->getLong("SizeOnDisk");

                    int gmId = data->getInt("GameType");
                    gameMode = GameType::byId(gmId);
                    hardcore = data->getBoolean("hardcore");

                    if (data->contains("allowCommands")) {
                        hasCheats = data->getBoolean("allowCommands");
                    } else {
                        hasCheats = (gameMode == GameType::CREATIVE);
                    }
                }
            }
        } catch (...) {
            Log::info("[getLevelList] Failed to parse NBT for %s, using defaults\n", levelId.c_str());
        }

        levels->push_back(new LevelSummary(levelId, levelName, lastPlayed,
                                           sizeOnDisk, gameMode,
                                           false, // requiresConversion
                                           hardcore, hasCheats));
        delete entry;
    }

    delete entries;

    // Sort by most recently played
    std::sort(levels->begin(), levels->end(),
              [](LevelSummary* a, LevelSummary* b) {
                  return a->compareTo(b) < 0;
              });

    return levels;
}

void McRegionLevelStorageSource::clearAll() { RegionFileCache::clear(); }

void McRegionLevelStorageSource::renameLevel(const std::string& levelId,
                                             const std::string& newLevelName) {
    File worldDir(baseDir, levelId);
    if (!worldDir.exists() || !worldDir.isDirectory()) {
        Log::info("[renameLevel] world folder not found: %s\n",
                  levelId.c_str());
        return;
    }
    File levelDat(worldDir, "level.dat");
    if (levelDat.exists()) {
        CompoundTag* root = nullptr;
        try {
            FileInputStream fis(levelDat);
            root = NbtIo::readCompressed(&fis);
            fis.close();
        } catch (...) {
            root = nullptr;
        }

        if (root != nullptr) {
            CompoundTag* data = root->getCompound("Data");
            if (data != nullptr) {
                data->putString("LevelName", newLevelName);
                try {
                    FileOutputStream fos(levelDat);
                    NbtIo::writeCompressed(root, &fos);
                    fos.close();
                } catch (...) {
                    Log::info("[renameLevel] failed to write level.dat\n");
                }
            }
            delete root;
        }
    }
    std::string sanitised = newLevelName;
    for (int i = 0; i < SharedConstants::ILLEGAL_FILE_CHARACTERS_LENGTH; ++i) {
        char bad = SharedConstants::ILLEGAL_FILE_CHARACTERS[i];
        std::replace(sanitised.begin(), sanitised.end(), bad, '_');
    }
    while (!sanitised.empty() && (sanitised.front() == ' '))
        sanitised.erase(0, 1);
    while (!sanitised.empty() && (sanitised.back() == ' '))
        sanitised.pop_back();
    if (sanitised.empty()) sanitised = levelId;

    if (sanitised != levelId) {
        File target(baseDir, sanitised);
        if (!target.exists()) {
            if (!worldDir.renameTo(target)) {
                Log::info("[renameLevel] rename folder failed: %s -> %s\n",
                          levelId.c_str(), sanitised.c_str());
            }
        }
    }
}

void McRegionLevelStorageSource::deleteLevel(const std::string& levelId) {
    RegionFileCache::clear();

    File worldDir(baseDir, levelId);
    if (!worldDir.exists()) return;

    std::vector<File*>* children = worldDir.listFiles();
    if (children != nullptr) {
        deleteRecursive(children);
        delete children;
    }
    worldDir._delete();
}

std::shared_ptr<LevelStorage> McRegionLevelStorageSource::selectLevel(
    ConsoleSaveFile* saveFile, const std::string& levelId,
    bool createPlayerDir) {
    //        return new LevelStorageProfilerDecorator(new
    //        McRegionLevelStorage(baseDir, levelId, createPlayerDir));
    return std::shared_ptr<LevelStorage>(
        new McRegionLevelStorage(saveFile, baseDir, levelId, createPlayerDir));
}

bool McRegionLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile,
                                               const std::string& levelId) {
    // check if there is old file format level data
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData == nullptr || levelData->getVersion() != 0) {
        delete levelData;
        return false;
    }
    delete levelData;

    return true;
}

bool McRegionLevelStorageSource::requiresConversion(
    ConsoleSaveFile* saveFile, const std::string& levelId) {
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData == nullptr || levelData->getVersion() != 0) {
        delete levelData;
        return false;
    }
    delete levelData;

    return true;
}

bool McRegionLevelStorageSource::convertLevel(ConsoleSaveFile* saveFile,
                                              const std::string& levelId,
                                              ProgressListener* progress) {
    assert(false);
    // I removed this while updating the saves to use the single save file
    // Will we ever use this convertLevel function anyway? The main issue is the
    // check for the hellFolder.exists() which would require a slight change to
    // the way our save files are structured
    return true;
}

void McRegionLevelStorageSource::convertRegions(
    File& baseFolder, std::vector<ChunkFile*>* chunkFiles, int currentCount,
    int totalCount, ProgressListener* progress) {
    assert(false);

    // 4J Stu - Removed, see comment in convertLevel above
}

void McRegionLevelStorageSource::eraseFolders(std::vector<File*>* folders,
                                              int currentCount, int totalCount,
                                              ProgressListener* progress) {
    File* folder;
    auto itEnd = folders->end();
    for (auto it = folders->begin(); it != itEnd; it++) {
        folder = *it;  // folders->at(i);

        std::vector<File*>* files = folder->listFiles();
        deleteRecursive(files);
        folder->_delete();

        currentCount++;
        int percent =
            (int)Math::round(100.0 * (double)currentCount / (double)totalCount);
        progress->progressStagePercentage(percent);
    }
}
