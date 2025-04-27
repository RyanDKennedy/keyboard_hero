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
    const char *query = "INSERT INTO Songs (name, duration)  VALUES ('%s', %f);";

    int formatted_query_mem_size = (strlen(query) + 256) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);

    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, name, duration) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    SY_ERROR_COND(sqlite3_exec(db, formatted_query, NULL, NULL, NULL) != SQLITE_OK, "Failed to insert song.");

    DBSong result;
    result.id = sqlite3_last_insert_rowid(db);
    strncpy(result.name, name, 256);
    result.duration = duration;

    return result;
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

void db_update_song(sqlite3 *db, DBSong song)
{
    const char *query = "UPDATE Songs SET name = '%s', duration = %f WHERE id = %ld;";

    int formatted_query_mem_size = (strlen(query) + 300) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);

    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, song.name, song.duration, song.id) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    SY_ERROR_COND(sqlite3_exec(db, formatted_query, NULL, NULL, NULL) != SQLITE_OK, "Failed to update song.");
}

DBNote db_create_note(sqlite3 *db, ssize_t song_id, uint32_t key, float timestamp, float duration)
{
    const char *query = "INSERT INTO Notes (song_id, key, timestamp, duration)  VALUES (%ld, %lu, %f, %f);";

    int formatted_query_mem_size = (strlen(query) + 256) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);

    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, song_id, key, timestamp, duration) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    SY_ERROR_COND(sqlite3_exec(db, formatted_query, NULL, NULL, NULL) != SQLITE_OK, "Failed to delete note.");

    DBNote result;
    result.id = sqlite3_last_insert_rowid(db);
    result.song_id = song_id;
    result.key = key;
    result.timestamp = timestamp;
    result.duration = duration;

    return result;
}

void db_delete_note(sqlite3 *db, ssize_t id)
{
    const char *query = "DELETE FROM Notes WHERE id = %ld;";

    int formatted_query_mem_size = (strlen(query) + 20) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);
    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, id) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    SY_ERROR_COND(sqlite3_exec(db, formatted_query, NULL, NULL, NULL) != SQLITE_OK, "Failed to delete note.");
}

void db_update_note(sqlite3 *db, DBNote note)
{
    const char *query = "UPDATE Notes SET song_id = %ld, key = %u, timestamp = %f, duration = %f WHERE id = %ld;";

    int formatted_query_mem_size = (strlen(query) + 256) * sizeof(char);
    char *formatted_query = (char*)alloca(formatted_query_mem_size);
    SY_ERROR_COND(snprintf(formatted_query, formatted_query_mem_size, query, note.song_id, note.key, note.timestamp, note.duration, note.id) == formatted_query_mem_size, "Failed to format query, you need to allocate more space, adjust number and recompile.");

    SY_ERROR_COND(sqlite3_exec(db, formatted_query, NULL, NULL, NULL) != SQLITE_OK, "Failed to update note.");
}


void db_get_all_notes(sqlite3 *db, DBNote *out_notes, size_t *out_notes_size)
{
    int status; // stores the result (success or failure) from functions.

    if (out_notes_size != NULL)
    {
	const char *records_query = "SELECT COUNT(*) FROM Notes;";

	// Prepare Statement
	sqlite3_stmt *statement = NULL;
	status = sqlite3_prepare_v2(db, records_query, strlen(records_query), &statement, NULL);
	SY_ERROR_COND(status != SQLITE_OK || statement == NULL, "Failed to prepare statement %d", status);
	
	// Step Statement
	while ((status = sqlite3_step(statement)) == SQLITE_ROW)
	{
	    *out_notes_size = sqlite3_column_int64(statement, 0);
	}
	
	SY_ERROR_COND(status != SQLITE_DONE, "Failed to step db %d", status);
	
	// Finalize Statement
	status = sqlite3_finalize(statement);
	SY_ERROR_COND(status != SQLITE_OK, "Failed to finalize db statement");

    }

    if (out_notes != NULL)
    {
	const char *records_query = "SELECT id, song_id, key, timestamp, duration FROM Notes ORDER BY timestamp ASC;";
	
	// Prepare Statement
	sqlite3_stmt *statement = NULL;
	status = sqlite3_prepare_v2(db, records_query, strlen(records_query), &statement, NULL);
	SY_ERROR_COND(status != SQLITE_OK || statement == NULL, "Failed to prepare statement %d", status);
	
	// Step Statement
	int row_num = 0;
	while ((status = sqlite3_step(statement)) == SQLITE_ROW)
	{
	    out_notes[row_num].id = sqlite3_column_int64(statement, 0);
	    out_notes[row_num].song_id = sqlite3_column_int64(statement, 1);
	    out_notes[row_num].key = sqlite3_column_int64(statement, 2);
	    out_notes[row_num].timestamp = sqlite3_column_double(statement, 3);
	    out_notes[row_num].duration = sqlite3_column_double(statement, 4);
	    ++row_num;
	}
	
	SY_ERROR_COND(status != SQLITE_DONE, "Failed to step db %d", status);
	
	// Finalize Statement
	status = sqlite3_finalize(statement);
	SY_ERROR_COND(status != SQLITE_OK, "Failed to finalize db statement");
	
    }

}
