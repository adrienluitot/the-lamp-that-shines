#include "utils.h"

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
bool utils_same_tables(uint8_t* table1, uint8_t* table2, uint16_t size) {
    bool areTablesSame = 1;

    for(uint16_t i = 0; i < size && areTablesSame; i++) {
        if(*(table1 + i) != *(table2 + i)) {
            areTablesSame = 0;
        }
    }
    
    return areTablesSame;
}


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
void utils_copy_tables(uint8_t* table1, uint8_t* table2, uint16_t size) {

    for(uint16_t i = 0; i < size; i++) {
        *(table2 + i) = *(table1 + i);
    }
    
    return;
}


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
void utils_reset_table(uint8_t* table, uint16_t size) {

    for(uint16_t i = 0; i < size; i++) {
        *(table + i) = 0;
    }
    
    return;
}