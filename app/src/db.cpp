#include "db.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sqlite3.h>

#include "sy_macros.hpp"

void db_init(sqlite3 **db)
{
    SY_ERROR_COND(sqlite3_open("app/KeyboardHero.db", db) != SQLITE_OK, "Failed to open db.");
    
    SY_ERROR_COND(sqlite3_exec(*db, "CREATE TABLE IF NOT EXISTS Songs (\
id integer NOT NULL PRIMARY KEY AUTOINCREMENT,				\
name text non NOT NULL UNIQUE,							\
duration real NOT NULL							\
);", NULL, NULL, NULL) != SQLITE_OK, "Failed to create Songs table");
    
    
    SY_ERROR_COND(sqlite3_exec(*db, "CREATE TABLE IF NOT EXISTS Notes (\
id integer NOT NULL PRIMARY KEY AUTOINCREMENT,				\
song_id integer NOT NULL,						\
key integer NOT NULL,							\
timestamp real NOT NULL,						\
duration real NOT NULL,							\
FOREIGN KEY(song_id) REFERENCES Songs(id)				\
);", NULL, NULL, NULL) != SQLITE_OK, "Failed to create Notes table");
    
}

void db_close(sqlite3 **db)
{
    SY_ERROR_COND(sqlite3_close(*db) != SQLITE_OK, "Failed to close db.");
    *db = NULL;
}

DBSong db_create_song(sqlite3 *db, const char *name, float duration)
{
    const char *query = "INSERT INTO Songs (name, duration) VALUES ('%s', %f);";

    int formatted_query_mem_size = (strlen(query) + 256) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);

    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, name, duration) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    int status = sqlite3_exec(db, formatted_query, NULL, NULL, NULL);
    if (status != SQLITE_OK)
    {
	DBSong result = {.id = -1};
	return result;
    }

    return db_get_song_from_name(db, name);
}

DBSong db_get_song_from_name(sqlite3 *db, const char *name)
{
    int status; // stores the result (success or failure) from functions.
    const char *records_query_unformatted = "SELECT id, duration FROM Songs WHERE name = '%s';";

    DBSong result = {.id = -1, .duration = 0.0f};
    strncpy(result.name, name, 256);

    char *record_query = (char*)alloca(strlen(records_query_unformatted) + 257);
    sprintf(record_query, records_query_unformatted, result.name);
    
    // Prepare Statement
    sqlite3_stmt *statement = NULL;
    status = sqlite3_prepare_v2(db, record_query, strlen(record_query), &statement, NULL);
    SY_ERROR_COND(status != SQLITE_OK || statement == NULL, "Failed to prepare statement %d", status);
    
    // Step Statement
    int row_num = 0;
    while ((status = sqlite3_step(statement)) == SQLITE_ROW)
    {
	result.id = sqlite3_column_int64(statement, 0);
	result.duration = sqlite3_column_double(statement, 1);
	++row_num;
    }
    
    SY_ERROR_COND(status != SQLITE_DONE, "Failed to step db %d", status);
    
    // Finalize Statement
    status = sqlite3_finalize(statement);
    SY_ERROR_COND(status != SQLITE_OK, "Failed to finalize db statement");

    return result;
}

void db_get_all_songs(sqlite3 *db, DBSong *out_songs, size_t *out_songs_size)
{
    int status; // stores the result (success or failure) from functions.

    if (out_songs_size != NULL)
    {
	const char *records_query = "SELECT COUNT(*) FROM Songs;";

	// Prepare Statement
	sqlite3_stmt *statement = NULL;
	status = sqlite3_prepare_v2(db, records_query, strlen(records_query), &statement, NULL);
	SY_ERROR_COND(status != SQLITE_OK || statement == NULL, "Failed to prepare statement %d", status);
	
	// Step Statement
	while ((status = sqlite3_step(statement)) == SQLITE_ROW)
	{
	    *out_songs_size = sqlite3_column_int64(statement, 0);
	}
	
	SY_ERROR_COND(status != SQLITE_DONE, "Failed to step db %d", status);
	
	// Finalize Statement
	status = sqlite3_finalize(statement);
	SY_ERROR_COND(status != SQLITE_OK, "Failed to finalize db statement");

    }

    if (out_songs != NULL)
    {
	const char *records_query = "SELECT id, name, duration FROM Songs;";
	
	// Prepare Statement
	sqlite3_stmt *statement = NULL;
	status = sqlite3_prepare_v2(db, records_query, strlen(records_query), &statement, NULL);
	SY_ERROR_COND(status != SQLITE_OK || statement == NULL, "Failed to prepare statement %d", status);
	
	// Step Statement
	int row_num = 0;
	while ((status = sqlite3_step(statement)) == SQLITE_ROW)
	{
	    out_songs[row_num].id = sqlite3_column_int64(statement, 0);
	    strncpy(out_songs[row_num].name, (const char*)sqlite3_column_text(statement, 1), 256);
	    out_songs[row_num].duration = sqlite3_column_double(statement, 2);
	    ++row_num;
	}
	
	SY_ERROR_COND(status != SQLITE_DONE, "Failed to step db %d", status);
	
	// Finalize Statement
	status = sqlite3_finalize(statement);
	SY_ERROR_COND(status != SQLITE_OK, "Failed to finalize db statement");
	
    }

}


