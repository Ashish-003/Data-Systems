#include "global.h"

void MatrixCatalogue::insertMatrix(Matrix* matrix)
{
    logger.log("MatrixCatalogue::~insertMatrix"); 
    this->matrices[matrix->matrixName] = matrix;
}
void MatrixCatalogue::deleteMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::deleteMatrix"); 
    this->matrices[matrixName]->unload();
    delete this->matrices[matrixName];
    this->matrices.erase(matrixName);
}
Matrix* MatrixCatalogue::getMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::getMatrix"); 
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}
bool MatrixCatalogue::isMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::isMatrix"); 
    if (this->matrices.count(matrixName))
        return true;
    return false;
}

void MatrixCatalogue::print()
{
    logger.log("MatrixCatalogue::print"); 
    cout << "\nRELATIONS" << endl;

    int rowCount = 0;
    for (auto rel : this->matrices)
    {
        cout << rel.first << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

void MatrixCatalogue::crossSwap(Matrix* m1, Matrix* m2)
{
    logger.log("MatrixCatalogue::crossSwap");
    for(int rowBlock = 0 ; rowBlock < m1->blockCount ; rowBlock++)
    {
        for(int colBlock = 0; colBlock < m1->blockCount ; colBlock++)
        {
            int pageId = rowBlock + colBlock * m1->blockCount;
            Cursor cursor1(m1->matrixName, pageId);
            Cursor cursor2(m2->matrixName, pageId);
            vector<vector<int>> myPage1, myPage2;
            for(int i = 0 ; i < m1->dimension ; i++)
            {
                myPage1.push_back(cursor1.getNext());
            }
            for(int i = 0 ; i < m2->dimension ; i++)
            {
                myPage2.push_back(cursor2.getNext());
            }
            bufferManager.writePage(m1->matrixName, pageId, myPage2, m2->dimension);
            bufferManager.writePage(m2->matrixName, pageId, myPage1, m1->dimension);
        }
    }
    bufferManager.clearPool();
}

MatrixCatalogue::~MatrixCatalogue(){
    logger.log("MatrixCatalogue::~MatrixCatalogue"); 
    for(auto matrix: this->matrices){
        matrix.second->unload();
        delete matrix.second;
    }
}

