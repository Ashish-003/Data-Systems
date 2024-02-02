// #include "cursor.h"

// enum IndexingStrategy
// {
//     BTREE,
//     HASH,
//     NOTHING
// };

/**
 * @brief The Matrix class holds the information realted to loaded matrix. It also
 * implements methods that interact with the parsers, executors, cursors and the
 * buffer manager.
 *
 */

class Matrix
{
public:
    string sourceFileName = "";
    string matrixName = "";
    uint columnCount = 0;
    uint blockCount = 0;
    uint dimension = 0;
    long long maxRowsPerBlock = 0;
    long long maxColsPerBlock = 0;
    vector<uint> rowsPerBlock;
    vector<uint> colsPerBlock;
    bool sparse = false;
    long long nonZeroValueCount = 0;
    Matrix();
    Matrix(string matrixName);
    Matrix(string matrixName, vector<string> columns);
    bool extractColumnNum(string firstLine);
    bool blockify();
    bool load();
    void print();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    bool isPermanent();
    void makePermanent();
    void unload();
    int getPageId(int row, int col);
    void tranpose();
    void isSparse();
    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
    }

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Matrix::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};
