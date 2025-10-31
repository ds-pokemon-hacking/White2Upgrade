#ifndef __TYPES_H
#define __TYPES_H

// Types.
#define TYPE_NORMAL 0 
#define TYPE_FIGHTING 1
#define TYPE_FLYING 2
#define TYPE_POISON 3 
#define TYPE_GROUND 4 
#define TYPE_ROCK 5   
#define TYPE_BUG 6    
#define TYPE_GHOST 7  
#define TYPE_STEEL 8  
#define TYPE_FIRE 9   
#define TYPE_WATER 10  
#define TYPE_GRASS 11  
#define TYPE_ELECTRIC 12
#define TYPE_PSYCHIC 13
#define TYPE_ICE 14    
#define TYPE_DRAGON 15  
#define TYPE_DARK 16   
#define TYPE_FAIRY 17
#define TYPE_COUNT_MAX (TYPE_FAIRY + 1)

// Type effectiveness labels.
#define NOT_EFFECTIVE 0
#define NOT_VERY_EFFECTIVE 2
#define EFFECTIVE 4
#define SUPER_EFFECTIVE 8

// Result labels.
#define RESULT_NOT_EFFECTIVE 0
#define RESULT_NOT_VERY_EFFECTIVE 2
#define RESULT_EFFECTIVE 3
#define RESULT_SUPER_EFFECTIVE 4

#endif