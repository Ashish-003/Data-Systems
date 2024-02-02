#include "global.h"
/**
 * @brief File contains method to process UPDATE commands.
 *
 * syntax:
 * UPDATE relation_name COLUMN column_name Operator value
 *
 * Operator = ADD | SUBTRACT | MULTIPLY
 *
 */

bool syntacticParseUPDATE()
{
    logger.log("syntacticParseUPDATE");
    if (tokenizedQuery.size() != 6 || tokenizedQuery[2] != "COLUMN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = UPDATE;
    parsedQuery.updateRelationName = tokenizedQuery[1];
    parsedQuery.updateColumnName = tokenizedQuery[3];

    try
    {
        parsedQuery.updateValue = stoi(tokenizedQuery[5]);
    }
    catch (...)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string updateOperator = tokenizedQuery[4];
    if (updateOperator == "ADD")
        parsedQuery.updateOperator = ADD;
    else if (updateOperator == "SUBTRACT")
        parsedQuery.updateOperator = SUBTRACT;
    else if (updateOperator == "MULTIPLY")
        parsedQuery.updateOperator = MULTIPLY;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseUPDATE()
{
    logger.log("semanticParseUPDATE");

    if (!tableCatalogue.isTable(parsedQuery.updateRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.updateColumnName, parsedQuery.updateRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}

void executeUPDATE()
{
    logger.log("executeUPDATE");
    bufferManager.clearPool();
    Table table = *tableCatalogue.getTable(parsedQuery.updateRelationName);
    Cursor cursor = table.getCursor();
    int columnIndex = table.getColumnIndex(parsedQuery.updateColumnName);
    vector<vector<int>> rows;
    int numRows = 0;
    for (int i = 0; i < table.blockCount; i++)
    {
        cout << "Block " << i << endl;
        Page myPage = bufferManager.getPage(table.tableName, i, true);
        rows = myPage.getRows();
        numRows = myPage.getRowCount();
        // if (i == 2)
        // {
        //     sleep(40);
        // }
        usleep(5 * 1e5);
        for (int j = 0; j < numRows; j++)
        {
            if (parsedQuery.updateOperator == ADD)
                rows[j][columnIndex] += parsedQuery.updateValue;
            else if (parsedQuery.updateOperator == SUBTRACT)
                rows[j][columnIndex] -= parsedQuery.updateValue;
            else if (parsedQuery.updateOperator == MULTIPLY)
                rows[j][columnIndex] *= parsedQuery.updateValue;
        }
        bufferManager.writePage(table.tableName, i, rows, numRows, true);
    }
    bufferManager.clearPool();
    table.makePermanent();
    return;
}