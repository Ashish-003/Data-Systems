#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() != 2 && tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    if(tokenizedQuery.size() == 2){
        parsedQuery.loadType = "TABLE";
        parsedQuery.loadRelationName = tokenizedQuery[1];
    }
    else
    {
        if(tokenizedQuery[1] != "MATRIX")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        parsedQuery.loadType = "MATRIX";
        parsedQuery.loadRelationName = tokenizedQuery[2];
    }
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Relation already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeLOAD()
{
    logger.log("executeLOAD");
    if(parsedQuery.loadType == "TABLE")
    {
        Table *table = new Table(parsedQuery.loadRelationName);
        if (table->load())
        {
            tableCatalogue.insertTable(table);
            cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
        }
    }else if(parsedQuery.loadType == "MATRIX")
    {
        Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
        if (matrix->load())
        {
            matrixCatalogue.insertMatrix(matrix);
            cout << "Loaded Matrix. Column Count: " << matrix->columnCount << " Row Count: " << matrix->columnCount << endl;
        }
    }
    return;
}