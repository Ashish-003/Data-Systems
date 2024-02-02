#include "global.h"
/**
 * @brief
 * SYNTAX: R <- GROUP BY <grouping_attribute> FROM <table_name> RETURN MAX|MIN|SUM|AVG(<attribute>)
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[3] != "BY" ||
        tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupbyResultRelationName = tokenizedQuery[0];
    parsedQuery.groupbyRelationName = tokenizedQuery[6];
    parsedQuery.groupbyColumnName = tokenizedQuery[4];

    string command = tokenizedQuery[8];
    regex re("^(.+)\\((.+)\\)$");
    smatch match;
    string aggreatorOperator = "";

    if (regex_search(command, match, re) == true)
    {
        if (match.size() != 3)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        logger.log(match.str(1));
        logger.log(match.str(2));
        aggreatorOperator = match.str(1);
        parsedQuery.groupbyAggregateColumnName = match.str(2);
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    if (aggreatorOperator == "MIN")
        parsedQuery.groupbyAggregateType = MIN;
    else if (aggreatorOperator == "MAX")
        parsedQuery.groupbyAggregateType = MAX;
    else if (aggreatorOperator == "SUM")
        parsedQuery.groupbyAggregateType = SUM;
    else if (aggreatorOperator == "AVG")
        parsedQuery.groupbyAggregateType = AVG;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.groupbyResultColumnName = aggreatorOperator + parsedQuery.groupbyAggregateColumnName;
    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupbyResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupbyColumnName, parsedQuery.groupbyRelationName) ||
        !tableCatalogue.isColumnFromTable(parsedQuery.groupbyAggregateColumnName, parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

pair<int, int> evaluateGroupbyOp(int value1, int value2, int count, AggregateType aggType)
{
    switch (aggType)
    {
    case MIN:
        return {min(value1, value2), count};
    case MAX:
        return {max(value1, value2), count};
    case SUM:
    case AVG:
        return {value1 + value2, ++count};
    default:
        return {-1, count};
    }
}

void executeGROUPBY()
{
    logger.log("executeGROUPBY");

    vector<string> ResultColumnList = {parsedQuery.groupbyColumnName, parsedQuery.groupbyResultColumnName};
    Table *resultantTable = new Table(parsedQuery.groupbyResultRelationName, ResultColumnList);

    Table table = *tableCatalogue.getTable(parsedQuery.groupbyRelationName);
    Cursor cursor = table.getCursor();
    int columnInd = table.getColumnIndex(parsedQuery.groupbyColumnName);
    int aggColumnInd = table.getColumnIndex(parsedQuery.groupbyAggregateColumnName);

    map<int, pair<int, int>> groups;
    vector<int> row;
    for (int blk = 0; blk < table.blockCount; blk++)
    {
        row.clear();
        Cursor cursor = Cursor(parsedQuery.groupbyRelationName, blk);
        row = cursor.getNext();
        while (!row.empty())
        {
            if (groups.find(row[columnInd]) != groups.end())
            {
                // updating the map
                pair operationResult = evaluateGroupbyOp(
                    row[aggColumnInd],
                    groups[row[columnInd]].first,
                    groups[row[columnInd]].second,
                    parsedQuery.groupbyAggregateType);

                groups[row[columnInd]] = operationResult;
            }
            else
            {
                // entering new key to map
                groups[row[columnInd]] = {row[aggColumnInd], 1};
            }
            row = cursor.getNext();
        }

        for (auto grp : groups)
        {
            vector<int> newRow = {grp.first, grp.second.first};
            newRow[1] /= parsedQuery.groupbyAggregateType == AVG ? grp.second.second : 1;
            resultantTable->writeRow<int>(newRow);
        }
        resultantTable->blockify();
        tableCatalogue.insertTable(resultantTable);
        cout << "BLOCK ACCESSES: " << BLOCK_ACCESS_COUNT << endl;
    }
    return;
}