#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "colors.h"
#include "pointcloud.h"

#define MAX_COLOR_NAME_LENGTH 128

typedef struct {
    const char *name;
    Color color;
} NamedColor;


static const NamedColor namedColors[] = {
    {"red", {220, 50, 47}},
    {"orange", {230, 130, 30}},
    {"yellow", {210, 190, 40}},
    {"green", {60, 180, 90}},
    {"cyan", {40, 180, 200}},
    {"blue", {0, 120, 215}},
    {"purple", {150, 80, 200}},
    {"magenta", {200, 60, 160}},
    {"pink", {230, 120, 160}},
    {"white", {230, 230, 230}},
    {"gray", {150, 150, 150}},
    {"grey", {150, 150 ,150}}, // for british people i guess...
    {"black", {30, 30, 30}}
};


int resolveNamedColor(const char *name, Color *result) {
    char normalizedName[MAX_COLOR_NAME_LENGTH + 1];

    // Prevents buffer overflow
    size_t length = strlen(name);
    if(length > MAX_COLOR_NAME_LENGTH){
        return 0;
    }

    // Cuts off whitespaces
    size_t start = 0;
    while(start < length && isspace((unsigned char)name[start])){
        start++;
    }
    size_t end = length;
    while(end > start && isspace((unsigned char)name[end - 1])){
        end--;
    }

    // Makes string lowercase
    size_t normalizedLength = end - start;
    for(size_t i = 0; i < normalizedLength; i++){
        normalizedName[i] = tolower((unsigned char)name[start + i]);
    }
    normalizedName[normalizedLength] = '\0';

    // Checks if string matches one of the named colors
    for(size_t i = 0; i < sizeof(namedColors)/sizeof(namedColors[0]); i++) {
        if(strcmp(normalizedName, namedColors[i].name) == 0) {
            *result = namedColors[i].color;
            return 1;
        }
    }

    // If failed return 0
    return 0;
}
