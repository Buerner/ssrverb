#include "Matrix.h"
#include <stdio.h>

SSRverb::Matrix::Matrix(unsigned init_height, unsigned init_width)
{
    resize(init_height, init_width);
}

SSRverb::Matrix::Matrix(unsigned square)
{
    resize(square);
}

// function which changes size of a non square matrix
void SSRverb::Matrix::resize(unsigned height_new, unsigned width_new)
{
    // update members
    height = height_new;
    width = width_new;
    
    // resize first and second dimension (loop through second)
    data.resize(height);
    for (unsigned row = 0; row < height; row++) {
        data[row].resize(width);
    }
}

// change size, square matrix (does not check for correct dimensions)
void SSRverb::Matrix::resize(unsigned square)
{
    resize(square, square);
}

// matrix vector multiplication implementation
std::vector<float> SSRverb::Matrix::operator*(std::vector<float> vec)
{
    std::vector<float> result(this->height);
    
    for (unsigned row=0; row < this->height; row++) {
        for (unsigned col=0; col < this->width; col++) {
            result[row] += this->data[row][col] * vec[col];
        }
    }
    return result;
}

// overwrite ostream operator for easy printing
std::ostream &operator<< (std::ostream &out, SSRverb::Matrix mtrx)
{
    out << "[ ";
    for( unsigned int line = 0; line < mtrx.height; line++)
    {
        out << "[ ";
        for( unsigned int column = 0; column < mtrx.width; column++ )
        {
            out << mtrx[line][column] << " ";
        }
        out << "];" << std::endl;
    }
    out << " ]";
    return out;
}
