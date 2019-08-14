#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "Common.h"
#include "Error.h"
#include <sqlite3.h>


class Database
{
public:
    Database(const char * zDbFilename);

    Database & create();
    Database & createTable();
    bool execute(const char * sql, Database * pDatabase);
    bool insert(string tblname, map<string, string> & kvMap);
    bool selectNewest(string tblname, map<string, string> & kvMap);
    bool select(string tblname, map<string, string> & kvMap);
    bool update(string tblname, string id, map<string, string> & kvMap);
    bool update(string tblname, map<string, string> & whereMap, map<string, string> & kvMap);
    void getResult(vector<map<string, string>> & resultMapVector_o);
    vector<map<string, string>> & getResult();

    bool findALL(string tblname);
    void printResult();

    void init();

    ~Database();

private:
    string dbFilename;
    string sqlSring;
    sqlite3 * pDb;
    char * zErrMsg;
    int rc;
    vector<map<string, string>> resultMapVector;

};


#endif