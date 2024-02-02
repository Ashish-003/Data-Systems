#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates matrix
 * statistics.
 *
 * @return true if the matrix has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNum(line))
        {
            this->isSparse();
            if (this->blockify())
                return true;
        }
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts number of columns
 * and set size related parameters for matrix
 * from the header line of the .csv data file.
 *
 * @param line
 * @return true if #columns successfully extracted
 * @return false otherwise
 */
bool Matrix::extractColumnNum(string firstLine)
{
    logger.log("Matrix::extractColumnNum");
    long long columnNum = 0;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        columnNum++;
    }
    this->columnCount = columnNum;
    this->dimension = floor(sqrt((float)(BLOCK_SIZE * 1000 / sizeof(int))));
    this->maxRowsPerBlock = this->dimension;
    this->maxColsPerBlock = this->dimension;
    this->blockCount = ceil((float)columnNum / this->dimension);
    return true;
}

/**
 * @brief This function splits all the rows/cols and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    string line, word;
    if (this->sparse)
    {
        vector<vector<int>> myPage(this->dimension, vector<int>(3, -1));
        ifstream fin(this->sourceFileName, ios::in);
        int blockNum = 0;
        int rowsInBlock = 0;
        for (int rowNum = 0; rowNum < this->columnCount; rowNum++)
        {
            getline(fin, line);
            stringstream s(line);
            for (int colNum = 0; colNum < this->columnCount; colNum++)
            {
                getline(s, word, ',');
                int num = stoi(word);
                if (num != 0)
                {
                    myPage[rowsInBlock][0] = rowNum;
                    myPage[rowsInBlock][1] = colNum;
                    myPage[rowsInBlock][2] = num;
                    rowsInBlock++;
                    if (rowsInBlock == this->maxRowsPerBlock)
                    {
                        bufferManager.writePage(this->matrixName, blockNum++, myPage, this->dimension);
                        this->rowsPerBlock.push_back(rowsInBlock);
                        rowsInBlock = 0;
                        myPage.assign(this->dimension, vector<int>(3, -1));
                    }
                }
            }
        }
        if (rowsInBlock != 0)
        {
            bufferManager.writePage(this->matrixName, blockNum++, myPage, this->dimension);
            this->rowsPerBlock.push_back(rowsInBlock);
            rowsInBlock = 0;
        }
    }
    else
    {
        vector<vector<int>> myPage(this->dimension, vector<int>(this->dimension, -1));
        int blockNum = 0;
        for (int columnNum = 0; columnNum < this->columnCount; columnNum += this->maxColsPerBlock)
        {
            ifstream fin(this->sourceFileName, ios::in);
            for (int rowNum = 0; rowNum < this->columnCount; rowNum += this->maxRowsPerBlock)
            {
                int i = rowNum;
                int j;
                for (; i < rowNum + this->maxRowsPerBlock && i < this->columnCount; i++)
                {
                    getline(fin, line);
                    stringstream s(line);
                    j = 0;
                    for (; j < columnNum; j++)
                        getline(s, word, ',');

                    for (; j < columnNum + this->maxColsPerBlock && j < this->columnCount; j++)
                    {
                        getline(s, word, ',');
                        myPage[i - rowNum][j - columnNum] = stoi(word);
                    }
                }
                bufferManager.writePage(this->matrixName, blockNum++, myPage, this->dimension);
                this->rowsPerBlock.push_back(i - rowNum);
                this->colsPerBlock.push_back(j - columnNum);
                myPage.assign(this->dimension, vector<int>(this->dimension, -1));
            }
            fin.close();
        }
    }
    return true;
}

/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((uint)PRINT_COUNT, this->columnCount);
    int rowNumber = 0;
    int columnNumber = 0;
    vector<int> row;
    if(this->sparse)
    {
        int pageNumber = 0;
        Cursor cursor(this->matrixName, pageNumber);
        row = cursor.getNext();
        for(int i = 0; i < count ; i++)
        {
            for(int j = 0; j < this->columnCount ; j++)
            {
                if(row[0] == i && row[1] == j)
                {
                    cout << row[2] << " ";
                    row = cursor.getNext();
                    if(row.size() == 0)
                    {
                        pageNumber++;
                        if(this->blockCount > pageNumber)
                            Cursor cursor(this->matrixName, pageNumber);
                    }
                }
                else
                {
                    cout << 0 << " ";
                }
            }
            cout << endl;
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            for (int j = 0; j < this->columnCount; j += maxColsPerBlock)
            {
                // int rowBlock = i/this->maxRowsPerBlock;
                // int colBlock = j/this->maxColsPerBlock;
                // int pageNumber = colBlock * this->blockCount + rowBlock;
                int pageNumber = this->getPageId(i, j);
                Cursor cursor(this->matrixName, pageNumber);
                row = cursor.getNext();
                for (int k = 0; k < i % this->dimension; k++)
                {
                    row = cursor.getNext();
                }
                for (int blockCol = 0; blockCol < this->colsPerBlock[pageNumber]; blockCol++)
                {
                    cout << row[blockCol] << " ";
                }
            }
            cout << endl;
        }
    }
}

/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}

/**
 * @brief This function returns the page_id for the row and column of the matrix
 *
 * @param row_index
 * @param column_index
 * @return int pageId
 */
