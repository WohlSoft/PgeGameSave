#ifndef PGE_GAMESAVE_DB_H
#define PGE_GAMESAVE_DB_H

#include <string>
#include <memory>
#include <cstdint>
#include <vector>

struct PGE_GameSaveDB_private;
class PGE_GameSaveDB
{
    std::unique_ptr<PGE_GameSaveDB_private> p;
public:
    PGE_GameSaveDB();
    PGE_GameSaveDB(const std::string &filePath);
    PGE_GameSaveDB(const PGE_GameSaveDB &o) = delete;
    PGE_GameSaveDB(PGE_GameSaveDB &&o);
    virtual ~PGE_GameSaveDB();
    bool open(const std::string &filePath);

    /**
     * @brief Close the game save
     */
    void close();

    enum DATA
    {
        DATA_NOTHING                = 0x00,
        DATA_PLAYER_META            = 0x01,
        DATA_WORLD_MAP_VISIBILITY   = 0x02,
        DATA_EVERYTHING             = 0xFFFFFFFF,
    };

    /**
     * @brief Load game state from the file.
     * @param resumeBackup Resume state with keeping the temporary data of the soft saves
     * @return true if game save has been resumed, false if any error has occouped
     */
    bool load(int dataToLoad, bool resumeBackup = false);

    /**
     * @brief Save current state of game and convert temporary data into persistent (hard save)
     * @return true if state has successfully saved
     */
    bool save();

    /**
     * @brief Process the soft saving of all data (write current state as temporary data) (soft save)
     * @return true if state has successfully saved
     */
    bool saveAuto();

    /**
     * @brief Process the soft saving of all data (write current state as temporary data) (soft save with memory clean-up)
     * @return true if state has successfully saved
     */
    bool saveAutoAndUnload(int dataToUnload);

    enum VAR_TYPE
    {
        //! As plane-text string
        VAR_STRING  = 0,
        //! As integer value
        VAR_INTEGER,
        //! As floating-point number
        VAR_FLOATING_POINT,
        //! As JSON converted from a lua-table
        VAR_JSON,
        //! Store in ecrypted by external function form
        VAR_EXT_ENCRYPTED /*(possibly will not be needed if cyphers will do encryption
                            before giving data to this function, or just used as marker to
                            have easier detect encrypted data from regular plane-text string)*/
    };

    enum VAR_ACCESS_LEVEL
    {
        VAR_ACCESS_GLOBAL = 0,
        VAR_ACCESS_THIS_LEVEL,
        VAR_ACCESS_ANY_LEVEL,
        VAR_ACCESS_WORLD,
    };

    bool variableGet(VAR_ACCESS_LEVEL al, const std::string &name, std::string  *output = nullptr, const std::string &defValue = "");
    bool variableGet(VAR_ACCESS_LEVEL al, const std::string &name, double       *output = nullptr, const double &defValue = 0.0);
    bool variableGet(VAR_ACCESS_LEVEL al, const std::string &name, int64_t      *output = nullptr, const int64_t &defValue = 0);

    bool variableSet(VAR_ACCESS_LEVEL al, const std::string &name, std::string  *output = nullptr);
    bool variableSet(VAR_ACCESS_LEVEL al, const std::string &name, double       *output = nullptr);
    bool variableSet(VAR_ACCESS_LEVEL al, const std::string &name, int64_t      *output = nullptr);

    //! Working state values are will be taken for a save
    struct SaveData
    {
        int32_t     m_lives  = 1;
        uint32_t    m_coins  = 0;
        uint32_t    m_score  = 0;
        uint32_t    m_stars  = 0;

        uint64_t    m_hubRecentWarp = 0;

        struct ItemViz
        {
            uint32_t array_id = 0;
            bool     state = false;
        };

        struct GottenStar
        {
            std::string levelFile;
            int32_t     legacySection = -1;
            int64_t     posX = 0;
            int64_t     posY = 0;
        };

        int64_t     m_worldPosX = 0;
        int64_t     m_worldPosY = 0;
        uint32_t    m_worldMusicId = 0;
        std::string m_worldMusicFile;

        std::vector<ItemViz> m_worldVisLevels;
        std::vector<ItemViz> m_worldVisPaths;
        std::vector<ItemViz> m_worldVisScenery;

        bool        m_gameWasCompleted = false;
    } m_data;
};

#endif // PGE_GAMESAVE_DB_H
