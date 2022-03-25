#ifndef UTILS_LIB
#define UTILS_LIB

#include <stdint.h>
#include "pico/stdlib.h"

/**************************************************************************
Function:
    bool utils_same_tables(uint8_t* table1, uint8_t* table2, uint8_t size)

Description:
    This function will compare 2 tables and returns if they wether have the
    same values or not
    
Parameters:
    uint8_t* table1 - Pointer of the first table
    uint8_t* table1 - Pointer of the second table
    uint8_t size - The size of the two tables
    
Return:
    True if tables have the sames values, false otherwise
**************************************************************************/
bool utils_same_tables(uint8_t* table1, uint8_t* table2, uint16_t size);


/**************************************************************************
Function:
    bool utils_copy_tables(uint8_t* table1, uint8_t* table2, uint8_t size)

Description:
    This function will copy a first table into an other one
    
Parameters:
    uint8_t* table1 - Pointer of the first table (model)
    uint8_t* table1 - Pointer of the new copy
    uint8_t size - The size of the two tables
    
Return:
    None
**************************************************************************/
void utils_copy_tables(uint8_t* table1, uint8_t* table2, uint16_t size);


/**************************************************************************
Function:
    bool utils_reset_table(uint8_t* table,uint8_t size)

Description:
    This function will copy a first table into an other one
    
Parameters:
    uint8_t* table - Pointer of the table to reset
    uint8_t size - The size of the table
    
Return:
    None
**************************************************************************/
void utils_reset_table(uint8_t* table, uint16_t size);


#endif