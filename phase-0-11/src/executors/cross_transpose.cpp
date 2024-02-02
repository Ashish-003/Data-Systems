#include "global.h"

bool syntacticParseCROSS_TRANSPOSE()
{
    logger.log("syntacticParseCROSS_JOIN");
    if(tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CROSS_TRANSPOSE;
    parsedQuery.transposeNameA = tokenizedQuery[1];
    parsedQuery.transposeNameB = tokenizedQuery[2];
    return true;
}

bool semanticParseCROSS_TRANSPOSE()
{
    logger.log("semanticCROSS_TRANSPOSE");
    if( !matrixCatalogue.isMatrix(parsedQuery.transposeNameA) || !matrixCatalogue.isMatrix(parsedQuery.transposeNameB))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    Matrix* matrixA = matrixCatalogue.getMatrix(parsedQuery.transposeNameA);
    Matrix* matrixB = matrixCatalogue.getMatrix(parsedQuery.transposeNameB);
    if(matrixA->columnCount != matrixB->columnCount)
    {
        cout << "SEMANTIC ERROR: Matrices not of same size" << endl;
        return false;
    }
    return true;
}

void executeCROSS_TRANSPOSE()
{
    logger.log("executeCROSS_TRANPOSE");
    Matrix* matrixA = matrixCatalogue.getMatrix(parsedQuery.transposeNameA);
    Matrix* matrixB = matrixCatalogue.getMatrix(parsedQuery.transposeNameB);
    matrixA->tranpose();
    matrixB->tranpose();
    matrixCatalogue.crossSwap(matrixA, matrixB);
    return;
}