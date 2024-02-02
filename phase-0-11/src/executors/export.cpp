#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() != 2 && tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if(tokenizedQuery.size() == 2)
    {
        parsedQuery.queryType = EXPORT;
        parsedQuery.exportRelationName = tokenizedQuery[1];
    }
    else if(tokenizedQuery.size() == 3)
    {
        if(tokenizedQuery[1] != "MATRIX")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        parsedQuery.queryType = EXPORT;
        parsedQuery.exportRelationName = tokenizedQuery[2];
    }
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName) || matrixCatalogue.isMatrix(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if(tokenizedQuery.size() == 2)
    {
        Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
        table->makePermanent();
    }else if(tokenizedQuery.size() == 3)
    {
        Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);
        matrix->makePermanent();
    }
    return;
}