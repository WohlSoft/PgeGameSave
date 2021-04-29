#include <sqlite3.h>
#include "pge_gamesave_db.h"

#include <memory.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <sstream>

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

class PGE_GameSaveDB_SqliteString {
private:
    char* sqliteString;
public:
    PGE_GameSaveDB_SqliteString() : sqliteString(nullptr)
    {}

    ~PGE_GameSaveDB_SqliteString() {
        if (sqliteString) {
            sqlite3_free(sqliteString);
        }
    }

    char** operator&() {
        return &sqliteString;
    }

    operator std::string() {
        if (!sqliteString)
            return std::string("");
        return std::string(sqliteString);
    }
};


struct PGE_GameSaveDB_private
{
    static int dummyCallback(void *, int /*argc*/, char ** /*argv*/, char ** /*azColName*/)
    {
        return 0;
    }

//    static int selectCallback(void *data, int argc, char **argv, char **azColName)
//    {
//        PGE_GameSaveDB_private *pgedb = reinterpret_cast<PGE_GameSaveDB_private *>(data);
//        PGE_assert(pgedb);
//        //for(int i = 0; i < argc; i++)
//        //    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//        //printf("\n");
//        return 0;
//    }

    typedef std::map<std::string, std::string> Row;
    typedef std::vector<Row> TableData;

    static int fillTableCallback(void *data, int argc, char **argv, char **azColName)
    {
        TableData *dstTable = reinterpret_cast<TableData*>(data);
        PGE_assert(dstTable);
        Row row;
        for(size_t i = 0; i < (size_t)argc; i++)
            row.insert({azColName[i], argv[i] ? argv[i] : ""});
        dstTable->push_back(row);
        return 0;
    }

    PGE_GameSaveDB *m_self = nullptr;

    sqlite3    *m_db = nullptr;
    std::string m_filePath;
    std::string m_errorString;

    struct Environment
    {
        //! Hard save ID (manully savedByUser). -1 means N/A
        int32_t saveId        = -1;
        //! Soft save ID (for backup-on-a-fly. On save discard can be flushed. On crash or power surge can be resumed). -1 means N/A
        int32_t saveTempId    = -1;
        //! Environment type
        PGE_GameSaveDB::ENVIRONMENT envType = PGE_GameSaveDB::ENV_NONE;
        //! Current filename
        std::string                 fileName;
    } m_env;

    bool tableExists(const char *tableName)
    {
        /* Execute SQL statement */
        std::string sql = std::string("select count(type) from sqlite_master where type='table' and name='") + tableName + "';";
        PGE_GameSaveDB_SqliteString zErrMsg;
        int rc = sqlite3_exec(m_db, sql.c_str(), dummyCallback, this, &zErrMsg);
        if(rc != SQLITE_OK)
            m_errorString = zErrMsg;
        return true;
    }

