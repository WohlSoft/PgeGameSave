#ifndef PGE_GAMESAVE_DB_H
#define PGE_GAMESAVE_DB_H

#include <string>

struct PGE_GameSaveDB_private;
class PGE_GameSaveDB
{
    PGE_GameSaveDB_private *p;
public:
    PGE_GameSaveDB();
    PGE_GameSaveDB(const std::string &filePath);
    PGE_GameSaveDB(const PGE_GameSaveDB &o);
    PGE_GameSaveDB(PGE_GameSaveDB &&o);
    virtual ~PGE_GameSaveDB();
    bool open(const std::string &filePath);
    void close();
};

#endif // PGE_GAMESAVE_DB_H
