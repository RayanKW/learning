#include<iostream>
#include <iterator>
#include<string>

using namespace std;

//linked list structure
struct Node{
    string name;
    int age;
    Node* next;
};
 int main(){
    Node pers1;
    pers1.name = "jon";
    pers1.age= 21;
    pers1.next = nullptr; //we leave the node empty

    Node pers2;
    pers2.name = "Bob";
    pers2.age = 23;
    pers2.next = nullptr; //node is left null



    cout<<"\nBefore attaching the nodes" <<endl;
    cout<<"person.next is: " <<pers1.next<<"a null pointer"<<endl;

    pers1.next = &pers2; //attaching the nodes

    cout <<"\n After attaching the nodes" <<endl;
    cout <<"the memory adress of pers2 is: "<< &pers2 <<endl;
    cout <<"\n now the new person1.next points to: "<<pers1.next<<"memory address of person2"<<endl;
    cout<<"person2.next points still to: "<<pers2.next<<"which is empty sinve points to a null pointer"<<endl;

    cout<<"we can access Bob through jon as: "<<pers1.next->name<<endl;

    return 0;
 };
