// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Data/ProfileData.hpp"
#include "SSVOpenHexagon/Data/StyleData.hpp"

#include <SSVStart/Assets/Assets.hpp>
#include <SSVStart/SoundPlayer/SoundPlayer.hpp>
#include <SSVStart/MusicPlayer/MusicPlayer.hpp>

#include <SSVUtils/Core/Assert/Assert.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <SFML/System.hpp>

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstddef>

namespace hg
{

namespace Steam
{
class steam_manager;
}

class MusicData;

class HGAssets
{
private:
    Steam::steam_manager& steamManager;

    bool playingLocally{true};
    bool levelsOnly{false};

    ssvs::AssetManager<> assetManager;
    ssvs::SoundPlayer soundPlayer;

public:
    ssvs::MusicPlayer musicPlayer;

private:
    std::unordered_map<std::string, LevelData> levelDatas;
    std::unordered_map<std::string, std::vector<std::string>>
        levelDataIdsByPack;

    std::unordered_map<std::string, PackData> packDatas;

    struct PackInfo
    {
        std::string id;
        ssvufs::Path path;
    };

    std::vector<PackInfo> packInfos;

    std::map<std::string, MusicData> musicDataMap;
    std::map<std::string, StyleData> styleDataMap;
    std::map<std::string, ProfileData> profileDataMap;
    ProfileData* currentProfilePtr{nullptr};

    [[nodiscard]] bool loadPackData(const ssvufs::Path& packPath);
    [[nodiscard]] bool loadPackInfo(const PackData& packData);

public:
    struct LoadInfo
    {
        unsigned int packs{0};
        unsigned int levels{0};
        unsigned int assets{0};
        std::vector<std::string> errorMessages;

        void addFormattedError(std::string& error)
        {
            if(error.empty())
            {
                return;
            }

            // Remove the first two characters
            error.erase(0, 2);

            // Replace first newline with '-', place a space before it,
            // and remove a space after it.
            std::size_t i = error.find('\n');
            error.insert(i, " ");
            error[++i] = '-';
            error.erase(++i, 1);

            // Remove all other newlines.
            while((i = error.find('\n', i)) != std::string::npos)
            {
                error.erase(i, 1);
            }

            errorMessages.emplace_back(error);
        }
    };

private:
    LoadInfo loadInfo;

public:
    HGAssets(Steam::steam_manager& mSteamManager, bool mLevelsOnly = false);

    LoadInfo& getLoadResults()
    {
        return loadInfo;
    }

    auto& operator()()
    {
        return assetManager;
    }

    template <typename T>
    T& get(const std::string& mId)
    {
        return assetManager.get<T>(mId);
    }

    const std::unordered_map<std::string, LevelData>& getLevelDatas()
    {
        return levelDatas;
    }

    const LevelData& getLevelData(const std::string& mAssetId)
    {
        const auto it = levelDatas.find(mAssetId);
        if(it == levelDatas.end())
        {
            ssvu::lo("getLevelData")
                << "Asset '" << mAssetId << "' not found\n";

            SSVU_ASSERT(!levelDatas.empty());
            return levelDatas.begin()->second;
        }

        return it->second;
    }

    const LevelData& getLevelData(
        const std::string& mPackId, const std::string& mId)
    {
        return getLevelData(mPackId + "_" + mId);
    }

    const std::vector<std::string>& getLevelIdsByPack(
        const std::string& mPackId)
    {
        SSVU_ASSERT(levelDataIdsByPack.count(mPackId) > 0);
        return levelDataIdsByPack.at(mPackId);
    }

    const std::unordered_map<std::string, PackData>& getPacksData()
    {
        return packDatas;
    }

    const PackData& getPackData(const std::string& mPackId)
    {
        SSVU_ASSERT(packDatas.count(mPackId) > 0);
        return packDatas.at(mPackId);
    }

    const std::vector<PackInfo>& getPackInfos()
    {
        return packInfos;
    }

    [[nodiscard]] bool loadAssets();

    void loadMusic(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadMusicData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadStyleData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadLevelData(const std::string& mPackId, const ssvufs::Path& mPath);
    void loadCustomSounds(
        const std::string& mPackId, const ssvufs::Path& mPath);

    const MusicData& getMusicData(
        const std::string& mPackId, const std::string& mId);
    const StyleData& getStyleData(
        const std::string& mPackId, const std::string& mId);

    [[nodiscard]] std::string reloadPack(
        const std::string& mPackId, const std::string& mPath);
    [[nodiscard]] std::string reloadLevel(const std::string& mPackId,
        const std::string& mPath, const std::string& mId);

    float getLocalScore(const std::string& mId);
    void setLocalScore(const std::string& mId, float mScore);

    void loadLocalProfiles();
    void saveCurrentLocalProfile();
    void setCurrentLocalProfile(const std::string& mName);
    ProfileData& getCurrentLocalProfile();
    const ProfileData& getCurrentLocalProfile() const;
    std::string getCurrentLocalProfileFilePath();
    ProfileData* getLocalProfileByName(const std::string& mName);
    const ProfileData* getLocalProfileByName(const std::string& mName) const;
    void createLocalProfile(const std::string& mName);
    std::size_t getLocalProfilesSize();
    std::vector<std::string> getLocalProfileNames();
    std::string getFirstLocalProfileName();

    bool pIsValidLocalProfile()
    {
        return currentProfilePtr != nullptr;
    }

    std::string pGetName() const
    {
        assert(playingLocally); // TODO
        return getCurrentLocalProfile().getName();
    }

    const std::vector<std::string>& pGetTrackedNames() const
    {
        assert(playingLocally); // TODO
        return getCurrentLocalProfile().getTrackedNames();
    }

    void pClearTrackedNames()
    {
        assert(playingLocally); // TODO
        getCurrentLocalProfile().clearTrackedNames();
    }

    void pAddTrackedName(const std::string& mName)
    {
        assert(playingLocally); // TODO
        getCurrentLocalProfile().addTrackedName(mName);
    }

    void pSaveCurrent()
    {
        if(!playingLocally)
        {
            return;
        }

        saveCurrentLocalProfile();
    }

    void pSetCurrent(const std::string& mName)
    {
        if(!playingLocally)
        {
            throw;
        }

        setCurrentLocalProfile(mName);
    }

    void pCreate(const std::string& mName)
    {
        if(!playingLocally)
        {
            throw;
        }

        createLocalProfile(mName);
    }

    bool pIsLocal() const
    {
        return playingLocally;
    }

    void pSetPlayingLocally(bool mPlayingLocally)
    {
        playingLocally = mPlayingLocally;
    }

    void refreshVolumes();
    void stopMusics();
    void stopSounds();

    void playSound(const std::string& mId,
        ssvs::SoundPlayer::Mode mMode = ssvs::SoundPlayer::Mode::Override);

    void playPackSound(const std::string& mPackId, const std::string& mId,
        ssvs::SoundPlayer::Mode mMode = ssvs::SoundPlayer::Mode::Override);

    void playMusic(const std::string& mPackId, const std::string& mId,
        sf::Time mPlayingOffset = sf::seconds(0));

    ssvs::SoundPlayer& getSoundPlayer() noexcept
    {
        return soundPlayer;
    }

    ssvs::MusicPlayer& getMusicPlayer() noexcept
    {
        return musicPlayer;
    }
};

} // namespace hg