    bool initializeGameSave()
    {
        /* Create the new tables if there are not existed */
        const char *queries[] =
                {
                    //0
                    "CREATE TABLE IF NOT EXISTS "
                    "EpisodeSetup("
                    " key           CHAR(50) PRIMARY KEY NOT NULL"
                    ",value         TEXT DEFAULT ''"
                    ");",

                    //1
                    "CREATE TABLE IF NOT EXISTS "
                    "GameSaves("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id INTEGER NOT NULL"
                    ",title VARCHAR DEFAULT ''"
                    ",type  INTEGER DEFAULT 0"
                    ");",

                    /* Values to store into this table:
                     *      WldMusicId   World map music id
                     *      WldMusicFile World map music file path
                     *      isGameComplete is Game Completed
                     */

                    //2
                    "CREATE TABLE IF NOT EXISTS "
                    "CharacterStates("
                    " id            INT NOT NULL"
                    ",playerId      INT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",state         INT DEFAULT 0"
                    ",health        INT DEFAULT 0"
                    ",legacyItemId    INT DEFAULT 0"
                    ",legacyMountType INT DEFAULT 0"
                    ",legacyMountId   INT DEFAULT 0"
                    ",PRIMARY KEY (id, playerId, save_id)"
                    ");",

                    //3
                    "CREATE TABLE IF NOT EXISTS "
                    "PlayerStates("
                    " id            INTEGER NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",characterId   INT DEFAULT 0"
                    ",lives         INT DEFAULT 0"
                    ",coins         INT DEFAULT 0"
                    ",score         INT DEFAULT 0"
                    ",commonHealth  INT DEFAULT 0"
                    ",totalStars    INT DEFAULT 0"
                    ",worldMapPoxX  INT DEFAULT 0"
                    ",worldMapPoxY  INT DEFAULT 0"
                    ",recentHubWarp INT DEFAULT -1"
                    ",PRIMARY KEY (id, save_id)"
                    ");",

                    //4
                    "CREATE TABLE IF NOT EXISTS "
                    "World_visibleLevels("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",arrayId INTEGER NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    //5
                    "CREATE TABLE IF NOT EXISTS "
                    "World_visiblePaths("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",arrayId INTEGER NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    //6
                    "CREATE TABLE IF NOT EXISTS "
                    "World_visibleSceneries("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",arrayId INTEGER NOT NULL"
                    ",isVisible     INT DEFAULT 1"
                    ");",

                    //7
                    "CREATE TABLE IF NOT EXISTS "
                    "World_gottenStars("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",levelFileName INTEGER NOT NULL"
                    ",arrayId       INT DEFAULT 0"
                    ",posX          INT DEFAULT 0"
                    ",posY          INT DEFAULT 0"
                    ");",

                    //8
                    /* A table for user data storing */
                    "CREATE TABLE IF NOT EXISTS "
                    "UserData("
                    " id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
                    ",save_id       INTEGER NOT NULL"
                    ",access        INTEGER DEFAULT 0"
                    ",filename      VARCHAR DEFAULT ''"
                    ",name          VARCHAR NOT NULL"
                    ",type          INTEGER DEFAULT 0"
                    ",value         BLOB"
                    ");"
                };

        std::string errors;

        /* Execute insertion queries */
        for(size_t i = 0; i < sizeof(queries) / sizeof(const char*); i++)
        {
            PGE_GameSaveDB_SqliteString zErrMsg; // RAII <3
            int rc = sqlite3_exec(m_db, queries[i], dummyCallback, 0, &zErrMsg);
            if( rc != SQLITE_OK )
                errors += static_cast<std::string>(zErrMsg) + '\n';
        }

        return errors.empty();
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

PGE_GameSaveDB::GameSaveList PGE_GameSaveDB::getchGameSaves()
{
    PGE_assert(p && p->m_db);
    GameSaveList list;
    PGE_GameSaveDB_private::TableData table;
    std::string sql = "SELECT * FROM GameSaves ORDER BY save_id ASC;";

    PGE_GameSaveDB_SqliteString zErrMsg;
    int rc = sqlite3_exec(p->m_db, sql.c_str(), PGE_GameSaveDB_private::fillTableCallback, &table, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        p->m_errorString = zErrMsg;
        return list;
    }

    for(PGE_GameSaveDB_private::Row &row : table)
    {
        GameSaveEntry se;
        se.id       = (uint32_t)std::stoul(row["save_id"].c_str(), NULL);
        se.type     = (GameSaveEntry::Type)std::stoul(row["type"].c_str(), NULL);
        se.title    = row["title"];
        if(se.title.empty())
        {
            std::ostringstream s;
            s << "Game Save #" << se.id;
            se.title = s.str();
        }
        list.push_back(se);
    }

    return list;
}

bool PGE_GameSaveDB::initGameSave(uint32_t save_id, bool clearBackup)
{
    /*TODO: Implement search of existing gamesave entry
     * and create if not exists, or use exist and generate working data
     * by copying of data into separated special gamesave slot */
    PGE_assert(p && p->m_db);
    int rc = 0;
    PGE_GameSaveDB_SqliteString zErrMsg;
    PGE_GameSaveDB_private::TableData table;
    std::string sql;

    if(clearBackup)
    {

    }

    sql += "SELECT * from GameSaves WHERE save_id=" + std::to_string(save_id) + " AND type=0 LIMIT 1;";
    rc = sqlite3_exec(p->m_db, sql.c_str(), PGE_GameSaveDB_private::fillTableCallback, &table, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        p->m_errorString = zErrMsg;
        return false;
    }

    if(table.size() > 0)
    {

    }

    return true;
}

bool PGE_GameSaveDB::load(int dataToLoad, bool resumeBackup)
{
    //TODO: Implement loading data of SaveData structure
    return false;
}

bool PGE_GameSaveDB::save()
{
    //TODO: Implement "hard" saving data of SaveData structure
    return false;
}

bool PGE_GameSaveDB::saveAuto()
{
    //TODO: Implement "soft" saving data of SaveData structure (aka, create backup of current "floating in air" state)
    return false;
}

void PGE_GameSaveDB::setEnvironment(ENVIRONMENT envType, const std::string &filename)
{
    p->m_env.envType    = envType;
    p->m_env.fileName   = filename;
}

bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name, std::string *output,
                                 const std::string &defValue, PGE_GameSaveDB::VAR_TYPE type)
{
    // Output must be set!
    PGE_assert(output);
    // Set target access level
    std::string access_level;
    switch(al)
    {
    case VAR_ACCESS_GLOBAL:
        access_level = "access=0";
        break;
    case VAR_ACCESS_THIS_LEVEL:
        access_level = "access=1 AND filename='" + p->m_env.fileName + "'";
        break;
    case VAR_ACCESS_ANY_LEVEL:
        access_level = "access=2";
        break;
    case VAR_ACCESS_WORLD:
        access_level = "access=3";
        break;
    }

    //Find existing field. If exists, modify, or create new
    PGE_GameSaveDB_private::TableData table;
    std::string sql = "SELECT * FROM UserData WHERE ";
    sql += "save_id=" + std::to_string(p->m_env.saveTempId) + " AND name='" + name + "' AND " + access_level + " LIMIT 1;";

    PGE_GameSaveDB_SqliteString zErrMsg;
    int rc = sqlite3_exec(p->m_db, sql.c_str(), PGE_GameSaveDB_private::fillTableCallback, &table, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        p->m_errorString = zErrMsg;
        *output = defValue;
        return false;
    }

    if(table.size() < 1)
    {
        *output = defValue;
        return false;
    }
    else
    {
        int64_t id = table[0].find("id") != table[0].end() ? std::atol(table[0]["id"].c_str()) : -1;
        if(id < 0)
        {
            p->m_errorString = "INVALID TABLE ID!!!";
            return false;
        }
        PGE_GameSaveDB_private::Row::iterator s = table[0].find("value");
        if(s != table[0].end())
        {
            switch(type)
            {
            case VTYPE_PLAIN_TEXT:
            case VTYPE_INTEGER:
            case VTYPE_FLOATING_POINT:
            case VTYPE_JSON:
                *output = s->second;
                break;
            case VTYPE_EXT_ENCRYPTED:
                {
                    //TODO: Implemen the decrypter callback here
                    *output = s->second;
                    break;
                }
            }
        }
        else
        {
            *output = defValue;
            return false;
        }
    }

    return true;
}

bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 double *output,
                                 const double &defValue)
{
    PGE_assert(output);
    std::string out;
    std::ostringstream defValueN;
    defValueN.precision(16);
    defValueN << defValue;
    bool ret = variableGet(al, name, &out, defValueN.str(), VTYPE_FLOATING_POINT);
    if(ret)
        *output = std::strtod(out.c_str(), NULL);
    return ret;
}


