#include <iostream>
using namespace std;

struc Pers{
  string name;
  int age;
  float salary;
  Pers* next;
};


class linkedList(){
  private:
    Node* head; //points to the first node

  public:
    linkedList(){
      head = nullptr;
    }

    void addNode(sting name, int age){
      Node* newNode = newNode;
      newNode->name = name;
      newNode->age = age;
      newNode ->next = head;
      head=newNode;
    }
}

int main(){
  pass;
}
