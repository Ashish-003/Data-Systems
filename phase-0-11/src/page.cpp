#include "global.h"
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName
 * @param pageIndex
 */
Page::Page(string tableName, int pageIndex)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    if (tableCatalogue.isTable(tableName))
    {
        Table table = *tableCatalogue.getTable(tableName);
        this->columnCount = table.columnCount;
        uint maxRowCount = table.maxRowsPerBlock;
        vector<int> row(columnCount, 0);
        this->rows.assign(maxRowCount, row);

        ifstream fin(pageName, ios::in);
        this->rowCount = table.rowsPerBlockCount[pageIndex];
        int number;
        for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
            {
                fin >> number;
                this->rows[rowCounter][columnCounter] = number;
            }
        }
        fin.close();
    }
    else if (matrixCatalogue.isMatrix(tableName))
    {
        Matrix matrix = *matrixCatalogue.getMatrix(tableName);
        ifstream fin(pageName, ios::in);
        if (matrix.sparse)
        {
            this->sparse = true;
        }
        this->columnCount = matrix.maxColsPerBlock;
        uint maxRowCount = matrix.maxRowsPerBlock;
        vector<int> row(this->columnCount, 0);
        this->rows.assign(maxRowCount, row);

        this->rowCount = matrix.dimension;
        int number;
        for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
            {
                fin >> number;
                this->rows[rowCounter][columnCounter] = number;
            }
        }
        fin.close();
    }
}

Page::Page(string tableName, int partIndex, int pageIndex, int partSize)
{
    logger.log("Page::Page");
    this->pageName = "../data/temp/" + tableName + "_part" + to_string(partIndex) + "_page" + to_string(pageIndex);
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->partitionIndex = partIndex;

    Table table = *tableCatalogue.getTable(tableName);
    uint maxRowCount = table.maxRowsPerBlock;
    vector<int> row(table.columnCount, 0);

    this->columnCount = table.columnCount;
    this->rowCount = partSize;
    this->rows.assign(this->rowCount, row);

    int number;
    ifstream fin(pageName, ios::in);
    for (uint rowIndex = 0; rowIndex < partSize; rowIndex++)
        for (int columnIndex = 0; columnIndex < table.columnCount; columnIndex++)
        {
            fin >> number;
            this->rows[rowIndex][columnIndex] = number;
        }
    fin.close();
}

/**
 * @brief Get row from page indexed by rowIndex
 *
 * @param rowIndex
 * @return vector<int>
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

vector<vector<int>> Page::getRows()
{
    logger.log("Page::getRows");
    return this->rows;
}

Page::Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    this->columnCount = rows[0].size();
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 *
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, ios::trunc);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}