bool PGE_GameSaveDB::variableGet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 int64_t *output,
                                 const int64_t &defValue)
{
    PGE_assert(output);
    std::string out;
    std::ostringstream defValueN;
    defValueN << defValue;
    bool ret = variableGet(al, name, &out, defValueN.str(), VTYPE_INTEGER);
    if(ret)
        *output = (int64_t)std::stoll(out.c_str(), NULL);
    return ret;
}


bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 const std::string &input,
                                 PGE_GameSaveDB::VAR_TYPE type)
{
    //Set target access level
    std::string access_level;
    std::string ecrypted_temp;
    const std::string *input_d = &input;
    switch(al)
    {
    case VAR_ACCESS_GLOBAL:
        access_level = "access=0";
        break;
    case VAR_ACCESS_THIS_LEVEL:
        access_level = "access=1 AND filename='" + p->m_env.fileName + "'";
        break;
    case VAR_ACCESS_ANY_LEVEL:
        access_level = "access=2";
        break;
    case VAR_ACCESS_WORLD:
        access_level = "access=3";
        break;
    }

    //Find existing field. If exists, modify, or create new
    PGE_GameSaveDB_private::TableData table;
    std::string sql = std::string("SELECT * FROM UserData WHERE name='") + name + "' "
                                  "AND " + access_level + " "
                                  "AND save_id=" + std::to_string(p->m_env.saveTempId) + " "
                                  "LIMIT 1;";
    PGE_GameSaveDB_SqliteString zErrMsg;
    int rc = sqlite3_exec(p->m_db, sql.c_str(), PGE_GameSaveDB_private::fillTableCallback, &table, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        p->m_errorString = zErrMsg;
        return false;
    }

    if(type == VTYPE_EXT_ENCRYPTED)
    {
        //TODO: Implemen the encrypter callback here
        ecrypted_temp = input;
        input_d = &ecrypted_temp;
    }

    sqlite3_stmt *stmt;
    if(table.size() < 1)
    {
        sqlite3_prepare(p->m_db, "INSERT INTO UserData (save_id, access, filename, name, type, value) "
                                 "VALUES (?, ?, ?, ?, ?, ?);", -1, &stmt, 0);
        sqlite3_bind_int64(stmt,  1, (int64_t)p->m_env.saveTempId);
        sqlite3_bind_int64(stmt,  2, (int64_t)al);
        sqlite3_bind_text(stmt, 3, (al == VAR_ACCESS_THIS_LEVEL ? p->m_env.fileName.c_str() : ""), -1, 0);
        sqlite3_bind_text(stmt, 4, name.c_str(), -1, 0);
        sqlite3_bind_int64(stmt,  5, (int64_t)type);
        sqlite3_bind_blob64(stmt, 6, input_d->c_str(), input_d->size(), 0);

        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE)
        {
            p->m_errorString = sqlite3_errmsg(p->m_db);
            return false;
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        //Modify
        int64_t id = table[0].find("id") != table[0].end() ? std::atol(table[0]["id"].c_str()) : -1;
        if(id < 0)
        {
            p->m_errorString = "INVALID TABLE ID!!!";
            return false;
        }
        sqlite3_prepare(p->m_db, "UPDATE UserData SET access=?, filename=?, name=?, type=?, value=? "
                                 "WHERE save_id=? AND id=? LIMIT 1;", -1, &stmt, 0);
        sqlite3_bind_int64(stmt,  1, (int64_t)al);
        sqlite3_bind_text(stmt, 2, (al == VAR_ACCESS_THIS_LEVEL ? p->m_env.fileName.c_str() : ""), -1, 0);
        sqlite3_bind_text(stmt, 3, name.c_str(), -1, 0);
        sqlite3_bind_int64(stmt,  4, (int64_t)type);
        sqlite3_bind_blob64(stmt, 5, input_d->c_str(), input_d->size(), 0);
        sqlite3_bind_int64(stmt,  6, (int64_t)p->m_env.saveTempId);
        sqlite3_bind_int64(stmt,  7, id);

        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE)
        {
            p->m_errorString = sqlite3_errmsg(p->m_db);
            return false;
        }
        sqlite3_finalize(stmt);
    }

    return true;
}

bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 double input)
{
    std::ostringstream n;
    n.precision(16);
    n << input;
    return PGE_GameSaveDB::variableSet(al, name, n.str(), VTYPE_FLOATING_POINT);
}

bool PGE_GameSaveDB::variableSet(PGE_GameSaveDB::VAR_ACCESS_LEVEL al,
                                 const std::string &name,
                                 int64_t input)
{
    return PGE_GameSaveDB::variableSet(al, name, std::to_string(input), VTYPE_INTEGER);
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
