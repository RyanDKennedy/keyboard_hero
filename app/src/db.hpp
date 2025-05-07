#pragma once

#include <cstdint>
#include <stdio.h>
#include <sqlite3.h>



struct DBSong
{
    ssize_t id; // id == -1 is a way to return errors
    char name[256];
    float duration; // in seconds
};

struct DBNote
{
    ssize_t id;
    ssize_t song_id;
    uint32_t key;
    float timestamp;
    float duration;
};

void db_init(sqlite3 **db);
void db_close(sqlite3 **db);

/**
 * @param db database
 * @param duration duration of song in seconds
 * @returns the created song
 * @returns the created song with id == -1 if the song couldn't be created
 */
DBSong db_create_song(sqlite3 *db, const char *name, float duration);
DBSong db_get_song_from_name(sqlite3 *db, const char *name);
void db_get_all_songs(sqlite3 *db, DBSong *out_songs, size_t *out_songs_size);

void db_delete_song(sqlite3 *db, ssize_t id);
// updates duration and name for the same id
void db_update_song(sqlite3 *db, DBSong song);

DBNote db_create_note(sqlite3 *db, ssize_t song_id, uint32_t key, float timestamp, float duration);
void db_delete_note(sqlite3 *db, ssize_t id);
void db_update_note(sqlite3 *db, DBNote note);
void db_get_all_notes_from_song(sqlite3 *db, ssize_t song_id, DBNote *out_notes, size_t *out_notes_size);
