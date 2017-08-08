#ifndef Matrix_hpp
#define Matrix_hpp

#include <iostream>
#include <vector>

namespace SSRverb {

class Matrix
{
public:
    Matrix(unsigned height = 0, unsigned width = 0);
    Matrix(unsigned square);
    
    // function which changes size of a non square matrix
    void resize(unsigned heigth_new, unsigned width_new);
    // change size, square matrix
    void resize(unsigned square);
    
    // overwrite bracket operator to set and get values
    std::vector<float> operator[](unsigned line) const { return data[line]; };
    std::vector<float> &operator[](unsigned line) {return data[line]; };
    
    // multiplication with a vector
    std::vector<float> operator*(std::vector<float> vec);
    
    // fried outstream in order to print data
    friend std::ostream& operator<< (std::ostream &out, Matrix mtrx);

    unsigned width;
    unsigned height;
    
protected:
    std::vector< std::vector<float> > data;
};

}

#endif /* defined(__FDN__Matrix__) */
