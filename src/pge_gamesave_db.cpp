#include <sqlite3.h>
#include "pge_gamesave_db.h"

#include <memory.h>
#include <stdio.h>

#ifdef PGE_ENGINE
//The advantage of SDL_assert is guarantee of message box showing when assert doesn't passing,
//therefore it is used in PGE Engine which is using SDL.
//for other applications, that are doesn't using SDL library, will use regular assert
#include <SDL2/SDL_assert.h>
#define PGE_assert SDL_assert
#else
#include <assert.h>
#define PGE_assert assert
#endif

struct PGE_GameSaveDB_private
{
    static int dummyCallback(void *, int /*argc*/, char ** /*argv*/, char ** /*azColName*/)
    {
        return 0;
    }

    static int selectCallback(void *data, int argc, char **argv, char **azColName)
    {
        PGE_GameSaveDB_private *pgedb = reinterpret_cast<PGE_GameSaveDB_private *>(data);
        PGE_assert(pgedb);
        //for(int i = 0; i < argc; i++)
        //    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        //printf("\n");
        return 0;
    }

    PGE_GameSaveDB *m_self = nullptr;

    sqlite3    *m_db = nullptr;
    std::string m_filePath;
    std::string m_errorString;

    bool tableExists(const char *tableName)
    {
        int rc;
        char *zErrMsg = 0;
        char sql[1024];
        memset(sql, 0, 1024);
        snprintf(sql, 1023, "select count(type) from sqlite_master where type='table' and name='%s';", tableName);
        /* Execute SQL statement */
        rc = sqlite3_exec(m_db, sql, dummyCallback, this, &zErrMsg);
        if(rc != SQLITE_OK)
        {
            m_errorString = std::string(zErrMsg);
            sqlite3_free(zErrMsg);
        }
        return true;
    }

    bool initializeGameSave()
    {
        /* Create the new tables if there are not existed */
        const char *queries[] =
                {
                    "CREATE TABLE IF NOT EXISTS "
                    "EpisodeSetup("
                    " key           CHAR(50) PRIMARY KEY NOT NULL"
                    ",value         TEXT DEFAULT ''"
                    ");",

                    /* Values to store into this table:
                     *      WldMusicId   World map music id
                     *      WldMusicFile World map music file path
                     *      isGameComplete is Game Completed
                     */

                    "CREATE TABLE IF NOT EXISTS "
                    "CharacterStates("
                    " id            INT NOT NULL"
                    ",playerId      INT NOT NULL"
                    ",state         INT DEFAULT 0"
                    ",health        INT DEFAULT 0"
                    ",legacyItemId    INT DEFAULT 0"
                    ",legacyMountType INT DEFAULT 0"
                    ",legacyMountId   INT DEFAULT 0"
                    ",PRIMARY KEY (id, playerId)"
                    ");",

                    "CREATE TABLE IF NOT EXISTS "
                    "PlayerStates("
                    " id            INT PRIMARY KEY NOT NULL"
                    ",characterId   INT DEFAULT 0"
                    ",lives         INT DEFAULT 0"
                    ",coins         INT DEFAULT 0"
                    ",score         INT DEFAULT 0"
                    ",commonHealth  INT DEFAULT 0"
                    ",totalStars    INT DEFAULT 0"
                    ",worldMapPoxX  INT DEFAULT 0"
                    ",worldMapPoxY  INT DEFAULT 0"
                    ",recentHubWarp INT DEFAULT -1"
                    ");",

                    "CREATE TABLE IF NOT EXISTS "
                    "World_visibleLevels("
                    " ArrayId       INT PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    "CREATE TABLE IF NOT EXISTS "
                    "World_visiblePaths("
                    " ArrayId       INT PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    "CREATE TABLE IF NOT EXISTS "
                    "World_visibleSceneries("
                    " ArrayId       INT PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    "CREATE TABLE IF NOT EXISTS "
                    "World_gottenStars("
                    " levelFileName INT PRIMARY KEY NOT NULL"
                    ",arrayId       INT DEFAULT 0"
                    ",posX          INT DEFAULT 0"
                    ",posY          INT DEFAULT 0"
                    ");",

                    /* A table for user data storing */
                    "CREATE TABLE IF NOT EXISTS "
                    "UserData("
                    " id            INT PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",access        INT DEFAULT 0"
                    ",type          INT DEFAULT 0"
                    ",value         BLOB"
                    ");"
                };

        char *zErrMsg = 0;
        std::string errors;

        /* Execute insertion queries */
        for(size_t i = 0; i < sizeof(queries) / sizeof(const char*); i++)
        {
            int rc = sqlite3_exec(m_db, queries[i], dummyCallback, 0, &zErrMsg);
            if( rc != SQLITE_OK )
            {
                errors += std::string(zErrMsg);
                errors.push_back('\n');
                sqlite3_free(zErrMsg);
            }
        }

        // === In next time to update existing tables, use next command ===
        //ALTER TABLE {tableName} ADD COLUMN COLNew {type};

        return true;
    }
};




PGE_GameSaveDB::PGE_GameSaveDB()
    : p(new PGE_GameSaveDB_private)
{
    p->m_self = this;
}

PGE_GameSaveDB::PGE_GameSaveDB(const std::string &filePath)
    : p(new PGE_GameSaveDB_private)
{
    p->m_self = this;
    open(filePath);
}


PGE_GameSaveDB::~PGE_GameSaveDB()
{
    PGE_assert(p);
    close();
}

bool PGE_GameSaveDB::open(const std::string &filePath)
{
    PGE_assert(p);
    int ret = sqlite3_open_v2(filePath.c_str(),
                              &p->m_db,
                              SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                              NULL);
    if(SQLITE_OK != ret)
        return false;
    if(!p->initializeGameSave())
        return false;
    return true;
}

void PGE_GameSaveDB::close()
{
    PGE_assert(p);
    if(p->m_db)
        sqlite3_close_v2(p->m_db);
    p->m_db = nullptr;
    p->m_errorString.clear();
    p->m_filePath.clear();
}

bool PGE_GameSaveDB::load(int dataToLoad, bool resumeBackup)
{
    return false;
}

bool PGE_GameSaveDB::save()
{
    return false;
}

bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name, std::string *output,
                                 const std::string &defValue, PGE_GameSaveDB::VAR_TYPE type)
{
    return false;
}

bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 double *output,
                                 const double &defValue)
{
    return false;
}

bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 int64_t *output,
                                 const int64_t &defValue)
{
    return false;
}

bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 const std::string &output,
                                 PGE_GameSaveDB::VAR_TYPE type)
{
    return false;
}

bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 double output)
{
    return false;
}

bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 int64_t output)
{
    return false;
}


PGE_GameSaveDB::SaveData::PlayerState PGE_GameSaveDB::SaveData::getPlayerState(size_t playerID)
{
    if((playerID == 0) || (playerID >= m_playerStates.size()))
        return PlayerState();// Invalid playable character
    return m_playerStates[playerID - 1];
}

void PGE_GameSaveDB::SaveData::setPlayerState(size_t playerID, const PGE_GameSaveDB::SaveData::PlayerState &state)
{
    if(playerID < 1)
        return;
    if(state.m_character < 1)
        return;
    if(state.m_character >= state.m_characters.size())
        return;//Invalid characer ID
    if(state.m_characters[state.m_character - 1].id < 1)
        return;//Invalid characer ID

    playerID -= 1;
    if(playerID >= m_playerStates.size())
    {
        while(playerID >= m_playerStates.size())
            m_playerStates.emplace_back();
    }
    m_playerStates[playerID] = state;
}
