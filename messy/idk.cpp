// this is a just a try(idk what the fuck is going on)
//
class vect{
  int num; //number vect
  int size; //size of the vector;
  double *v; //массив компонент
  static int amount; 
  public:
  // constructor: they are special methods that are called evry time when an object of a class is created
              //a constructor must have the same name as the class itself
              //they dont have a return
              //automatically called when an object is created
              //if we do not declare our custom constructor c++ compiler will create a default one for us
        // TYPES OF CONSTRUCTORS:
          // -> Defult constructors
          //      ==> this is created by defaukt when a constructor is not defined and takes no pars
            //eg:
#include <iostream>
using namespace std;

// Class with no explicity defined constructors
class A {
public:
};

int main() {
  
  	// Creating object
    A a;
    return 0;
}
          // -> Parameterized constructor:
          //      ==> this constructor lets us pass the argurments to initialise an objects members.
          //      // created by adding parameters to the constructor and using them to set the values of the members
#include <iostream>
using namespace std;

class A {
public:
    int val;
  
  	// Parameterized Constructor
    A(int x) {
        val = x;
    }
};

int main() {
  
  	// Creating object with a parameter
    A a(10);
    cout << a.val;
    return 0;
}
          // -> move constructor
          //    ==> this is a special type of cinstructor that transfers the resources of an object into another instead of copying them.
          //    it uses move semantics(often std::move)to takw ownershop of memory or handle from temporary object, avoiding extra copies
          //    and improving performance.
          //    It is typically called when objects are returned by value or when temporaries are used.
#include <iostream>
#include <vector>
using namespace std; 

class MyClass {
private:
    int b;

public:
    // Constructor
    MyClass(int &&a) : b(move(a)) {
        cout << "Move constructor called!" << endl;
    }

    void display() {
            cout << b <<endl;
    }
};

int main() {
    int a = 4;
    MyClass obj1(move(a));  // Move constructor is called

    obj1.display();
    return 0;
}
          // -> copy constructors
          //    ==> a copy constructor creates an object using another object of the same class
#include <iostream>
using namespace std;

class A {
public:
    int val;
    
    // Parameterized constructor
    A(int x) {
        val = x;
    }
    
    // Copy constructor
    A(A& a) {
        val = a.val;
    }
};

int main() {
    A a1(20);
    
    // Creating another object from a1
    A a2(a1);
    
  	cout << a2.val;
    return 0;
}
  // class: type of data

  // creating a constructor:
   // vect();
  //  vect(int &, *double)
  //
};


//read this:
~vect ();
void print();
vect operator+(vect r);
};
int vect:: amount=0;
void vect::print(){
  cout<<"vector N"<<num<<\n;
  ...
}


///techers notes;
//copy constructor
vect::vect(vect&x){
  amount++; num=amount;
  dim = x.dim;
  v=new double(dim);
  for (int i=0; i<dim; ++):
    pass;

}

//start here
vect vect::operator+(vect r){
vect tmp = vect(r); //this is a copy of the vect r i.t the vect tmp is a copy of the vector r
for (int i=0;i<tmp.dim;++)
  tmp.v(i)+=v(i);
return tmp;


vect operator-(vect&l, vect&r)
{
  vect tmp;
  temp.dim = l.dim;
  for (int i= 0;i<e.dim;i++)
    tmp.v(i)=l.v[i]+r[i];
  return tmp;
}
}

//1.)read topic 4 of the book.
//2.) write a program
