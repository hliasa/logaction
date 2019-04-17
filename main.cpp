#include <iostream>
#include <sqlite3.h>
#include <string.h>
#include <fstream>
#include "log.h"



using namespace std;

const string STATUS_SUCCESS = "success";
const string STATUS_FAIL = "fail";
const string STATUS_DEFAULT = "default";
const string HOME_DIR = getenv("HOME");
const string DB_FILE = HOME_DIR + "/.config/logaction.db";
const string RESET = "\033[0m";
const string RED = "\033[31m";      /* Red */
const string GREEN = "\033[32m";      /* Green */

static int callback(void *NotUsed,int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static int add_log(sqlite3 *db, string msg) {
    string sql;
    sqlite3_stmt *stmt;
    sql = "INSERT INTO log (TEXT) VALUES (?);";
    sqlite3_prepare_v2(db, sql.c_str(),-1,&stmt,nullptr);
    sqlite3_bind_text(stmt, 1, msg.c_str(),-1,SQLITE_STATIC);
    return sqlite3_step(stmt);
}

static int del_log(sqlite3 *db, int id) {
    string sql;
    sqlite3_stmt *stmt;
    sql = "DELETE FROM log WHERE rowid = ?;";
    sqlite3_prepare_v2(db, sql.c_str(),-1,&stmt,nullptr);
    sqlite3_bind_int(stmt, 1, id);
    return sqlite3_step(stmt);
}

static int list_log(sqlite3 *db, int count) {
    string sql;
    sqlite3_stmt *stmt;
    sql = "SELECT t.* FROM (SELECT *,rowid FROM log ORDER BY rowid DESC LIMIT ?) t ORDER BY rowid ASC";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, count);
    while (1) {
        int s;
        s = sqlite3_step(stmt);
        if (s == SQLITE_ROW ){
            Log tmp_log;
            tmp_log.msg = std::string(reinterpret_cast<const char*>( sqlite3_column_text(stmt,0) ));
            tmp_log.timestamp = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,1)));
            tmp_log.id = sqlite3_column_int(stmt,3);
            tmp_log.status = sqlite3_column_int(stmt,2);
            tmp_log.print();

        }
        else if (s == SQLITE_DONE) {
            break;
        }
        else {
            fprintf (stderr, "Failed.\n");
            exit (1);
        }
    }
    return 0;
}

static int edit_status(sqlite3 *db, int id, string status) {
    string sql;
    int int_status;
    sqlite3_stmt *stmt;
    if ( status.compare(STATUS_SUCCESS) == 0 ) {
        int_status = 1;
    } else if ( status.compare(STATUS_FAIL) == 0 ) {
        int_status = 2;
    } else if ( status.compare(STATUS_DEFAULT) == 0 ) {
        int_status = 0;
    } else {
        return 1;
    }

    sql = "UPDATE log SET status=? WHERE rowid=?";
    sqlite3_prepare_v2(db,sql.c_str(),-1,&stmt,nullptr);
    sqlite3_bind_int(stmt, 1, int_status);
    sqlite3_bind_int(stmt, 2, id);
    return sqlite3_step(stmt);
}

static int edit_log(sqlite3 *db, int id, string message) {
    string sql;
    sqlite3_stmt *stmt;
    sql = "UPDATE log SET TEXT = ? WHERE rowid = ?;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, message.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt,2,id);

    return sqlite3_step(stmt);
}

static void print_usage() {
    cout<<"Usage:"<<endl<<
          "\tlogaction <command> <parameters>"<<endl<<
          "\tlogaction \"log message\""<<endl<<
          "command list:"<<endl<<
          "\tadd \"log message\""<<endl<<
          "\tlist <number of last logs>"<<endl<<
          "\tedit <log id> <log message>"<<endl<<
          "\tstatus <log id> <fail/success/default>"<<endl<<
          "\tdel <log id>"<<endl;
}

int main(int argc, char *argv[])
{
    sqlite3 *db;
    char *zErrMsg = nullptr;
    int rc;
    ifstream file(DB_FILE, ios::binary);
    if ( !file ) {
        cout<<"DB file doesn't exist. Let's try creating it."<<endl;
        fstream tmp;
        tmp.open(DB_FILE, ios::out);
        if ( !tmp ) {
            cout<<RED<<"Error creating file:"<<RESET<<DB_FILE<<endl;
            return 1;
        } else {
            cout<<GREEN<<"Database file created!"<<RESET<<endl;
        }
        int tmp_rc;
        tmp_rc = sqlite3_open(DB_FILE.c_str(), &db);
        if ( tmp_rc ) {
            fprintf(stderr, "Can't open DB: %s\n", sqlite3_errmsg(db));
            return 1;
        } else {
            string create_sql;
            create_sql = "CREATE TABLE log(TEXT TEXT NOT NULL,TIMESTAMP TIMESTAMP DEFAULT (datetime('now','localtime')), STATUS INT DEFAULT 0);";
            tmp_rc = sqlite3_exec(db, create_sql.c_str(), callback, nullptr, &zErrMsg);
        }
    }
    file.close();
    rc = sqlite3_open(DB_FILE.c_str(), &db);
    if ( rc ) {
        fprintf(stderr, "Can't open DB: %s\n", sqlite3_errmsg(db));
        return 1;

    }
    /*
     * Determine parameter count and act accordingly.
     */
    if ( argc == 2 ) {
        string command(argv[1]);
        if ( command.compare("list") == 0 ) {
            //list with no arguments (print last 10 logs)
            list_log(db,10);
            sqlite3_close(db);
            return 0;
        } else {
            if ( add_log(db,argv[1]) == SQLITE_DONE ) {
                cout<<GREEN<<"Log added"<<RESET<<endl;
                sqlite3_close(db);
                return 0;
            } else {
                cout<<RED<<"Couldn't add log"<<RESET<<endl;
                sqlite3_close(db);
                return 1;
            }
        }
    } else if ( argc > 2 ) {
        string command(argv[1]);
        if ( command.compare("add") == 0 ) {
            add_log(db,argv[2]);
            sqlite3_close(db);
            return 0;
        }
        if ( command.compare("del") == 0 ) {
            //delete sequence
            del_log(db,atoi(argv[2]));
            sqlite3_close(db);
            return 0;
        }
        if ( command.compare("list") == 0 ) {
            //list sequence
            list_log(db,atoi(argv[2]));
            sqlite3_close(db);
            return 0;
        }
        if ( command.compare("status") == 0 ) {
            //change status sequence
            if ( argc == 4 && edit_status(db,atoi(argv[2]),argv[3]) == SQLITE_DONE ) {
                cout<<"Edited successfully"<<endl;
                sqlite3_close(db);
                return 0;
            } else {
                cout<<RED<<"Wrong arguments."<<RESET<<endl;
                print_usage();
                sqlite3_close(db);
                return 1;
            }
        }
        if ( command.compare("edit") == 0 ) {
            if ( edit_log(db, atoi(argv[2]), argv[3]) == SQLITE_DONE ) {
                cout<<GREEN<<"Edited successfully."<<RESET<<endl;
                sqlite3_close(db);
                return 0;
            } else {
                cout<<RED<<"Something is wrong"<<RESET<<endl;
                sqlite3_close(db);
                return 1;
            }
        }
    } else {
        print_usage();
        sqlite3_close(db);
        return 0;
    }
}
