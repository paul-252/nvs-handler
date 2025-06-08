/*
 nvs_entry.c
 Date: 08.06.2025
 Created by: Paul Mulligan
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "nvs_entry.h"

NVS_Entry* nvs_entries[MAX_NVS_RAM];

// static functions
static uint32_t compute_crc32(const void *data, size_t len);
static bool write_nvs_buffer_to_file(NVS_Write_Buffer* nvs_buffer, FILE* file);

// API functions
/*
 * Function: nvs_init()
 * Description: This opens a file with stored NVS key-values with its metadata (crc and deleted flag etc.)
                and copies the nvs key-values into RAM if the data is not corrupted or deleted.

                If this was real NVS storage and not just a file, I would say the following would be performed here at least :

                * Check that the NVS partition exists. Perhaps by looking up a flash partition table to find the address range like in ESP systems
                * If it is the first time being used, erase the partition.
                * If encryption is used, generate and / or initialize encryption keys
                * Copy the key-values to RAM. (This may not be mandatory. We could also read and write directly from to flash. Let's discuss )
 * @param [in] mock_corruption - true to mock corrupted data being read, false for not. Just for initial testing.
 * @return true if the NVS was initialized properly, false if not.
*/
bool nvs_init(bool mock_corruption)
{
    NVS_Entry entry;
    FILE *file;
    uint32_t entry_idx = 0;

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        nvs_entries[i] = NULL;
    }

    file = fopen(NVS_FILE, "ab+");
    if (!file)
    {
        printf("Error! No NVS file found.\n");
        return false;
    }

    printf("File %s open\n", NVS_FILE );

    while (fread(&entry, sizeof(NVS_Entry), 1, file) == 1)
    {
        // Make the crc32 incorrect to simulate corruption
        if (mock_corruption == true)
        {
            entry.crc32 -= 1;
        }

        uint32_t computed_crc = compute_crc32(&entry.data, sizeof(Data));

        if (computed_crc != entry.crc32)
        {
            printf("Error! Corrupted NVS Entry detected!\n");
            // The flash page should now be flagged that it has corrupted data. Either here or in a lower layer closer to the hardware.
            // Does this mean that the there was just a partial write error from the application or driver? If so we can overwrite it?
            // or there could be a problem with the actual flash page in hardware. This needs to be handled, but maybe out of scope for here.
            continue;
        }

         nvs_entries[entry_idx] = malloc(sizeof(NVS_Entry));

         strncpy(nvs_entries[entry_idx]->data.key, entry.data.key, KEY_MAX_LEN);
         nvs_entries[entry_idx]->data.type = entry.data.type;
         nvs_entries[entry_idx]->deleted = false;
         nvs_entries[entry_idx]->modified = false;

         switch( nvs_entries[entry_idx]->data.type)
         {
            case TYPE_UINT32:
                 nvs_entries[entry_idx]->data.value.uint_val = entry.data.value.uint_val;
                 printf("Copied nvs key %s with data %u to index %d\n", entry.data.key, entry.data.value.uint_val , entry_idx);
                 break;
            case TYPE_STRING:
                strncpy( nvs_entries[entry_idx]->data.value.str_val, entry.data.value.str_val, VALUE_STR_MAX_LEN);
                printf("Copied nvs key %s with data %s to index %d\n", entry.data.key, entry.data.value.str_val , entry_idx);
                break;
            default :
                printf("ERROR! Unknown data type\n");
                continue;
         }

         entry_idx++;
    }

    fclose(file);

    return true;
}

/*
 * Function: nvs_deinit()
 * Description: Free up the dynamic memory in RAM which was used for the NVS data
*/
void nvs_deinit()
{
    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        if (nvs_entries[i] !=  NULL)
        {
            free(nvs_entries[i]);
            nvs_entries[i] = NULL;
        }
        else
        {
            return;
        }
    }
}

/*
 * Function: nvs_delete_entry()
 *
 * Description: Delete an NVS entry. Simply update its deleted flag if key is found. It should then be erased from flash later.
 *
 * @param [in] key - The NVS key to look up
 * @return true if the NVS was found and flagged for deleted, false if not
*/
bool nvs_delete_entry(const char *key)
{
    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        if (nvs_entries[i] ==  NULL)
        {
            return false;
        }

        if ( strncmp(nvs_entries[i]->data.key, key, KEY_MAX_LEN) == 0 )
        {
            nvs_entries[i]->deleted = true;
            return true;
        }
    }

    return false;
}

