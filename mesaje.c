#include <sqlite3.h>
#include <stdio.h>

int main(void) {

    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open("../mess.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char *sql = "DROP TABLE IF EXISTS mess;"
                "CREATE TABLE mess(username_send TEXT, username_recv TEXT, message TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {

        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);

        return 1;
    }

    sqlite3_close(db);

    return 0;
}
