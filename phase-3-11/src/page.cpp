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
Page::Page(string tableName, int pageIndex, bool isUpdate)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    Table table = *tableCatalogue.getTable(tableName);
    this->columnCount = table.columnCount;
    uint maxRowCount = table.maxRowsPerBlock;
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    if (!isUpdate)
    {
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
    else
    {
        FILE_DESCRIPTOR = open(pageName.c_str(), O_APPEND | O_CREAT);
        int savedCin = dup(STDIN_FILENO);
        dup2(FILE_DESCRIPTOR, STDIN_FILENO);

        cout << "Waiting for lock" << endl;
        flock(FILE_DESCRIPTOR, LOCK_EX);
        cout << "Received Lock for page: " << this->pageName << endl;
        this->rowCount = table.rowsPerBlockCount[pageIndex];
        int number;
        for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
            {
                cin >> number;
                this->rows[rowCounter][columnCounter] = number;
            }
        }
        dup2(savedCin, STDIN_FILENO);
        // close(savedCin);
    }
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

/**
 * @brief Get the Rows from the page
 *
 * @return vector<vector<int>>
 */
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
void Page::writePage(bool isUpdate)
{
    logger.log("Page::writePage");
    if (!isUpdate)
    {
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
    else
    {
        int fd = open(pageName.c_str(), O_WRONLY | O_TRUNC);
        int savedOut = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
            {
                if (columnCounter != 0)
                    cout << " ";
                cout << this->rows[rowCounter][columnCounter];
            }
            cout << endl;
        }
        dup2(savedOut, STDOUT_FILENO);
        // flock(fd, LOCK_UN);
        close(fd);
        close(FILE_DESCRIPTOR);
        close(savedOut);
        cout << "Lock Released " << this->pageName << endl;
    }
}

/**
 * @brief Get the Row Count object
 *
 * @return int
 */
int Page::getRowCount()
{
    logger.log("Page::getRowCount");
    return this->rowCount;
}
