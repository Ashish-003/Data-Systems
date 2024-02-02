#include "global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN USING NESTED relation_name1, relation_name2 ON column_name1 bin_op column_name2 BUFFER buffer_size
 * SYNTAX: R <- JOIN USING PARTHASH relation_name1, relation_name2 ON column_name1 bin_op column_name2 BUFFER buffer_size
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[7] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    if (tokenizedQuery[1].compare("<-") != 0)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[2].compare("JOIN") != 0)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[3].compare("USING") != 0)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[4] == "NESTED")
    {
        parsedQuery.joinAlgorithm = NESTED;
    }
    else if (tokenizedQuery[4] == "PARTHASH")
    {
        parsedQuery.joinAlgorithm = PARTHASH;
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    if (tokenizedQuery[11].compare("BUFFER") != 0)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    try
    {
        parsedQuery.joinBufferSize = stoi(tokenizedQuery[12]);
    }
    catch (...)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    if (parsedQuery.joinBinaryOperator != EQUAL)
        parsedQuery.joinAlgorithm = NESTED;

    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    // checking tables
    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    // checking buffer size

    if (parsedQuery.joinAlgorithm == NESTED && parsedQuery.joinBufferSize < 3)
    {
        cout << "SEMANTIC ERROR: Buffer size too small" << endl;
        return false;
    }

    if (parsedQuery.joinAlgorithm == PARTHASH && parsedQuery.joinBufferSize < 2)
    {
        cout << "SEMANTIC ERROR: Buffer size too small" << endl;
        return false;
    }

    // checking columns
    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) ||
        !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

int min(int x, int y)
{
    if (x < y)
        return x;
    else
        return y;
}

void combineVectors(vector<int> &result, vector<int> v1, vector<int> v2)
{
    for (auto x : v1)
    {
        result.push_back(x);
    }
    for (auto x : v2)
    {
        result.push_back(x);
    }
}

void joinPartition(Table *t1, Table *t2, int key, int t1Blocks, int t2Blocks, int t1Rows, int t2Rows,
                   int t1ColumnIndex, int t2ColumnIndex, Table *newTable, vector<vector<int>> &finalRows, int swapped)
{
    // loading all the rows of the smaller partition
    vector<vector<int>> rowsTable1;
    vector<vector<int>> temp;
    for (int i = 0; i < t1Blocks; i++)
    {

        Page table1Page = bufferManager.getPartition(t1->tableName, key, i, min(t1Rows, t1->maxRowsPerBlock));
        if (t1Rows > t1->maxRowsPerBlock)
        {
            t1Rows -= t1->maxRowsPerBlock;
        }
        temp = table1Page.getRows();
        for (auto row : temp)
        {
            rowsTable1.push_back(row);
        }
    }

    // LOADING EACH BLOCK OF THE LARGER PARTITION AND THEN CHECKING

    for (int i = 0; i < t2Blocks; i++)
    {
        Page table2Page = bufferManager.getPartition(t2->tableName, key, i, min(t2Rows, t2->maxRowsPerBlock));
        temp = table2Page.getRows();
        if (t2Rows > t2->maxRowsPerBlock)
        {
            t2Rows -= t2->maxRowsPerBlock;
        }
        for (auto row1 : rowsTable1)
        {
            for (auto row2 : temp)
            {
                if (row1[t1ColumnIndex] != row2[t2ColumnIndex])
                    continue;

                vector<int> here;
                if (!swapped)
                {
                    combineVectors(here, row1, row2);
                    // for (auto x : row1)
                    // {
                    //     here.push_back(x);
                    // }
                    // for (auto x : row2)
                    // {
                    //     here.push_back(x);
                    // }
                }
                else
                {
                    combineVectors(here, row2, row1);
                    // for (auto x : row2)
                    // {
                    //     here.push_back(x);
                    // }
                    // for (auto x : row1)
                    // {
                    //     here.push_back(x);
                    // }
                }
                finalRows.push_back(here);
                if (finalRows.size() == newTable->maxRowsPerBlock)
                {
                    newTable->addRows(finalRows);
                    finalRows.clear();
                }
            }
        }
    }
}