int Matrix::getPageId(int row, int col)
{
    int rowBlock = row / this->maxRowsPerBlock;
    int colBlock = col / this->maxColsPerBlock;
    return (colBlock * this->blockCount) + rowBlock;
}

/**
 * @brief This function performs the transpose of the matrix in-place
 * and clears the bufferManager pool
 *
 */
void Matrix::tranpose()
{
    for (int rowBlock = 0; rowBlock < this->blockCount; rowBlock++)
    {
        for (int colBlock = 0; colBlock < this->blockCount; colBlock++)
        {
            int pageId = rowBlock + colBlock * this->blockCount;
            if (rowBlock > colBlock)
                continue;
            else if (rowBlock == colBlock)
            {
                Cursor cursor(this->matrixName, pageId);
                vector<vector<int>> myPage;
                for (int i = 0; i < this->dimension; i++)
                {
                    myPage.push_back(cursor.getNext());
                }
                for (int i = 0; i < this->dimension; i++)
                {
                    for (int j = i + 1; j < this->dimension; j++)
                    {
                        swap(myPage[i][j], myPage[j][i]);
                    }
                }
                bufferManager.writePage(this->matrixName, pageId, myPage, this->dimension);
            }
            else
            {
                int newPageId = colBlock + rowBlock * this->blockCount;
                Cursor cursor1(this->matrixName, pageId);
                Cursor cursor2(this->matrixName, newPageId);
                vector<vector<int>> myPage1, myPage2;
                for (int i = 0; i < this->dimension; i++)
                {
                    myPage1.push_back(cursor1.getNext());
                }
                for (int i = 0; i < this->dimension; i++)
                {
                    for (int j = i + 1; j < this->dimension; j++)
                    {
                        swap(myPage1[i][j], myPage1[j][i]);
                    }
                }
                for (int i = 0; i < this->dimension; i++)
                {
                    myPage2.push_back(cursor2.getNext());
                }
                for (int i = 0; i < this->dimension; i++)
                {
                    for (int j = i + 1; j < this->dimension; j++)
                    {
                        swap(myPage2[i][j], myPage2[j][i]);
                    }
                }
                bufferManager.writePage(this->matrixName, newPageId, myPage1, this->dimension);
                bufferManager.writePage(this->matrixName, pageId, myPage2, this->dimension);
            }
        }
    }
    bufferManager.clearPool();
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    int rowNumber = 0;
    int columnNumber = 0;
    vector<int> row;
    string newFile = "../data/" + this->matrixName + ".csv";
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    ofstream fout(newFile, ios::out);
    for (int i = 0; i < this->columnCount; i++)
    {
        for (int j = 0; j < this->columnCount; j += maxColsPerBlock)
        {
            int pageNumber = this->getPageId(i, j);
            Cursor cursor(this->matrixName, pageNumber);
            row = cursor.getNext();
            for (int k = 0; k < i % this->dimension; k++)
            {
                row = cursor.getNext();
            }
            for (int blockCol = 0; blockCol < this->colsPerBlock[pageNumber]; blockCol++)
            {
                fout << row[blockCol];
                if (j + blockCol == this->columnCount - 1)
                    continue;
                fout << ",";
            }
            logger.log("Matrix::getNextDone");
        }
        fout << endl;
    }
    fout.close();
}

void Matrix::isSparse()
{
    logger.log("Matrix::isSparse");
    fstream fin(this->sourceFileName, ios::in);
    long long zeroCount = 0;
    string line, word, first_line;
    long long nonZeroCount = 0;
    for (int i = 0; i < this->columnCount; i++)
    {
        getline(fin, line);
        stringstream s(line);
        for (int j = 0; j < this->columnCount; j++)
        {
            getline(s, word, ',');
            int num = stoi(word);
            if (num == 0)
                zeroCount++;
            else
                nonZeroCount++;
        }
    }
    if (zeroCount > 0.6 * this->columnCount * this->columnCount)
    {
        this->sparse = true;
        this->nonZeroValueCount = nonZeroCount;
        this->dimension = floor((float)(BLOCK_SIZE * 1000 / (3 * sizeof(int))));
        this->maxRowsPerBlock = this->dimension;
        this->maxColsPerBlock = 3;
        this->blockCount = ceil((float)this->nonZeroValueCount / this->dimension);
    }
    return;
}