/*
 * Function: nvs_set_uint32()
 *
 * Description: set uint32 value based on its key. If the key is not found, add a new key-value entry
 *
 * @param [in] key - The NVS key to look up
 * @param [in] val - The uint32 value to set
 * @return true if the NVS key-value was updated, false if not
*/
bool nvs_set_uint32(const char *key, uint32_t val)
{
    bool valid_key = false;

    printf("nvs_set_uint32\n");

    if (strlen(key) >= KEY_MAX_LEN)
    {
        printf("ERROR! Provided key %s is too large. Maximum size is %d\n", key, KEY_MAX_LEN );
        return false;
    }

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        // If we get to a NULL entry, key has not been found. Add a new one.
        if (nvs_entries[i] == NULL)
        {
            printf("No entry found. entering key %s with value %d into RAM at index %d\n", key, val, i);
            nvs_entries[i] = calloc(1, sizeof(NVS_Entry));
            strncpy(nvs_entries[i]->data.key, key, KEY_MAX_LEN - 1);
            nvs_entries[i]->data.type = TYPE_UINT32;

            valid_key = true;
        }

        else if ( strncmp(nvs_entries[i]->data.key, key, KEY_MAX_LEN) == 0 )
        {
            if (nvs_entries[i]->deleted == true )
            {
                printf("ERROR! %s has been deleted. Unable to update.\n", key );
                return false;
            }
            // key found
            if (nvs_entries[i]->data.type != TYPE_UINT32)
            {
                printf("ERROR! Value for key %s is not of TYPE_UINT32 \n", key );
                return false;
            }

            valid_key = true;
        }

        if ( valid_key == true )
        {
            // Update the value
            nvs_entries[i]->data.value.uint_val = val;
            nvs_entries[i]->modified = true;
            return true;
        }
    }

     // If we get here, the memory is full
    printf("ERROR! No more space in memory to add a new NVS entry\n" );

    return false;
}

/*
 * Function: nvs_set_string()
 *
 * Description: set string value based on its key. If the key is not found, add a new key-value entry
 *
 * @param [in] key     - The NVS key to look up
 * @param [in] str_val - The string value to set
 * @return true if the NVS key-value was updated, false if not
*/
bool nvs_set_string(const char *key, const char *str_val)
{
    bool valid_key = false;

    if (strlen(key) >= KEY_MAX_LEN)
    {
        printf("ERROR! Provided key %s is too large. Maximum size is %d\n", key, KEY_MAX_LEN );
        return false;
    }

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        // If we get to a NULL entry, key has not been found. Add a new one.
        if (nvs_entries[i] == NULL)
        {
            printf("No entry found entering key %s with value %s into RAM at index %d\n", key, str_val, i);
            nvs_entries[i] = calloc(1, sizeof(NVS_Entry));

            strncpy(nvs_entries[i]->data.key, key, KEY_MAX_LEN - 1);
            nvs_entries[i]->data.type = TYPE_STRING;

            valid_key = true;
        }

        else if (strncmp(nvs_entries[i]->data.key, key, KEY_MAX_LEN) == 0)
        {
            // key found
            if (nvs_entries[i]->data.type != TYPE_STRING)
            {
                printf("ERROR! Value for key %s is not of TYPE_STRING \n", key );
                return false;
            }

            if (nvs_entries[i]->deleted == true )
            {
                printf("ERROR! %s has been deleted. Unable to update.\n", key );
                return false;
            }

            valid_key = true;
        }

        if (valid_key == true)
        {
            // Update the value
            strncpy(nvs_entries[i]->data.value.str_val, str_val, VALUE_STR_MAX_LEN - 1);
            nvs_entries[i]->modified = true;
            return true;
        }
    }

    // If we get here the memory is full
    printf("ERROR! No more space in memory to add a new NVS entry\n" );

    return false;
}

/*
 * Function: nvs_get_uint32()
 *
 * Description: get uint32 value based on its key
 *
 * @param [in] key - The NVS key to look up
 * @param [out] val - The uint32 value to return
 * @return true if the NVS key-value was found, false if not
*/
bool nvs_get_uint32(const char *key, uint32_t *val)
{
    if (strlen(key) >= KEY_MAX_LEN)
    {
        printf("ERROR! Provided key %s is too large. Maximum size is %d\n", key, KEY_MAX_LEN );
        return false;
    }

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        // If we get to a NULL entry, key has not been found.
        if (nvs_entries[i] == NULL)
        {
            printf("ERROR! key %s not found\n", key);
            return false;
        }

        if ( strncmp(nvs_entries[i]->data.key, key, KEY_MAX_LEN) == 0 )
        {
           // key found
           if (nvs_entries[i]->data.type != TYPE_UINT32)
           {
                printf("ERROR! Value for key %s is not of TYPE_UINT32 \n", key );
                return false;
           }

           if (nvs_entries[i]->deleted == true )
           {
                printf("ERROR! %s has been deleted. Unable to update.\n", key );
                return false;
           }

           // Update the value
           *val = nvs_entries[i]->data.value.uint_val;
           return true;

        }
    }

    printf("ERROR! key %s not found\n", key);

    return false;
}

