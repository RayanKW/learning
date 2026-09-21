#include <iostream>
#include <stdexcept>
#include <iomanip>

using namespace std;

//we create the class vector
class vect{
  private:
    int dim;
    double* v;
    int num;
    static int count;

  public:
    vect(int d = 1) : dim(d), num(++count){
      v = new double[dim];
      for (int i = 0; i<dim; i++)
        v[i]  = 0.0;
      cout << "created a vector #" << num <<"of size" << dim <<endl;
    }

    vect(int d, double* arr) : dim(d), num(++count){
      v = new double[dim];
      for (int i =0; i<dim; i++)
        v[i] = arr[i];
      cout << "created vector #" << num << "of size" <<dim <<"from an array" <<endl;
    }

    // Destroctor
    ~vect(){
      cout << "deleted the vector #" <<num << "size " <<dim << endl;
      delete[] v;
      count --;
    }
};

int vect::count = 0;

int main(){
cout <<"we created the vector"<<endl;

vect v1;
vect v2(3);
vect v3(5);

cout<<"press enter to continue"<<endl;
cin.get();
return 0;
}
