/*
 main.c
 Date: 08.06.2025
 Created by: Paul Mulligan
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nvs_entry.h"

int main()
{
    bool ret;

    printf("Calling nvs_init() with no corruption\n");

    ret = nvs_init(false);
    if(ret)
    {
        printf("nvs_init() ok\n");
    }
    else
    {
         printf("Error with nvs_init()\n");
    }

    printf("=======================================================================================\n");

    const char* key1 = "sample_rate";
    uint32_t val1 = 44100;

    printf("Calling nvs_set_uint32 with key %s and value %d\n", key1, val1);

    ret = nvs_set_uint32(key1, val1);
    if(!ret)
    {
         printf("Error with nvs_set_uint32\n");
    }

    printf("=======================================================================================\n");

    uint32_t read_val;

    printf("Calling nvs_get_uint32 for key %s\n", key1);

    ret = nvs_get_uint32(key1, &read_val);
    if (ret)
    {
        printf("Value read back from get_uint32 is %u\n", val1);
    }
    else
    {
        printf("Error with nvs_get_uint32\n");
    }

    printf("=======================================================================================\n");

    const char* key2 = "name";
    const char* val2 = "Paul-M";

    printf("Calling nvs_set_string with key %s and value %s\n", key2, val2);
    ret = nvs_set_string(key2, val2);
    if(!ret)
    {
        printf("Error with nvs_set_string\n");
    }

    printf("=======================================================================================\n");

    char str [VALUE_STR_MAX_LEN] = {0};

    printf("Calling nvs_get_string for key %s\n", key2);
    ret = nvs_get_string(key2, str, sizeof(str));
    if (ret)
    {
        printf("Value read back from get_string is %s\n", str);
    }
    else
    {
        printf("Error with nvs_get_string\n");
    }

    printf("=======================================================================================\n");

    const char* key3 = "baud_rate";
    uint32_t val3 = 115200;

    printf("Calling nvs_set_uint32 with key %s and value %d\n", key3, val3);

    ret = nvs_set_uint32(key3, val3);
    if(!ret)
    {
        printf("Error with nvs_set_uint32\n");
    }

    printf("=======================================================================================\n");

    printf("Calling nvs_delete_entry() for key %s\n", key1);
    ret = nvs_delete_entry(key1);
    if (ret)
    {
        printf("key %s deleted\n", key1);
    }
    else
    {
        printf("Error deleting key %s\n", key1);
    }

    printf("=======================================================================================\n");

    printf("Calling nvs_commit with no corruption\n");

    ret = nvs_commit(false);
    if (ret)
    {
        printf("nvs_commit() success\n");
    }
    else
    {
         printf("Error with nvs_commit()\n");
    }


    printf("=======================================================================================\n");

    nvs_deinit();

    printf("Calling nvs_init() with corruption\n");

    ret = nvs_init(true);
    if(ret)
    {
        printf("nvs_init() ok\n");
    }
    else
    {
         printf("Error with nvs_init()\n");
    }

    return 0;
}