void executeJOIN()
{
    logger.log("executeJOIN");
    int bufferSize = parsedQuery.joinBufferSize;
    vector<string> columnNames;
    Table *table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    for (int i = 0; i < table1->columnCount; i++)
    {
        columnNames.push_back(table1->columns[i]);
    }
    Table *table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    for (int i = 0; i < table2->columnCount; i++)
    {
        columnNames.push_back(table2->columns[i]);
    }
    int firstColumnIndex = table1->getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColumnIndex = table2->getColumnIndex(parsedQuery.joinSecondColumnName);
    Table *newTable = new Table(parsedQuery.joinResultRelationName, columnNames);
    vector<vector<int>> table1Rows;
    vector<vector<int>> table2Rows;
    vector<vector<int>> finalTable;
    Cursor cursorTable1 = table1->getCursor();
    Cursor cursorTable2 = table2->getCursor();
    if (parsedQuery.joinAlgorithm == NESTED)
    {
        // NESTED JOIN
        int table1Row = 0;
        int table1Page = 0;
        int tabel2Row = 0;
        int table2Page = 0;
        while (true)
        {
            if (table1Row >= table1->rowCount || table1Page >= table1->blockCount)
            {
                break;
            }

            // Loading from first table
            table1Rows.clear();

            int currentBlocks = 0;
            cursorTable1 = table1->getCursor();

            for (int i = 0; i < table1Page; i++)
            {
                table1->getNextPage(&cursorTable1);
            }
            while (currentBlocks < bufferSize - 2 && table1Page < table1->blockCount)
            {
                for (int i = 0; i < table1->rowsPerBlockCount[table1Page]; i++)
                {
                    table1Rows.push_back(cursorTable1.getNext());
                    table1Row++;
                }
                table1->getNextPage(&cursorTable1);
                table1Page++;
                currentBlocks++;
            }

            // Loading from second table
            cursorTable2 = table2->getCursor();
            for (int i = 0; i < table2->blockCount; i++)
            {
                table2Rows.clear();
                for (int j = 0; j < table2->rowsPerBlockCount[i]; j++)
                {
                    table2Rows.push_back(cursorTable2.getNext());
                }
                for (auto r1 : table1Rows)
                {
                    for (auto r2 : table2Rows)
                    {
                        if (evaluateBinOp(r1[firstColumnIndex], r2[secondColumnIndex], parsedQuery.joinBinaryOperator))
                        {
                            vector<int> currRow;
                            for (auto x : r1)
                            {
                                currRow.push_back(x);
                            }
                            for (auto x : r2)
                            {
                                currRow.push_back(x);
                            }
                            finalTable.push_back(currRow);
                        }
                        if (finalTable.size() == newTable->maxRowsPerBlock)
                        {
                            newTable->addRows(finalTable);
                            finalTable.clear();
                        }
                    }
                }
                if (finalTable.size() != 0)
                {
                    newTable->addRows(finalTable);
                    finalTable.clear();
                }
                table2->getNextPage(&cursorTable2);
            }
        }
    }
    else
    {
        // PARTHASH JOIN
        map<int, vector<vector<int>>> rowsBymap;
        map<int, int> numPageByKeyTable1;
        map<int, int> numPageByKeyTable2;
        map<int, int> rowsByKeyTable1;
        map<int, int> rowsByKeyTable2;
        Cursor cursorTable1 = table1->getCursor();
        int M = bufferSize - 1;

        for (int i = 0; i < table1->rowCount; i++)
        {
            vector<int> row = cursorTable1.getNext();
            int value = row[firstColumnIndex];
            value %= M;
            rowsBymap[value].push_back(row);
            rowsByKeyTable1[value] += 1;
            if (rowsBymap[value].size() == table1->maxRowsPerBlock)
            {
                table1->addPartition(value, numPageByKeyTable1[value], rowsBymap[value]);
                numPageByKeyTable1[value]++;
                rowsBymap[value].clear();
            }
        }
        for (auto items : rowsBymap)
        {
            if (items.second.size() != 0)
            {
                table1->addPartition(items.first, numPageByKeyTable1[items.first], items.second);
                numPageByKeyTable1[items.first]++;
            }
        }
        rowsBymap.clear();

        Cursor cursorTable2 = table2->getCursor();
        for (int i = 0; i < table2->rowCount; i++)
        {
            vector<int> row = cursorTable2.getNext();
            int value = row[secondColumnIndex];
            value %= M;
            rowsBymap[value].push_back(row);
            rowsByKeyTable2[value] += 1;
            if (rowsBymap[value].size() == table2->maxRowsPerBlock)
            {
                table2->addPartition(value, numPageByKeyTable2[value], rowsBymap[value]);
                numPageByKeyTable2[value]++;
                rowsBymap[value].clear();
            }
        }
        for (auto items : rowsBymap)
        {
            if (items.second.size() != 0)
            {
                table2->addPartition(items.first, numPageByKeyTable2[items.first], items.second);
                numPageByKeyTable2[items.first]++;
            }
        }

        for (auto items : numPageByKeyTable1)
        {
            int key = items.first;
            if (numPageByKeyTable2[key] != 0)
            {
                if (rowsByKeyTable1[key] < rowsByKeyTable2[key])
                {
                    joinPartition(table1, table2, key, numPageByKeyTable1[key], numPageByKeyTable2[key],
                                  rowsByKeyTable1[key], rowsByKeyTable2[key],
                                  firstColumnIndex, secondColumnIndex, newTable, finalTable, 0);
                }
                else
                {
                    joinPartition(table2, table1, key, numPageByKeyTable2[key], numPageByKeyTable1[key],
                                  rowsByKeyTable2[key], rowsByKeyTable1[key],
                                  secondColumnIndex, firstColumnIndex, newTable, finalTable, 1);
                }
            }
        }
        if (finalTable.size() != 0)
        {
            newTable->addRows(finalTable);
            finalTable.clear();
        }
    }
    tableCatalogue.insertTable(newTable);
    cout << "BLOCK ACCESSES: " << BLOCK_ACCESS_COUNT << endl;

    return;
}
