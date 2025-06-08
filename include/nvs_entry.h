/*
 nvs_entry.h
 Date: 08.06.2025
 Created by: Paul Mulligan
*/

#ifndef NVS_ENTRY_H
#define NVS_ENTRY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define KEY_MAX_LEN 32
#define VALUE_STR_MAX_LEN 64
#define NVS_ENTRY_SIZE 128
#define MAX_NVS_RAM 100 // The maximum number of NVS entries we can store in RAM. Just a random value.

#define NVS_FILE "nvs_flash_crc.bin"

typedef enum
{
    TYPE_UINT32,
    TYPE_STRING
} ValueType;

typedef struct
{
    ValueType type;
    char key[KEY_MAX_LEN];
    union
    {
        uint32_t uint_val;
        char str_val[VALUE_STR_MAX_LEN];
    } value;
} Data;

typedef struct __attribute__((aligned(128)))
{
    uint32_t crc32; // checksum to detect corruption
    bool deleted;
    bool modified;
    Data data;
} NVS_Entry;

typedef NVS_Entry NVS_Write_Buffer[2] ;

// API
bool nvs_init(bool mock_corruption);
void nvs_deinit();
bool nvs_commit();

bool nvs_set_uint32(const char *key, uint32_t val);
bool nvs_set_string(const char *key, const char *val);

bool nvs_get_uint32(const char *key, uint32_t *val);
bool nvs_get_string(const char *key, char *buf, size_t buf_len);

bool nvs_delete_entry(const char *key);

#endif // NVS_ENTRY_H
