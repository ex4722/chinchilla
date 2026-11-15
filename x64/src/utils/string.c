#include "utils/string.h"

void memset(void *buf, u8 c, u64 size) {
    for (u64 i = 0; i < size; i++) {
        ((u8 *)buf)[i] = c;
    }
}

u8 memcmp(void *s1, void * s2, u8 length){
    for(int i = 0; i < length; i++){
        if(((u8*)s1)[i] != ((u8*)s2)[i]){
            return  ((u8*)s1)[i] - ((u8*)s2)[i];
       }
    }
    return 0;
}

u64 strlen(char *str){
    u64 i = -1;
    while(str[++i]){
    }
    return i;
}
