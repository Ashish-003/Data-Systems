#include "global.h"
/**
 * @brief 
 * SYNTAX: PRINT relation_name
 */
bool syntacticParsePRINT()
{
    logger.log("syntacticParsePRINT");
    if (tokenizedQuery.size() != 2 && tokenizedQuery.size()!=3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if(tokenizedQuery.size() == 2)
    {
        parsedQuery.queryType = PRINT;
        parsedQuery.printRelationName = tokenizedQuery[1];
    }
    else if(tokenizedQuery.size() == 3)
    {
        if(tokenizedQuery[1] != "MATRIX")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        parsedQuery.queryType = PRINT;
        parsedQuery.printRelationName = tokenizedQuery[2];
    }
    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");
    if (!tableCatalogue.isTable(parsedQuery.printRelationName) && !matrixCatalogue.isMatrix(parsedQuery.printRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

void executePRINT()
{
    logger.log("executePRINT");
    if(tokenizedQuery.size() == 2)
    {
        Table* table = tableCatalogue.getTable(parsedQuery.printRelationName);
        table->print();
    }
    else
    {
        Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
        matrix->print();
    }
    return;
}
