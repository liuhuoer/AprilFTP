#include "Database.h"



static int callback(void * pDatabase, int argc, char ** argv, char ** azColName)
{
    if(pDatabase != NULL)
    {
        vector< map<string, string>> & resultMapVector = ((Database *)pDatabase)->getResult();
        map<string, string> kvMap;
        for(int i = 0; i < argc; ++i)
        {
            kvMap.insert(std::pair<string, string>(azColName[i], argv[i]));
        }
        resultMapVector.push_back(kvMap);
    }
    return 0;
}

Database::Database(const char * zDbFilename) : dbFilename(zDbFilename)
{
    DIR * d = opendir(ROOTDIR);
    if(d)
    {
        fprintf(stderr, "Already exists: %s\n", ROOTDIR);
        closedir(d);
    }else if(mkdir(ROOTDIR, 0777) == -1){
        char buf[MAXLINE];
        fprintf(stdout, "Error(%s): %s\n", ROOTDIR, strerror_r(errno, buf, MAXLINE));
    }else{
        fprintf(stdout, "Driectory created: %s\n");
    }

    // create AprilFTP/.AprilFTP/ working directory
    d = opendir(KERNELDIR);
    if(d)
    {
        fprintf(stderr, "Already exists: %s\n", KERNELDIR);
        closedir(d);
    }else if(mkdir(KERNELDIR, 0777) == -1){
        char buf[MAXLINE];
        fprintf(stdout, "Error(%s): %s\n", KERNELDIR, strerror_r(errno, buf, MAXLINE));
    }else{
        fprintf(stdout, "Directory created: %s\n", KERNELDIR);
    }

    //create AprilFTP/.AprilFTP/ghost working directory
    d = opendir(GHOSTDIR);
    if(d)
    {
        fprintf(stderr, "Already exists: %s\n", GHOSTDIR);
        closedir(d);
    }else if(mkdir(GHOSTDIR, 0777) == -1){
        char buf[MAXLINE];
        fprintf(stdout, "Error(%s): %s\n", GHOSTDIR, strerror_r(errno, buf, MAXLINE));
    }else{
        fprintf(stdout, "Directory created: %s\n", GHOSTDIR);
    }

    //clean()
    zErrMsg = NULL;
    /* open database */
    string dirString = KERNELDIR;
    rc = sqlite3_open((dirString + dbFilename).c_str(), &pDb);
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(pDb));
        exit(0);
    }else{
        fprintf(stdout, "Open database successfully: %s\n", (dirString + dbFilename).c_str());
    }
    
}


bool Database::execute(const char * sql, Database * pDatabase)
{
    /* Clear resultMap*/
    resultMapVector.clear();
    /* Execute SQL statement*/
    rc = sqlite3_exec(pDb, sql, callback, pDatabase, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "\033[31mDatabase execute error: %s\033[0m\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }else{
        fprintf(stdout, "\033p32mDatabase execute successfully\033[0m\n");
        printResult();
        return true;
    }
}

bool Database::insert(string tblname, map<string, string> & kvMap)
{
    /* Create SQL statement*/
    sqlSring.clear();
    string valString;
    sqlSring += "INSERT INTO " + tblname + "('";
    valString += "VALUES ('";

    map<string, string>::iterator it = kvMap.begin();

    sqlSring += it->first;
    valString += it->second;
    for( ++it; it != kvMap.end(); ++it)
    {
        sqlSring += "', '" + it->first;
        valString += "', ," + it->second;
    }
    sqlSring += "')";
    valString += "')";
    sqlSring += valString;

    std::cout << "insert: " << sqlSring << std::endl;
    /* Execute SQL statement and return*/
    return execute(sqlSring.c_str(), this);
}

bool Database::select(string tblname, map<string, string> & kvMap)
{
    /* Construct SQL statement*/
    sqlSring.clear();
    sqlSring += "SELECT * from ";
    sqlSring += tblname;
    sqlSring += " WHERE ";

    map<string, string>::iterator it = kvMap.begin();
    sqlSring += it->first + "='" + it->second + "'";
    for(++it; it != kvMap.end(); ++it)
    {
        sqlSring += " and " + it->first + "='" + it->second + "'";
    }
    std::cout << "query: " << sqlSring << std::endl;
    /*Execute SQL statement*/
    return execute(sqlSring.c_str(), this);
}

bool Database::update(string tblname, string id, map<string, string> & kvMap)
{
    /* Construct SQL statement*/
    sqlSring.clear();
    sqlSring += "UPDATE ";
    sqlSring += tblname;
    sqlSring += "SET";

    map<string, string>::iterator it = kvMap.begin();
    sqlSring += it->first + "='" + it->second + "'";
    for(++it; it != kvMap.end(); ++it)
    {
        sqlSring += ", " + it->first + "='" + it->second + "'";
    }
    sqlSring += " WHERE id = '" + id + "'";
    std::cout << "update: " << sqlSring << std::endl;
    /* Excute SQL statment*/
    return execute(sqlSring.c_str(), this);
}

bool Database::update(string tblname, map<string, string> & whereMap, map<string, string> & kvMap)
{
    /* Construct SQL statement*/
    sqlSring.clear();
    sqlSring += "UPDATE ";
    sqlSring += tblname;
    sqlSring += "SET";

    map<string, string>::iterator it = kvMap.begin();
    sqlSring += it->first + "='" + it->second + "'";
    for(++it; it != kvMap.end(); ++it)
    {
        sqlSring += ", " + it->first + "='" + it->second + "'";
    }
    sqlSring += " WHERE ";

    map<string, string>::iterator iter = whereMap.begin();
    sqlSring += iter->first + "='" + iter->second + "'";
    for(++iter; iter != whereMap.end(); ++iter)
    {
        sqlSring += " and " + iter->first + "='" + iter->second + "'";
    }
    std::cout << "update: " << sqlSring << std::endl;
    /* Excute SQL statment*/
    return execute(sqlSring.c_str(), this);
}


void Database::getResult(vector< map<string, string>> & resultMapVector_o)
{
    resultMapVector_o.swap(this->resultMapVector);
}

vector< map<string, string>> & Database::getResult()
{
    return this->resultMapVector;
}


void Database::printResult()
{
    for(vector< map<string, string>>::iterator iter = resultMapVector.begin(); iter != resultMapVector.end(); ++iter)
    {
        for(map<string, string>::iterator it = iter->begin(); it != iter->end(); ++it)
            std::cout << it->first << ": " << it->second << "\n";
        std::cout << "\n";
    }
}