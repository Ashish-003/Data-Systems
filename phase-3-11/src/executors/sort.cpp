#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order BUFFER buffer_size
 *
 * sorting_order = ASC | DESC
 * BUFFER is optional, default buffer size is 10
 */

bool syntacticParseSORT()
{
    logger.log("syntacticParseSORT");
    if ((tokenizedQuery.size() != 8 && tokenizedQuery.size() != 10) || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery.size() == 10)
    {
        if (tokenizedQuery[8] != "BUFFER")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        try
        {
            parsedQuery.sortBufferSize = stoi(tokenizedQuery[9]);
        }
        catch (...)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];
    if (sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if (sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseSORT()
{
    logger.log("semanticParseSORT");

    if (tableCatalogue.isTable(parsedQuery.sortResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (parsedQuery.sortBufferSize < 3)
    {
        cout << "SEMANTIC ERROR: Buffer size too small" << endl;
        return false;
    }

    return true;
}

struct Row
{
    vector<int> elements;
    int columnIndex;

    // constructor
    Row(vector<int> elements, int columnIndex) : elements(elements), columnIndex(columnIndex) {}
};

struct Compare
{
    bool operator()(Row &v1, Row &v2)
    {
        if (parsedQuery.sortingStrategy == DESC)
        {
            return v1.elements[v1.columnIndex] < v2.elements[v2.columnIndex];
        }
        return v1.elements[v1.columnIndex] > v2.elements[v2.columnIndex];
    }
};

struct PairRow
{
    pair<int, vector<int>> elements;
    int columnIndex;

    // constructor
    PairRow(pair<int, vector<int>> elements, int columnIndex) : elements(elements), columnIndex(columnIndex) {}
};

struct ComparePairRow
{
    bool operator()(PairRow &v1, PairRow &v2)
    {
        if (parsedQuery.sortingStrategy == DESC)
        {
            return v1.elements.second[v1.columnIndex] < v2.elements.second[v2.columnIndex];
        }
        return v1.elements.second[v1.columnIndex] > v2.elements.second[v2.columnIndex];
    }
};

void executeSORT()
{
    logger.log("executeSORT");
    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);
    // vector<string> tableColumns(table.columns);
    Table *sortedTable = new Table(parsedQuery.sortRelationName, table.columns);
    int columnId = table.getColumnIndex(parsedQuery.sortColumnName);
    int initialRun = ceil((float)table.blockCount / parsedQuery.sortBufferSize);
    int currentBlock = 0;
    for (int i = 0; i < initialRun; i++)
    {
        priority_queue<Row, vector<Row>, Compare> pq;
        Table *tempT = new Table(parsedQuery.sortResultRelationName + "Run" + to_string(i), table.columns);
        for (int j = 0; j < parsedQuery.sortBufferSize && currentBlock < table.blockCount; j++)
        {
            Cursor tableCursor(table.tableName, currentBlock);
            int rowNumber = 0;
            while (rowNumber < table.rowsPerBlockCount[currentBlock])
            {
                Row temp = Row(tableCursor.getNext(), columnId);
                pq.push(temp);
                rowNumber++;
            }
            currentBlock++;
        }
        while (!pq.empty())
        {
            Row row = pq.top();
            pq.pop();
            tempT->writeRow(row.elements);
        }
        tempT->blockify();
        tableCatalogue.insertTable(tempT);
    }

    // merge the chunks
    priority_queue<PairRow, vector<PairRow>, ComparePairRow> pqp;
    Table *finalTable = new Table(parsedQuery.sortResultRelationName, table.columns);
    vector<vector<int>> finalRows;
    vector<Cursor> cursors;
    for (int i = 0; i < initialRun; i++)
    {
        string tempTableName = parsedQuery.sortResultRelationName + "Run" + to_string(i);
        Cursor temp = Cursor(tempTableName, 0);
        cursors.push_back(temp);
        pqp.push(PairRow({i, temp.getNext()}, columnId));
    }
    cout << "Merged the chunks" << endl;
    vector<int> checker;
    while (!pqp.empty())
    {
        PairRow top = pqp.top();
        pqp.pop();
        finalRows.push_back(top.elements.second);
        int currentRunIndex = top.elements.first;
        checker = cursors[currentRunIndex].getNext();
        if (!checker.empty())
        {
            pqp.push(PairRow({currentRunIndex, checker}, columnId));
        }
        if (finalRows.size() == finalTable->maxRowsPerBlock)
        {
            for (auto ele : finalRows)
            {
                finalTable->writeRow(ele);
            }
            finalRows.clear();
        }
    }
    if (finalRows.size() != 0)
    {
        for (auto ele : finalRows)
        {
            finalTable->writeRow(ele);
        }
    }
    if (finalTable->blockify())
    {
        tableCatalogue.insertTable(finalTable);
    }
    else
    {
        cout << "No Data in table" << endl;
        finalTable->unload();
        delete finalTable;
    }
    return;
}