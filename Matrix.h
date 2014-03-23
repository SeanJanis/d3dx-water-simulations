#pragma once

#include <deque>
#include <string>
using namespace std;

template <class Type>
class CMatrix
{
public:
   CMatrix()
   {
      m_nNumRows = 0;
      m_nNumCols = 0;

      m_Matrix.resize(m_nNumRows);
      for (int c = 0; c < (int)m_Matrix.size(); c++)
      {
         m_Matrix[c].resize(m_nNumCols);
      }
   }

   CMatrix(int nNumRows, int nNumCols)
   {
      m_nNumRows = nNumRows;
      m_nNumCols = nNumCols;

      m_Matrix.resize(m_nNumRows);
      for (int c = 0; c < (int)m_Matrix.size(); c++)
      {
         m_Matrix[c].resize(m_nNumCols);
      }
   }

   CMatrix(const CMatrix& right)
   {
      m_nNumRows = right.m_nNumRows;
      m_nNumCols = right.m_nNumCols;

      m_Matrix.resize(right.m_nNumRows);
      for (int c = 0; c < (int)m_Matrix.size(); c++)
      {
         m_Matrix[c].resize(right.m_nNumCols);
      }

      //
      // Set all matrix values equal to this matrix's values.
      //

      for (int r = 0; r < (int)m_nNumRows; r++)
      {
         deque<Type>& currRow = m_Matrix[r];
         deque<Type> currRightRow = right.m_Matrix[r];

         for (int c = 0; c < (int)m_nNumCols; c++)
         {
            currRow[c] = currRightRow[c];
         }
      }
   }

   virtual ~CMatrix(void)
   {

   }

   CMatrix& operator=(const CMatrix& right)
   {
      //
      // Prevent Aliasing.
      //

      if (this != &right)
      {
         m_nNumRows = right.m_nNumRows;
         m_nNumCols = right.m_nNumCols;

         m_Matrix.resize(right.m_nNumRows);
         for (int c = 0; c < (int)m_Matrix.size(); c++)
         {
            m_Matrix[c].resize(right.m_nNumCols);
         }

         //
         // Set all matrix values equal to this matrix's values.
         //

         for (int r = 0; r < (int)m_nNumRows; r++)
         {
            deque<Type>& currRow = m_Matrix[r];
            deque<Type> currRightRow = right.m_Matrix[r];

            for (int c = 0; c < (int)m_nNumCols; c++)
            {
               currRow[c] = currRightRow[c];
            }
         }
      }

      return *this;
   }

   CMatrix operator*(const CMatrix& right)
   {
      //
      // Don't copy matrices with invalid multiplication dimensions.
      //

      if (m_nNumCols != right.m_nNumRows)
      {
         return *this;
      }
     
      //
      // Create a Result matrix with new dimensions.
      //

      CMatrix<Type> resultMatrix(m_nNumRows, right.m_nNumCols);

      //
      // Multiply the two matrices together.
      //
      
      for (int r = 0; r < m_nNumRows; r++) 
      {
         for (int c = 0; c < right.m_nNumCols; c++)
         {
            for (int i = 0; i < m_nNumCols; i++) 
            {
               resultMatrix[r][c] += m_Matrix[r][i] * right.m_Matrix[i][c];
            }
         }
      } 

      return resultMatrix;
   }

   deque<Type>& operator[](int nIndex)
   {
      //
      // Range Checking
      //

      if ((nIndex < 0) || (nIndex >= m_nNumRows))
      {
         return m_Matrix[0];
      }

      return m_Matrix[nIndex];
   }

   int GetNumRows()
   {
      return m_nNumRows;
   }

   int GetNumCols()
   {
      return m_nNumCols;
   }

protected:
   deque<deque<Type>> m_Matrix;

   int m_nNumRows;
   int m_nNumCols;
};