/*
 * Function: nvs_get_string()
 *
 * Description: get string value based on its key.
 *
 * @param [in] key      - The NVS key to look up
 * @param [out] buf     - The string buffer to return the string value
 * @param [in] buf_len  - The length of the string buffer
 * @return true if the NVS key-value was updated, false if not
*/
bool nvs_get_string(const char *key, char *buf, size_t buf_len)
{
    if (strlen(key) >= KEY_MAX_LEN)
    {
        printf("ERROR! Provided key %s is too large. Maximum size is %d\n", key, KEY_MAX_LEN );
        return false;
    }

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        // If we get to a NULL entry, key has not been found.
        if (nvs_entries[i] == NULL)
        {
            printf("ERROR! key %s not found\n", key);
            return false;
        }

        if (strncmp(nvs_entries[i]->data.key, key, KEY_MAX_LEN) == 0)
        {
            // key found
            if (nvs_entries[i]->data.type != TYPE_STRING)
            {
                printf("ERROR! Value for key %s is not of TYPE_STRING\n", key );
                return false;
            }

            if (nvs_entries[i]->deleted == true)
            {
                printf("ERROR! %s has been deleted. Unable to update.\n", key );
                return false;
            }

            // Update the output buffer
            strncpy(buf, nvs_entries[i]->data.value.str_val, buf_len);
            return true;
        }
    }

    printf("ERROR! key %s not found\n", key);
    return false;
}

/*
 * Function: nvs_commit()
 * Description: This opens a file and re-writes the NVS entries in RAM to it. Ignoring the entries that have been deleted.
                The data is written 256 bytes ta a time due to the page size of the MX25R8035F NOPR flash memory.

                This is a very simple implementation.

                This does not simulate how how the actual handling of the writing to flash would and should be. So many things to consider. Here are a few :

                * We need to keep flash writes to a minimum to preserve its life. The smallest unit that can be written to is 256 bytes (page size) so how to organize the page writes efficiently.
                * Only NVS key-values that have been modified in RAM should be written.
                * NVS entries that have been deleted, do we delete them now from flash or later? The smallest unit that can be erased is 4KB sector, so this needs to be handled well.
                * A wear-levelling algorithim should ensure an even distribution of writes across the NVS flash partition.

 * @return true if the NVS was written to succesfully, false if not.
*/

bool nvs_commit()
{
    printf("nvs_commit\n");

    bool commited = false;

    FILE *file = fopen(NVS_FILE, "wb+");
    if (!file)
    {
        printf("Error! No NVS file found.\n");
        return false;
    }

    NVS_Write_Buffer nvs_buffer;
    int counter = 0;

    for (int i = 0; i < MAX_NVS_RAM; i++)
    {
        // End of values to write
        if (nvs_entries[i] == NULL)
        {
            // An NVS entry is in the write buffer
            if (counter > 0)
            {
                commited = write_nvs_buffer_to_file(&nvs_buffer, file);
            }

            break;
        }

        if (nvs_entries[i]->deleted == true)
        {
            continue;
        }

        nvs_entries[i]->crc32 = compute_crc32(&nvs_entries[i]->data, sizeof(Data));

        nvs_buffer[counter++] = *nvs_entries[i];

        if (counter == 2)
        {
            commited = write_nvs_buffer_to_file(&nvs_buffer, file);

            if (commited == false)
            {
                break;
            }

            counter = 0;
        }
    }

    fclose(file);

    return commited;
}

static uint32_t compute_crc32(const void *data, size_t len)
{
    // Simple CRC32 implementation (polynomial 0xEDB88320)
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *buf = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & -(int)(crc & 1));
        }
    }

    return ~crc;
}

static bool write_nvs_buffer_to_file(NVS_Write_Buffer* nvs_buffer, FILE* file)
{
    int num_written = fwrite(nvs_buffer, sizeof(NVS_Write_Buffer), 1, file);
    if (num_written != 1)
    {
        printf("Error writing to file\n");
        return false;
    }

    memset(nvs_buffer, 0, sizeof(NVS_Write_Buffer));

    return true;
}
